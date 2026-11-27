#include "hal/serial.hpp"

#include <iostream>

namespace HAL {

void Serial::Setup()
{
}

size_t Serial::Write(uint8_t byte)
{
    std::cout.put(static_cast<char>(byte));
    return std::cout ? 1 : 0;
}

size_t Serial::Write(const uint8_t *buffer, size_t size)
{
    std::cout.write(reinterpret_cast<const char *>(buffer), size);
    return std::cout ? size : 0;
}

bool Serial::AvailableForWrite()
{
    return true;
}

bool Serial::Available()
{
    return std::cin.rdbuf()->in_avail() > 0;
}

char Serial::Read()
{
    return static_cast<char>(std::cin.get());
}

} // namespace HAL
