
#include "defs.h"
#include "Buzzer.h"
#include "Context.h"
#include "Puncher.h"
#include "Shell.h"
#include "Bluetooth.h"
#include "Network.h"
#include "OutMux.h"
#include "CpuFreq.h"
#include "ActivityCounter.h"
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <SPI.h>

constexpr const int TIME_TO_SLEEP_MS = 500;

Buzzer buzzer;
Context context{buzzer};
Puncher puncher{context};
OutMux outMux;
Shell shell{outMux, context, buzzer};

unsigned long long prevCardTimeMs = 0;

enum OperationMode
{
    OM_ACTIVE = 0,
    OM_ECO,
    OM_BLUETOOTH,
    OM_WIFI,

    OM_MODE_COUNT
};
OperationMode operation_mode = OM_ECO;

Bluetooth bluetooth {outMux, context, shell};
Network network {outMux, context, shell, buzzer};

OperationMode GetNextOpMode(OperationMode mode)
{
    switch (operation_mode) {
    case OM_ACTIVE:
    case OM_ECO:
        return OM_BLUETOOTH;
    case OM_BLUETOOTH:
        return OM_WIFI;
    case OM_WIFI:
        return OM_ECO;
    default:
        break;
    }
    return OM_ECO;
}

void TransitionOperationMode(OperationMode next_mode)
{
    switch (operation_mode) {
    case OM_ACTIVE:
    case OM_ECO:
        break;
    case OM_BLUETOOTH:
        bluetooth.SwitchOff();
        break;
    case OM_WIFI:
        network.SwitchOff();
        break;
    default:
        break;
    }

    operation_mode = next_mode;

    switch (operation_mode) {
    case OM_ACTIVE:
    case OM_ECO:
        SetCpuFreq(10);
        buzzer.SignalOk();
        break;
    case OM_BLUETOOTH:
        SetCpuFreq(80);
        buzzer.SignalBluetooth();
        bluetooth.SwitchOn();
        break;
    case OM_WIFI:
        SetCpuFreq(80);
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

// Hold some pins in the high state during sleep to keep RFC522 and DS3231 switched off and to avoid parasitic current
static constexpr const int HIGH_PINS[] = {MOSFET_PIN, LED_CONFIRM_PIN, BUZZER_PIN};

static void EnterSleep()
{
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << WAKEUP_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);

    // stop buses
    SPI.end();

    // Configure high impedance state for the I²C and SPI
    auto high_z = [](int pin) {
        gpio_set_direction((gpio_num_t)pin, GPIO_MODE_DISABLE);
        gpio_set_pull_mode((gpio_num_t)pin, GPIO_FLOATING);
    };

    high_z(MOSI);
    high_z(MISO);
    high_z(SCK);
    high_z(SS);

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
    prevCardTimeMs = millis();
}

void setup()
{
    gpio_deep_sleep_hold_dis();
    for (int pin : HIGH_PINS) {
        gpio_hold_dis(static_cast<gpio_num_t>(pin));
    }

    pinMode(MOSFET_PIN, OUTPUT);
    pinMode(WAKEUP_PIN, INPUT_PULLUP);

    // 9600 allows for reliable communication with automated scripts like sync-clock.py
    Serial.begin(9600);

    buzzer.Setup();

    // Set the CPU frequency to 10 MHz for power optimization
    setCpuFrequencyMhz(10);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));

    if (context.Setup()) {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            Serial.println(F("Failed to initialize"));
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

    prevCardTimeMs = millis();

    if (initialization_ok) {
        buzzer.SignalOk();
    }
}

void loop()
{
    digitalWrite(MOSFET_PIN, LOW);

    for (int i = 0; i < 2; ++i) {
        auto res = puncher.Punch();
        if (!res) {
            if (operation_mode == OM_ECO) {
                operation_mode = OM_ACTIVE;
            }
            buzzer.ConfirmPunch();
            prevCardTimeMs = millis();
        } /*else {
            if (res == ErrorCode::NO_CARD) {
                Serial.print('.');
            } else {
                Serial.println((int)res);
            }
            Serial.flush();
        }*/

        if (res == ErrorCode::CARD_IS_FULL) {
            prevCardTimeMs = millis();
            buzzer.SignalCardFull();
        }

        if (res == ErrorCode::SERVICE_CARD) {
            prevCardTimeMs = millis();
            TransitionOperationMode(GetNextOpMode(operation_mode));
        }
    }

    if (operation_mode == OM_ECO && !ActivityCounter::Instance().IsBusy()) {
        digitalWrite(MOSFET_PIN, HIGH);
        //Serial.flush();
        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_MS * 1000ull);
        esp_light_sleep_start();
        digitalWrite(MOSFET_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(50));
        puncher.Setup();
    }

    if (operation_mode == OM_ACTIVE && millis() - prevCardTimeMs >= context.GetTActMs()) {
        operation_mode = OM_ECO;
        prevCardTimeMs = millis();
    }

    if (operation_mode == OM_ECO && millis() - prevCardTimeMs >= context.GetTEcoMs()) {
        EnterSleep();
    }
}
