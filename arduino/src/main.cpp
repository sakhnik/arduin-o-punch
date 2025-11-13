
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"
#ifdef ESP32
#  include "Bluetooth.h"
#  include "Network.h"
#endif //ESP32
#include "OutMux.h"

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
OutMux outMux;
Shell shell{outMux, context, buzzer};

#ifdef ESP32

enum OperationMode
{
    OM_NORMAL = 0,
    OM_BLUETOOTH,
    OM_WIFI,

    OM_MODE_COUNT
};
OperationMode operation_mode = OM_NORMAL;

Bluetooth bluetooth {outMux, context, shell};
Network network {outMux, context, shell, buzzer};

void AdvanceOperationMode()
{
    switch (operation_mode) {
    case OM_BLUETOOTH:
        bluetooth.SwitchOff();
        break;
    case OM_WIFI:
        network.SwitchOff();
        break;
    default:
        break;
    }
    operation_mode = static_cast<OperationMode>((operation_mode + 1) % OM_MODE_COUNT);
    switch (operation_mode) {
    case OM_NORMAL:
        buzzer.SignalOk();
        break;
    case OM_BLUETOOTH:
        buzzer.SignalBluetooth();
        bluetooth.SwitchOn();
        break;
    case OM_WIFI:
        // It takes a second or so to initialize WiFi. The CPU will be busy.
        // Play a confirmation melody only after that to avoid distortion.
        network.SwitchOn();
        buzzer.SignalWifi();
        break;
    default:
        // Shouldn't be reached
        break;
    }
}

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
    network.Setup();
#endif

    if (initialization_ok) {
        buzzer.SignalOk();
    }
}

// Don't loop here to make sure serialEvent() is processed.
void loop()
{
    auto res = puncher.Punch();
    if (!res) {
        buzzer.ConfirmPunch();
    } /*else {
        if (res == ErrorCode::NO_CARD) {
            Serial.print('.');
        } else {
            Serial.println((int)res);
        }
        Serial.flush();
    }*/

    if (res == ErrorCode::CARD_IS_FULL) {
        buzzer.SignalCardFull();
    }

#ifdef ESP32
    if (res == ErrorCode::SERVICE_CARD) {
        AdvanceOperationMode();
    }
#endif //ESP32

    buzzer.Tick();
    shell.Tick();
#ifdef ESP32
    shell.OnSerial();
    bluetooth.Tick();
    network.Tick();
#endif
}

#ifdef ARDUINO_AVR_PRO
void serialEvent()
{
    shell.OnSerial();
}
#endif //ARDUINO_AVR_PRO
