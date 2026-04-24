#include "Operation.h"
#include "Buzzer.h"
#include "Settings.h"
#include "Bluetooth.h"
#include "Network.h"
#include "CpuFreq.h"
#include "defs.h"
#include "ActivityCounter.h"
#include "RtcLog.h"
#include <SPI.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <ESP32Time.h>
#include <Preferences.h>

extern ESP32Time rtc;

namespace {

Preferences prefs;

// Hold some pins in the high state during sleep to keep RFC522 and DS3231 switched off and to avoid parasitic current
static constexpr const int HIGH_PINS[] = {MOSFET_PIN, LED_CONFIRM_PIN, BUZZER_PIN};

constexpr const int TIME_TO_SLEEP_MS = 500;

constexpr const char *const PREF_STATS = "stats";
constexpr const char *const PREF_PREV_TRANSITION = "prev-trans";
constexpr const char *const PREF_PREV_MODE = "prev-mode";
constexpr const char *const PREF_TIMES = "times";

} //namespace;

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

    if (RtcLog::IsReliable()) {
        // Restore the previous mode to preserve the accurate statistics
        prefs.begin(PREF_STATS, true);
        prevTransitionTime = prefs.getULong(PREF_PREV_TRANSITION, rtc.getEpoch());
        auto prevMode = prefs.getInt(PREF_PREV_MODE, static_cast<int>(Mode::Sleep));
        if (prevMode < static_cast<int>(Mode::Count)) {
            mode = static_cast<Mode>(prevMode);
        }
        prefs.getBytes(PREF_TIMES, timeStats.data(), sizeof(timeStats));
        prefs.end();
    } else {
        prevTransitionTime = rtc.getEpoch();
    }

    // Transition to the default mode and record the time in the sleep/previous mode
    TransitionTo(Mode::Eco, /*silent=*/true);

    prevCardTimeMs = millis();
}

void Operation::SetupLate()
{
    // Stay up 2 minutes unless a card has been punched
    prevCardTimeMs = millis() + 2 * 60000 - settings.GetEcoMs();
}

Operation::Mode Operation::GetNextMode()
{
    switch (mode) {
    case Mode::Active:
    case Mode::Eco:
        return Mode::BLE;
    case Mode::BLE:
        return Mode::WiFi;
    case Mode::WiFi:
        return Mode::Eco;
    default:
        break;
    }
    return Mode::Eco;
}

void Operation::TransitionTo(Mode nextMode, bool silent)
{
    switch (mode) {
    case Mode::Active:
    case Mode::Eco:
        break;
    case Mode::BLE:
        bluetooth.SwitchOff();
        break;
    case Mode::WiFi:
        network.SwitchOff();
        break;
    default:
        break;
    }

    // Record the time spent in the previous mode
    auto now = rtc.getEpoch();
    {
        LockGuard lock{mutex};
        timeStats[static_cast<size_t>(mode)] += now - prevTransitionTime;
        prevTransitionTime = now;
        prefs.begin(PREF_STATS, false);
        prefs.putULong(PREF_PREV_TRANSITION, now);
        prefs.putInt(PREF_PREV_MODE, static_cast<int>(nextMode));
        prefs.end();
    }

    mode = nextMode;

    switch (mode) {
    case Mode::Active:
    case Mode::Eco:
        SetCpuFreq(10);
        if (!silent) {
            buzzer.SignalOk();
        }
        break;
    case Mode::BLE:
        SetCpuFreq(80);
        if (!silent) {
            buzzer.SignalBluetooth();
        }
        bluetooth.SwitchOn();
        break;
    case Mode::WiFi:
        SetCpuFreq(80);
        // It takes a second or so to initialize WiFi. The CPU will be busy.
        // Play a confirmation melody only after that to avoid distortion.
        network.SwitchOn();
        if (!silent) {
            buzzer.SignalWifi();
        }
        break;
    case Mode::Sleep:
        EnterSleep();
        break;
    default:
        // Shouldn't be reached
        break;
    }
}

void Operation::EnterSleep()
{
    prefs.begin(PREF_STATS, false);
    prefs.putULong(PREF_PREV_TRANSITION, prevTransitionTime);
    prefs.putBytes(PREF_TIMES, timeStats.data(), sizeof(timeStats));
    prefs.end();

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
    if (mode == Mode::Eco) {
        TransitionTo(Mode::Active, /*silent=*/true);
    }
    prevCardTimeMs = millis();
}

void Operation::TransitionToNext()
{
    TransitionTo(GetNextMode());
}

bool Operation::CheckSnooze()
{
    auto millisNow = millis();

    if (mode == Mode::Eco && millisNow - prevCardTimeMs >= settings.GetEcoMs()) {
        TransitionTo(Mode::Sleep);
        // Never reached
        return false;
    }

    if (mode == Mode::Active && millisNow - prevCardTimeMs >= settings.GetActiveMs()) {
        TransitionTo(Mode::Eco, /*silent=*/true);
        prevCardTimeMs = millisNow;
        return false;
    }

    if (mode == Mode::Eco && !ActivityCounter::Instance().IsBusy()) {
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

void Operation::ResetStats()
{
    LockGuard lock{mutex};
    prevTransitionTime = rtc.getEpoch();
    timeStats.fill(0);
}

Operation::StatsT Operation::GetStats()
{
    decltype(timeStats) snapshot;
    auto now = rtc.getEpoch();
    {
        LockGuard lock{mutex};
        snapshot = timeStats;
        // Account the ongoing mode
        snapshot[static_cast<size_t>(mode)] += now - prevTransitionTime;
    }
    return snapshot;
}

std::string Operation::DumpStats()
{
    auto toMAH = [](uint32_t s, float current)  {
        return current * s / (60 * 60);
    };

    auto getCurrent = [](Mode m) -> float {
        switch (m) {
        case Mode::Active: return 20.f;
        case Mode::Eco: return 10.f;
        case Mode::BLE: return 100.f;
        case Mode::WiFi: return 100.f;
        case Mode::Sleep: return 0.05f;
        default: return 0.f;
        }
    };

    uint32_t totalTime = 0;
    float totalEnergy = 0;

    auto getLine = [&, this](uint32_t time, float current) -> std::string {
        totalTime += time;
        auto mah = toMAH(time, current);
        totalEnergy += mah;
        char line[64];
        sprintf(line, "%8d (%4d:%02d:%02d) %6.1f mAh", time, time / (60 * 60), (time / 60) % 60, time % 60, mah);
        return line;
    };

    auto snapshot = GetStats();

    std::string s;
    s += "Active: " + getLine(snapshot[static_cast<size_t>(Mode::Active)], 20.f) + "\n";
    s += "Eco:    " + getLine(snapshot[static_cast<size_t>(Mode::Eco)], 10.f) + "\n";
    s += "BLE:    " + getLine(snapshot[static_cast<size_t>(Mode::BLE)], 100.f) + "\n";
    s += "WiFi:   " + getLine(snapshot[static_cast<size_t>(Mode::WiFi)], 100.f) + "\n";
    s += "Sleep:  " + getLine(snapshot[static_cast<size_t>(Mode::Sleep)], 0.05f) + "\n\n";
    auto avgCur = totalEnergy * (60 * 60) / totalTime;
    s += "Total:  " + getLine(totalTime, avgCur) + "  I ~ ";
    char buf[32];
    sprintf(buf, "%.2f", avgCur);
    s += buf;
    s += " mA\n";

    return s;
}
