#include "hal/time.h"
#include <unistd.h>

namespace HAL {

void Delay(uint32_t ms)
{
    usleep(ms * 1000);
}

} //namespace HAL;
