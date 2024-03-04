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
        DATA_CORRUPTED,
        NO_SERIAL,
        WRONG_CARD,
        CARD_IS_FULL,
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
};

#ifdef BUILD_TEST

#include <iostream>

inline std::ostream& operator<<(std::ostream &os, ErrorCode e)
{
    switch (e) {
    case ErrorCode::NO_CARD:
        os << "No card";
        break;
    case ErrorCode::DATA_CORRUPTED:
        os << "Corrupt";
        break;
    case ErrorCode::NO_SERIAL:
        os << "No serial";
        break;
    case ErrorCode::WRONG_CARD:
        os << "Wrong card";
        break;
    case ErrorCode::CARD_IS_FULL:
        os << "Full";
        break;
    default:
        os << "Error(" << static_cast<int>(e) << ")";
        break;
    }
    return os;
}

#endif //BUILD_TEST
