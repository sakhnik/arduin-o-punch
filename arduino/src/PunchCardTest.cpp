#ifdef BUILD_TEST

#include "doctest/doctest.h"
#include "PunchCard.h"
#include <array>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace AOP;

namespace {

struct GlobalInit
{
    GlobalInit()
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }
} global_init;

constexpr uint8_t DEF_KEY[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

struct TestMifare : AOP::IMifare
{
    using BlockT = std::array<uint8_t, IMifare::BLOCK_SIZE>;
    std::array<BlockT, IMifare::BLOCK_COUNT> blocks;
    uint8_t authSector = -1;
    int failWrites = 0;

    TestMifare(uint8_t startSector)
    {
        for (auto &block : blocks)
            block.fill(0);
        for (int sector = 0; sector < 16; ++sector)
            memcpy(blocks[sector * 4 + 3].data(), DEF_KEY, sizeof(DEF_KEY));
        blocks[PunchCard::INDEX_KEY_BLOCK][PunchCard::SECTOR_OFFSET] = startSector;
    }

    int GetFailWrites() const
    {
        return failWrites;
    }

    void SetFailWrites(int v)
    {
        failWrites = v;
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        authSector = sector;
        return memcmp(key, blocks[4 * sector + 3].data(), sizeof(DEF_KEY));
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &) override
    {
        if (BlockToSector(block) != authSector)
            return 1;
        memcpy(data, blocks[block].data(), IMifare::BLOCK_SIZE);
        return 0;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t blockSize) override
    {
        if (BlockToSector(block) != authSector)
            return 1;
        if (failWrites) {
            --failWrites;
            memset(blocks[block].data(), 0, IMifare::BLOCK_SIZE);
            return 3;
        }
        memcpy(blocks[block].data(), data, blockSize);
        return 0;
    }
};

}  //namespace;

namespace doctest {

template<> struct StringMaker<AOP::Punch> {
    static String convert(const AOP::Punch &punch) {
        doctest::String out;
        out += "(";
        out += std::to_string(static_cast<unsigned>(punch.GetStation())).c_str();
        out += ", ";
        out += std::to_string(punch.GetTimestamp()).c_str();
        out += ")";
        return out;
    }
};

template<> struct StringMaker<std::vector<AOP::Punch>> {
    static String convert(const std::vector<AOP::Punch> &punches) {
        doctest::String out;
        out += std::to_string(punches.size()).c_str();
        out += " punches: [";
        const char *comma = "";
        for (const auto &punch : punches) {
            out += comma;
            comma = ", ";
            out += StringMaker<AOP::Punch>::convert(punch);
        }
        out += "]";
        return out;
    }
};

} //namespace;

TEST_CASE("PunchCard Punch")
{
    TestMifare mifare(2);
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(readOut.empty());

    std::vector<Punch> punches = {Punch(31, 100), Punch(32, 130), Punch(33, 221)};
    for (int i = 0; i != punches.size(); ++i) {
        CHECK(0 == punchCard.Punch(punches[i]));
        CHECK(0 == punchCard.ReadOut(readOut));
        CHECK(i + 1 == readOut.size());
        CHECK(punches[i] == readOut[i]);
    }

    CHECK(0 == punchCard.ReadOut(readOut));
    //CHECK(punches.size() == readOut.size());
    CHECK(punches == readOut);
}

TEST_CASE("PunchCard Punch asynchronous")
{
    TestMifare mifare(3);
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(readOut.empty());

    // Stations could potentially be not synchronized, the timestamps can go down occasionally.
    std::vector<Punch> punches = {Punch(31, 200), Punch(32, 130), Punch(33, 100)};
    for (int i = 0; i != punches.size(); ++i) {
        REQUIRE(0 == punchCard.Punch(punches[i]));
        REQUIRE(0 == punchCard.ReadOut(readOut));
        REQUIRE(i + 1 == readOut.size());
        CHECK(punches[i] == readOut[i]);
    }

    REQUIRE(0 == punchCard.ReadOut(readOut));
    //CHECK(punches.size() == readOut.size());
    CHECK(punches == readOut);
}

