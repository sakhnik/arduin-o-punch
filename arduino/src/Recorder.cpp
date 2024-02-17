#include "Recorder.h"
#include <stddef.h>

namespace AOP {

struct Header
{
    // Start offset after _begin
    uint8_t start_lo;
    uint8_t start_hi;
    // Length of the byte range (multiplied by 8)
    uint8_t length;
    uint8_t bits_per_card;
    // Format timestamp in days (unix_timestamp / seconds_in_day)
    uint8_t day_lo;
    uint8_t day_hi;
};

#define ADDROF(begin, field) (begin - sizeof(Header) + offsetof(Header, field))

Recorder::Recorder(int begin, int size, IEeprom &eeprom)
    : _begin(begin + sizeof(Header))
    , _eeprom{eeprom}
    , _size(size - sizeof(Header))
{
}

void Recorder::Setup()
{
    Restore();
}

void Recorder::Restore()
{
    uint8_t start_lo = _eeprom.Read(ADDROF(_begin, start_lo));
    uint8_t start_hi = _eeprom.Read(ADDROF(_begin, start_hi));
    _offset = start_hi == 0xff ? 0 : static_cast<int>(start_hi) << 8 | start_lo;
    _offset += _begin;

    uint8_t length_val = _eeprom.Read(ADDROF(_begin, length));
    if (length_val == 0xff)
        length_val = 0;
    _length = static_cast<int>(length_val) << LENGTH_SHIFT;

    uint8_t bits_per_card_val = _eeprom.Read(ADDROF(_begin, bits_per_card));
    if (bits_per_card_val == 0 || bits_per_card_val == 0xff)
        bits_per_card_val = 1;
    _bits_per_card = bits_per_card_val;

    uint8_t day_lo = _eeprom.Read(ADDROF(_begin, day_lo));
    uint8_t day_hi = _eeprom.Read(ADDROF(_begin, day_hi));
    _format_day = start_hi == 0xff ? 0 : static_cast<uint16_t>(day_hi) << 8 | day_lo;
}

void Recorder::StoreOffset()
{
    int start = _offset - _begin;
    uint8_t start_lo = start & 0xff;
    uint8_t start_hi = start >> 8;
    if (_eeprom.Read(ADDROF(_begin, start_lo)) != start_lo)
        _eeprom.Write(ADDROF(_begin, start_lo), start_lo);
    if (_eeprom.Read(ADDROF(_begin, start_hi)) != start_hi)
        _eeprom.Write(ADDROF(_begin, start_hi), start_hi);
}

void Recorder::StoreLength()
{
    uint8_t length_val = _length >> LENGTH_SHIFT;
    if (_eeprom.Read(ADDROF(_begin, length)) != length_val)
        _eeprom.Write(ADDROF(_begin, length), length_val);
}

void Recorder::StoreBitsPerCard()
{
    if (_eeprom.Read(ADDROF(_begin, bits_per_card)) != _bits_per_card)
        _eeprom.Write(ADDROF(_begin, bits_per_card), _bits_per_card);
}

void Recorder::StoreFormatDay()
{
    uint8_t day_lo = _format_day & 0xff;
    uint8_t day_hi = _format_day >> 8;
    if (_eeprom.Read(ADDROF(_begin, day_lo)) != day_lo)
        _eeprom.Write(ADDROF(_begin, day_lo), day_lo);
    if (_eeprom.Read(ADDROF(_begin, day_hi)) != day_hi)
        _eeprom.Write(ADDROF(_begin, day_hi), day_hi);
}

int8_t Recorder::Format(uint16_t count, uint8_t bits_per_card, uint32_t timestamp)
{
    // Round bits_per_card to the nearest power of 2 for proper byte alignment
    if (bits_per_card == 0)
        bits_per_card = 1;
    else if (bits_per_card <= 2)
        ;
    else if (bits_per_card <= 4)
        bits_per_card = 4;
    else
        bits_per_card = 8;

    // How many bytes are required to accomodate the runners?
    int bit_count = count * bits_per_card;
    uint8_t bit_tail = bit_count & 7;
    if (bit_tail)
        bit_count += 8 - bit_tail;
    int byte_count = bit_count >> 3;

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
    _bits_per_card = bits_per_card;
    StoreBitsPerCard();
    _format_day = timestamp / SECONDS_IN_DAY;
    StoreFormatDay();

    // Clear the memory span
    int addr_end = _begin + _size;
    for (int i = 0, addr = _offset; i < _length; ++i, ++addr) {
        if (addr >= addr_end)
            addr -= _size;
        if (_eeprom.Read(addr) != 0)
            _eeprom.Write(addr, 0);
    }
    return 0;
}

int8_t Recorder::Record(uint16_t card, int8_t increment)
{
    int offset = (card * _bits_per_card) >> 3;
    int bit_offset = (card * _bits_per_card) & 7;
    if (offset >= _length)
        return -1;  // The card beyond the range
    int addr = _offset + offset;
    if (addr >= _begin + _size)
        addr -= _size;
    uint8_t byte_val = _eeprom.Read(addr);
    uint8_t val_mask = (1 << _bits_per_card) - 1;
    int8_t val = (byte_val >> bit_offset) & val_mask;
    val += increment;
    // Check the limits: can't go beyond 0 punches and above whatever can be accomodated in the available bits.
    if (val < 0)
        val = 0;
    if (val >= val_mask)
        val = val_mask;
    int new_byte_val = byte_val & ~(val_mask << bit_offset);
    new_byte_val |= val << bit_offset;
    if (byte_val != new_byte_val)
        _eeprom.Write(addr, new_byte_val);
    return 0;
}

uint8_t Recorder::GetRecordCount(uint16_t card)
{
    int offset = (card * _bits_per_card) >> 3;
    int bit_offset = (card * _bits_per_card) & 7;
    if (offset >= _length)
        return false;  // The card beyond the range
    int addr = _offset + offset;
    if (addr >= _begin + _size)
        addr -= _size;
    uint8_t byte_val = _eeprom.Read(addr);
    uint8_t val_mask = (1 << _bits_per_card) - 1;
    return (byte_val >> bit_offset) & val_mask;
}

void Recorder::List(IVisitor &visitor, void *ctx)
{
    for (int i = 0, n = _length * 8 / _bits_per_card; i < n; ++i) {
        auto count = GetRecordCount(i);
        if (count)
            visitor.OnCard(i, count, ctx);
    }
}

} //namespace AOP;
