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
    // TODO: maybe better to call from a hardware timer ISR
    _timer.tick();
}

namespace {

constexpr uint8_t START = 1;
constexpr uint8_t DIT = 5;
constexpr uint8_t DAH = 15;
constexpr uint8_t P = 5;
constexpr uint8_t P_L = 25;

const uint8_t RTC_FAIL[] = {START, DIT, P, DAH, P, DIT, P_L,    DAH, P_L,    DAH, P, DIT, P, DAH, P, DIT, P};  // RTC = .-. - -.-.
const uint8_t KEY_DEF[] = {START, DAH, P, DIT, P, DAH, P_L,    DIT, P_L,    DAH, P, DIT, P, DAH, P, DAH, P};  // KEY = -.- . -.--
const uint8_t CONFIRM[] = {START, DIT, P, DAH, P};  // A = .-

} //namespace;

void Buzzer::SignalRTCFail()
{
    _player.Play(RTC_FAIL, RTC_FAIL + sizeof(RTC_FAIL));
}

void Buzzer::SignalDefaultKey()
{
    _player.Play(KEY_DEF, KEY_DEF + sizeof(KEY_DEF));
}

void Buzzer::Player::Play(const uint8_t *duration, const uint8_t *duration_end)
{
    if (_task)
        _timer.cancel(_task);
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
    digitalWrite(LED_CONFIRM_PIN, cur_state);
    digitalWrite(BUZZER_PIN, cur_state);
    self->state = 1 - self->state;
    if (++self->duration != self->duration_end)
    {
        self->_task = self->_timer.in(static_cast<uint16_t>(cur_duration) << 3, OnTimeout, self);
        return false;
    }
    return true;
}

void Buzzer::ConfirmPunch()
{
    // Confirm with the led (the builtin isn't available together with SPI).
    _player.Play(CONFIRM, CONFIRM + sizeof(CONFIRM));
}
