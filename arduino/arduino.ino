#include <arduino-timer.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
//#include <MFRC522DriverI2C.h>
#include <MFRC522DriverPinSimple.h>

#define ENABLE_SERIAL 1

#if ENABLE_SERIAL
#include <MFRC522Debug.h>
#endif

auto timer = timer_create_default();

MFRC522DriverPinSimple ss_pin(10); // Configurable, see typical pin layout above.
MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
MFRC522 mfrc522{driver};  // Create MFRC522 instance.

void setup() {

#if ENABLE_SERIAL
    Serial.begin(115200);
    while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4).
#endif //ENABLE_SERIAL

    mfrc522.PCD_Init();  // Init MFRC522 board.

#if ENABLE_SERIAL
    MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);  // Show details of PCD - MFRC522 Card Reader details.
#endif //ENABLE_SERIAL
}

#include "src/IMifare.h"
#include "src/PunchCard.h"

struct MifareClassic : AOP::IMifare
{
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

void loop() {
    // TODO: better to call from a hardware timer ISR
    timer.tick();

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( !mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Select one of the cards.
    if ( !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    if (MFRC522Constants::PICC_TYPE_MIFARE_1K != mfrc522.PICC_GetType(mfrc522.uid.sak))
    {
        mfrc522.PICC_HaltA();
        return;
    }

    static MifareClassic mifareClassic;
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

    //if (!res)
    //{
    //    // Confirm with the led (the builtin isn't available together with SPI).
    //    digitalWrite(LED_BUILTIN, 1);
    //    timer.in(400, [](void*) { digitalWrite(LED_BUILTIN, 0); return false; });
    //}
}
