#include "Context.h"

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
}

uint32_t Context::GetClock() const
{
    return millis() + _clock_offset;
}

void Context::SetClock(uint32_t clock)
{
    _clock_offset = clock - millis();
}

uint8_t Context::GetId() const
{
    return _id;
}

void Context::SetId(uint8_t id)
{
    _id = id;
}
