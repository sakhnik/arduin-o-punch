#pragma once

#include <Arduino.h>

class Context;
class Buzzer;

class Shell
{
public:
    Shell(Context &context, Buzzer &buzzer)
        : _context{context}
        , _buzzer{buzzer}
    {
    }

    void Setup();
    void OnSerial();
    boolean Tick();

private:
    Context &_context;
    Buzzer &_buzzer;
    String _buffer;
    uint8_t _echo_idx = 0;
    uint32_t _echo_timeout = 0;

    void _Process();
    void _PrintId();
    void _SetId(const char *id);
    void _SetKey(const char *key);
    void _PrintKey();
    void _SetClock(const char *clock);
    void _PrintClock();
};
