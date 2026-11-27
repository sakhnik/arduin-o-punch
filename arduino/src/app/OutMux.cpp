#include "OutMux.hpp"
#include "hal/serial.hpp"
#include "hal/time.hpp"

OutMux::OutMux()
{
}

size_t OutMux::Write(uint8_t ch)
{
    return HAL::Serial::Write(&ch, 1);
}

size_t OutMux::Write(const uint8_t *buffer, size_t size)
{
    // If there's a bluetooth or tcp connection, forward everything there
    if (_client)
        _client->Write(buffer, size);
    if (_enable_serial) {
        // Measure the time it took to send the data to the serial port.
        // If the port it stuck because the terminal was disconnected,
        // stop sending data to it.
        // This will make the other client more reponsive.
        auto start = HAL::Millis();
        auto ret = HAL::Serial::Write(buffer, size);
        auto finish = HAL::Millis();
        if (finish - start > 50) {
            _enable_serial = false;
        }
        return ret;
    }
    return size;
}

bool OutMux::AvailableForWrite()
{
    return HAL::Serial::AvailableForWrite();
}
