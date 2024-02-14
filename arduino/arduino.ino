
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

// Start assuming that no sleeping is required
uint8_t _timeout_hr = -1;
unsigned long _timeout_ms = -1ul;
unsigned long _last_punch_time = 0;

namespace {
    // Check the actual timeout from the settings context. If a change is noticed,
    // recalculate timeout_ms.
    unsigned long GetTimeoutMs()
    {
        // Caching to avoid constant multiplication
        auto timeout_hr = context.GetTimeoutHours();
        if (timeout_hr == _timeout_hr)
            return _timeout_ms;
        _timeout_hr = timeout_hr;
        return _timeout_ms = 60ul * 60ul * 1000ul * timeout_hr;
    }
} //namespace;

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

    bool initialization_ok = true;

    if (context.IsKeyDefault())
    {
        buzzer.SignalDefaultKey();
        initialization_ok = false;
    }

    puncher.Setup();
    shell.Setup();
    _last_punch_time = millis();

    if (initialization_ok)
    {
        buzzer.SignalOk();
    }
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

    if (res == ErrorCode::CARD_IS_FULL)
    {
        _last_punch_time = now;
        buzzer.SignalCardFull();
    }

    buzzer.Tick();
    shell.Tick();

    // Sleep to economize power after configured hours of no punches
    if (now - _last_punch_time > GetTimeoutMs())
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
