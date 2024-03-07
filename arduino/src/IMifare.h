#pragma once

#include <stdint.h>

namespace AOP {

struct IMifare
{
    static constexpr int BLOCK_SIZE = 16;
    static constexpr int BLOCK_COUNT = 64;
    static constexpr int SECTOR_COUNT = 16;

    static constexpr uint8_t KEY_SIZE = 6;
    static const uint8_t KEY_DEFAULT[KEY_SIZE];
    // Default access bits that allow using KeyB for data
    static constexpr uint8_t ACCESS_BITS_SIZE = 3;
    static const uint8_t ACCESS_BITS[ACCESS_BITS_SIZE];

    virtual ~IMifare() {}
    virtual uint8_t BlockToSector(uint8_t block) const = 0;
    virtual uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) = 0;
    virtual uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &dataSize) = 0;
    virtual uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t dataSize) = 0;
};

} //namespace AOP;
