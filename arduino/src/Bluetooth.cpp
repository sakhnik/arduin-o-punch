#include "Bluetooth.h"
#include "Context.h"
#include "Shell.h"
#include "Utils.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <memory>

namespace {

static const char *SHELL_SERVICE_UUID  = "16404bac-eab0-422c-955f-fb13799c00fa";
static const char *STDIN_UUID    = "16404bac-eab1-422c-955f-fb13799c00fa";
static const char *STDOUT_UUID   = "16404bac-eab2-422c-955f-fb13799c00fa";

static const char *CONFIG_SERVICE_UUID  = "26404bac-eab0-422c-955f-fb13799c00fa";
static const char *CONFIG_ID_UUID       = "26404bac-eab1-422c-955f-fb13799c00fa";
static const char *CONFIG_KEY_UUID      = "26404bac-eab2-422c-955f-fb13799c00fa";
static const char *CONFIG_WIFISSID_UUID = "26404bac-eab3-422c-955f-fb13799c00fa";
static const char *CONFIG_WIFIPASS_UUID = "26404bac-eab4-422c-955f-fb13799c00fa";

constexpr const int CHARACTERISTIC_SIZE = 32;

NimBLEServer *server = nullptr;
NimBLECharacteristic *stdinCharacteristic = nullptr;
NimBLECharacteristic *stdoutCharacteristic = nullptr;

bool deviceConnected = false;

char localName[16] = "AOP ";

class ServerCallbacks
    : public NimBLEServerCallbacks
{
public:
    void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override
    {
        deviceConnected = true;
    }

    void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override
    {
        deviceConnected = false;
        NimBLEDevice::startAdvertising();
    }
};

class StdinCallbacks
    : public NimBLECharacteristicCallbacks
{
public:
    StdinCallbacks(Shell &shell) : _shell(shell) {}

    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &connInfo) override
    {
        std::string value = c->getValue();
        if (!value.empty()) {
            _shell.ProcessInput(reinterpret_cast<const uint8_t *>(value.data()), value.size());
        }
    }

private:
    Shell &_shell;
};

class ConfigIdCallbacks
    : public NimBLECharacteristicCallbacks
{
public:
    ConfigIdCallbacks(Context &context) : _context(context) {}

    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &) override
    {
        _context.SetId(c->getValue<uint8_t>());
    }

private:
    Context &_context;
};

class ConfigKeyCallbacks
    : public NimBLECharacteristicCallbacks
{
public:
    ConfigKeyCallbacks(Context &context) : _context(context) {}

    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &) override
    {
        std::string value = c->getValue();
        _context.OnNewKey(value);
    }

private:
    Context &_context;
};

struct BleContext
{
    ServerCallbacks serverCallbacks;
    std::vector<std::unique_ptr<NimBLECharacteristicCallbacks>> characteristicCallbacks;

    NimBLECharacteristicCallbacks* Manage(std::unique_ptr<NimBLECharacteristicCallbacks> c)
    {
        characteristicCallbacks.emplace_back(std::move(c));
        return characteristicCallbacks.back().get();
    }
};

std::unique_ptr<BleContext> bleContext;

} // namespace

Bluetooth::Bluetooth(OutMux &outMux, Context &context, Shell &shell)
    : _outMux{outMux}
    , _context{context}
    , _shell{shell}
{
}

void Bluetooth::Setup()
{
    _last_write_time = millis();
    _txSignal = xSemaphoreCreateBinary();
}

void Bluetooth::SwitchOn()
{
    assert(!_taskHandle);

    _stopRequested.store(false);

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
    while (!_stopRequested.load()) {

        // Wait until there is something to send
        xSemaphoreTake(_txSignal, portMAX_DELAY);

        while (!_stopRequested.load()) {

            auto chunk = (LockGuard{_txMutex}, _outBuffer.Get(CHARACTERISTIC_SIZE));
            if (!chunk.size) {
                break;
            }

            if (deviceConnected) {
                stdoutCharacteristic->setValue(chunk.data, chunk.size);
                stdoutCharacteristic->notify();
            }

            // pacing
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }

    _taskHandle = nullptr;
    vTaskDelete(nullptr);
}

bool Bluetooth::_Start()
{
    NimBLEDevice::init("");

    *AOP::PrintNum(_context.GetId(), localName + 4) = 0;
    Serial.println(localName);

    NimBLEDevice::setDeviceName(localName);
    NimBLEDevice::setMTU(247);

    bleContext.reset(new BleContext());

    server = NimBLEDevice::createServer();
    server->setCallbacks(&bleContext->serverCallbacks);

    // Shell service
    NimBLEService *shellService = server->createService(SHELL_SERVICE_UUID);
    stdinCharacteristic = shellService->createCharacteristic(STDIN_UUID, NIMBLE_PROPERTY::WRITE);
    stdinCharacteristic->createDescriptor("2901")->setValue("stdin");
    stdinCharacteristic->setCallbacks(bleContext->Manage(std::make_unique<StdinCallbacks>(_shell)));
    stdoutCharacteristic = shellService->createCharacteristic(STDOUT_UUID, NIMBLE_PROPERTY::NOTIFY);
    stdoutCharacteristic->createDescriptor("2901")->setValue("stdout");

    // Configuration service
    NimBLEService *configService = server->createService(CONFIG_SERVICE_UUID);
    auto *idCharacteristic = configService->createCharacteristic(CONFIG_ID_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    idCharacteristic->createDescriptor("2901")->setValue("id");
    {
        BLE2904 *desc = new BLE2904();
        desc->setFormat(BLE2904::FORMAT_UINT8);
        desc->setUnit(0x2700);
        idCharacteristic->addDescriptor(desc);
    }
    idCharacteristic->setValue(_context.GetId());
    idCharacteristic->setCallbacks(bleContext->Manage(std::make_unique<ConfigIdCallbacks>(_context)));

    auto* keyCharacteristic = configService->createCharacteristic(CONFIG_KEY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    keyCharacteristic->createDescriptor("2901")->setValue("key");

    auto key = _context.GetKey();
    keyCharacteristic->setValue(key.data(), key.size());
    keyCharacteristic->setCallbacks(bleContext->Manage(std::make_unique<ConfigKeyCallbacks>(_context)));

    _context.Subscribe([&]() {
        idCharacteristic->setValue(_context.GetId());
        auto key = _context.GetKey();
        keyCharacteristic->setValue(key.data(), key.size());
    });

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SHELL_SERVICE_UUID);
    adv->addServiceUUID(CONFIG_SERVICE_UUID);
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

    if (server) {
        server->getAdvertising()->stop();
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
