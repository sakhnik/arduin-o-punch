
#include "Buzzer.h"
#include "Settings.h"
#include "Puncher.h"
#include "Shell.h"
#include "Bluetooth.h"
#include "Network.h"
#include "OutMux.h"
#include "Operation.h"
#include "RtcLog.h"

Buzzer buzzer;
Settings settings{buzzer};
OutMux outMux;
Shell shell{outMux, settings, buzzer};

Bluetooth bluetooth{outMux, settings, shell};
Network network {outMux, settings, shell, buzzer};
Operation operation{buzzer, settings, bluetooth, network};
Puncher puncher{settings, operation};

void setup()
{
    RtcLog::Init();

    Serial.begin(115200);

    buzzer.Setup();

    if (operation.Setup() || settings.Setup()) {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            Serial.println(F("Failed to initialize"));
        }
    }

    bool initialization_ok = true;

    if (settings.IsKeyDefault()) {
        buzzer.SignalDefaultKey();
        initialization_ok = false;
    }

    puncher.Setup();
    shell.Setup();
    shell.SetOperation(&operation);
    bluetooth.Setup();
    network.Setup();

    operation.SetupLate();

    if (initialization_ok) {
        buzzer.SignalOk();
    }
}

void loop()
{
    // Check the serial for input and prohibit light sleep while the serial is active.
    auto nowMs = millis();
    while (Serial.available()) {
        auto ch = Serial.read();
        if (0 < ch && ch < 0xf0) {
            shell.PutChar(ch);
            operation.TransitionToActive();
        }
    }

    for (int i = 0; i < 2; ++i) {
        auto res = puncher.Punch();
        if (!res) {
            operation.TransitionToActive();
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
            operation.TransitionToActive();
            buzzer.SignalCardFull();
        } else if (res == ErrorCode::SERVICE_CARD) {
            operation.TransitionToNext();
        } else if (res == ErrorCode::DEBUG_CARD) {
            buzzer.ConfirmDebug();
        }
    }

    if (operation.CheckSnooze()) {
        puncher.Setup();
    }
}
