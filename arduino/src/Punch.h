#pragma once

#include <stdint.h>

namespace AOP {

class Punch
{
public:
    static const int STORAGE_SIZE = 4;

    Punch(int station, uint32_t timestamp)
        : _station{station}
        , _timestamp{timestamp}
    {
    }

    Punch(const uint8_t *data, int offset)
        : _station{data[offset] & 0xff}
        , _timestamp{GetTimestamp(data, offset)}
    {
    }

    int GetStation() const { return _station; }
    uint32_t GetTimestamp() const { return _timestamp; }

    void Serialize(uint8_t *data, int offset)
    {
        data[offset] = _station & 0xff;
        // Store timestamp & 0xffff. The recording is consecutive, and it's safe to assume
        // that the overflow can only happen by incrementing the next digit by 1.
        data[offset + 1] = _timestamp & 0xff;
        data[offset + 2] = (_timestamp >> 8) & 0xff;
    }

    bool operator==(const Punch &o) const
    {
        return _station == o._station && _timestamp == o._timestamp;
    }

private:
    int _station;
    uint32_t _timestamp;

    static uint32_t GetTimestamp(const uint8_t *data, int offset)
    {
        return  (data[offset + 1] & 0xff) | ((data[offset + 2] & 0xff) << 8);
    }
};

} //namespace AOP;
