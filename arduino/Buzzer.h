#pragma once

#include <arduino-timer.h>

class Context;

class Buzzer
{
public:
    Buzzer(Context &);

    void Setup();
    void Tick();

    void ConfirmPunch();

private:
    Context &_context;
    Timer<> _timer = timer_create_default();
    Timer<>::Task _confirmation = {};
};
