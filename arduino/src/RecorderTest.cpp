#ifdef BUILD_TEST

#include "doctest/doctest.h"
#include "Recorder.h"
#include <string>

using namespace AOP;

namespace {

struct TestEeprom : Recorder::IEeprom
{
    std::string &mem;

    TestEeprom(std::string &mem) : mem{mem} { }

    uint8_t Read(int addr) override
    {
        return mem.at(addr);
    }

    void Write(int addr, uint8_t data) override
    {
        mem.at(addr) = data;
    }
};

struct Collector : Recorder::IVisitor
{
    std::string cards;
    const char *comma = "";

    void OnCard(int card, void *ctx) override
    {
        Collector *self = reinterpret_cast<Collector *>(ctx);
        self->cards += comma;
        comma = ", ";
        self->cards += std::to_string(card);
    }

    static std::string GetList(Recorder &rec)
    {
        Collector collector;
        rec.List(collector, &collector);
        return collector.cards;
    }
};

} //namespace;

TEST_CASE("No recording")
{
    std::string mem(34, -1);
    TestEeprom eeprom{mem};
    Recorder rec(0, 34, eeprom);
    CHECK(0 == rec.Format(0));
    CHECK(-1 == rec.Record(0));
    CHECK(!rec.IsRecorded(0));
    CHECK(-1 == rec.Record(1));
    CHECK(!rec.IsRecorded(1));
    CHECK("" == Collector::GetList(rec));
}

TEST_CASE("Simple range 256 cards")
{
    std::string mem(37, -1);
    TestEeprom eeprom{mem};
    Recorder rec(0, 37, eeprom);
    for (int i = 0; i < 32; ++i)
    {
        CAPTURE(i);
        CHECK(0 == rec.Format(250));
        CHECK("" == Collector::GetList(rec));
        CHECK(!rec.IsRecorded(0));
        CHECK(0 == rec.Record(0));
        CHECK(rec.IsRecorded(0));
        CHECK("0" == Collector::GetList(rec));
        CHECK(!rec.IsRecorded(1));
        CHECK(0 == rec.Record(1));
        CHECK(rec.IsRecorded(1));
        CHECK("0, 1" == Collector::GetList(rec));
        CHECK(!rec.IsRecorded(127));
        CHECK(0 == rec.Record(127));
        CHECK(rec.IsRecorded(127));
        CHECK("0, 1, 127" == Collector::GetList(rec));
        CHECK(!rec.IsRecorded(255));
        CHECK(0 == rec.Record(255));
        CHECK(rec.IsRecorded(255));
        CHECK("0, 1, 127, 255" == Collector::GetList(rec));
        CHECK(!rec.IsRecorded(256));
        CHECK(-1 == rec.Record(256));
        CHECK("0, 1, 127, 255" == Collector::GetList(rec));
    }
}

#endif //BUILD_TEST
