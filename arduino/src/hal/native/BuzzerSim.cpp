#include "BuzzerSim.hpp"
#include <array>
#include <cmath>

BuzzerSim::BuzzerSim()
    : thread_(&BuzzerSim::Run, this)
{
}

BuzzerSim::~BuzzerSim()
{
    running_ = false;

    if (thread_.joinable())
        thread_.join();
}

void BuzzerSim::Start()
{
    enabled_ = true;
}

void BuzzerSim::Stop()
{
    enabled_ = false;
}

void BuzzerSim::Run()
{
    pa_sample_spec spec {
        .format = PA_SAMPLE_S16NE,
        .rate = sample_rate,
        .channels = 1
    };

    // Low-latency buffering (default pa_simple latency is very large)
    pa_buffer_attr attr {};
    attr.maxlength = static_cast<uint32_t>(-1);
    attr.tlength   = buffer_size * sizeof(int16_t) * 4; // ~23 ms
    attr.prebuf    = static_cast<uint32_t>(-1);
    attr.minreq    = static_cast<uint32_t>(-1);
    attr.fragsize  = static_cast<uint32_t>(-1);

    int error = 0;

    pa_simple* audio = pa_simple_new(
        nullptr,
        "POSIX HAL",
        PA_STREAM_PLAYBACK,
        nullptr,
        "Buzzer",
        &spec,
        nullptr,
        &attr,
        &error
    );

    if (!audio) {
        fprintf(stderr, "PulseAudio: %s\n", pa_strerror(error));
        return;
    }

    std::array<int16_t, buffer_size> buffer{};

    double phase = 0.0;
    double gain = 0.0;   // current envelope level, 0..1

    constexpr double phase_step =
        2.0 * M_PI * frequency / sample_rate;

    constexpr double amplitude = 6000.0;

    // ~5 ms ramp: long enough to kill clicks, short enough for fast Morse
    constexpr double ramp_step = 1.0 / (0.005 * sample_rate);

    while (running_) {
        const double target = enabled_ ? 1.0 : 0.0;

        for (auto& sample : buffer) {
            if (gain < target)
                gain = std::min(gain + ramp_step, target);
            else if (gain > target)
                gain = std::max(gain - ramp_step, target);

            // Smoothstep shaping for a gentler curve than a linear ramp
            const double shaped = gain * gain * (3.0 - 2.0 * gain);

            sample = static_cast<int16_t>(
                amplitude * shaped * std::sin(phase));

            phase += phase_step;
            if (phase >= 2.0 * M_PI)
                phase -= 2.0 * M_PI;
        }

        if (pa_simple_write(
                audio,
                buffer.data(),
                buffer.size() * sizeof(buffer[0]),
                &error) < 0) {
            fprintf(stderr, "PulseAudio: %s\n", pa_strerror(error));
            break;
        }
    }

    pa_simple_flush(audio, &error);
    pa_simple_free(audio);
}
