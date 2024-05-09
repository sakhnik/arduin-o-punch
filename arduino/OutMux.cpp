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
    // If there's a bluetooth or tcp connection, forward everything there
    if (_client)
        _client->Write(buffer, size);
    return Serial.write(buffer, size);
}

int OutMux::availableForWrite(void)
{
    return Serial.availableForWrite();
}
