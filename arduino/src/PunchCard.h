
#include "Punch.h"
#include "IMifare.h"

#ifdef BUILD_TEST
# include <vector>
#endif

namespace AOP {

class PunchCard
{
    static constexpr uint8_t HEADER_BLOCK = 1;

    IMifare *_mifare;
    const uint8_t *_key;
    uint8_t *_key_receiver;
    uint8_t _auth_sector = 0xff;

public:
    PunchCard(IMifare *, const uint8_t *key, uint8_t *key_receiver = nullptr);

    static void DummyProgress(uint8_t, uint8_t) { }
    using ProgressT = decltype(&DummyProgress);

    uint8_t Punch(Punch punch, ProgressT progress = &DummyProgress);
#ifdef BUILD_TEST
    uint8_t ReadOut(std::vector<AOP::Punch> &punches, ProgressT progress = &DummyProgress);
#endif

    // Keep in sync with Android's Puncher
    static constexpr const auto DESC_OFFSET = 0;
    static constexpr const uint8_t DESC_RUNNER = 1;
    static constexpr const uint8_t DESC_SERVICE = 2;

    static constexpr const auto ID_OFFSET = 1;
    static constexpr const auto SERVICE_ID = 0;

    static constexpr const auto TIMESTAMP_OFFSET = 3;
    static constexpr const auto INDEX_OFFSET = 7;
    static constexpr const auto KEY_OFFSET = 8;

private:
    uint8_t _Authenticate(uint8_t sector);

    struct _Address
    {
        uint8_t block;
        uint8_t offset;
    };
    _Address _GetPunchAddr(uint8_t index);
};

} //namespace AOP;
