#include "Operation.h"
#include "Buzzer.h"
#include "Settings.h"
#include "Bluetooth.h"
#include "Network.h"
#include "CpuFreq.h"
#include "defs.h"
#include "ActivityCounter.h"
#include <SPI.h>

// Hold some pins in the high state during sleep to keep RFC522 and DS3231 switched off and to avoid parasitic current
static constexpr const int HIGH_PINS[] = {MOSFET_PIN, LED_CONFIRM_PIN, BUZZER_PIN};

constexpr const int TIME_TO_SLEEP_MS = 500;

Operation::Operation(Buzzer &buzzer, Settings &settings, Bluetooth &bluetooth, Network &network)
    : buzzer{buzzer}
    , settings{settings}
    , bluetooth{bluetooth}
    , network{network}
{
}

void Operation::Setup()
{
    gpio_deep_sleep_hold_dis();
    for (int pin : HIGH_PINS) {
        gpio_hold_dis(static_cast<gpio_num_t>(pin));
    }

    pinMode(MOSFET_PIN, OUTPUT);
    pinMode(WAKEUP_PIN, INPUT_PULLUP);

    // Set the CPU frequency to 10 MHz for power optimization
    setCpuFrequencyMhz(10);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));

    // Power on the RFID reader
    digitalWrite(MOSFET_PIN, LOW);

    prevCardTimeMs = millis();
}

Operation::Mode Operation::GetNextMode()
{
    switch (mode) {
    case Mode::ACTIVE:
    case Mode::ECO:
        return Mode::BLE;
    case Mode::BLE:
        return Mode::WIFI;
    case Mode::WIFI:
        return Mode::ECO;
    default:
        break;
    }
    return Mode::ECO;
}

void Operation::TransitionTo(Mode nextMode)
{
    switch (mode) {
    case Mode::ACTIVE:
    case Mode::ECO:
        break;
    case Mode::BLE:
        bluetooth.SwitchOff();
        break;
    case Mode::WIFI:
        network.SwitchOff();
        break;
    default:
        break;
    }

    mode = nextMode;

    switch (mode) {
    case Mode::ACTIVE:
    case Mode::ECO:
        SetCpuFreq(10);
        buzzer.SignalOk();
        break;
    case Mode::BLE:
        SetCpuFreq(80);
        buzzer.SignalBluetooth();
        bluetooth.SwitchOn();
        break;
    case Mode::WIFI:
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

void Operation::EnterSleep()
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

void Operation::TransitionToActive()
{
    if (mode == Mode::ECO) {
        mode = Mode::ACTIVE;
    }
    prevCardTimeMs = millis();
}

void Operation::TransitionToNext()
{
    prevCardTimeMs = millis();
    TransitionTo(GetNextMode());
}

bool Operation::CheckSnooze()
{
    if (mode == Mode::ECO && millis() - prevCardTimeMs >= settings.GetEcoMs()) {
        EnterSleep();
        // Never reached
        return false;
    }

    if (mode == Mode::ACTIVE && millis() - prevCardTimeMs >= settings.GetActiveMs()) {
        mode = Mode::ECO;
        prevCardTimeMs = millis();
        return false;
    }

    if (mode == Mode::ECO && !ActivityCounter::Instance().IsBusy()) {
        digitalWrite(MOSFET_PIN, HIGH);
        //Serial.flush();
        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_MS * 1000ull);
        esp_light_sleep_start();
        digitalWrite(MOSFET_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(50));
        // Did snooze, need to reset the puncher
        return true;
    }

    return false;
}
