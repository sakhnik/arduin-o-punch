#pragma once

#if defined(BUILD_TEST) || defined(BUILD_WA)
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
        DATA_CORRUPTED,
        NO_SERIAL,
        WRONG_CARD,
        CARD_IS_FULL,
        DUPLICATE_PUNCH,
    };

    uint8_t code;

    operator uint8_t() const
    {
        return code;
    }

    ErrorCode(uint8_t code)
        : code{code}
    {
    }

    const char* ToStr() const;
};

#if defined(BUILD_TEST)

#include <iostream>

inline std::ostream& operator<<(std::ostream &os, ErrorCode e)
{
    return os << e.ToStr();
}

#endif //BUILD_TEST
