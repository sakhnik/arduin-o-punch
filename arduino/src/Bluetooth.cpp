#include "Bluetooth.h"
#include "Context.h"
#include "Shell.h"
#include "Utils.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <memory>

namespace {

static const char *SHELL_SERVICE_UUID   = "16404bac-eab0-422c-955f-fb13799c00fa";
static const char *SHELL_STDIN_UUID     = "16404bac-eab1-422c-955f-fb13799c00fa";
static const char *SHELL_STDOUT_UUID    = "16404bac-eab2-422c-955f-fb13799c00fa";

static const char *CONFIG_SERVICE_UUID  = "26404bac-eab0-422c-955f-fb13799c00fa";
static const char *CONFIG_ID_UUID       = "26404bac-eab1-422c-955f-fb13799c00fa";
static const char *CONFIG_KEY_UUID      = "26404bac-eab2-422c-955f-fb13799c00fa";
static const char *CONFIG_WIFISSID_UUID = "26404bac-eab3-422c-955f-fb13799c00fa";
static const char *CONFIG_WIFIPASS_UUID = "26404bac-eab4-422c-955f-fb13799c00fa";

constexpr const int CHUNK_SIZE = 32;

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

            auto chunk = (LockGuard{_txMutex}, _outBuffer.Get(CHUNK_SIZE));
            if (!chunk.size) {
                break;
            }

            if (deviceConnected) {
                stdoutChr->setValue(chunk.data, chunk.size);
                stdoutChr->notify();
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
        }
    });
    stdoutChr = shellSvc->createCharacteristic(SHELL_STDOUT_UUID, NIMBLE_PROPERTY::NOTIFY);
    stdoutChr->createDescriptor("2901")->setValue("stdout");

    // Configuration service
    NimBLEService *configSvc = server->createService(CONFIG_SERVICE_UUID);
    auto *idChr = configSvc->createCharacteristic(CONFIG_ID_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    idChr->createDescriptor("2901")->setValue("id");
    {
        BLE2904 *desc = new BLE2904();
        desc->setFormat(BLE2904::FORMAT_UINT8);
        desc->setUnit(0x2700);
        idChr->addDescriptor(desc);
    }
    idChr->setValue(_context.GetId());
    setCb(idChr, [&]() {
        _context.SetId(idChr->getValue<uint8_t>());
    });

    auto* keyChr = configSvc->createCharacteristic(CONFIG_KEY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    keyChr->createDescriptor("2901")->setValue("key");
    auto key = _context.GetKey();
    keyChr->setValue(key.data(), key.size());
    setCb(keyChr, [&]() {
        std::string value = keyChr->getValue();
        _context.SetKey(value);
    });

    auto* wifissidChr = configSvc->createCharacteristic(CONFIG_WIFISSID_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    wifissidChr->createDescriptor("2901")->setValue("WiFi SSID");
    {
        BLE2904 *desc = new BLE2904();
        desc->setFormat(BLE2904::FORMAT_UTF8);
        desc->setUnit(0x2700);
        wifissidChr->addDescriptor(desc);
    }
    wifissidChr->setValue(_context.GetWifiSsid());
    setCb(wifissidChr, [&]() {
        std::string value = wifissidChr->getValue();
        _context.SetWifiSsid(value);
    });

    auto* wifipassChr = configSvc->createCharacteristic(CONFIG_WIFIPASS_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    wifipassChr->createDescriptor("2901")->setValue("WiFi pass");
    {
        BLE2904 *desc = new BLE2904();
        desc->setFormat(BLE2904::FORMAT_UTF8);
        desc->setUnit(0x2700);
        wifipassChr->addDescriptor(desc);
    }
    wifipassChr->setValue(_context.GetWifiPass());
    setCb(wifipassChr, [&]() {
        std::string value = wifipassChr->getValue();
        _context.SetWifiPass(value);
    });

    _context.Subscribe([&]() {
        idChr->setValue(_context.GetId());
        auto key = _context.GetKey();
        keyChr->setValue(key.data(), key.size());
        wifissidChr->setValue(_context.GetWifiSsid());
        wifipassChr->setValue(_context.GetWifiPass());
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
