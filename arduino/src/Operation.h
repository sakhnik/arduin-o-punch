#pragma once

#include "Mutex.h"
#include <array>

class Buzzer;
class Bluetooth;
class Network;
class Settings;

class Operation
{
public:
    Operation(Buzzer &, Settings &, Bluetooth &, Network &);

    void Setup();
    void SetupLate();

    enum class Mode
    {
        Active = 0,
        Eco,
        BLE,
        WiFi,
        Sleep,

        Count
    };

    void TransitionToActive();
    void TransitionToNext();
    bool CheckSnooze();

    void ResetStats();
    std::string DumpStats();

private:
    Buzzer &buzzer;
    Settings &settings;
    Bluetooth &bluetooth;
    Network &network;
    // Start in the sleep mode to record the sleep time
    Mode mode = Mode::Sleep;
    Mutex mutex;
    uint32_t prevCardTimeMs = 0;
    uint32_t prevTransitionTime = 0;
    std::array<uint32_t, static_cast<size_t>(Mode::Count)> timeStats = {};

    void TransitionTo(Mode, bool silent = false);
    Mode GetNextMode();
    void EnterSleep();
};
