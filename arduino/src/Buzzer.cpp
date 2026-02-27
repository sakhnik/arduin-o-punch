#include "Buzzer.h"
#include "defs.h"
#include <Arduino.h>

Buzzer::Buzzer()
{
}

void Buzzer::Setup()
{
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_CONFIRM_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_CONFIRM_PIN, LOW);

    _queue = xQueueCreate(8, sizeof(Melody));

    xTaskCreate(TaskEntry, "buzzer", 2048, this, 1, &_taskHandle);
}

void Buzzer::Play(const Melody &melody)
{
    xQueueSend(_queue, &melody, 0);
}

bool Buzzer::IsIdle()
{
    return uxQueueMessagesWaiting(_queue) == 0;
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
        if (xQueueReceive(_queue, &melody, portMAX_DELAY) != pdTRUE)
            continue;

        uint8_t idx = 0;
        uint8_t state = LOW;

        while (true) {
            uint8_t duration = melody.sequence[idx];

            if (duration == 0) {
                digitalWrite(BUZZER_PIN, LOW);
                digitalWrite(LED_CONFIRM_PIN, LOW);
                break; // melody finished
            }

            if (idx >= melody.interrupt_idx) {
                Melody next;
                if (xQueueReceive(_queue, &next, 0) == pdTRUE) {
                    melody = next;
                    idx = 0;
                    state = LOW;
                    continue;
                }
            }

            digitalWrite(BUZZER_PIN, state);
            digitalWrite(LED_CONFIRM_PIN, state);

            state = !state;
            idx++;

            vTaskDelay(pdMS_TO_TICKS(duration * 8));
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

// KEY = -.- . -.--
const uint8_t KEY_DEF_SEQUENCE[] = {START, DAH, P, DIT, P, DAH, P_L,    DIT, P_L,    DAH, P, DIT, P, DAH, P, DAH, P, 0};
const Buzzer::Melody KEY_DEF{KEY_DEF_SEQUENCE, 255};

// A = .-
const uint8_t CONFIRM_SEQUENCE[] = {START, DIT, P, DAH, P, 0};
const Buzzer::Melody CONFIRM{CONFIRM_SEQUENCE, 2};  // allow interrupting after the dot

// F = ..-.
const uint8_t CARD_FULL_SEQUENCE[] = {START, DIT, P, DIT, P, DAH, P, DIT, P, 0};
const Buzzer::Melody CARD_FULL{CARD_FULL_SEQUENCE, 255};

// OK = --- -.-
const uint8_t OK_SEQUENCE[] = {START, DAH, P, DAH, P, DAH, P_L,    DAH, P, DIT, P, DAH, P, 0};
const Buzzer::Melody OK{OK_SEQUENCE, 255};

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

void Buzzer::SignalDefaultKey()
{
    Play(Morse::KEY_DEF);
}

void Buzzer::ConfirmPunch()
{
    Play(Morse::CONFIRM);
}

void Buzzer::SignalCardFull()
{
    Play(Morse::CARD_FULL);
}

void Buzzer::SignalOk()
{
    Play(Morse::OK);
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
