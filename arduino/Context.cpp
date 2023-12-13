#include "Context.h"
#include "Buzzer.h"
#include <EEPROM.h>
#include <RTClib.h>

#ifdef BUILD_TEST
# include <cstddef>
#endif

RTC_DS3231 rtc;
//RTC_Millis rtc;

#define ADDROF(field) (ADDRESS + offsetof(Context, field))

Context::Context(Buzzer &buzzer)
    : _buzzer{buzzer}
{
}

int8_t Context::Setup()
{
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        _buzzer.SignalRTCFail();
        return -1;
    }

    EEPROM.get(ADDROF(_id), _id);
    EEPROM.get(ADDROF(_key), _key);
    EEPROM.get(ADDROF(_timeout_hr), _timeout_hr);

    if (IsKeyDefault())
    {
        _buzzer.SignalDefaultKey();
    }
    return 0;
}

bool Context::IsKeyDefault() const
{
    for (int i = 0; i < KEY_SIZE; ++i)
    {
        if (_key[i] != 0xff)
            return false;
    }
    return true;
}

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
    EEPROM.put(ADDROF(_key), _key);
}

DateTime Context::GetDateTime() const
{
    return rtc.now();
}

uint32_t Context::GetClock(const DateTime *date_time) const
{
    if (!date_time)
    {
        auto now = rtc.now();
        return GetClock(&now);
    }
    // TODO: subsecond resolution
    uint32_t clock = date_time->hour();
    clock = clock * 60 + date_time->minute();
    clock = clock * 60 + date_time->second();
    return clock * 1000;
}

void Context::SetClock(uint32_t clock)
{
    auto ms = clock % 1000;
    clock /= 1000;
    auto sec = clock % 60;
    clock /= 60;
    auto min = clock % 60;
    clock /= 60;
    // Trick or treat!
    rtc.adjust(DateTime(2023, 10, 31, clock, min, sec));
}

void Context::SetId(uint8_t id)
{
    _id = id;
    EEPROM.put(ADDROF(_id), _id);
}

void Context::SetTimeoutHours(uint8_t hours)
{
    _timeout_hr = hours;
    EEPROM.put(ADDROF(_timeout_hr), _timeout_hr);
}
