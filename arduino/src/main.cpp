
#include "Buzzer.h"
#include "Settings.h"
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
Network network{outMux, settings, shell, buzzer};
Operation operation{buzzer, settings, outMux, bluetooth, network};

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

    shell.Setup();
    shell.SetOperation(&operation);
    bluetooth.Setup();
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    network.Setup();
#endif

    operation.SetupLate();

    if (initialization_ok) {
        buzzer.SignalOk();
    }
}

void loop()
{
    // Check the serial for input and prohibit light sleep while the serial is active.
    while (Serial.available()) {
        auto ch = Serial.read();
        if (0 < ch && ch < 0xf0) {
            shell.PutChar(ch);
            operation.TransitionToActive();
        }
    }

    operation.Loop();
}
