#include "hal/time.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace HAL {

void Delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

} //namespace HAL;
