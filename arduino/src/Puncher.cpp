#include "Puncher.h"
#include "Settings.h"
#include "IMifare.h"
#include "PunchCard.h"
#include "Buzzer.h"
#include "Operation.h"
#include "OutMux.h"
#include "RtcLog.h"
#include "defs.h"

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <endian.h>

//#define LOGGER
#ifdef LOGGER
#include "Logger.h"
#endif

namespace {

MFRC522DriverPinSimple ss_pin = SPI_SS;
MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
MFRC522 mfrc522{driver};  // Create MFRC522 instance.
unsigned long lastPunchTimeMs = 0;

} //namespace;

Puncher::Puncher(Settings &settings, Operation &operation, Buzzer &buzzer, OutMux &outMux)
    : _settings{settings}
    , _operation{operation}
    , _buzzer{buzzer}
    , _outMux{outMux}
{
}

void Puncher::Setup()
{
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);
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
        if (status != MFRC522Constants::StatusCode::STATUS_OK) {
            Serial.print(F("Auth failed: "));
            Serial.println(static_cast<int>(status));
            // When the authentication fails, timeout happens and the card isn't available anymore.
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();

            // Re-select card
            if (!mfrc522.PICC_IsNewCardPresent() ||
                !mfrc522.PICC_ReadCardSerial()) {
                Serial.println("Card lost");
            }
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

#pragma pack(push, 1)
struct DebugInfo
{
    uint8_t version = 0;
    uint16_t bootCount;
    uint8_t lastResetReason;
    uint32_t timeStats[static_cast<size_t>(Operation::Mode::Count)];
};
#pragma pack(pop)

} //namespace;

ErrorCode Puncher::Punch()
{
    // Soft reset the RFID chip periodically when the buzzer is silent to avoid distraction
    static int loopCount = 0;
    if (++loopCount > 100 && _buzzer.IsIdle()) {
        loopCount = 0;
        mfrc522.PCD_Reset();
        mfrc522.PCD_Init();
    }

    auto getInhibitTimeMs = [this]() {
        switch (_settings.GetActiveCardMode()) {
        case Settings::CardMode::Punch:
            break;
        case Settings::CardMode::ReadOut:
        case Settings::CardMode::Format:
            return 2000;
        }
        return 0;
    };

    // Throttle card processing when formatting and reading out
    auto now = millis();
    if (now - lastPunchTimeMs < getInhibitTimeMs()) {
        return ErrorCode::NO_CARD;
    }

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

    ErrorCode res = ErrorCode::OK;

    switch (_settings.GetActiveCardMode()) {
    case Settings::CardMode::Punch:
        res = DoPunch();
        break;
    case Settings::CardMode::ReadOut:
        res = DoReadOut();
        break;
    case Settings::CardMode::Format:
        res = DoFormat();
        break;
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    if (res == ErrorCode::OK) {
        lastPunchTimeMs = now;
    }

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

ErrorCode Puncher::DoPunch()
{
#ifdef LOGGER
    logger.Clear();
    Serial.println(F("----"));
#endif

    struct Callback : AOP::PunchCard::ICallback
    {
        Operation &operation;
        uint16_t card_id{};

        Callback(Operation &operation)
            : operation{operation}
        {
        }

        void OnCardId(uint16_t card_id) override
        {
            this->card_id = card_id;
        }

        std::string GetDebugInfo() override
        {
            DebugInfo info;
            info.bootCount = RtcLog::bootCount;
            info.lastResetReason = RtcLog::lastReset;
            auto stats = operation.GetStats();
            static_assert(stats.size() == std::size(info.timeStats));
            for (size_t i = 0; i < stats.size(); ++i) {
                info.timeStats[i] = htole32(stats[i]);
            }
            return std::string(reinterpret_cast<const char *>(&info), sizeof(DebugInfo));
        }

        void ConfirmDebugInfo(size_t) override
        {
        }
    } callback{_operation};

    MifareClassic mifareClassic{mfrc522};
    AOP::PunchCard punchCard{&mifareClassic, _settings.GetKey(), &callback};
    uint32_t timestamp = _settings.GetClock(nullptr) / 1000;
    AOP::Punch punch{_settings.GetId(), timestamp};
    auto res = punchCard.Punch(punch);
    if (res == ErrorCode::OK) {
        // If the punch was successful, record the card id.
        _settings.GetRecorder().Record(callback.card_id);
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

    if (res == ErrorCode::DUPLICATE_PUNCH)
        res = ErrorCode::OK;
    return res;
}

ErrorCode Puncher::DoReadOut()
{
    _buzzer.SignalDit();

    MifareClassic mifareClassic{mfrc522};
    AOP::PunchCard punchCard{&mifareClassic, _settings.GetKey()};
    AOP::PunchCard::CardReadOut readOut;
    auto res = punchCard.ReadOut(readOut);
    if (res != ErrorCode::OK) {
        return res;
    }
    _outMux.print(F("card="));
    _outMux.println(readOut.cardId);
    _outMux.print(F("punches="));
    _outMux.println(readOut.punches.size());
    for (const auto &punch : readOut.punches) {
        _outMux.printf("%d %d\r\n", punch.GetStation(), punch.GetTimestamp());
    }
    return res;
}

ErrorCode Puncher::DoFormat()
{
    _buzzer.SignalDit();

    uint16_t cardId = std::atoi(_settings.GetCardModeArg().c_str());
    _outMux.print(F("card-format "));
    _outMux.println(cardId);
    MifareClassic mifareClassic{mfrc522};
    AOP::PunchCard punchCard{&mifareClassic, _settings.GetKey()};
    auto res = punchCard.Format(cardId, _settings.GetKnownKeys());
    if (res != ErrorCode::OK) {
        return res;
    }
    return ErrorCode::OK;
}
