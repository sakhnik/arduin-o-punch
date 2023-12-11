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

    struct Player
    {
        Timer<> &_timer;
        Timer<>::Task _task = {};

        const uint8_t *duration{};
        const uint8_t *duration_end{};
        uint8_t state{};

        Player(Timer<> &timer) : _timer{timer} { }

        void Play(const uint8_t *duration, const uint8_t *duration_end);
        static bool OnTimeout(void *ctx);
    };

    Player _player;
};
