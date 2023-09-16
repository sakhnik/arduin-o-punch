#ifdef BUILD_TEST

#include "doctest/doctest.h"
#include "PunchCard.h"
#include <array>
#include <cstring>

using namespace AOP;

namespace {

constexpr uint8_t DEF_KEY[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

struct TestMifare : AOP::IMifare
{
    using BlockT = std::array<uint8_t, IMifare::BLOCK_SIZE>;
    std::array<BlockT, IMifare::BLOCK_COUNT> blocks;

    TestMifare()
    {
        for (auto &block : blocks)
            block.fill(0);
        for (int sector = 0; sector < 16; ++sector)
            memcpy(blocks[sector * 4 + 3].data(), DEF_KEY, sizeof(DEF_KEY));
        blocks[1][5] = 1;
        blocks[2][0] = 0;
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    int AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        return memcmp(key, blocks[4 * sector + 3].data(), sizeof(DEF_KEY));
    }

    int ReadBlock(uint8_t block, uint8_t *data) override
    {
        memcpy(data, blocks[block].data(), IMifare::BLOCK_SIZE);
        return 0;
    }

    int WriteBlock(uint8_t block, uint8_t *data) override
    {
        memcpy(blocks[block].data(), data, IMifare::BLOCK_SIZE);
        return 0;
    }
};

}  //namespace;

TEST_CASE("PunchCard Punch")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(readOut.empty());

    std::vector<Punch> punches = {Punch(31, 100), Punch(32, 130), Punch(33, 221)};
    for (int i = 0; i != punches.size(); ++i)
    {
        CHECK(0 == punchCard.Punch(punches[i]));
        CHECK(0 == punchCard.ReadOut(readOut));
        CHECK(i + 1 == readOut.size());
        CHECK(punches[i] == readOut[i]);
    }

    CHECK(0 == punchCard.ReadOut(readOut));
    //CHECK(punches.size() == readOut.size());
    CHECK(punches == readOut);
}

TEST_CASE("PunchCard max punches")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, DEF_KEY);

    auto testPunch = [](int i) {
        return Punch(i + 1, 100 * (i + 1));
    };

    auto maxPunches = 16 * (16 * 3 - 2) / Punch::STORAGE_SIZE - 1;
    for (int i = 0; i < maxPunches; ++i)
    {
        CHECK(0 == punchCard.Punch(testPunch(i)));
    }
    // No more space
    REQUIRE(-1 == punchCard.Punch(Punch(1, 65000)));

    std::vector<Punch> readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    REQUIRE(maxPunches == readOut.size());
    for (int i = 0; i < maxPunches; ++i)
    {
        CHECK(testPunch(i) == readOut[i]);
    }
}

#endif //BUILD_TEST
