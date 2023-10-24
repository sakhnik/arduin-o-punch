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

private:
    Context &_context;
    Buzzer &_buzzer;
    String _buffer;

    void _Process();
    void _SetKey(const char *key);
    void _PrintKey();
    void _SetClock(const char *clock);
    void _PrintClock();
};
