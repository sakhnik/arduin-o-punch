#include "Buzzer.hpp"
#include "defs.h"
#include "ActivityCounter.hpp"
#include "hal/gpio.hpp"
#include "hal/time.hpp"

Buzzer::Buzzer()
{
}

void Buzzer::Setup()
{
    HAL::GpioModeOutput(BUZZER_PIN);
    HAL::GpioWrite(BUZZER_PIN, true);

    _queue.Setup(8, sizeof(Melody));

    _task.Setup(TaskEntry, this, "buzzer", 2048, 1);
}

void Buzzer::Play(const Melody &melody)
{
    _queue.Send(&melody, 0);
}

bool Buzzer::IsIdle()
{
    return _queue.Size() == 0;
}

void Buzzer::TaskEntry(void *arg)
{
    static_cast<Buzzer *>(arg)->TaskLoop();
}

void Buzzer::TaskLoop()
{
    Melody melody;

    while (true) {
        // Wait for first melody
        if (!_queue.Receive(&melody))
            continue;

        ActivityCounter::Scoped activity;

        uint8_t idx = 0;
        bool state = true;

        while (true) {
            uint8_t duration = melody.sequence[idx];

            if (duration == 0) {
                HAL::GpioWrite(BUZZER_PIN, true);
                break; // melody finished
            }

            if (idx >= melody.interrupt_idx) {
                Melody next;
                if (!_queue.Receive(&next, 0)) {
                    melody = next;
                    idx = 0;
                    state = true;
                    continue;
                }
            }

            HAL::GpioWrite(BUZZER_PIN, state);

            state = !state;
            idx++;

            HAL::Delay(duration * 8);
        }
    }
}

