#include "hal/gpio.h"
#include <Arduino.h>

namespace Hal {

void GpioModeOutput(int pin)
{
    pinMode(pin, OUTPUT);
}

void GpioWrite(int pin, bool value)
{
    digitalWrite(pin, value ? HIGH : LOW);
}

} //namespace Hal;
