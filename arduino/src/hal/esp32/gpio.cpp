#include "hal/gpio.hpp"
#include <Arduino.h>

namespace HAL {

void GpioModeOutput(int pin)
{
    pinMode(pin, OUTPUT);
}

void GpioWrite(int pin, bool value)
{
    digitalWrite(pin, value ? HIGH : LOW);
}

} //namespace HAL;
