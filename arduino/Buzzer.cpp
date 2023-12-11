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

void Buzzer::Player::Play(const uint8_t *duration, const uint8_t *duration_end)
{
    // If another signal is being played, stop it
    if (_task)
        _timer.cancel(_task);
    // Start executing the new melody from silence for distinction
    this->duration = duration;
    this->duration_end = duration_end;
    this->state = LOW;
    OnTimeout(this);
}

bool Buzzer::Player::OnTimeout(void *ctx)
{
    Player *self = reinterpret_cast<Player *>(ctx);
    uint8_t cur_duration = *self->duration;
    uint8_t cur_state = self->state;
    // Confirm with the led (the builtin isn't available together with SPI).
    digitalWrite(LED_CONFIRM_PIN, cur_state);
    digitalWrite(BUZZER_PIN, cur_state);
    self->state = 1 - self->state;  // Toggle the state
    if (++self->duration != self->duration_end)
    {
        self->_task = self->_timer.in(static_cast<uint16_t>(cur_duration) << 3, OnTimeout, self);
        return false;
    }
    return true;
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
const uint8_t RTC_FAIL[] = {START, DIT, P, DAH, P, DIT, P_L,    DAH, P_L,    DAH, P, DIT, P, DAH, P, DIT, P};
// KEY = -.- . -.--
const uint8_t KEY_DEF[] = {START, DAH, P, DIT, P, DAH, P_L,    DIT, P_L,    DAH, P, DIT, P, DAH, P, DAH, P};
// A = .-
const uint8_t CONFIRM[] = {START, DIT, P, DAH, P};

} //namespace;

void Buzzer::SignalRTCFail()
{
    _player.Play(RTC_FAIL, RTC_FAIL + sizeof(RTC_FAIL));
}

void Buzzer::SignalDefaultKey()
{
    _player.Play(KEY_DEF, KEY_DEF + sizeof(KEY_DEF));
}

void Buzzer::ConfirmPunch()
{
    _player.Play(CONFIRM, CONFIRM + sizeof(CONFIRM));
}