TEST_CASE("PunchCard max punches")
{
    TestMifare mifare(14);
    PunchCard punchCard(&mifare, DEF_KEY);

    auto testPunch = [](int i) {
        return Punch(PunchCard::START_STATION + i, 10000 + i * 100);
    };

    auto maxPunches = PunchCard::GetMaxPunches();
    for (int i = 0; i < maxPunches; ++i) {
        CHECK(0 == punchCard.Punch(testPunch(i)));
    }
    // No more space
    REQUIRE(ErrorCode::CARD_IS_FULL == punchCard.Punch(Punch(51, 65000)));

    std::vector<Punch> readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    REQUIRE(maxPunches == readOut.size());
    for (int i = 0; i < maxPunches; ++i) {
        CHECK(testPunch(i) == readOut[i]);
    }
}

TEST_CASE("PunchCard max repeated punches")
{
    TestMifare mifare(13);
    PunchCard punchCard(&mifare, DEF_KEY);

    auto testPunch = [](int i) {
        return Punch(PunchCard::START_STATION + i, 10000 + i * 100);
    };

    auto maxPunches = PunchCard::GetMaxPunches();
    for (int i = 0; i < maxPunches; ++i) {
        CAPTURE(i);
        // The start station can punch many times, the last timestamp counts
        if (!i) {
            auto p = testPunch(i);
            p.SetTimestamp(p.GetTimestamp() - 100);
            REQUIRE(0 == punchCard.Punch(p));
        }
        REQUIRE(0 == punchCard.Punch(testPunch(i)));
        // Only the first timestamp counts for the rest of the stations
        if (i) {
            auto p = testPunch(i);
            p.SetTimestamp(p.GetTimestamp() + 100);
            REQUIRE(0 == punchCard.Punch(p));
        }

        std::vector<Punch> readOut;
        REQUIRE(0 == punchCard.ReadOut(readOut));
        REQUIRE(i + 1 == readOut.size());
    }
    // No more space
    REQUIRE(ErrorCode::CARD_IS_FULL == punchCard.Punch(Punch(51, 65000)));

    std::vector<Punch> readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    REQUIRE(maxPunches == readOut.size());
    for (int i = 0; i < maxPunches; ++i) {
        CHECK(testPunch(i) == readOut[i]);
    }
}

TEST_CASE("PunchCard Clear")
{
    TestMifare mifare(7);
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> punches = {Punch(31, 100), Punch(39, 130)};
    for (int i = 0; i != punches.size(); ++i) {
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
    TestMifare mifare(12);
    PunchCard punchCard(&mifare, DEF_KEY);

    std::vector<Punch> punches = {Punch(31, 100), Punch(39, 130), Punch(PunchCard::START_STATION, 150)};
    for (int i = 0; i != punches.size(); ++i) {
        CHECK(0 == punchCard.Punch(punches[i]));
    }

    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(1 == readOut.size());
    CHECK(punches[2] == readOut[0]);
}

TEST_CASE("PunchCard Recover from failed write")
{
    // Some cheap cards may lose data when timeout occurs. The puncher should
    // be resilient and never lose ability to continue punching even after
    // occasional data loss in one block because of unsuccessful write operation.
    TestMifare mifare(7);
    PunchCard punchCard(&mifare, DEF_KEY);

    auto testPunch = [](int i) {
        return Punch(PunchCard::START_STATION + i, 10000 + i * 100);
    };

    std::vector<int> success;

    for (int i = 0; i < 100; ++i) {
        bool exceptionAnticipated = false;
        if (std::rand() % 100 < 10) {
            mifare.SetFailWrites(std::rand() % 4);
        }
        if (mifare.GetFailWrites() > 0) {
            exceptionAnticipated = true;
        }
        if (auto res = punchCard.Punch(testPunch(i))) {
            CHECK(exceptionAnticipated);
        } else {
            success.push_back(i);
        }
    }

    CHECK(!success.empty());

    mifare.SetFailWrites(0);
    std::vector<Punch> readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(success.size() == readOut.size());
    for (int i = 0; i < success.size(); ++i) {
        CHECK(testPunch(success[i]) == readOut[i]);
    }
}

#endif //BUILD_TEST
