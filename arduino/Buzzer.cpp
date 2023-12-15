#include "Buzzer.h"
#include "defs.h"

Buzzer::Buzzer()
    : _player{_timer}
{
}

void Buzzer::Setup()
{
}

void Buzzer::Tick()
{
    // Handle the timer tasks
    _timer.tick();
}

void Buzzer::Player::Play(const Melody &melody)
{
    // If another signal is being played, stop it
    if (_task)
        _timer.cancel(_task);
    // Start executing the new melody from silence for distinction
    this->melody = melody;
    this->idx = 0;
    this->state = LOW;
    OnTimeout(this);
}

bool Buzzer::Player::OnTimeout(void *ctx)
{
    Player *self = reinterpret_cast<Player *>(ctx);
    uint8_t cur_duration = self->melody.sequence[self->idx];
    if (!cur_duration)
        return true;
    uint8_t cur_state = self->state;
    // Confirm with the led (the builtin isn't available together with SPI).
    digitalWrite(LED_CONFIRM_PIN, cur_state);
    digitalWrite(BUZZER_PIN, cur_state);
    self->state = 1 - self->state;  // Toggle the state
    ++self->idx;
    self->_task = self->_timer.in(static_cast<uint16_t>(cur_duration) << 3, OnTimeout, self);
    return false;
}

namespace {

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

} //namespace;

void Buzzer::SignalRTCFail()
{
    _player.Play(RTC_FAIL);
}

void Buzzer::SignalDefaultKey()
{
    _player.Play(KEY_DEF);
}

void Buzzer::ConfirmPunch()
{
    _player.Play(CONFIRM);
}

void Buzzer::SignalCardFull()
{
    _player.Play(CARD_FULL);
}
