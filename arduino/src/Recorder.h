#pragma once

#include <stdint.h>

namespace AOP {

class Recorder
{
public:
    static constexpr const uint8_t LENGTH_SHIFT = 3;  // 64 runners : 8 runners/byte = 8 bytes
    static constexpr const uint32_t SECONDS_IN_DAY = 60ul * 60ul * 24ul;

    struct IEeprom
    {
        virtual uint8_t Read(uint16_t addr) = 0;
        virtual void Write(uint16_t addr, uint8_t data) = 0;
    };

    Recorder(IEeprom &);

    // Setup EEPROM limits after the external EEPROM has been initialized
    void Setup(uint16_t begin, uint16_t size);

    int16_t GetSize() const
    {
        return _length * 8 / _bits_per_card;
    }

    uint8_t GetBitsPerRecord() const
    {
        return _bits_per_card;
    }

    uint16_t GetFormatDay() const
    {
        return _format_day;
    }

    int8_t Format(uint16_t count, uint8_t bits_per_card, uint32_t timestamp);
    int8_t Record(uint16_t card, int8_t increment = 1);
    uint8_t GetRecordCount(uint16_t card);

    struct IVisitor
    {
        virtual void OnCard(uint16_t card, uint8_t count, void *ctx) = 0;
    };

    void List(IVisitor &, void *ctx);

private:
    uint16_t _begin, _size;
    IEeprom &_eeprom;
    uint16_t _offset{}, _length{};
    uint8_t _bits_per_card{1};
    uint16_t _format_day{0};

    void Restore();
    void StoreOffset();
    void StoreLength();
    void StoreBitsPerCard();
    void StoreFormatDay();
};

} //namespace AOP;
