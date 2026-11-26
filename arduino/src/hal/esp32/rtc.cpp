#include "hal/rtc.h"
#include "defs.h"
#include <RTClib.h>

namespace HAL {

RTC_DS3231 rtc;

bool RTC::Setup()
{
    Wire.setPins(I2C_SDA, I2C_SCL);
    return rtc.begin();
}

AOP::DateTime RTC::Now()
{
    auto dt = rtc.now();
    return AOP::DateTime{dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second()};
}

void RTC::Adjust(const AOP::DateTime &dt)
{
    rtc.adjust({dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second});
}

} //namespace HAL;
