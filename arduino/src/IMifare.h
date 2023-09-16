#pragma once

#include <stdint.h>

namespace AOP {

struct IMifare
{
    static constexpr int BLOCK_SIZE = 16;
    static constexpr int BLOCK_COUNT = 64;

    virtual ~IMifare() {}
    virtual uint8_t BlockToSector(uint8_t block) const = 0;
    virtual int AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) = 0;
    virtual int ReadBlock(uint8_t block, uint8_t *data) = 0;
    virtual int WriteBlock(uint8_t block, uint8_t *data) = 0;
};

} //namespace AOP;
