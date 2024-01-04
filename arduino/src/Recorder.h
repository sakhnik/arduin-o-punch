#pragma once

#include <stdint.h>

namespace AOP {

class Recorder
{
public:
    static const int LENGTH_SHIFT = 3;  // 64 runners : 8 runners/byte = 8 bytes

    struct IEeprom
    {
        virtual uint8_t Read(int addr) = 0;
        virtual void Write(int addr, uint8_t data) = 0;
    };

    Recorder(int begin, int size, IEeprom &);
    void Setup();

    int16_t GetSize() const { return _length * 8 / _bits_per_card; }
    uint8_t GetBitsPerRecord() const { return _bits_per_card; }
    int8_t Format(uint16_t count, uint8_t bits_per_card = 1);
    int8_t Record(uint16_t card, int8_t increment = 1);
    uint8_t GetRecordCount(uint16_t card);

    struct IVisitor
    {
        virtual void OnCard(uint16_t card, uint8_t count, void *ctx) = 0;
    };

    void List(IVisitor &, void *ctx);

private:
    int _begin, _size;
    IEeprom &_eeprom;
    int _offset{}, _length{};
    uint8_t _bits_per_card{1};

    void Restore();
    void StoreOffset();
    void StoreLength();
    void StoreBitsPerCard();
};

} //namespace AOP;
