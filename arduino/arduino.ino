
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"


Buzzer buzzer;
Context context;
Puncher puncher{context};
#if ENABLE_SERIAL
Shell shell{context, buzzer};
#endif //ENABLE_SERIAL

void setup() {
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    buzzer.Setup();

#if ENABLE_SERIAL
    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);
#endif //ENABLE_SERIAL

    puncher.Setup();

#if ENABLE_SERIAL
    shell.Setup();
#endif //ENABLE_SERIAL

}


// Don't loop here to make sure serialEvent() is processed.
void loop()
{
    auto res = puncher.Punch();
    if (!res)
    {
        buzzer.ConfirmPunch();
    }

    buzzer.Tick();
    shell.Tick();
}

#if ENABLE_SERIAL
void serialEvent()
{
    shell.OnSerial();
}
#endif //ENABLE_SERIAL
