
#include "app/Buzzer.h"
#include "app/Settings.h"
#include "app/Shell.h"
#include "app/Bluetooth.h"
#include "app/Network.h"
#include "app/OutMux.h"
#include "app/Operation.h"
#include "app/RtcLog.h"

Buzzer buzzer;
Settings settings{buzzer};

#ifdef ESP32
OutMux outMux;
Shell shell{outMux, settings, buzzer};

Bluetooth bluetooth{outMux, settings, shell};
Network network{outMux, settings, shell, buzzer};
Operation operation{buzzer, settings, outMux, bluetooth, network};
#endif //ESP32

void setup()
{
#ifdef ESP32
    RtcLog::Init();

    Serial.begin(115200);
#endif //ESP32

    buzzer.Setup();
    settings.Setup();

#ifdef ESP32
    if (operation.Setup()) {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            Serial.println(F("Failed to initialize"));
        }
    }

    settings.SetupLate();
#endif

    bool initialization_ok = true;
#ifdef ESP32

    if (settings.IsKeyDefault()) {
        buzzer.SignalDefaultKey();
        initialization_ok = false;
    }

    shell.Setup();
    shell.SetOperation(&operation);
    bluetooth.Setup();
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    network.Setup();
#endif

    operation.SetupLate();
#endif //ESP32

    if (initialization_ok) {
#ifdef ESP32
        if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
            buzzer.SignalE();
        } else {
#endif
            buzzer.SignalOk();
#ifdef ESP32
        }
#endif
    }
}

void loop()
{
#ifdef ESP32
    // Check the serial for input and prohibit light sleep while the serial is active.
    while (Serial.available()) {
        auto ch = Serial.read();
        if (0 < ch && ch < 0xf0) {
            shell.PutChar(ch);
            operation.TransitionToActive();
        }
    }

    operation.Loop();
#endif //ESP32
}

#ifdef ESP32
#else

#include <unistd.h>

int main(int argc, char *argv[])
{
    setup();
    while (true) {
        loop();
        sleep(0);
    }
    return 0;
}

#endif //ESP32
