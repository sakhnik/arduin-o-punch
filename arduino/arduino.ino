#include <arduino-timer.h>

#include "defs.h"
#include "Puncher.h"

auto timer = timer_create_default();
Timer<>::Task confirmation = {};
Puncher puncher;

void setup() {
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

#if ENABLE_SERIAL
    Serial.begin(115200);
    while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4).
#endif //ENABLE_SERIAL

    puncher.Setup();
}

void loop() {
    // TODO: better to call from a hardware timer ISR
    timer.tick();

    auto res = puncher.Punch();
    if (!res)
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
}
