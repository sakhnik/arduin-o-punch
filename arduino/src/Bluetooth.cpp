#include "Bluetooth.h"
#include "Context.h"
#include "Shell.h"
#include "Utils.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

namespace {

static const char* SERVICE_UUID  = "16404bac-eab0-422c-955f-fb13799c00fa";
static const char* STDIN_UUID    = "16404bac-eab1-422c-955f-fb13799c00fa";
static const char* STDOUT_UUID   = "16404bac-eab2-422c-955f-fb13799c00fa";

constexpr const int CHARACTERISTIC_SIZE = 32;

NimBLEServer* server = nullptr;
NimBLEService* service = nullptr;
NimBLECharacteristic* stdinCharacteristic = nullptr;
NimBLECharacteristic* stdoutCharacteristic = nullptr;

bool deviceConnected = false;

char localName[16] = "AOP ";

} // namespace

class StdinCallbacks
    : public NimBLECharacteristicCallbacks
{
public:
    StdinCallbacks(Shell &shell) : _shell(shell) {}

    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &connInfo) override
    {
        std::string value = c->getValue();
        if (!value.empty()) {
            _shell.ProcessInput(
                (const uint8_t*)value.data(),
                value.size()
            );
        }
    }

private:
    Shell &_shell;
};

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

Bluetooth::Bluetooth(OutMux &outMux, Context &context, Shell &shell)
    : _outMux{outMux}
    , _context{context}
    , _shell{shell}
{
}

void Bluetooth::Setup()
{
    _last_write_time = millis();
    _txMutex = xSemaphoreCreateMutex();
    _txSignal = xSemaphoreCreateBinary();
}

void Bluetooth::SwitchOn()
{
    if (!_is_active) {
        _Start();
        _is_active = true;
    }
}

void Bluetooth::SwitchOff()
{
    if (_is_active) {
        _Stop();
        _is_active = false;
    }
}

void Bluetooth::_TaskEntry(void* arg)
{
    static_cast<Bluetooth*>(arg)->_Task();
}

void Bluetooth::_Task()
{
    while (_is_active) {

        // Wait until there is something to send
        xSemaphoreTake(_txSignal, portMAX_DELAY);

        while (_is_active) {

            // --- get chunk safely ---
            xSemaphoreTake(_txMutex, portMAX_DELAY);
            auto chunk = _outBuffer.Get(CHARACTERISTIC_SIZE);
            xSemaphoreGive(_txMutex);

            if (!chunk.size) {
                break;
            }

            if (deviceConnected) {
                stdoutCharacteristic->setValue(chunk.data, chunk.size);
                stdoutCharacteristic->notify();
            }

            // pacing
            vTaskDelay(pdMS_TO_TICKS(2));
        }
    }

    vTaskDelete(nullptr);
}

bool Bluetooth::_Start()
{
    NimBLEDevice::init("");

    *AOP::PrintNum(_context.GetId(), localName + 4) = 0;
    Serial.println(localName);

    NimBLEDevice::setDeviceName(localName);
    NimBLEDevice::setMTU(247);

    server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    service = server->createService(SERVICE_UUID);

    stdinCharacteristic = service->createCharacteristic(STDIN_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    stdoutCharacteristic = service->createCharacteristic(STDOUT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    stdinCharacteristic->setCallbacks(new StdinCallbacks(_shell));

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setName(localName);
    adv->start();

    _outMux.SetClient(this);

    _is_active = true;
    xTaskCreate(_TaskEntry, "ble_tx", 4096, this, 2, &_taskHandle);

    Serial.println("BLE started");
    return true;
}

bool Bluetooth::_Stop()
{
    _is_active = false;

    if (_taskHandle) {
        vTaskDelete(_taskHandle);
        _taskHandle = nullptr;
    }

    _outMux.SetClient(nullptr);

    if (server) {
        server->getAdvertising()->stop();
    }

    NimBLEDevice::deinit(true);

    Serial.println("BLE stopped");

    return false;
}

void Bluetooth::Write(const uint8_t *buffer, size_t size)
{
    if (!_is_active) {
        return;
    }

    xSemaphoreTake(_txMutex, portMAX_DELAY);
    _outBuffer.Add(buffer, size);
    xSemaphoreGive(_txMutex);

    xSemaphoreGive(_txSignal);  // wake TX task
}
