#include "ErrorCode.h"

#ifdef ARDUINO
# include <MFRC522v2.h>
#endif

namespace {

char buf[4] = "";
constexpr const char *const DIGITS = "0123456789ABCDEF";

} //namespace;

const char* ErrorCode::ToStr() const
{
    switch (code) {
    case ErrorCode::OK:
        return "OK";
    case ErrorCode::DUPLICATE_PUNCH:
        return "DUP";
    case ErrorCode::NO_CARD:
        return "NOCARD";
    case ErrorCode::DATA_CORRUPTED:
        return "CORRUPT";
    case ErrorCode::NO_SERIAL:
        return "NOSER";
    case ErrorCode::WRONG_CARD:
        return "WRCARD";
    case ErrorCode::CARD_IS_FULL:
        return "FULL";
#ifdef ARDUINO
    case MFRC522Constants::STATUS_ERROR:
        return "RC COMM";    // Error in communication.
    case MFRC522Constants::STATUS_COLLISION:
        return "RC COLL"; // Collision detected.
    case MFRC522Constants::STATUS_TIMEOUT:
        return "RC TMOUT";     // Timeout in communication.
    case MFRC522Constants::STATUS_NO_ROOM:
        return "RC BUF";      // A buffer is not big enough.
    case MFRC522Constants::STATUS_INTERNAL_ERROR:
        return "RC INT";   // Internal error in the code. Should not happen ;-)
    case MFRC522Constants::STATUS_INVALID:
        return "RC ARG"; // Invalid argument.
    case MFRC522Constants::STATUS_CRC_WRONG:
        return "RC CRC";       // The CRC_A does not match.
    case MFRC522Constants::STATUS_UNKNOWN:
        return "RC UNK";
    case MFRC522Constants::STATUS_MIFARE_NACK:
        return "RC NACK";    // A MIFARE PICC responded with NAK.
#endif
    default:
        buf[0] = DIGITS[code >> 4];
        buf[1] = DIGITS[code & 0xf];
        buf[2] = 0;
        return buf;
    }
}
