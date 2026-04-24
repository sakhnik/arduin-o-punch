
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

    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

    buzzer.Setup();
    operation.Setup();

    if (settings.Setup()) {
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

    Serial.onEvent(
        ARDUINO_HW_CDC_RX_EVENT,
        [](void *arg, esp_event_base_t base, int32_t id, void *data) {
            while (Serial.available()) {
                shell.PutChar(Serial.read());
            }
        }
    );

    operation.SetupLate();

    if (initialization_ok) {
        buzzer.SignalOk();
    }
}

void loop()
{
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
