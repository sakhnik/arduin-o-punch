#include "Puncher.h"
#include "Context.h"
#include "src/IMifare.h"
#include "src/PunchCard.h"

//#define LOGGER
#ifdef LOGGER
#include "Logger.h"
#endif

Puncher::Puncher(Context &context)
    : _context{context}
{
}

void Puncher::Setup()
{
    mfrc522.PCD_Init();  // Init MFRC522 board.
}

namespace {

#ifdef LOGGER
Logger logger;
#endif

struct MifareClassic : AOP::IMifare
{
    MFRC522 &mfrc522;

    MifareClassic(MFRC522 &mfrc522)
        : mfrc522{mfrc522}
    {
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
#ifdef LOGGER
        logger.Out("A");
        logger.Out(sector);
        logger.Out("\r\n");
#endif
        auto firstBlock = sector * 4;
        auto status = mfrc522.PCD_Authenticate(
                          MFRC522Constants::PICC_Command::PICC_CMD_MF_AUTH_KEY_A,
                          firstBlock,
                          (MFRC522Constants::MIFARE_Key *)key,
                          &mfrc522.uid
                      );
        if (status) {
            Serial.print(F("Auth failed: "));
            Serial.println(static_cast<int>(status));
        }
        return status;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &dataSize) override
    {
#ifdef LOGGER
        logger.Out("R");
        logger.Out(block);
        logger.Out(" ");
#endif
        auto status = mfrc522.MIFARE_Read(block, data, &dataSize);
        if (status) {
            Serial.print(F("Read failed: "));
            Serial.println(static_cast<int>(status));
        } else {
#ifdef LOGGER
            logger.Out(data, 16);
#endif
        }
#ifdef LOGGER
        logger.Out("\r\n");
#endif
        return status;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t dataSize) override
    {
#ifdef LOGGER
        logger.Out("W");
        logger.Out(block);
        logger.Out(" ");
        logger.Out(data, 16);
        logger.Out("\r\n");
#endif
        auto status = mfrc522.MIFARE_Write(block, const_cast<uint8_t *>(data), IMifare::BLOCK_SIZE);
        if (status) {
            Serial.print(F("Write failed: "));
            Serial.println(static_cast<int>(status));
        }
        return status;
    }
};

} //namespace;

ErrorCode Puncher::Punch()
{
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return ErrorCode::NO_CARD;
    }

    // Select one of the cards.
    if (!mfrc522.PICC_ReadCardSerial()) {
        return ErrorCode::NO_SERIAL;
    }

    if (MFRC522Constants::PICC_TYPE_MIFARE_1K != mfrc522.PICC_GetType(mfrc522.uid.sak)) {
        mfrc522.PICC_HaltA();
        return ErrorCode::WRONG_CARD;
    }

#ifdef LOGGER
    logger.Clear();
    Serial.println(F("----"));
#endif
    struct Callback : AOP::PunchCard::ICallback
    {
        uint16_t card_id{};

        void OnCardId(uint16_t card_id) override
        {
            this->card_id = card_id;
        }
    } callback;

    MifareClassic mifareClassic{mfrc522};
    AOP::PunchCard punchCard{&mifareClassic, _context.GetKey(), &callback};
    uint32_t timestamp = _context.GetClock(nullptr) / 1000;
    AOP::Punch punch{_context.GetId(), timestamp};
    auto res = punchCard.Punch(punch);
    if (res == ErrorCode::OK) {
        // If the punch was successful, record the card id.
        _context.GetRecorder().Record(callback.card_id);
    }
    // The station could be configured to clear a card
    //auto res = punchCard.Clear();
    Serial.print(F("Punch: "));
    Serial.print(res.ToStr());
    if (res == ErrorCode::OK || res == ErrorCode::DUPLICATE_PUNCH) {
        Serial.print(F(" card="));
        Serial.print(callback.card_id);
    }
    Serial.println();
#ifdef LOGGER
    logger.Print();
#endif

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    if (res == ErrorCode::DUPLICATE_PUNCH)
        res = ErrorCode::OK;
    return res;
}

void Puncher::AntennaOn()
{
    mfrc522.PCD_AntennaOn();
}

void Puncher::AntennaOff()
{
    mfrc522.PCD_AntennaOff();
}
