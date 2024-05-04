#pragma once

#include <arduino-timer.h>
#include <CircularBuffer.h>

class Buzzer
{
public:
    Buzzer();

    void Setup();
    void Tick();

    void ConfirmPunch();
    void SignalDefaultKey();
    void SignalRTCFail();
    void SignalCardFull();
    void SignalOk();
    void SignalBluetooth();
    void SignalWifi();

    struct Melody
    {
        // Play given sequence of durations starting from low and toggling between high and low on every other duration
        // Duration is measured in 8 ms quanta
        const uint8_t *sequence = nullptr;
        // After this number of durations, can interrupt the melody and play the next one.
        uint8_t interrupt_idx = 0;

        Melody(const uint8_t *sequence = nullptr, uint8_t interrupt_idx = 255)
            : sequence{sequence}
            , interrupt_idx{interrupt_idx}
        {
        }
    };

private:
    Timer<> _timer = timer_create_default();

    // Play pulse codes, like Morse
    struct Player
    {
        Timer<> &_timer;
        Timer<>::Task _task = {};

        // Pending melodies
        CircularBuffer<Melody, 8> melodies;
        // The melody being currently played
        Melody melody;
        // Index into the melody sequence
        uint8_t idx{};
        // Buzzer state (high=1, low=0)
        uint8_t state{};

        Player(Timer<> &timer) : _timer{timer} { }

        void Play(const Melody &);

        static bool OnTimeout(void *ctx);
    };

    Player _player;
};
