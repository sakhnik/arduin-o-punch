
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"
#ifdef ESP32
#  include "Bluetooth.h"
#endif //ESP32
#include "OutMux.h"

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
OutMux outMux;
Shell shell{outMux, context, buzzer};
#ifdef ESP32
Bluetooth bluetooth {outMux, context, shell};
#endif //ESP32

void setup()
{
    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

#ifdef ESP32
    // Set the CPU frequency to 40 MHz for power optimization
    setCpuFrequencyMhz(40);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));
#endif //ESP32

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
#ifdef ESP32
    bluetooth.Setup();
#endif

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

#ifdef ESP32
    if (res == ErrorCode::SERVICE_CARD) {
        buzzer.SignalService();
        bluetooth.Toggle();
    }
#endif //ESP32

    buzzer.Tick();
    shell.Tick();
#ifdef ESP32
    shell.OnSerial();
    bluetooth.Tick();
#endif
}

#ifdef ARDUINO_AVR_PRO
void serialEvent()
{
    shell.OnSerial();
}
#endif //ARDUINO_AVR_PRO
