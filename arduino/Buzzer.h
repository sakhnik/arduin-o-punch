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

    // Play pulse codes, like Morse
    struct Player
    {
        Timer<> &_timer;
        Timer<>::Task _task = {};

        const uint8_t *duration{};
        const uint8_t *duration_end{};
        uint8_t state{};

        Player(Timer<> &timer) : _timer{timer} { }

        // Play given sequence of durations starting from low and toggling between high and low on every other duration
        // Duration is measured in 8 ms quanta
        void Play(const uint8_t *duration, const uint8_t *duration_end);

        static bool OnTimeout(void *ctx);
    };

    Player _player;
};
