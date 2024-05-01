#include "OutMux.h"

OutMux::OutMux()
    : HardwareSerial{Serial}
{
}

size_t OutMux::write(uint8_t ch)
{
    return write(&ch, 1);
}

size_t OutMux::write(const uint8_t *buffer, size_t size)
{
    auto written = Serial.write(buffer, size);
    // Duplicate the output to the client (bluetooth) if connected.
    if (_client)
        _client->Write(buffer, written);
    return written;
}

int OutMux::availableForWrite(void)
{
    return Serial.availableForWrite();
}
