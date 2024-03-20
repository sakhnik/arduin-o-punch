#include "Logger.h"

namespace {
constexpr const char *const DIGITS = "0123456789ABCDEF";
} //namespace;

Logger::Logger()
{
    buf.reserve(128);
}

void Logger::Clear()
{
    buf = "";
}

void Logger::Print()
{
    Serial.println(buf.c_str());
}

void Logger::Out(uint16_t num)
{
    uint16_t hi = num / 10;
    if (hi)
        Out(hi);
    buf += DIGITS[num % 10];
}

void Logger::Out(const char *s)
{
    buf += s;
}

void Logger::Out(const uint8_t *data, uint8_t size)
{
    for (uint8_t i = 0; i < size; ++i) {
        buf += DIGITS[data[i] >> 4];
        buf += DIGITS[data[i] & 0xF];
    }
}
