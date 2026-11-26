#include "hal/time.h"
#include <unistd.h>

namespace Hal {

void Delay(uint32_t ms)
{
    usleep(ms * 1000);
}

} //namespace Hal;
