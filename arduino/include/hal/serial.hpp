#pragma once

#include <cstddef>
#include <cstdint>

namespace HAL {

class Serial
{
public:
    static void Setup();

    static size_t Write(uint8_t byte);
    static size_t Write(const uint8_t *buffer, size_t size);
    static bool AvailableForWrite();
    static bool Available();
    static char Read();
};

} //namespace HAL;
