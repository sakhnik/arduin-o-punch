#pragma once

class Buzzer;
class Bluetooth;
class Network;
class Settings;

class Operation
{
public:
    Operation(Buzzer &, Settings &, Bluetooth &, Network &);

    void Setup();

    enum class Mode
    {
        ACTIVE = 0,
        ECO,
        BLE,
        WIFI,
    };

    void TransitionToActive();
    void TransitionToNext();
    bool CheckSnooze();

private:
    Buzzer &buzzer;
    Settings &settings;
    Bluetooth &bluetooth;
    Network &network;
    Mode mode;
    unsigned long long prevCardTimeMs = 0;

    void TransitionTo(Mode);
    Mode GetNextMode();
    void EnterSleep();
};
