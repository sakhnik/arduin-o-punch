#pragma once

#include "aop/DateTime.hpp"

namespace HAL {

class RTC
{
public:
    static bool Setup();
    static AOP::DateTime Now();
    static void Adjust(const AOP::DateTime &);
};

} //namespace HAL;
