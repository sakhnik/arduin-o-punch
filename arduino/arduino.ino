#include <arduino-timer.h>

#include "defs.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"

auto timer = timer_create_default();
Timer<>::Task confirmation = {};

Context context;
Puncher puncher{context};
#if ENABLE_SERIAL
Shell shell;
#endif //ENABLE_SERIAL

void setup() {
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

#if ENABLE_SERIAL
    Serial.begin(115200);
#endif //ENABLE_SERIAL

    puncher.Setup();

#if ENABLE_SERIAL
    shell.Setup();
#endif //ENABLE_SERIAL

    // Blink continuously to indicate initialization prompt
    confirmation = timer.every(250, [](void*) {
        digitalWrite(LED_CONFIRM_PIN, !digitalRead(LED_CONFIRM_PIN));
        return true;
    });
}

void ConfirmPunch()
{
    // Confirm with the led (the builtin isn't available together with SPI).
    if (confirmation)
        timer.cancel(confirmation);
    digitalWrite(LED_CONFIRM_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    confirmation = timer.in(400,
        [](void*) {
            digitalWrite(LED_CONFIRM_PIN, LOW);
            digitalWrite(BUZZER_PIN, LOW);
            return false;
        }
    );
}

void loop()
{
    // Don't loop here to make sure serialEvent() is processed.

    // TODO: better to call from a hardware timer ISR
    timer.tick();

    auto res = puncher.Punch();
    if (!res)
    {
        ConfirmPunch();
    }
}

#if ENABLE_SERIAL
void serialEvent()
{
    shell.OnSerial();
}
#endif //ENABLE_SERIAL
