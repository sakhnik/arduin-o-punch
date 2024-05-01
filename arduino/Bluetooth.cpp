#ifdef ESP32

#include "Bluetooth.h"
#include "Context.h"
#include "Shell.h"
#include <ArduinoBLE.h>

Bluetooth::Bluetooth(OutMux &outMux, Context &context, Shell &shell)
    : _outMux{outMux}
    , _context{context}
    , _shell{shell}
{
}

namespace {

char* PrintNum(uint8_t num, char *buf)
{
    if (!num)
        return buf;
    buf = PrintNum(num / 10, buf);
    *buf = '0' + num % 10;
    return ++buf;
}

BLEService serialService("16404bac-eab0-422c-955f-fb13799c00fa");
constexpr const int CHARACTERISTIC_SIZE = 32;
BLECharacteristic stdinCharacteristic("16404bac-eab1-422c-955f-fb13799c00fa", BLERead | BLEWrite, CHARACTERISTIC_SIZE);
BLECharacteristic stdoutCharacteristic("16404bac-eab2-422c-955f-fb13799c00fa", BLERead | BLENotify, CHARACTERISTIC_SIZE);
char localName[16] = "AOP ";

} // namespace;

void Bluetooth::Setup()
{
    _last_write_time = millis();
}

void Bluetooth::Toggle()
{
    _is_active = _is_active ? _Stop() : _Start();
}

void Bluetooth::Tick()
{
    if (!_is_active) {
        return;
    }

    // Poll for BLE events
    BLE.poll();
    if (stdinCharacteristic.written()) {
        uint8_t data[CHARACTERISTIC_SIZE];
        int bytesRead = stdinCharacteristic.readValue(data, sizeof(data));
        if (bytesRead > 0) {
            _shell.ProcessInput(data, bytesRead);
        }
    }

    // The characteristic value should be updated gradually to give
    // the transmitter time to process everything.
    auto now = millis();
    if (now - _last_write_time > 1) {
        _last_write_time = now;
        auto chunk = _outBuffer.Get(CHARACTERISTIC_SIZE);
        if (chunk.size) {
            stdoutCharacteristic.writeValue(chunk.data, chunk.size);
        }
    }
}

bool Bluetooth::_Start()
{
    setCpuFrequencyMhz(80);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));

    if (!BLE.begin()) {
        Serial.println("Starting BLE failed");
        return false;
    }

    *PrintNum(_context.GetId(), localName + 4) = 0;
    Serial.println(localName);
    BLE.setLocalName(localName);
    BLE.setAdvertisedService(serialService);
    serialService.addCharacteristic(stdinCharacteristic);
    serialService.addCharacteristic(stdoutCharacteristic);

    BLE.addService(serialService);

    BLE.advertise();
    _outMux.SetClient(this);
    Serial.println("BLE started");
    return true;
}

bool Bluetooth::_Stop()
{
    _outMux.SetClient(nullptr);
    BLE.end();
    Serial.println("BLE stopped");
    setCpuFrequencyMhz(40);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));
    return false;
}

void Bluetooth::Write(const uint8_t *buffer, size_t size)
{
    // Schedule the data for writing at appropriate pace
    _outBuffer.Add(buffer, size);
}

#endif
