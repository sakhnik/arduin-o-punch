#pragma once

#include <stdint.h>

namespace AOP {

class Punch
{
public:
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

    int GetStation() const
    {
        return _station;
    }

    uint32_t GetTimestamp() const
    {
        return _timestamp;
    }

    void SetTimestamp(uint8_t timestamp)
    {
        _timestamp = timestamp;
    }

    void SerializeStation(uint8_t *data, int offset)
    {
        data[offset] = _station & 0xff;
    }

    void SerializeTimestamp(uint8_t *data, int offset)
    {
        // Store timestamp & 0xffffff.
        data[offset] = _timestamp & 0xff;
        data[offset + 1] = (_timestamp >> 8) & 0xff;
        data[offset + 2] = (_timestamp >> 16) & 0xff;
    }

    void SerializeTimestamp(uint8_t *data, int offset, uint32_t timestamp0)
    {
        auto dt = _timestamp - timestamp0;
        data[offset] = dt & 0xff;
        data[offset + 1] = (dt >> 8) & 0xff;
    }

    bool operator==(const Punch &o) const
    {
        return _station == o._station && _timestamp == o._timestamp;
    }

    static uint8_t GetStation(const uint8_t *data, int offset)
    {
        return data[offset] & 0xff;
    }

    static uint32_t GetTimestamp(const uint8_t *data, int offset)
    {
        return  (data[offset] & 0xff) | ((data[offset + 1] & 0xff) << 8) | ((data[offset + 2] & 0xff) << 16);
    }

    static uint32_t GetTimestamp(const uint8_t *data, int offset, uint32_t timestamp0)
    {
        return timestamp0 + data[offset] + (static_cast<uint32_t>(data[offset + 1]) << 8);
    }

private:
    uint8_t _station;
    uint32_t _timestamp;
};

} //namespace AOP;
