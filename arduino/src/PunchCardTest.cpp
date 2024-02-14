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
        blocks[1][PunchCard::INDEX_OFFSET] = 1;
        blocks[2][0] = 0;  // start punch
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        return memcmp(key, blocks[4 * sector + 3].data(), sizeof(DEF_KEY));
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &) override
    {
        memcpy(data, blocks[block].data(), IMifare::BLOCK_SIZE);
        return 0;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t blockSize) override
    {
        memcpy(blocks[block].data(), data, blockSize);
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
        return Punch(PunchCard::START_STATION + i + 1, 12345 * (i + 1));
    };

    auto maxPunches = 16 * (16 * 3 - 2) / Punch::STORAGE_SIZE - 1;
    for (int i = 0; i < maxPunches; ++i)
    {
        CHECK(0 == punchCard.Punch(testPunch(i)));
    }
    // No more space
    REQUIRE(ErrorCode::CARD_IS_FULL == punchCard.Punch(Punch(51, 65000)));

    std::vector<Punch> readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    REQUIRE(maxPunches == readOut.size());
    for (int i = 0; i < maxPunches; ++i)
    {
        CHECK(testPunch(i) == readOut[i]);
    }
}

TEST_CASE("PunchCard Clear")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> punches = {Punch(31, 100), Punch(39, 130)};
    for (int i = 0; i != punches.size(); ++i)
    {
        CHECK(0 == punchCard.Punch(punches[i]));
    }

    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(punches == readOut);

    punchCard.Clear();
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(0 == readOut.size());
}

TEST_CASE("PunchCard Clear at Start")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> punches = {Punch(31, 100), Punch(39, 130), Punch(PunchCard::START_STATION, 150)};
    for (int i = 0; i != punches.size(); ++i)
    {
        CHECK(0 == punchCard.Punch(punches[i]));
    }

    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(1 == readOut.size());
    CHECK(punches[2] == readOut[0]);
}

#endif //BUILD_TEST
