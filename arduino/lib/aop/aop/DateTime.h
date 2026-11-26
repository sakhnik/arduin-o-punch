#pragma once

#include <cstdint>

namespace AOP {

struct DateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    constexpr explicit DateTime(uint16_t y = 2000, uint8_t mo = 1, uint8_t d = 1, uint8_t h = 0, uint8_t mi = 0, uint8_t s = 0)
        : year(y), month(mo), day(d)
        , hour(h), minute(mi), second(s)
    {
    }

    static constexpr bool IsLeapYear(uint16_t y)
    {
        return y % 4 == 0 &&
            (y % 100 != 0 || y % 400 == 0);
    }

    static constexpr uint8_t DaysInMonth(uint16_t y, uint8_t m)
    {
        constexpr uint8_t days[] = {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        if (m < 1 || m > 12)
            return 0;

        return m == 2 && IsLeapYear(y) ? 29 : days[m - 1];
    }

    constexpr bool IsValid() const
    {
        return month >= 1 && month <= 12 &&
            day >= 1 && day <= DaysInMonth(year, month) &&
            hour < 24 && minute < 60 && second < 60;
    }

    // Days since 1970-01-01.
    static constexpr int64_t DaysFromCivil(int y, unsigned m, unsigned d)
    {
        y -= m <= 2;

        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

        return era * 146097 + static_cast<int64_t>(doe) - 719468;
    }

    // 0 = Sunday, 1 = Monday, ..., 6 = Saturday.
    constexpr uint8_t DayOfWeek() const
    {
        // 1970-01-01 was a Thursday (4).
        // days % 7 can be negative before 1970, so shift into
        // the positive range first (+ 11 is + 4 mod 7).
        return static_cast<uint8_t>((DaysFromCivil(year, month, day) % 7 + 11) % 7);
    }

    // Convert to Unix timestamp (UTC).
    constexpr int64_t ToUnix() const
    {
        return DaysFromCivil(year, month, day) * 86400
            + hour * 3600
            + minute * 60
            + second;
    }

    // Convert Unix timestamp (UTC) to DateTime.
    // Years outside 0..65535 are truncated by the uint16_t field.
    static constexpr DateTime FromUnix(int64_t timestamp)
    {
        int64_t days = timestamp / 86400;
        int64_t seconds = timestamp % 86400;

        // Handle timestamps before 1970.
        if (seconds < 0) {
            --days;
            seconds += 86400;
        }

        const int64_t z = days + 719468;
        const int64_t era = (z >= 0 ? z : z - 146096) / 146097;

        const unsigned doe = static_cast<unsigned>(z - era * 146097);
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;

        int y = static_cast<int>(yoe + era * 400);

        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);

        const unsigned mp = (5 * doy + 2) / 153;
        const unsigned d = doy - (153 * mp + 2) / 5 + 1;

        const unsigned m = mp < 10 ? mp + 3 : mp - 9;

        y += m <= 2;

        return DateTime(
            static_cast<uint16_t>(y),
            static_cast<uint8_t>(m),
            static_cast<uint8_t>(d),
            static_cast<uint8_t>(seconds / 3600),
            static_cast<uint8_t>((seconds / 60) % 60),
            static_cast<uint8_t>(seconds % 60)
        );
    }

    // Note: comparisons go through ToUnix(), so they are only
    // meaningful for valid values (see IsValid()).
    constexpr bool operator==(const DateTime& other) const
    {
        return ToUnix() == other.ToUnix();
    }

    constexpr bool operator!=(const DateTime& other) const
    {
        return !(*this == other);
    }

    constexpr bool operator<(const DateTime& other) const
    {
        return ToUnix() < other.ToUnix();
    }

    constexpr bool operator>(const DateTime& other) const
    {
        return other < *this;
    }

    constexpr bool operator<=(const DateTime& other) const
    {
        return !(other < *this);
    }

    constexpr bool operator>=(const DateTime& other) const
    {
        return !(*this < other);
    }
};

static_assert(DateTime(1970, 1, 1).DayOfWeek() == 4);   // Thu
static_assert(DateTime(2000, 1, 1).DayOfWeek() == 6);   // Sat
static_assert(DateTime(2000, 3, 1).DayOfWeek() == 3);   // Wed
static_assert(DateTime(1969, 12, 31).DayOfWeek() == 3); // Wed (pre-1970)
static_assert(DateTime(2024, 2, 29).DayOfWeek() == 4);  // Thu (leap day)

static_assert(DateTime(1970, 1, 1).ToUnix() == 0);
static_assert(DateTime::FromUnix(-1) == DateTime(1969, 12, 31, 23, 59, 59));
static_assert(DateTime::FromUnix(951782400) == DateTime(2000, 2, 29));
static_assert(DateTime::FromUnix(0) == DateTime(1970, 1, 1));
static_assert(DateTime(2024, 2, 30).IsValid() == false);

} //namespace AOP;
