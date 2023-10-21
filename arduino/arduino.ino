#include <arduino-timer.h>

#include "defs.h"
#include "Puncher.h"
#include "Shell.h"

auto timer = timer_create_default();
Timer<>::Task confirmation = {};

const uint8_t DEFAULT_KEY[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t key[6] = {};
bool key_initialized = false;

Puncher puncher{key};

void setup() {
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

#if ENABLE_SERIAL
    static Shell shell(115200);
#endif //ENABLE_SERIAL

    puncher.Setup();

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

    auto res = puncher.Punch(key_initialized ? key : DEFAULT_KEY);
    if (!res)
    {
        ConfirmPunch();
        key_initialized = true;
    }
}
