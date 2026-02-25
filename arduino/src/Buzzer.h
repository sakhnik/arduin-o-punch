#pragma once

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class Buzzer
{
public:
    Buzzer();
    void Setup();

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

    bool IsIdle();

    void ConfirmPunch();
    void SignalDefaultKey();
    void SignalRTCFail();
    void SignalCardFull();
    void SignalOk();
    void SignalBluetooth();
    void SignalWifi();
    void SignalDit();
    void SignalDah();
    void SignalPause();
    void SignalNumber(uint16_t);

private:
    QueueHandle_t _queue;
    TaskHandle_t _taskHandle = nullptr;

    static void TaskEntry(void *arg);
    void TaskLoop();
    void Play(const Melody &melody);
};
