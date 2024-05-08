#pragma once

#if defined(BUILD_TEST) || defined(BUILD_WA)
# include <cstdint>
#else
# include <Arduino.h>
#endif

namespace AOP {

char* PrintNum(uint8_t num, char *buf);

} //namespace AOP;
