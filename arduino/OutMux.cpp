#include "OutMux.h"

OutMux::OutMux()
    : HardwareSerial{Serial}
{
}

size_t OutMux::write(const uint8_t *buffer, size_t size)
{
    return Serial.write(buffer, size);
}
