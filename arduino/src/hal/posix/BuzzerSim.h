#pragma once

#include <pulse/simple.h>
#include <pulse/error.h>

#include <atomic>
#include <cstdio>
#include <thread>

class BuzzerSim
{
public:
    BuzzerSim();
    ~BuzzerSim();

    void Start();
    void Stop();

private:
    static constexpr unsigned sample_rate = 44100;
    static constexpr unsigned frequency = 2000;
    static constexpr size_t buffer_size = 256;

    std::atomic<bool> running_{true};
    std::atomic<bool> enabled_{false};

    std::thread thread_;

    void Run();
};
