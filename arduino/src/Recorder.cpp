#include "Recorder.h"

namespace AOP {

Recorder::Recorder(int begin, int size, IEeprom &eeprom)
    : _begin{begin + 4}
    , _eeprom{eeprom}
    , _size{size - 4}
{
}

void Recorder::Setup()
{
    Restore();
}

void Recorder::Restore()
{
    uint8_t start_lo = _eeprom.Read(_begin - 4);
    uint8_t start_hi = _eeprom.Read(_begin - 3);
    _offset = start_hi == 0xff ? 0 : static_cast<int>(start_hi) << 8 | start_lo;
    _offset += _begin;

    uint8_t length_val = _eeprom.Read(_begin - 2);
    if (length_val == 0xff)
        length_val = 0;
    _length = static_cast<int>(length_val) << LENGTH_SHIFT;
}

void Recorder::StoreOffset()
{
    int start = _offset - _begin;
    uint8_t start_lo = start & 0xff;
    uint8_t start_hi = start >> 8;
    if (_eeprom.Read(_begin - 4) != start_lo)
        _eeprom.Write(_begin - 4, start_lo);
    if (_eeprom.Read(_begin - 3) != start_hi)
        _eeprom.Write(_begin - 3, start_hi);
}

void Recorder::StoreLength()
{
    uint8_t length_val = _length >> LENGTH_SHIFT;
    if (_eeprom.Read(_begin - 2) != length_val)
        _eeprom.Write(_begin - 2, length_val);
}

int8_t Recorder::Format(int count)
{
    // How many bytes are required to accomodate the runners?
    int byte_count = count >> 3;
    if (count & 7)
        ++byte_count;

    // Round to the length precision
    int mask = (1 << LENGTH_SHIFT) - 1;
    if ((byte_count & mask) != 0)
        byte_count = ((byte_count >> LENGTH_SHIFT) + 1) << LENGTH_SHIFT;

    // Check the bounds
    if (byte_count > _size)
        return -1;   // no way to accomodate that many

    // Calculate the new offset and length
    _offset += _length;
    if (_offset >= _begin + _size)
        _offset -= _size;
    StoreOffset();
    _length = byte_count;
    StoreLength();

    // Clear the memory span
    int addr_end = _begin + _size;
    for (int i = 0, addr = _offset; i < _length; ++i, ++addr)
    {
        if (addr >= addr_end)
            addr -= _size;
        if (_eeprom.Read(addr) != 0)
            _eeprom.Write(addr, 0);
    }
    return 0;
}

int8_t Recorder::Record(int card, bool set)
{
    int offset = card >> 3;
    int bit_offset = card & 7;
    if (offset >= _length)
        return -1;  // The card beyond the range
    int addr = _offset + offset;
    if (addr >= _begin + _size)
        addr -= _size;
    uint8_t val = _eeprom.Read(addr);
    uint8_t new_val = set ? (val | (uint8_t{1} << bit_offset)) : (val & ~(uint8_t{1} << bit_offset));
    if (val != new_val)
        _eeprom.Write(addr, new_val);
    return 0;
}

bool Recorder::IsRecorded(int card)
{
    int offset = card >> 3;
    int bit_offset = card & 7;
    if (offset >= _length)
        return false;  // The card beyond the range
    int addr = _offset + offset;
    if (addr >= _begin + _size)
        addr -= _size;
    uint8_t val = _eeprom.Read(addr);
    return (val & (1 << bit_offset)) != 0;
}

struct IVisitor
{
    virtual void OnCard(int card, void *ctx) = 0;
};

void Recorder::List(IVisitor &visitor, void *ctx)
{
    int card{};
    int addr_end = _begin + _size;
    for (int i = 0, addr = _offset; i < _length; ++i, ++addr)
    {
        if (addr >= addr_end)
            addr -= _size;
        uint8_t val = _eeprom.Read(addr);
        for (int j = 0; j < 8; ++j, ++card)
        {
            if (0 != (val & (1 << j)))
                visitor.OnCard(card, ctx);
        }
    }
}

} //namespace AOP;
