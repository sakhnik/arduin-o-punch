
#include "Punch.h"
#include "IMifare.h"
#include "ErrorCode.h"

#ifdef BUILD_TEST
# include <vector>
#endif

namespace AOP {

class PunchCard
{
public:
    static constexpr const uint8_t CHECK_STATION = 1;
    static constexpr const uint8_t START_STATION = 10;
    static constexpr const uint8_t FINISH_STATION = 255;

    // Keep in sync with Android's Puncher
    //
    // Header format (sector 0, block 1):
    // 0     | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9 ...
    // DESC  | ID_LO | ID_HI | ..FORMATTING TIMESTAMP (LE).. | XOR CS| INDEX | KEY_A (SERVICE CARD)

    static constexpr uint8_t HEADER_BLOCK = 1;

    static constexpr const auto DESC_OFFSET = 0;
    static constexpr const uint8_t DESC_RUNNER = 1;
    static constexpr const uint8_t DESC_SERVICE = 2;

    static constexpr const auto ID_OFFSET = 1;
    static constexpr const auto SERVICE_ID = 0;

    static constexpr const auto TIMESTAMP_OFFSET = 3;
    static constexpr const auto XOR_OFFSET = 7;
    static constexpr const auto INDEX_OFFSET = 8;
    static constexpr const auto KEY_OFFSET = 9;

public:
    struct ICallback
    {
        virtual void OnNewKey(const uint8_t *key) = 0;
        // Notify about the card ID when punched successfully
        virtual void OnCardId(uint16_t card) = 0;
    };

    PunchCard(IMifare *, const uint8_t *key, ICallback *callback = nullptr);

    static void DummyProgress(uint8_t, uint8_t) { }
    using ProgressT = decltype(&DummyProgress);

    // Punch a card with the given information about station and timestamp
    ErrorCode Punch(Punch punch, ProgressT progress = &DummyProgress);

    // Clear previous punches from a card
    ErrorCode Clear(ProgressT progress = &DummyProgress);

#ifdef BUILD_TEST
    uint8_t ReadOut(std::vector<AOP::Punch> &punches, ProgressT progress = &DummyProgress);
#endif

private:
    IMifare *_mifare;
    const uint8_t *_key;
    ICallback *_callback;
    uint8_t _auth_sector = 0xff;

    uint8_t _Authenticate(uint8_t sector);
    uint8_t _CheckIntegrity(const uint8_t *header);

    struct _Address
    {
        uint8_t block;
        uint8_t offset;
    };
    _Address _GetPunchAddr(uint8_t index);
};

} //namespace AOP;
