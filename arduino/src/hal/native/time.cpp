#include "hal/time.h"
#include <unistd.h>
#include <chrono>

namespace HAL {

void Delay(uint32_t ms)
{
    usleep(ms * 1000);
}

uint32_t Millis()
{
    using namespace std::chrono;

    static const auto start = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - start).count();
}

} //namespace HAL;
