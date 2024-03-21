
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
Shell shell{context, buzzer};

void setup()
{
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

    buzzer.Setup();

    if (context.Setup()) {
        while (true)
            buzzer.Tick();
    }

    bool initialization_ok = true;

    if (context.IsKeyDefault()) {
        buzzer.SignalDefaultKey();
        initialization_ok = false;
    }

    puncher.Setup();
    shell.Setup();

    if (initialization_ok) {
        buzzer.SignalOk();
    }
}

// Don't loop here to make sure serialEvent() is processed.
void loop()
{
    auto now = millis();

    auto res = puncher.Punch();
    if (!res) {
        buzzer.ConfirmPunch();
    }

    if (res == ErrorCode::CARD_IS_FULL) {
        buzzer.SignalCardFull();
    }

    buzzer.Tick();
    shell.Tick();
#ifdef ESP32
    shell.OnSerial();
#endif
}

#ifdef ARDUINO_AVR_PRO
void serialEvent()
{
    shell.OnSerial();
}
#endif //ARDUINO_AVR_PRO
