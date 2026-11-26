#pragma once

#include "DateTime.h"

namespace AOP {

// Europe/Kyiv time rules (valid for 2012 onward):
//   Standard time: EET  (UTC+2)
//   Daylight time: EEST (UTC+3)
//   DST starts on the last Sunday of March at 03:00 EET (-> 04:00 EEST)
//   DST ends on the last Sunday of October at 04:00 EEST (-> 03:00 EET)
//
// The RTC (DS3231) is assumed to hold standard time (UTC+2).
class TZ
{
public:
    // Is DST active at the given standard-time clock reading?
    static constexpr bool IsDst(const DateTime &dt)
    {
        if (dt.month < 3 || dt.month > 10)
            return false;

        if (dt.month > 3 && dt.month < 10)
            return true;

        const uint8_t transition = LastSunday(dt.year, dt.month);

        // March: DST begins at 03:00 standard time.
        if (dt.month == 3) {
            if (dt.day != transition)
                return dt.day > transition;

            return dt.hour >= 3;
        }

        // October: DST ends at 04:00 EEST, which is 03:00 standard time.
        if (dt.day != transition)
            return dt.day < transition;

        return dt.hour < 3;
    }

    // Convert RTC standard time (UTC+2) to daylight-adjusted local time.
    static constexpr DateTime ToLocal(const DateTime &standardTime)
    {
        return IsDst(standardTime) ? AddSeconds(standardTime, 3600) : standardTime;
    }

    // Convert daylight-adjusted local time to RTC standard time.
    //
    // Two local hours are special on transition days:
    //  - Spring, 03:00-03:59 local does not exist. It is treated as DST
    //    and shifted back an hour (-> 02:xx standard).
    //  - Autumn, 03:00-03:59 local occurs twice. It is treated as the
    //    second occurrence (EET), so it maps to 03:xx standard.
    static constexpr DateTime ToStandard(const DateTime &localTime)
    {
        return IsDst(localTime) ? AddSeconds(localTime, -3600) : localTime;
    }

    // Day of the month of the last Sunday in the given month.
    static constexpr uint8_t LastSunday(uint16_t year, uint8_t month)
    {
        const uint8_t last = DateTime::DaysInMonth(year, month);
        return static_cast<uint8_t>(last - DateTime(year, month, last).DayOfWeek());
    }

private:
    static constexpr DateTime AddSeconds(const DateTime &dt, int64_t seconds)
    {
        return DateTime::FromUnix(dt.ToUnix() + seconds);
    }
};

} //namespace AOP;
