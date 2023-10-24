#include "Context.h"

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
}

uint32_t Context::GetClock()
{
    return millis() + _clock_offset;
}

void Context::SetClock(uint32_t clock)
{
    _clock_offset = clock - millis();
}
