#include "Context.h"
#include <EEPROM.h>

#ifdef BUILD_TEST
# include <cstddef>
#endif

#define ADDROF(field) (ADDRESS + offsetof(Context, field))

void Context::Setup()
{
    EEPROM.get(ADDROF(_id), _id);
    EEPROM.get(ADDROF(_key), _key);
}

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
    EEPROM.put(ADDROF(_key), _key);
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
    EEPROM.put(ADDROF(_id), _id);
}
