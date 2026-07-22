#ifdef BUILD_TEST

#include <doctest/doctest.h>
#include "Recorder.h"
#include <string>
#include <random>

using namespace AOP;

namespace {

struct Collector : Recorder::IVisitor
{
    struct Entry
    {
        uint16_t card;
        uint32_t timestamp;
    };

    std::vector<Entry> entries;

    void OnCard(uint16_t card, uint32_t timestamp, void *) override
    {
        entries.push_back({card, timestamp});
    }
};

} // namespace

TEST_CASE("Empty recorder")
{
    Recorder rec;
    rec.Setup(1000);

    Collector c;
    rec.List(c, nullptr);

    CHECK(c.entries.empty());

    auto ts = rec.GetTimestamps(42);
    CHECK(ts.empty());
}

TEST_CASE("Single punch")
{
    Recorder rec;
    rec.Setup(100);

    rec.Record(42, 105);

    Collector c;
    rec.List(c, nullptr);

    REQUIRE(c.entries.size() == 1);
    CHECK(c.entries[0].card == 42);
    CHECK(c.entries[0].timestamp == 105);

    auto ts = rec.GetTimestamps(42);

    REQUIRE(ts.size() == 1);
    CHECK(ts[0] == 105);
}

TEST_CASE("Multiple punches preserve order")
{
    Recorder rec;
    rec.Setup(100);

    rec.Record(1, 105);
    rec.Record(2, 110);
    rec.Record(3, 130);

    Collector c;
    rec.List(c, nullptr);

    REQUIRE(c.entries.size() == 3);

    CHECK(c.entries[0].card == 3);
    CHECK(c.entries[0].timestamp == 130);

    CHECK(c.entries[1].card == 2);
    CHECK(c.entries[1].timestamp == 110);

    CHECK(c.entries[2].card == 1);
    CHECK(c.entries[2].timestamp == 105);
}

TEST_CASE("GetTimestamps returns all timestamps for a card")
{
    Recorder rec;
    rec.Setup(100);

    rec.Record(10, 105);
    rec.Record(20, 120);
    rec.Record(10, 150);
    rec.Record(30, 180);
    rec.Record(10, 200);

    auto ts = rec.GetTimestamps(10);

    REQUIRE(ts.size() == 3);

    CHECK(ts[0] == 200);
    CHECK(ts[1] == 150);
    CHECK(ts[2] == 105);
}

TEST_CASE("Different cards are filtered correctly")
{
    Recorder rec;
    rec.Setup(0);

    rec.Record(1, 10);
    rec.Record(2, 20);
    rec.Record(1, 30);

    auto ts1 = rec.GetTimestamps(1);
    auto ts2 = rec.GetTimestamps(2);
    auto ts3 = rec.GetTimestamps(3);

    REQUIRE(ts1.size() == 2);
    CHECK(ts1[0] == 30);
    CHECK(ts1[1] == 10);

    REQUIRE(ts2.size() == 1);
    CHECK(ts2[0] == 20);

    CHECK(ts3.empty());
}

TEST_CASE("Ring buffer keeps newest MAX_PUNCHES records")
{
    Recorder rec;
    rec.Setup(0);

    constexpr int N = 10000 + 100;

    for (int i = 1; i <= N; ++i)
        rec.Record(static_cast<uint16_t>(i), i);

    Collector c;
    rec.List(c, nullptr);

    REQUIRE(c.entries.size() == 10000);

    CHECK(c.entries.front().timestamp == N);
    CHECK(c.entries.back().timestamp == 101);
}

TEST_CASE("Zero delta is preserved")
{
    Recorder rec;
    rec.Setup(100);

    rec.Record(1, 100);
    rec.Record(2, 100);

    Collector c;
    rec.List(c, nullptr);

    REQUIRE(c.entries.size() == 2);

    CHECK(c.entries[0].timestamp == 100);
    CHECK(c.entries[1].timestamp == 100);
}

#endif //BUILD_TEST
