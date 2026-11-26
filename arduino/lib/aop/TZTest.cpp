#ifdef BUILD_TEST

#include <doctest/doctest.h>
#include "TZ.h"

#include <cstdio>
#include <initializer_list>

using namespace AOP;

// Readable failure output for DateTime values.
namespace doctest {
template <>
struct StringMaker<DateTime> {
    static String convert(const DateTime& d) {
        char buf[32];
        std::snprintf(buf, sizeof buf, "%04u-%02u-%02u %02u:%02u:%02u",
                      static_cast<unsigned>(d.year),
                      static_cast<unsigned>(d.month),
                      static_cast<unsigned>(d.day),
                      static_cast<unsigned>(d.hour),
                      static_cast<unsigned>(d.minute),
                      static_cast<unsigned>(d.second));
        return String(buf);
    }
};
}  // namespace doctest

namespace {

// uint8_t would print as a character on failure, so widen to int.
int LastSun(uint16_t year, uint8_t month) {
    return TZ::LastSunday(year, month);
}

DateTime Std(uint16_t y, uint8_t mo, uint8_t d,
             uint8_t h = 0, uint8_t mi = 0, uint8_t s = 0) {
    return DateTime(y, mo, d, h, mi, s);
}

}  // namespace

// ---------------------------------------------------------------------------
// Compile-time guarantees (fail the build rather than the test run)
// ---------------------------------------------------------------------------

static_assert(TZ::LastSunday(2026, 3) == 29, "");
static_assert(TZ::LastSunday(2026, 10) == 25, "");
static_assert(TZ::IsDst(DateTime(2026, 3, 29, 3, 0, 0)), "");
static_assert(!TZ::IsDst(DateTime(2026, 10, 25, 3, 0, 0)), "");
static_assert(TZ::ToLocal(DateTime(2026, 7, 1, 12)) == DateTime(2026, 7, 1, 13), "");

// ---------------------------------------------------------------------------
// LastSunday
// ---------------------------------------------------------------------------

TEST_CASE("LastSunday matches published Kyiv transition dates") {
    struct Row { uint16_t year; int march; int october; };
    const Row rows[] = {
        {2020, 29, 25},
        {2025, 30, 26},
        {2026, 29, 25},
        {2027, 28, 31},  // last Sunday is also the last day of October
        {2028, 26, 29},
        {2029, 25, 28},
    };

    for (const Row& r : rows) {
        CAPTURE(r.year);
        CHECK_EQ(LastSun(r.year, 3), r.march);
        CHECK_EQ(LastSun(r.year, 10), r.october);
    }
}

TEST_CASE("LastSunday is always a Sunday in the last seven days of the month") {
    for (uint16_t year = 2012; year <= 2040; ++year) {
        CAPTURE(year);
        for (int month : {3, 10}) {
            CAPTURE(month);
            const int day = LastSun(year, static_cast<uint8_t>(month));
            const DateTime d(year, static_cast<uint8_t>(month), static_cast<uint8_t>(day));
            CHECK_EQ(int(d.DayOfWeek()), 0);
            CHECK_GT(day, 24);
            CHECK_LE(day, 31);
        }
    }
}

// ---------------------------------------------------------------------------
// IsDst
// ---------------------------------------------------------------------------

TEST_CASE("IsDst is false outside the DST months") {
    CHECK_FALSE(TZ::IsDst(Std(2026, 1, 15, 12)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 2, 28, 23, 59, 59)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 11, 1, 0)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 12, 31, 23, 59, 59)));
}

TEST_CASE("IsDst is true in the middle of summer") {
    CHECK(TZ::IsDst(Std(2026, 4, 1, 0)));
    CHECK(TZ::IsDst(Std(2026, 7, 1, 12)));
    CHECK(TZ::IsDst(Std(2026, 9, 30, 23, 59, 59)));
}

