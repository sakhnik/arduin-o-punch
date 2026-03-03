
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"
#include "Bluetooth.h"
#include "Network.h"
#include "OutMux.h"
#include <esp_sleep.h>

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
OutMux outMux;
Shell shell{outMux, context, buzzer};

unsigned long long activityTimeout = 60ul * 5 * 1000;
unsigned long long prevCardTime = 0;

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

static void EnterSleep()
{
    //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 1000000);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << WAKEUP_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);
    Serial.println("Going to deep sleep");

    Wire.end();  // stop I2C

    // Configure high impedance state for the I²C and SPI
    static constexpr const int HIGH_Z_PINS[] = {SDA, SCL, MISO, MOSI, SCK};
    for (auto pin : HIGH_Z_PINS) {
        gpio_reset_pin(static_cast<gpio_num_t>(pin));   // HIGH-Z
        gpio_hold_en(static_cast<gpio_num_t>(pin));
    }

    // Hold some pins in the high state during sleep to keep RFC522 and DS3231 switched off and to avoid parasitic current
    static constexpr const int HIGH_PINS[] = {MOSFET_PIN, LED_CONFIRM_PIN, BUZZER_PIN};
    for (int pin : HIGH_PINS) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        gpio_hold_en(static_cast<gpio_num_t>(pin));
    }

    // Enable the pins hold
    gpio_deep_sleep_hold_en();

    delay(100);
    esp_deep_sleep_start();
    // Should never reach!
    prevCardTime = millis();
}

void setup()
{
    prevCardTime = millis();

    pinMode(LED_CONFIRM_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(MOSFET_PIN, OUTPUT);
    pinMode(WAKEUP_PIN, INPUT_PULLUP);

    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

    // Set the CPU frequency to 10 MHz for power optimization
    setCpuFrequencyMhz(10);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));

    buzzer.Setup();

    if (context.Setup()) {
        while (true) {
            vTaskDelay(1);
        }
    }

    bool initialization_ok = true;

    if (context.IsKeyDefault()) {
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

// Don't loop here to make sure serialEvent() is processed.
void loop()
{
    digitalWrite(MOSFET_PIN, LOW);

    auto res = puncher.Punch();
    if (!res) {
        buzzer.ConfirmPunch();
        prevCardTime = millis();
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

    if (res == ErrorCode::SERVICE_CARD) {
        AdvanceOperationMode();
    }

    bluetooth.Tick();
    network.Tick();

    if (millis() - prevCardTime >= activityTimeout) {
        EnterSleep();
    }
}
