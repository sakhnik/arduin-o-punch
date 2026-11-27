#include "hal/gpio.hpp"
#include "defs.h"
#include "BuzzerSim.hpp"

#include <cstdio>
#include <unistd.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>

namespace HAL {

static BuzzerSim buzzer;

void GpioModeOutput(int pin)
{
    printf("GPIO %d: OUTPUT\n", pin);
}

void GpioWrite(int pin, bool value)
{
    if (pin == BUZZER_PIN) {
        if (value)
            buzzer.Stop();
        else
            buzzer.Start();
    }
    printf("GPIO %d: %s\n", pin, value ? "HIGH" : "LOW");
}

} //namespace HAL;
