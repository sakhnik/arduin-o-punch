
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
    // Header format: 4 most recent punches in a block
    // The count field is only applicable to the header blocks, it's ignored when the full record
    // is archived.
    //
    // 0     | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9     | A     | B     | C     | D     | E     | F     |
    // CNT   | ID1   | ID2   | ID3   | ID4   | TIMESTAMP1 (LE)       | DT2 (LE)      | DT3 (LE)      | DT4 (LE)      | --    | --    |

    static constexpr auto INDEX_SECTOR = 0;
    static constexpr auto HEADER_BLOCK1 = 1;
    static constexpr auto HEADER_BLOCK2 = 2;
    static constexpr auto INDEX_KEY_BLOCK = 3;

    // ID is written into KeyB
    static constexpr auto ID_OFFSET = 10;
    static constexpr auto SECTOR_OFFSET = 12;

    static constexpr auto PUNCHES_PER_BLOCK = 4;
    static constexpr auto PUNCHES_PER_SECTOR = PUNCHES_PER_BLOCK * 3;

    static constexpr auto COUNT_OFFSET = 0;
    static constexpr auto STATION_OFFSET = 1;
    static constexpr auto TIME1_OFFSET = 5;
    static constexpr auto TIME2_OFFSET = 8;

    static constexpr const auto TIMESTAMP_OFFSET = 3;
    static constexpr const auto XOR_OFFSET = 7;
    static constexpr const auto INDEX_OFFSET = 8;

    // Default access bits that allow using KeyB for data
    static constexpr const uint8_t ACCESS_BITS[] = {0xff, 0x07, 0x80};

public:
    struct ICallback
    {
        // Notify about the card ID when punched successfully
        virtual void OnCardId(uint16_t card) = 0;
    };

    static int GetMaxPunches()
    {
        return (IMifare::SECTOR_COUNT - 2) * PUNCHES_PER_SECTOR;
    }

    PunchCard(IMifare *, const uint8_t *key, ICallback *callback = nullptr);

    // Punch a card with the given information about station and timestamp
    ErrorCode Punch(Punch punch);

    // Clear previous punches from a card without changing the format information
    ErrorCode Clear();

#ifdef BUILD_TEST
    using PunchesT = std::vector<AOP::Punch>;
    uint8_t ReadOut(PunchesT &);
    void _ReadPunchesFromBlock(uint8_t count, const uint8_t *data, PunchesT &);
#endif

private:
    IMifare *_mifare;
    const uint8_t *_key;
    ICallback *_callback;
    uint8_t _auth_sector = 0xff;

    uint8_t _Authenticate(uint8_t sector);
    uint8_t _ClearPunches(uint8_t startSector);
    uint8_t _RecoverHeader(uint8_t startSector, uint8_t *header);

    struct _Address
    {
        uint8_t block;
        uint8_t offset;
    };
    _Address _GetPunchAddr(uint8_t index);
    uint8_t _GetPunchBlock(uint8_t index, uint8_t startSector);
};

} //namespace AOP;
