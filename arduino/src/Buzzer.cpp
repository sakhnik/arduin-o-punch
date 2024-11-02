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
    // If another melody is being played, enqueue
    if (_task) {
        melodies.push(melody);
        return;
    }
    // Start executing the new melody from silence for distinction
    this->melody = melody;
    this->idx = 0;
    this->state = LOW;
    OnTimeout(this);
}

bool Buzzer::Player::OnTimeout(void *ctx)
{
    Player *self = reinterpret_cast<Player *>(ctx);
    // Is there another melody to play and we should go to it?
    bool to_next_melody = self->idx >= self->melody.interrupt_idx && !self->melodies.isEmpty();
    // If there's another melody to play, this one can be already interrupted.
    if (to_next_melody) {
        self->melody = self->melodies.shift();
        self->idx = 0;
        self->state = LOW;
    }
    // If the melody has ended, remove the timer and relax.
    uint8_t cur_duration = self->melody.sequence[self->idx];
    if (!cur_duration) {
        self->_timer.cancel(self->_task);
        return true;
    }
    uint8_t cur_state = self->state;
    // Confirm with the led (the builtin isn't available together with SPI).
    digitalWrite(LED_CONFIRM_PIN, cur_state);
    digitalWrite(BUZZER_PIN, cur_state);
    self->state = 1 - self->state;  // Toggle the state
    ++self->idx;
    self->_task = self->_timer.in(static_cast<uint16_t>(cur_duration) << 3, OnTimeout, self);
    return false;
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

} //namespace Morse;

void Buzzer::SignalRTCFail()
{
    _player.Play(Morse::RTC_FAIL);
}

void Buzzer::SignalDefaultKey()
{
    _player.Play(Morse::KEY_DEF);
}

void Buzzer::ConfirmPunch()
{
    _player.Play(Morse::CONFIRM);
}

void Buzzer::SignalCardFull()
{
    _player.Play(Morse::CARD_FULL);
}

void Buzzer::SignalOk()
{
    _player.Play(Morse::OK);
}

void Buzzer::SignalBluetooth()
{
    _player.Play(Morse::BT);
}

void Buzzer::SignalWifi()
{
    _player.Play(Morse::WF);
}

void Buzzer::SignalDit()
{
    _player.Play(Morse::Dit);
}

void Buzzer::SignalDah()
{
    _player.Play(Morse::Dah);
}
