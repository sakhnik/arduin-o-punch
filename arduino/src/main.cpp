
#include "app/Buzzer.hpp"
#include "app/Settings.hpp"
#include "app/Shell.hpp"
#include "app/Bluetooth.hpp"
#include "app/Network.hpp"
#include "app/OutMux.hpp"
#include "app/Operation.hpp"
#include "app/RtcLog.hpp"
#include "hal/serial.hpp"

Buzzer buzzer;
Settings settings{buzzer};
OutMux outMux;
Shell shell{outMux, settings, buzzer};

#ifdef ESP32
Bluetooth bluetooth{outMux, settings, shell};
Network network{outMux, settings, shell, buzzer};
Operation operation{buzzer, settings, outMux, bluetooth, network};
#endif //ESP32

void setup()
{
#ifdef ESP32
    RtcLog::Init();

#endif //ESP32
    HAL::Serial::Setup();

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

    if (settings.IsKeyDefault()) {
        buzzer.SignalDefaultKey();
        initialization_ok = false;
    }

    shell.Setup();

#ifdef ESP32
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
    // Check the serial for input and prohibit light sleep while the serial is active.
    while (HAL::Serial::Available()) {
        auto ch = HAL::Serial::Read();
        if (0 < ch && ch < 0xf0) {
            shell.PutChar(ch);
#ifdef ESP32
            operation.TransitionToActive();
#endif
        }
    }

#ifdef ESP32
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
