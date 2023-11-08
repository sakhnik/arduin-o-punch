
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
Shell shell{context, buzzer};

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

void serialEvent()
{
    shell.OnSerial();
}
