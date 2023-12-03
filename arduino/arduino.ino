
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"
#include <LowPower.h>

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
Shell shell{context, buzzer};
unsigned long _last_punch_time = 0;

void setup() {
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

    buzzer.Setup();

    if (context.Setup())
    {
        while (true)
            buzzer.Tick();
    }

    puncher.Setup();
    shell.Setup();
    _last_punch_time = millis();
}

// Don't loop here to make sure serialEvent() is processed.
void loop()
{
    auto now = millis();

    auto res = puncher.Punch();
    if (!res)
    {
        _last_punch_time = now;
        buzzer.ConfirmPunch();
    }

    buzzer.Tick();
    shell.Tick();

    // Sleep to economize power after 1 hour of no punches
    if (unsigned(now - _last_punch_time) > 60 * 60 * 1000)
    {
        puncher.AntennaOff();
        LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
        puncher.AntennaOn();
    }
}

void serialEvent()
{
    shell.OnSerial();
    _last_punch_time = millis();
}
