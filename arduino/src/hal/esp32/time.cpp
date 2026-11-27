#include "hal/time.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>

namespace HAL {

void Delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

uint32_t Millis()
{
    return millis();
}

} //namespace HAL;