TEST_CASE("IsDst spring transition 2026 (Sun Mar 29, 03:00 standard)") {
    CHECK_FALSE(TZ::IsDst(Std(2026, 3, 28, 23, 59, 59)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 3, 29, 0, 0, 0)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 3, 29, 2, 59, 59)));
    CHECK(TZ::IsDst(Std(2026, 3, 29, 3, 0, 0)));
    CHECK(TZ::IsDst(Std(2026, 3, 30, 0, 0, 0)));
}

TEST_CASE("IsDst autumn transition 2026 (Sun Oct 25, 03:00 standard)") {
    CHECK(TZ::IsDst(Std(2026, 10, 24, 23, 59, 59)));
    CHECK(TZ::IsDst(Std(2026, 10, 25, 0, 0, 0)));
    CHECK(TZ::IsDst(Std(2026, 10, 25, 2, 59, 59)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 10, 25, 3, 0, 0)));
    CHECK_FALSE(TZ::IsDst(Std(2026, 10, 26, 0, 0, 0)));
}

TEST_CASE("IsDst 2027, where the October transition is on the 31st") {
    CHECK_FALSE(TZ::IsDst(Std(2027, 3, 28, 2, 59, 59)));
    CHECK(TZ::IsDst(Std(2027, 3, 28, 3, 0, 0)));
    CHECK(TZ::IsDst(Std(2027, 10, 31, 2, 59, 59)));
    CHECK_FALSE(TZ::IsDst(Std(2027, 10, 31, 3, 0, 0)));
}

// ---------------------------------------------------------------------------
// ToLocal
// ---------------------------------------------------------------------------

TEST_CASE("ToLocal in winter and summer") {
    CHECK_EQ(TZ::ToLocal(Std(2026, 1, 1, 12)), Std(2026, 1, 1, 12));
    CHECK_EQ(TZ::ToLocal(Std(2026, 7, 1, 12)), Std(2026, 7, 1, 13));
}

TEST_CASE("ToLocal at the transitions") {
    CHECK_EQ(TZ::ToLocal(Std(2026, 3, 29, 2, 59, 59)), Std(2026, 3, 29, 2, 59, 59));
    CHECK_EQ(TZ::ToLocal(Std(2026, 3, 29, 3, 0, 0)), Std(2026, 3, 29, 4, 0, 0));
    CHECK_EQ(TZ::ToLocal(Std(2026, 10, 25, 2, 59, 59)), Std(2026, 10, 25, 3, 59, 59));
    CHECK_EQ(TZ::ToLocal(Std(2026, 10, 25, 3, 0, 0)), Std(2026, 10, 25, 3, 0, 0));
}

TEST_CASE("ToLocal rolls over midnight and year boundaries") {
    CHECK_EQ(TZ::ToLocal(Std(2026, 6, 30, 23, 30)), Std(2026, 7, 1, 0, 30));
    CHECK_EQ(TZ::ToLocal(Std(2026, 12, 31, 23, 59, 59)), Std(2026, 12, 31, 23, 59, 59));
}

// ---------------------------------------------------------------------------
// ToStandard
// ---------------------------------------------------------------------------

TEST_CASE("ToStandard in winter and summer") {
    CHECK_EQ(TZ::ToStandard(Std(2026, 1, 1, 12)), Std(2026, 1, 1, 12));
    CHECK_EQ(TZ::ToStandard(Std(2026, 7, 1, 12)), Std(2026, 7, 1, 11));
    CHECK_EQ(TZ::ToStandard(Std(2026, 7, 1, 0, 30)), Std(2026, 6, 30, 23, 30));
}

TEST_CASE("ToStandard around the spring transition") {
    CHECK_EQ(TZ::ToStandard(Std(2026, 3, 29, 2, 59, 59)), Std(2026, 3, 29, 2, 59, 59));
    CHECK_EQ(TZ::ToStandard(Std(2026, 3, 29, 4, 0, 0)), Std(2026, 3, 29, 3, 0, 0));
}

TEST_CASE("ToStandard for the nonexistent spring hour shifts back an hour") {
    CHECK_EQ(TZ::ToStandard(Std(2026, 3, 29, 3, 30, 0)), Std(2026, 3, 29, 2, 30, 0));
}

