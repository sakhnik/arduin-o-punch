#include "hal/rtc.hpp"

#include <chrono>
#include <mutex>

namespace HAL {

using namespace std::chrono;

static std::mutex rtc_mutex;
static int64_t rtc_timestamp = 0;
static steady_clock::time_point rtc_reference;

bool RTC::Setup()
{
    std::lock_guard lock(rtc_mutex);
    rtc_timestamp = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    rtc_reference = steady_clock::now();
    return true;
}

AOP::DateTime RTC::Now()
{
    std::lock_guard lock(rtc_mutex);
    auto elapsed = duration_cast<seconds>(steady_clock::now() - rtc_reference).count();
    return AOP::DateTime::FromUnix(rtc_timestamp + elapsed);
}

void RTC::Adjust(const AOP::DateTime &dt)
{
    std::lock_guard lock(rtc_mutex);
    rtc_timestamp = dt.ToUnix();
    rtc_reference = steady_clock::now();
}

} // namespace HAL
