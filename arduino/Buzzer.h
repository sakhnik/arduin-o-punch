#pragma once

#include <arduino-timer.h>

class Buzzer
{
public:
    Buzzer();

    void Setup();
    void Tick();

    void ConfirmPunch();
    void SignalDefaultKey();
    void SignalRTCFail();

private:
    Timer<> _timer = timer_create_default();
    Timer<>::Task _confirmation = {};
};
