
#include "Buzzer.h"
#include "Settings.h"
#include "Puncher.h"
#include "Shell.h"
#include "Bluetooth.h"
#include "Network.h"
#include "OutMux.h"
#include "Operation.h"
#include <esp_sleep.h>
#include <driver/gpio.h>

Buzzer buzzer;
Settings settings{buzzer};
Puncher puncher{settings};
OutMux outMux;
Shell shell{outMux, settings, buzzer};

Bluetooth bluetooth{outMux, settings, shell};
Network network {outMux, settings, shell, buzzer};
Operation operation{buzzer, settings, bluetooth, network};

void setup()
{
    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

    operation.Setup();
    buzzer.Setup();

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
        }

        if (res == ErrorCode::SERVICE_CARD) {
            operation.TransitionToNext();
        }
    }

    if (operation.CheckSnooze()) {
        puncher.Setup();
    }
}
