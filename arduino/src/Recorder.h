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

    int16_t GetSize() const { return _length * 8; }
    int8_t Format(int count);
    int8_t Record(uint16_t card, bool set = true);
    bool IsRecorded(uint16_t card);

    struct IVisitor
    {
        virtual void OnCard(uint16_t card, void *ctx) = 0;
    };

    void List(IVisitor &, void *ctx);

private:
    int _begin, _size;
    IEeprom &_eeprom;
    int _offset{}, _length{};

    void Restore();
    void StoreOffset();
    void StoreLength();
};

} //namespace AOP;
