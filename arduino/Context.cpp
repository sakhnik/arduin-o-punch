#include "Context.h"

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
    _key_initialized = true;
}

void Context::SetKeyHex(const char *key)
{
    // TODO: parse and copy
    _key_initialized = true;
}
