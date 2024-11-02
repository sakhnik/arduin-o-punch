#pragma once

#include <Arduino.h>

class Logger
{
    String buf;

public:
    Logger();

    void Clear();
    void Print();
    void Out(uint16_t num);
    void Out(const char *s);
    void Out(const uint8_t *data, uint8_t size);
};