TEST_CASE("ToStandard around the autumn transition") {
    CHECK_EQ(TZ::ToStandard(Std(2026, 10, 25, 2, 59, 59)), Std(2026, 10, 25, 1, 59, 59));
    CHECK_EQ(TZ::ToStandard(Std(2026, 10, 25, 4, 0, 0)), Std(2026, 10, 25, 4, 0, 0));
}

TEST_CASE("ToStandard resolves the ambiguous autumn hour to EET") {
    CHECK_EQ(TZ::ToStandard(Std(2026, 10, 25, 3, 0, 0)), Std(2026, 10, 25, 3, 0, 0));
    CHECK_EQ(TZ::ToStandard(Std(2026, 10, 25, 3, 59, 59)), Std(2026, 10, 25, 3, 59, 59));
}

// ---------------------------------------------------------------------------
// Sweeps over 2012..2040
// ---------------------------------------------------------------------------

TEST_CASE("Hourly sweep: offsets, transition count and transition moments") {
    for (uint16_t year = 2012; year <= 2040; ++year) {
        CAPTURE(year);

        const int64_t start = DateTime(year, 1, 1).ToUnix();
        const int64_t end = DateTime(static_cast<uint16_t>(year + 1), 1, 1).ToUnix();

        bool prevDst = false;
        int changes = 0;
        int badOffsets = 0;
        int springDay = 0, springHour = -1, springMonth = 0;
        int autumnDay = 0, autumnHour = -1, autumnMonth = 0;

        for (int64_t t = start; t < end; t += 3600) {
            const DateTime std = DateTime::FromUnix(t);
            const bool dst = TZ::IsDst(std);

            if (TZ::ToLocal(std).ToUnix() - t != (dst ? 3600 : 0))
                ++badOffsets;

            if (dst != prevDst) {
                ++changes;
                if (dst) {
                    springMonth = std.month;
                    springDay = std.day;
                    springHour = std.hour;
                } else {
                    autumnMonth = std.month;
                    autumnDay = std.day;
                    autumnHour = std.hour;
                }
            }
            prevDst = dst;
        }

        CHECK_EQ(badOffsets, 0);
        CHECK_EQ(changes, 2);

        CHECK_EQ(springMonth, 3);
        CHECK_EQ(springDay, LastSun(year, 3));
        CHECK_EQ(springHour, 3);

        CHECK_EQ(autumnMonth, 10);
        CHECK_EQ(autumnDay, LastSun(year, 10));
        CHECK_EQ(autumnHour, 3);
    }
}

TEST_CASE("Round trip standard -> local -> standard, except the ambiguous hour") {
    for (uint16_t year = 2012; year <= 2040; ++year) {
        CAPTURE(year);

        const int64_t start = DateTime(year, 1, 1).ToUnix();
        const int64_t end = DateTime(static_cast<uint16_t>(year + 1), 1, 1).ToUnix();
        const int autumnDay = LastSun(year, 10);

        int mismatches = 0;

        for (int64_t t = start; t < end; t += 3600) {
            const DateTime std = DateTime::FromUnix(t);

            // 02:xx standard on the autumn transition day maps to the
            // ambiguous local 03:xx hour, which resolves to 03:xx standard.
            const bool ambiguous =
                std.month == 10 && std.day == autumnDay && std.hour == 2;
            if (ambiguous)
                continue;

            if (!(TZ::ToStandard(TZ::ToLocal(std)) == std))
                ++mismatches;
        }

        CHECK_EQ(mismatches, 0);
    }
}

TEST_CASE("The ambiguous autumn hour is the one documented exception") {
    const DateTime std = Std(2026, 10, 25, 2, 30, 0);
    const DateTime local = TZ::ToLocal(std);

    CHECK_EQ(local, Std(2026, 10, 25, 3, 30, 0));
    CHECK_EQ(TZ::ToStandard(local), Std(2026, 10, 25, 3, 30, 0));  // not 02:30
}

#endif //BUILD_TEST
