#ifdef BUILD_TEST

#include "PunchCard.h"
#include <array>
#include <string>  // std::to_string()
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iomanip>

#include <doctest/doctest.h>

using namespace AOP;

namespace {

struct GlobalInit
{
    GlobalInit()
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }
} global_init;

struct TestMifare : AOP::IMifare
{
    using BlockT = std::array<uint8_t, IMifare::BLOCK_SIZE>;
    std::array<BlockT, IMifare::BLOCK_COUNT> blocks;
    uint8_t authSector = -1;
    int failWrites = 0;
    int failDelay = 0;

    TestMifare()
    {
        for (auto &block : blocks)
            block.fill(0);
        for (int sector = 0; sector < 16; ++sector)
            memcpy(blocks[sector * 4 + 3].data(), IMifare::KEY_DEFAULT.data(), IMifare::KEY_SIZE);
    }

    int GetFailWrites() const
    {
        return failWrites;
    }

    void SetFailWrites(int delay, int fails)
    {
        failDelay = delay;
        failWrites = fails;
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        assert(sector < IMifare::SECTOR_COUNT);
        authSector = sector;
        return memcmp(key, blocks[4 * sector + 3].data(), IMifare::KEY_SIZE);
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
            if (!failDelay) {
                --failWrites;
                memset(blocks[block].data(), 0xFF, IMifare::BLOCK_SIZE);
                return 3;
            }
            --failDelay;
        }
        memcpy(blocks[block].data(), data, blockSize);
        return 0;
    }

    void Print(std::ostream& os = std::cout) const
    {
        for (size_t i = 0; i < BLOCK_COUNT; ++i)
        {
            os << std::setw(2) << i << ":";
            for (uint8_t b : blocks[i]) {
                os << ' ' << std::hex << std::setw(2) << std::setfill('0') << unsigned(b);
            }
            os << std::dec << std::setfill(' ') << '\n';
        }
    }
};

}  //namespace;

namespace doctest {

template<> struct StringMaker<AOP::Punch>
{
    static String convert(const AOP::Punch &punch)
    {
        doctest::String out;
        out += "(";
        out += std::to_string(static_cast<unsigned>(punch.GetStation())).c_str();
        out += ", ";
        out += std::to_string(punch.GetTimestamp()).c_str();
        out += ")";
        return out;
    }
};

template<> struct StringMaker<std::vector<AOP::Punch>>
{
    static String convert(const std::vector<AOP::Punch> &punches)
    {
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
    TestMifare mifare;
    PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
    REQUIRE(0 == punchCard.Format(123, {}));

    PunchCard::CardReadOut readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(123 == readOut.cardId);
    CHECK(readOut.punches.empty());

    std::vector<Punch> punches = {Punch(31, 100), Punch(32, 130), Punch(33, 221)};
    for (int i = 0; i != punches.size(); ++i) {
        CHECK(0 == punchCard.Punch(punches[i]));
        CHECK(0 == punchCard.ReadOut(readOut));
        CHECK(123 == readOut.cardId);
        CHECK(i + 1 == readOut.punches.size());
        CHECK(punches[i] == readOut.punches[i]);
    }

    CHECK(0 == punchCard.ReadOut(readOut));
    //CHECK(punches.size() == readOut.size());
    CHECK(punches == readOut.punches);
}

TEST_CASE("PunchCard Punch asynchronous")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
    REQUIRE(0 == punchCard.Format(12345, {}));

    PunchCard::CardReadOut readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(12345 == readOut.cardId);
    CHECK(readOut.punches.empty());

    // Stations could potentially be not synchronized, the timestamps can go down occasionally.
    std::vector<Punch> punches = {Punch(31, 200), Punch(32, 130), Punch(33, 100)};
    for (int i = 0; i != punches.size(); ++i) {
        REQUIRE(0 == punchCard.Punch(punches[i]));
        REQUIRE(0 == punchCard.ReadOut(readOut));
        CHECK(12345 == readOut.cardId);
        REQUIRE(i + 1 == readOut.punches.size());
        CHECK(punches[i] == readOut.punches[i]);
    }

    REQUIRE(0 == punchCard.ReadOut(readOut));
    //CHECK(punches.size() == readOut.size());
    CHECK(punches == readOut.punches);
}

TEST_CASE("PunchCard max punches")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
    REQUIRE(0 == punchCard.Format(123, {}));

    auto testPunch = [](int i) {
        return Punch(PunchCard::START_STATION + i, 10000 + i * 100);
    };

    auto maxPunches = PunchCard::GetMaxPunches();
    for (int i = 0; i < maxPunches; ++i) {
        CHECK(0 == punchCard.Punch(testPunch(i)));
    }
    // No more space
    REQUIRE(ErrorCode::CARD_IS_FULL == punchCard.Punch(Punch(51, 65000)));

    PunchCard::CardReadOut readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    CHECK(readOut.cardId == 123);
    REQUIRE(maxPunches == readOut.punches.size());
    for (int i = 0; i < maxPunches; ++i) {
        CHECK(testPunch(i) == readOut.punches[i]);
    }
}

