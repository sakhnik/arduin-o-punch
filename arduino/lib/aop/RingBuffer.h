#pragma once

#include <cstdint>
#include <cstring>

namespace AOP {

template <int capacity>
class RingBuffer
{
public:
    int Size() const
    {
        return _size;
    }

    // Add to the buffer, potentially overwriting the tail
    void Add(const uint8_t *data, int size)
    {
        int overfill = size - capacity;
        overfill > 0 ? _AddImpl(data + overfill, size - overfill) : _AddImpl(data, size);
    }

    struct Chunk
    {
        int size;
        const uint8_t *data;
    };

    // Get a contiguous block of data from the head of the buffer.
    // When there's no more data left, chunk.size == 0.
    Chunk Get(int size)
    {
        if (!_size)
            return {0, nullptr};
        if (size > _size)
            size = _size;
        int end = _head + size;
        if (end > capacity)
            end = capacity;
        int retSize = end - _head;
        Chunk ret{retSize, _data + _head};
        _size -= retSize;
        _head = (_head + retSize) % capacity;
        return ret;
    }

private:
    int _head = 0;
    int _size = 0;
    uint8_t _data[capacity];

    void _AddImpl(const uint8_t *data, int size)
    {
        int overlap = _size + size - capacity;
        if (overlap > 0) {
            _size -= overlap;
            _head = (_head + overlap) % capacity;
        }
        int tail = (_head + _size) % capacity;
        int spaceLeft = capacity - tail;
        if (size <= spaceLeft) {
            memcpy(_data + tail, data, size);
        } else {
            memcpy(_data + tail, data, spaceLeft);
            memcpy(_data, data + spaceLeft, size - spaceLeft);
        }
        _size += size;
    }
};

} //namespace AOP;
