#include <arduino-timer.h>

#include "defs.h"
#include "Puncher.h"

auto timer = timer_create_default();
Puncher puncher;

void setup() {

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
    //if (!res)
    //{
    //    // Confirm with the led (the builtin isn't available together with SPI).
    //    digitalWrite(LED_BUILTIN, 1);
    //    timer.in(400, [](void*) { digitalWrite(LED_BUILTIN, 0); return false; });
    //}
}