namespace Morse {

// Short pause for distinction from previous or interrupted signals
constexpr uint8_t START = 1;
// Morse sounds
constexpr uint8_t DIT = 5;
constexpr uint8_t DAH = 15;
// Pause between dots and dashes
constexpr uint8_t P = 5;
// Pause between letters
constexpr uint8_t P_L = 25;

// RTC = .-. - -.-.
const uint8_t RTC_FAIL_SEQUENCE[] = {START, DIT, P, DAH, P, DIT, P_L,    DAH, P_L,    DAH, P, DIT, P, DAH, P, DIT, P, 0};
const Buzzer::Melody RTC_FAIL{RTC_FAIL_SEQUENCE, 255};

// MFRC = -- ..-. .-. -.-.
const uint8_t MFRC_FAIL_SEQUENCE[] = {START, /*M*/DAH, P, DAH, P_L, /*F*/DIT, P, DIT, P, DAH, P, DIT, P_L, /*R*/DIT, P, DAH, P, DIT, P_L, /*C*/DAH, P, DIT, P, DAH, P, DIT, P, 0};
const Buzzer::Melody MFRC_FAIL{MFRC_FAIL_SEQUENCE, 255};

// KEY = -.- . -.--
const uint8_t KEY_DEF_SEQUENCE[] = {START, DAH, P, DIT, P, DAH, P_L,    DIT, P_L,    DAH, P, DIT, P, DAH, P, DAH, P, 0};
const Buzzer::Melody KEY_DEF{KEY_DEF_SEQUENCE, 255};

// A = .-
const uint8_t CONFIRM_SEQUENCE[] = {START, DIT, P, DAH, P, 0};
const Buzzer::Melody CONFIRM{CONFIRM_SEQUENCE, 2};  // allow interrupting after the dot

// D = -..
const uint8_t DEBUG_SEQUENCE[] = {START, DAH, P, DIT, P, DIT, P, 0};
const Buzzer::Melody DEBUG{DEBUG_SEQUENCE, 255};

// F = ..-.
const uint8_t CARD_FULL_SEQUENCE[] = {START, DIT, P, DIT, P, DAH, P, DIT, P, 0};
const Buzzer::Melody CARD_FULL{CARD_FULL_SEQUENCE, 255};

// OK = --- -.-
const uint8_t OK_SEQUENCE[] = {START, DAH, P, DAH, P, DAH, P_L,    DAH, P, DIT, P, DAH, P, 0};
const Buzzer::Melody OK{OK_SEQUENCE, 255};

// E = .
const uint8_t E_SEQUENCE[] = {START, DIT, P, 0};
const Buzzer::Melody E{E_SEQUENCE, 255};

// BT = -... -
const uint8_t BT_SEQUENCE[] = {START, DAH, P, DIT, P, DIT, P, DIT, P_L,   DAH, P,   0};
const Buzzer::Melody BT{BT_SEQUENCE, 255};

// WF = .-- ..-.
const uint8_t WF_SEQUENCE[] = {START, DIT, P, DAH, P, DAH, P_L,     DIT, P, DIT, P, DAH, P, DIT, P, 0};
const Buzzer::Melody WF{WF_SEQUENCE, 255};

// .
const uint8_t DIT_SEQUENCE[] = {START, DIT, P, 0};
const Buzzer::Melody Dit{DIT_SEQUENCE, 255};

// -
const uint8_t DAH_SEQUENCE[] = {START, DAH, P, 0};
const Buzzer::Melody Dah{DAH_SEQUENCE, 255};

const uint8_t PAUSE_SEQUENCE[] = {P_L, 0};
const Buzzer::Melody Pause{PAUSE_SEQUENCE, 255};

// Digits
const uint8_t DIGIT_0_SEQUENCE[] = {START, DAH, P, DAH, P, DAH, P, DAH, P, DAH, P_L, 0};
const uint8_t DIGIT_1_SEQUENCE[] = {START, DIT, P, DAH, P, DAH, P, DAH, P, DAH, P_L, 0};
const uint8_t DIGIT_2_SEQUENCE[] = {START, DIT, P, DIT, P, DAH, P, DAH, P, DAH, P_L, 0};
const uint8_t DIGIT_3_SEQUENCE[] = {START, DIT, P, DIT, P, DIT, P, DAH, P, DAH, P_L, 0};
const uint8_t DIGIT_4_SEQUENCE[] = {START, DIT, P, DIT, P, DIT, P, DIT, P, DAH, P_L, 0};
const uint8_t DIGIT_5_SEQUENCE[] = {START, DIT, P, DIT, P, DIT, P, DIT, P, DIT, P_L, 0};
const uint8_t DIGIT_6_SEQUENCE[] = {START, DAH, P, DIT, P, DIT, P, DIT, P, DIT, P_L, 0};
const uint8_t DIGIT_7_SEQUENCE[] = {START, DAH, P, DAH, P, DIT, P, DIT, P, DIT, P_L, 0};
const uint8_t DIGIT_8_SEQUENCE[] = {START, DAH, P, DAH, P, DAH, P, DIT, P, DIT, P_L, 0};
const uint8_t DIGIT_9_SEQUENCE[] = {START, DAH, P, DAH, P, DAH, P, DAH, P, DIT, P_L, 0};
const Buzzer::Melody Digits[] = {
    {DIGIT_0_SEQUENCE, 255},
    {DIGIT_1_SEQUENCE, 255},
    {DIGIT_2_SEQUENCE, 255},
    {DIGIT_3_SEQUENCE, 255},
    {DIGIT_4_SEQUENCE, 255},
    {DIGIT_5_SEQUENCE, 255},
    {DIGIT_6_SEQUENCE, 255},
    {DIGIT_7_SEQUENCE, 255},
    {DIGIT_8_SEQUENCE, 255},
    {DIGIT_9_SEQUENCE, 255},
};

} //namespace Morse;

void Buzzer::SignalRTCFail()
{
    Play(Morse::RTC_FAIL);
}

void Buzzer::SignalMFRCFail()
{
    Play(Morse::MFRC_FAIL);
}

void Buzzer::SignalDefaultKey()
{
    Play(Morse::KEY_DEF);
}

void Buzzer::ConfirmPunch()
{
    Play(Morse::CONFIRM);
}

void Buzzer::ConfirmDebug()
{
    Play(Morse::DEBUG);
}

void Buzzer::SignalCardFull()
{
    Play(Morse::CARD_FULL);
}

void Buzzer::SignalOk()
{
    Play(Morse::OK);
}

void Buzzer::SignalE()
{
    Play(Morse::E);
}

void Buzzer::SignalBluetooth()
{
    Play(Morse::BT);
}

void Buzzer::SignalWifi()
{
    Play(Morse::WF);
}

void Buzzer::SignalDit()
{
    Play(Morse::Dit);
}

void Buzzer::SignalDah()
{
    Play(Morse::Dah);
}

void Buzzer::SignalPause()
{
    Play(Morse::Pause);
}

void Buzzer::SignalNumber(uint16_t num)
{
    if (!num) {
        return;
    }
    SignalNumber(num / 10);
    Play(Morse::Digits[num % 10]);
}
