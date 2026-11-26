#pragma once

#include "DateTime.h"

namespace Hal {

class RTC
{
public:
    static bool Setup();
    static AOP::DateTime Now();
    static void Adjust(const AOP::DateTime &);
};

} //namespace Hal;
