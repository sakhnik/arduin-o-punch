#pragma once

#ifdef BUILD_TEST
# include <cstdint>
#else
# include "Arduino.h"
#endif

namespace AOP {

struct IKeyReceiver
{
    virtual ~IKeyReceiver() { }
    virtual void OnNewKey(const uint8_t *key) = 0;
};

} //namespace AOP;
