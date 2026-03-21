#include "Bluetooth.h"
#include "Context.h"
#include "Shell.h"
#include "CpuFreq.h"
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

void Bluetooth::Tick()
{
    if (!_is_active) {
        return;
    }

    // TX pacing
    auto now = millis();
    if (now - _last_write_time > 1) {
        _last_write_time = now;

        auto chunk = _outBuffer.Get(CHARACTERISTIC_SIZE);
        if (chunk.size && deviceConnected) {
            stdoutCharacteristic->setValue(chunk.data, chunk.size);
            stdoutCharacteristic->notify();
        }
    }
}

bool Bluetooth::_Start()
{
    SetCpuFreq(80);

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

    service->start();

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setName(localName);
    adv->start();

    _outMux.SetClient(this);

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

    Serial.println("BLE stopped");
    SetCpuFreq(10);

    return false;
}

void Bluetooth::Write(const uint8_t *buffer, size_t size)
{
    _outBuffer.Add(buffer, size);
}
