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
    if (_enable_serial) {
        // Measure the time it took to send the data to the serial port.
        // If the port it stuck because the terminal was disconnected,
        // stop sending data to it.
        // This will make the other client more reponsive.
        auto start = millis();
        auto ret = Serial.write(buffer, size);
        auto finish = millis();
        if (finish - start > 50) {
            _enable_serial = false;
        }
        return ret;
    }
    return size;
}

int OutMux::availableForWrite(void)
{
    return Serial.availableForWrite();
}
