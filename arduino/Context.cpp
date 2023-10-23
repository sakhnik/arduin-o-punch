#include "Context.h"

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
}
