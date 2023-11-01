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

uint32_t Context::GetClock() const
{
    // TODO: subsecond resolution
    DateTime now = rtc.now();
    uint32_t clock = now.hour();
    clock = clock * 60 + now.minute();
    clock = clock * 60 + now.second();
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

uint8_t Context::GetId() const
{
    return _id;
}

void Context::SetId(uint8_t id)
{
    _id = id;
    EEPROM.put(ADDROF(_id), _id);
}
