#pragma once

#include "OperationMode.h"
#include <cstdint>
#include <cstddef>

class OutMux;

#pragma pack(push, 1)

class Stats
{
public:
    uint16_t size = sizeof(Stats);
    uint8_t version = 0;
    uint16_t bootCount;
    uint8_t lastResetReason;
    uint32_t timeStats[static_cast<size_t>(OperationMode::Count)];

    void ToN();
    void FromN();
    void Print(OutMux &);

private:
    void DumpTimeStats(OutMux &);
};

#pragma pack(pop)
