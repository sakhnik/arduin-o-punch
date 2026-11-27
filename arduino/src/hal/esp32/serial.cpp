#include "hal/serial.hpp"
#include <HardwareSerial.h>

namespace HAL {

void Serial::Setup()
{
    ::Serial.begin(115200);
}

size_t Serial::Write(uint8_t byte)
{
    return ::Serial.write(byte);
}

size_t Serial::Write(const uint8_t *buffer, size_t size)
{
    return ::Serial.write(buffer, size);
}

bool Serial::AvailableForWrite()
{
    return ::Serial.availableForWrite();
}

bool Serial::Available()
{
    return ::Serial.available();
}

char Serial::Read()
{
    return ::Serial.read();
}

} //namespace HAL;
