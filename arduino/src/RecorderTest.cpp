#ifdef BUILD_TEST

#include "doctest/doctest.h"
#include "Recorder.h"
#include <string>
#include <random>

using namespace AOP;

namespace {

std::random_device rd;

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
    const char *space = "";

    void OnCard(uint16_t card, uint8_t count, void *ctx) override
    {
        Collector *self = reinterpret_cast<Collector *>(ctx);
        self->cards += space;
        space = " ";
        self->cards += std::to_string(card) + ":" + std::to_string(static_cast<uint16_t>(count));
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
    rec.Setup();
    CHECK(0 == rec.Format(0));
    CHECK(-1 == rec.Record(0));
    CHECK(0 == rec.GetRecordCount(0));
    CHECK(-1 == rec.Record(1));
    CHECK(0 == rec.GetRecordCount(1));
    CHECK("" == Collector::GetList(rec));
}

TEST_CASE("Simple range 256 cards")
{
    std::string mem(37, -1);
    TestEeprom eeprom{mem};
    Recorder rec(0, 37, eeprom);
    rec.Setup();
    for (int i = 0; i < 32; ++i)
    {
        CAPTURE(i);
        CHECK(0 == rec.Format(250));
        CHECK("" == Collector::GetList(rec));
        CHECK(0 == rec.GetRecordCount(0));
        CHECK(0 == rec.Record(0));
        CHECK(1 == rec.GetRecordCount(0));
        CHECK("0:1" == Collector::GetList(rec));
        CHECK(0 == rec.GetRecordCount(1));
        CHECK(0 == rec.Record(1));
        CHECK(1 == rec.GetRecordCount(1));
        CHECK("0:1 1:1" == Collector::GetList(rec));
        CHECK(0 == rec.GetRecordCount(127));
        CHECK(0 == rec.Record(127));
        CHECK(1 == rec.GetRecordCount(127));
        CHECK("0:1 1:1 127:1" == Collector::GetList(rec));
        CHECK(0 == rec.GetRecordCount(255));
        CHECK(0 == rec.Record(255));
        CHECK(1 == rec.GetRecordCount(255));
        CHECK("0:1 1:1 127:1 255:1" == Collector::GetList(rec));
        CHECK(0 == rec.GetRecordCount(256));
        CHECK(-1 == rec.Record(256));
        CHECK("0:1 1:1 127:1 255:1" == Collector::GetList(rec));
        CHECK(0 == rec.Record(255, -1));
        CHECK(0 == rec.GetRecordCount(255));
        CHECK("0:1 1:1 127:1" == Collector::GetList(rec));
    }
}

TEST_CASE("Restore record")
{
    std::string mem(37, -1);
    TestEeprom eeprom{mem};
    Recorder rec(0, 37, eeprom);
    rec.Setup();
    CHECK(0 == rec.Format(250));
    CHECK("" == Collector::GetList(rec));
    {
        Recorder rec2(0, 37, eeprom);
        rec2.Setup();
        CHECK("" == Collector::GetList(rec2));
    }
    CHECK(0 == rec.GetRecordCount(0));
    CHECK(0 == rec.Record(0));
    CHECK(1 == rec.GetRecordCount(0));
    CHECK("0:1" == Collector::GetList(rec));
    {
        Recorder rec2(0, 37, eeprom);
        rec2.Setup();
        CHECK("0:1" == Collector::GetList(rec2));
    }
    CHECK(0 == rec.GetRecordCount(1));
    CHECK(0 == rec.Record(1));
    CHECK(1 == rec.GetRecordCount(1));
    CHECK("0:1 1:1" == Collector::GetList(rec));
    {
        Recorder rec2(0, 37, eeprom);
        rec2.Setup();
        CHECK("0:1 1:1" == Collector::GetList(rec2));
    }
    CHECK(0 == rec.GetRecordCount(127));
    CHECK(0 == rec.Record(127));
    CHECK(1 == rec.GetRecordCount(127));
    CHECK("0:1 1:1 127:1" == Collector::GetList(rec));
    {
        Recorder rec2(0, 37, eeprom);
        rec2.Setup();
        CHECK("0:1 1:1 127:1" == Collector::GetList(rec2));
    }
    CHECK(0 == rec.GetRecordCount(255));
    CHECK(0 == rec.Record(255));
    CHECK(1 == rec.GetRecordCount(255));
    CHECK("0:1 1:1 127:1 255:1" == Collector::GetList(rec));
    {
        Recorder rec2(0, 37, eeprom);
        rec2.Setup();
        CHECK("0:1 1:1 127:1 255:1" == Collector::GetList(rec2));
    }
    CHECK(0 == rec.GetRecordCount(256));
    CHECK(-1 == rec.Record(256));
    CHECK("0:1 1:1 127:1 255:1" == Collector::GetList(rec));
}

TEST_CASE("Random record")
{
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> count_distr(31, 1024);

    std::string mem(133, -1);
    TestEeprom eeprom{mem};
    Recorder rec(0, 133, eeprom);
    rec.Setup();
    for (int i = 0; i < 32; ++i)
    {
        CAPTURE(i);
        int count = count_distr(gen);
        CAPTURE(count);
        std::uniform_int_distribution<int> card_step_distr(1, count / 8);
        CHECK(0 == rec.Format(count));
        CHECK("" == Collector::GetList(rec));

        std::string record;
        const char *space = "";
        for (int j = 0; j < count; j += card_step_distr(gen))
        {
            CHECK(0 == rec.GetRecordCount(j));
            CHECK(0 == rec.Record(j));
            CHECK(1 == rec.GetRecordCount(j));
            record += space;
            space = " ";
            record += std::to_string(j) + ":1";
            CHECK(record == Collector::GetList(rec));

            {
                Recorder rec2(0, 133, eeprom);
                rec2.Setup();
                REQUIRE(1 == rec2.GetRecordCount(j));
                REQUIRE(0 == rec2.GetRecordCount(j + 1));
                REQUIRE(record == Collector::GetList(rec2));
            }
        }
    }
}

#endif //BUILD_TEST
