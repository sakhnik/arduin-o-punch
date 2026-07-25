#include "Bluetooth.h"
#include "Settings.h"
#include "Shell.h"
#include "Utils.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <memory>

namespace {

static const char *SHELL_SERVICE_UUID   = "16404bac-eab0-422c-955f-fb13799c00fa";
static const char *SHELL_STDIN_UUID     = "16404bac-eab1-422c-955f-fb13799c00fa";
static const char *SHELL_STDOUT_UUID    = "16404bac-eab2-422c-955f-fb13799c00fa";

static uint16_t negotiatedMTU = 23;

NimBLEServer *server = nullptr;
NimBLECharacteristic *stdinChr = nullptr;
NimBLECharacteristic *stdoutChr = nullptr;

bool deviceConnected = false;

char localName[16] = "AOP ";

class ServerCallbacks
    : public NimBLEServerCallbacks
{
public:
    void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override
    {
        deviceConnected = true;
        negotiatedMTU = connInfo.getMTU();
    }

    void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override
    {
        deviceConnected = false;
        NimBLEDevice::startAdvertising();
    }
};

class Callbacks
    : public NimBLECharacteristicCallbacks
{
public:
    using FuncT = std::function<void(void)>;

    Callbacks(FuncT func) : _func{func} {}

    void onWrite(NimBLECharacteristic *, NimBLEConnInfo &) override
    {
        _func();
    }

private:
    FuncT _func;
};

struct BleContext
{
    ServerCallbacks serverCallbacks;
    std::vector<NimBLECharacteristic *> chrs;
    std::vector<std::unique_ptr<NimBLECharacteristicCallbacks>> chrCallbacks;

    NimBLECharacteristicCallbacks* ManageCb(NimBLECharacteristic *c, std::unique_ptr<NimBLECharacteristicCallbacks> cb)
    {
        chrs.push_back(c);
        chrCallbacks.emplace_back(std::move(cb));
        return chrCallbacks.back().get();
    }

    ~BleContext()
    {
        // Heap corruption if we don't reset callbacks
        for (size_t i = 0; i < chrs.size(); ++i) {
            chrs[i]->setCallbacks(nullptr);
        }
    }
};

std::unique_ptr<BleContext> bleContext;

} // namespace

Bluetooth::Bluetooth(OutMux &outMux, Settings &settings, Shell &shell)
    : _outMux{outMux}
    , _settings{settings}
    , _shell{shell}
{
}

void Bluetooth::Setup()
{
    _lastStdinTimeMs.store(millis());
    _txSignal = xSemaphoreCreateBinary();
}

void Bluetooth::SwitchOn()
{
    assert(!_taskHandle);

    _stopRequested.store(false);
    _lastStdinTimeMs.store(millis());

    _Start();
}

void Bluetooth::SwitchOff()
{
    assert(_taskHandle);

    _stopRequested.store(true);
    xSemaphoreGive(_txSignal);

    // wait for task to exit
    while (_taskHandle) {
        vTaskDelay(1);
    }

    _Stop();
}

void Bluetooth::_TaskEntry(void* arg)
{
    static_cast<Bluetooth*>(arg)->_Task();
}

void Bluetooth::_Task()
{
    size_t chunkSize = std::max<size_t>(20, negotiatedMTU - 3);

    while (!_stopRequested.load()) {

        // Wait until there is something to send
        xSemaphoreTake(_txSignal, portMAX_DELAY);

        while (!_stopRequested.load()) {

            auto chunk = (LockGuard{_txMutex}, _outBuffer.Get(chunkSize));
            if (!chunk.size) {
                break;
            }

            if (deviceConnected) {
                stdoutChr->setValue(chunk.data, chunk.size);
                stdoutChr->notify();
            }

            // pacing
            vTaskDelay(pdMS_TO_TICKS(30));
        }
    }

    _taskHandle = nullptr;
    vTaskDelete(nullptr);
}

bool Bluetooth::_Start()
{
    NimBLEDevice::init("");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P6);

    *AOP::PrintNum(_settings.GetId(), localName + 4) = 0;
    Serial.println(localName);

    NimBLEDevice::setDeviceName(localName);
    NimBLEDevice::setMTU(247);

    bleContext.reset(new BleContext());

    server = NimBLEDevice::createServer();
    server->setCallbacks(&bleContext->serverCallbacks);

    auto setCb = [&](NimBLECharacteristic *c, auto func) {
        c->setCallbacks(bleContext->ManageCb(c, std::make_unique<Callbacks>([=]() { func(); })));
    };

    // Shell service
    NimBLEService *shellSvc = server->createService(SHELL_SERVICE_UUID);
    stdinChr = shellSvc->createCharacteristic(SHELL_STDIN_UUID, NIMBLE_PROPERTY::WRITE);
    stdinChr->createDescriptor("2901")->setValue("stdin");
    setCb(stdinChr, [&]() {
        std::string value = stdinChr->getValue();
        if (!value.empty()) {
            _shell.ProcessInput(reinterpret_cast<const uint8_t *>(value.data()), value.size());
            _lastStdinTimeMs.store(millis());
        }
    });
    stdoutChr = shellSvc->createCharacteristic(SHELL_STDOUT_UUID, NIMBLE_PROPERTY::NOTIFY);
    stdoutChr->createDescriptor("2901")->setValue("stdout");

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SHELL_SERVICE_UUID);
    adv->setName(localName);
    adv->start();

    _outBuffer.Clear();
    _outMux.SetClient(this);

    xTaskCreate(_TaskEntry, "ble_tx", 4096, this, 2, &_taskHandle);

    Serial.println("BLE started");
    return true;
}

bool Bluetooth::_Stop()
{
    _outMux.SetClient(nullptr);
    _settings.Unsubscribe(_subscription_handle);
    _subscription_handle = -1;

    if (server) {
        server->getAdvertising()->stop();
        server->setCallbacks(nullptr);
    }

    NimBLEDevice::deinit(true);
    vTaskDelay(pdMS_TO_TICKS(50));  // Let NimBLE finishes its tasks

    bleContext.reset();

    Serial.println("BLE stopped");

    return false;
}

void Bluetooth::Write(const uint8_t *buffer, size_t size)
{
    if (_stopRequested.load() || !_taskHandle) {
        return;
    }

    LockGuard{_txMutex}, _outBuffer.Add(buffer, size);

    xSemaphoreGive(_txSignal);  // wake TX task
}

unsigned Bluetooth::GetInactivitySeconds()
{
    return (millis() - _lastStdinTimeMs.load()) / 1000;
}
