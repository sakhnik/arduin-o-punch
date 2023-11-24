#include "Puncher.h"
#include "defs.h"
#include "Context.h"
#include "src/IMifare.h"
#include "src/PunchCard.h"


Puncher::Puncher(Context &context)
    : _context{context}
{
}

void Puncher::Setup()
{
    mfrc522.PCD_Init();  // Init MFRC522 board.
    _last_init_time = millis();
}

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
        auto firstBlock = sector * 4;
        auto status = mfrc522.PCD_Authenticate(
                MFRC522Constants::PICC_Command::PICC_CMD_MF_AUTH_KEY_A,
                firstBlock,
                (MFRC522Constants::MIFARE_Key *)key,
                &mfrc522.uid
                );
        if (status)
        {
            Serial.print(F("Auth failed: "));
            Serial.println(static_cast<int>(status));
        }
        return status;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &dataSize) override
    {
        auto status = mfrc522.MIFARE_Read(block, data, &dataSize);
        if (status)
        {
            Serial.print(F("Read failed: "));
            Serial.println(static_cast<int>(status));
        }
        return status;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t dataSize) override
    {
        auto status = mfrc522.MIFARE_Write(block, const_cast<uint8_t *>(data), IMifare::BLOCK_SIZE);
        if (status)
        {
            Serial.print(F("Write failed: "));
            Serial.println(static_cast<int>(status));
        }
        return status;
    }
};

ErrorCode Puncher::Punch()
{
    // Reinit MFRC522 periodically
    auto now = millis();
    if (unsigned(now - _last_init_time) > 10000)
    {
        _last_init_time = now;
        mfrc522.PCD_Init();
    }

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return ErrorCode::NO_CARD;
    }

    // Select one of the cards.
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return ErrorCode::NO_SERIAL;
    }

    if (MFRC522Constants::PICC_TYPE_MIFARE_1K != mfrc522.PICC_GetType(mfrc522.uid.sak))
    {
        mfrc522.PICC_HaltA();
        return ErrorCode::WRONG_CARD;
    }

    MifareClassic mifareClassic{mfrc522};
    AOP::PunchCard punchCard{&mifareClassic, _context.GetKey(), &_context};
    uint32_t timestamp = _context.GetClock(nullptr) / 1000;
    AOP::Punch punch{_context.GetId(), timestamp};
    auto res = punchCard.Punch(punch);
    // The station could be configured to clear a card
    //auto res = punchCard.Clear();
    Serial.print(F("Punch result: "));
    Serial.println(static_cast<int>(res));

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return res;
}
