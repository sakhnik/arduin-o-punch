#pragma once

#include <stdint.h>

namespace AOP {

class Punch
{
public:
    static const int STORAGE_SIZE = 4;

    // Keep in sync with Android's Puncher
    //
    // Punch format:
    // 0     | 1     | 2     | 3     |
    // STA-N | TIMEL | TIMEM | TIMEH.|

    Punch(uint8_t station, uint32_t timestamp)
        : _station{station}
        , _timestamp{timestamp}
    {
    }

    Punch(const uint8_t *data, int offset)
        : _station{data[offset]}
        , _timestamp{GetTimestamp(data, offset)}
    {
    }

    int GetStation() const { return _station; }
    uint32_t GetTimestamp() const { return _timestamp; }

    void Serialize(uint8_t *data, int offset)
    {
        data[offset] = _station & 0xff;
        // Store timestamp & 0xffffff.
        data[offset + 1] = _timestamp & 0xff;
        data[offset + 2] = (_timestamp >> 8) & 0xff;
        data[offset + 3] = (_timestamp >> 16) & 0xff;
    }

    bool operator==(const Punch &o) const
    {
        return _station == o._station && _timestamp == o._timestamp;
    }

private:
    uint8_t _station;
    uint32_t _timestamp;

    static uint32_t GetTimestamp(const uint8_t *data, int offset)
    {
        return  (data[offset + 1] & 0xff) | ((data[offset + 2] & 0xff) << 8) | ((data[offset + 3] & 0xff) << 16);
    }
};

} //namespace AOP;
