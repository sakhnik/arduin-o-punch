#include "Bluetooth.h"
#include "Context.h"
#include "Shell.h"
#ifdef ESP32
#  include <ArduinoBLE.h>
#endif //ESP32

Bluetooth::Bluetooth(Context &context, Shell &shell)
    : _context{context}
    , _shell{shell}
{
}

#ifndef ESP32

void Bluetooth::Setup()
{
}

void Bluetooth::Toggle()
{
}

void Bluetooth::Tick()
{
}

#else //ESP32

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
BLEStringCharacteristic stdinCharacteristic("16404bac-eab1-422c-955f-fb13799c00fa", BLERead | BLEWrite, 31);
BLEStringCharacteristic stdoutCharacteristic("16404bac-eab2-422c-955f-fb13799c00fa", BLERead | BLENotify, 31);
char localName[16] = "AOP ";

} // namespace;

void Bluetooth::Setup()
{
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
    //stdinCharacteristic.written();
    //stdinCharacteristic.value();
    //stdoutCharacteristic.writeValue("asdf");
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
    Serial.println("BLE started");
    return true;
}

bool Bluetooth::_Stop()
{
    BLE.end();
    Serial.println("BLE stopped");
    setCpuFrequencyMhz(40);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));
    return false;
}

#endif
