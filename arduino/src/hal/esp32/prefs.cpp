#include "hal/prefs.h"
#include <Preferences.h>

namespace HAL {

Preferences prefs;

Prefs::Prefs(const char *name, bool readOnly)
{
    prefs.begin(name, readOnly);
}

Prefs::~Prefs()
{
    prefs.end();
}

uint8_t Prefs::GetU8(const char *key, uint8_t def)
{
    return prefs.getUChar(key, def);
}

uint16_t Prefs::GetU16(const char *key, uint16_t def)
{
    return prefs.getUShort(key, def);
}

void Prefs::GetBytes(const char *key, uint8_t *data, size_t size)
{
    prefs.getBytes(key, data, size);
}

size_t Prefs::GetBytesLength(const char *key)
{
    return prefs.getBytesLength(key);
}

std::string Prefs::GetString(const char *key)
{
    return prefs.getString(key).c_str();
}

void Prefs::PutU8(const char *key, uint8_t val)
{
    prefs.putUChar(key, val);
}

void Prefs::PutU16(const char *key, uint16_t val)
{
    prefs.putUShort(key, val);
}

void Prefs::PutBytes(const char *key, const uint8_t *data, size_t size)
{
    prefs.putBytes(key, data, size);
}

void Prefs::PutString(const char *key, const char *val)
{
    prefs.putString(key, val);
}

} //namespace HAL;