TEST_CASE("PunchCard max repeated punches")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
    REQUIRE(0 == punchCard.Format(123, {}));

    auto testPunch = [](int i) {
        return Punch(PunchCard::START_STATION + i, 10000 + i * 100);
    };

    auto maxPunches = PunchCard::GetMaxPunches();
    for (int i = 0; i < maxPunches; ++i) {
        CAPTURE(i);
        REQUIRE(0 == punchCard.Punch(testPunch(i)));
        // Only the first timestamp counts for the rest of the stations
        if (i) {
            auto p = testPunch(i);
            p.SetTimestamp(p.GetTimestamp() + 100);
            REQUIRE(ErrorCode::DUPLICATE_PUNCH == punchCard.Punch(p));
        }

        PunchCard::CardReadOut readOut;
        REQUIRE(0 == punchCard.ReadOut(readOut));
        REQUIRE(i + 1 == readOut.punches.size());
    }
    // No more space
    REQUIRE(ErrorCode::CARD_IS_FULL == punchCard.Punch(Punch(51, 65000)));

    PunchCard::CardReadOut readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    REQUIRE(maxPunches == readOut.punches.size());
    for (int i = 0; i < maxPunches; ++i) {
        CHECK(testPunch(i) == readOut.punches[i]);
    }
}

TEST_CASE("PunchCard Clear")
{
    TestMifare mifare;
    PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
    REQUIRE(0 == punchCard.Format(123, {}));

    std::vector<Punch> punches = {Punch(31, 100), Punch(39, 130)};
    for (int i = 0; i != punches.size(); ++i) {
        CHECK(0 == punchCard.Punch(punches[i]));
    }

    PunchCard::CardReadOut readOut;
    CHECK(0 == punchCard.ReadOut(readOut));
    CHECK(punches == readOut.punches);
}

TEST_CASE("PunchCard Recover from failed write")
{
    // Some cheap cards may lose data when timeout occurs. The puncher should
    // be resilient and never lose ability to continue punching even after
    // occasional data loss in one block because of unsuccessful write operation.
    TestMifare mifare;
    PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
    REQUIRE(0 == punchCard.Format(123, {}));

    auto testPunch = [](int i) {
        return Punch(PunchCard::START_STATION + i, 10000 + i * 100);
    };

    int successfulPunches = 0;
    for (int i = 0; i < 100; ++i) {
        bool exceptionAnticipated = false;
        if (std::rand() % 100 < 10) {
            mifare.SetFailWrites(std::rand() % 4, std::rand() % 4);
        }
        while (true) {
            if (mifare.GetFailWrites() > 0) {
                exceptionAnticipated = true;
            }
            auto res = punchCard.Punch(testPunch(i));
            if (res != ErrorCode::OK && res != ErrorCode::DUPLICATE_PUNCH) {
                REQUIRE(exceptionAnticipated);
            } else {
                ++successfulPunches;
                PunchCard::CardReadOut readOut;
                REQUIRE(0 == punchCard.ReadOut(readOut));
                REQUIRE(successfulPunches == readOut.punches.size());
                break;
            }
        }
    }

    PunchCard::CardReadOut readOut;
    REQUIRE(0 == punchCard.ReadOut(readOut));
    REQUIRE(100 == readOut.punches.size());
    for (int i = 0; i < 100; ++i) {
        REQUIRE(testPunch(i) == readOut.punches[i]);
    }
}

TEST_CASE("PunchCard Multiple Format")
{
    TestMifare mifare;

    auto getPunch = [](int i, int j) {
        return Punch(11 + i + j, 100 + i + j);
    };

    auto getCardId = [](int i) {
        return 1000 + i;
    };

    for (int i = 1; i < 100; ++i) {
        PunchCard punchCard(&mifare, IMifare::KEY_DEFAULT);
        REQUIRE(0 == punchCard.Format(getCardId(i), {}));

        for (int j = 0; j != i; ++j) {
            REQUIRE(0 == punchCard.Punch(getPunch(i, j)));
        }

        PunchCard::CardReadOut readOut;
        REQUIRE(0 == punchCard.ReadOut(readOut));
        REQUIRE(getCardId(i) == readOut.cardId);
        REQUIRE(i == readOut.punches.size());
        for (int j = 0; j < i; ++j) {
            REQUIRE(getPunch(i, j) == readOut.punches[j]);
        }

        std::vector<PunchCard::CardReadOut> allRuns;
        REQUIRE(0 == punchCard.ReadOut(100, allRuns));
        REQUIRE(allRuns.size() <= i);
        for (int j = 0; j < allRuns.size(); ++j) {
            auto n = i - j;
            REQUIRE(allRuns[j].cardId == getCardId(n));
            REQUIRE(allRuns[j].punches.size() == n);
            for (int k = 0; k != n; ++k) {
                REQUIRE(allRuns[j].punches[k] == getPunch(i - j, k));
            }
        }
    }
}

#endif //BUILD_TEST
