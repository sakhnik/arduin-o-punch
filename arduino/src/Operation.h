#pragma once

class Buzzer;
class Bluetooth;
class Network;
class Context;

class Operation
{
public:
    Operation(Buzzer &, Context &, Bluetooth &, Network &);

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
    Context &context;
    Bluetooth &bluetooth;
    Network &network;
    Mode mode;
    unsigned long long prevCardTimeMs = 0;

    void TransitionTo(Mode);
    Mode GetNextMode();
    void EnterSleep();
};
