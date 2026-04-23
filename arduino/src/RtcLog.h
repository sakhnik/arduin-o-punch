#pragma once

#include <esp_system.h>

struct RtcLog
{
public:
    static void Init()
    {
        isRTCReliable = magic == 0xCAFEBABE;
        if (!isRTCReliable) {
            bootCount = 0;
            magic = 0xCAFEBABE;
        }

        ++bootCount;
        lastReset = esp_reset_reason();
    }

    static bool IsReliable() { return isRTCReliable; }

    static RTC_DATA_ATTR uint32_t magic;
    static RTC_DATA_ATTR uint32_t bootCount;
    static RTC_DATA_ATTR uint32_t lastReset;

private:
    static bool isRTCReliable;
};
