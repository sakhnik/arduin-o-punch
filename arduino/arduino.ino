#include <arduino-timer.h>

#include "defs.h"
#include "Puncher.h"

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
    Serial.begin(115200);
    while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4).
#endif //ENABLE_SERIAL

    puncher.Setup();
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

void loop() {
    // Blink continuously to indicate initialization prompt
    confirmation = timer.every(250, [](void*) {
        digitalWrite(LED_CONFIRM_PIN, !digitalRead(LED_CONFIRM_PIN));
        return true;
    });

    // Wait for a service card to initialize the key
    while (true)
    {
        // TODO: better to call from a hardware timer ISR
        timer.tick();

        auto res = puncher.Punch(DEFAULT_KEY);
        if (!res)
        {
            ConfirmPunch();
            key_initialized = true;
            break;
        }
    }

    while (true)
    {
        timer.tick();

        auto res = puncher.Punch(key);
        if (!res)
        {
            ConfirmPunch();
        }
    }
}
