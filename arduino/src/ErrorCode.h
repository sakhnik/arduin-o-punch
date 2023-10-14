#pragma once

#ifdef BUILD_TEST
# include <cstdint>
#else
# include <Arduino.h>
#endif

struct ErrorCode
{
    enum Error : uint8_t
    {
        OK = 0,
        // Mifare codes can be around here
        // But we don't care about actual values

        NO_CARD = 0x80,
        BAD_CHECKSUM,
        NO_SERIAL,
        WRONG_CARD,
        CARD_IS_FULL,
    };

    uint8_t code;

    operator uint8_t() const { return code; }

    ErrorCode(uint8_t code)
        : code{code}
    {
    }
};
