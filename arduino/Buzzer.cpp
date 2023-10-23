#include "Buzzer.h"
#include "defs.h"

void Buzzer::Setup()
{
    // Blink continuously to indicate initialization prompt
    _confirmation = _timer.every(250, [](void*) {
        digitalWrite(LED_CONFIRM_PIN, !digitalRead(LED_CONFIRM_PIN));
        return true;
    });
}

void Buzzer::Tick()
{
    // TODO: maybe better to call from a hardware timer ISR
    _timer.tick();
}

void Buzzer::ConfirmPunch()
{
    // Confirm with the led (the builtin isn't available together with SPI).
    if (_confirmation)
        _timer.cancel(_confirmation);
    digitalWrite(LED_CONFIRM_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    _confirmation = _timer.in(400,
        [](void*) {
            digitalWrite(LED_CONFIRM_PIN, LOW);
            digitalWrite(BUZZER_PIN, LOW);
            return false;
        }
    );
}
