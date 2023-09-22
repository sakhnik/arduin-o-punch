#include "Puncher.h"
#include "defs.h"
#include "src/IMifare.h"
#include "src/PunchCard.h"

#if ENABLE_SERIAL
#include <MFRC522Debug.h>
#endif


void Puncher::Setup()
{
    mfrc522.PCD_Init();  // Init MFRC522 board.

#if ENABLE_SERIAL
    MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);  // Show details of PCD - MFRC522 Card Reader details.
#endif //ENABLE_SERIAL
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
#if ENABLE_SERIAL
            Serial.print(F("Auth failed: "));
            Serial.println(static_cast<int>(status));
#endif //ENABLE_SERIAL
        }
        return status;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &dataSize) override
    {
        auto status = mfrc522.MIFARE_Read(block, data, &dataSize);
        if (status)
        {
#if ENABLE_SERIAL
            Serial.print(F("Read failed: "));
            Serial.println(static_cast<int>(status));
#endif //ENABLE_SERIAL
        }
        return status;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t dataSize) override
    {
        auto status = mfrc522.MIFARE_Write(block, const_cast<uint8_t *>(data), IMifare::BLOCK_SIZE);
        if (status)
        {
#if ENABLE_SERIAL
            Serial.print(F("Write failed: "));
            Serial.println(static_cast<int>(status));
#endif //ENABLE_SERIAL
        }
        return status;
    }
};

uint8_t Puncher::Punch()
{
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return E_NO_CARD;
    }

    // Select one of the cards.
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return E_NO_SERIAL;
    }

    if (MFRC522Constants::PICC_TYPE_MIFARE_1K != mfrc522.PICC_GetType(mfrc522.uid.sak))
    {
        mfrc522.PICC_HaltA();
        return E_WRONG_CARD;
    }

    MifareClassic mifareClassic{mfrc522};
    static const uint8_t key[6] = {0};
    AOP::PunchCard punchCard{&mifareClassic, key};
    uint32_t timestamp = millis() / 1000;
    AOP::Punch punch{13, timestamp};
    auto res = punchCard.Punch(punch);
#if ENABLE_SERIAL
    Serial.print(F("Punch result: "));
    Serial.println(static_cast<int>(res));
#endif //ENABLE_SERIAL

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return E_OK;
}
