#pragma once

#include <arduino-timer.h>

class Buzzer
{
public:
    void Setup();
    void Tick();

    void ConfirmPunch();

private:
    Timer<> _timer = timer_create_default();
    Timer<>::Task _confirmation = {};
};
