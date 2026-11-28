#pragma once

#include "OutMux.hpp"

class Settings;
class Shell;
class Buzzer;

class Network
{
public:
    Network(OutMux &, Settings &, Shell &, Buzzer &);
    ~Network();

#if defined(ENABLE_WIFI) && ENABLE_WIFI
    void Setup();
    void SwitchOn();
    void SwitchOff();
    unsigned GetInactivitySeconds();
#endif

private:
    void *_impl = nullptr;
};
