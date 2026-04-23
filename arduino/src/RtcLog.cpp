#include "RtcLog.h"

RTC_DATA_ATTR uint32_t RtcLog::magic = 0;
RTC_DATA_ATTR uint32_t RtcLog::bootCount = 0;
RTC_DATA_ATTR uint32_t RtcLog::lastReset = 0;
bool RtcLog::isRTCReliable = false;
