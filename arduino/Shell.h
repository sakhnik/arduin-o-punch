#pragma once

#include <Arduino.h>

class Context;

class Shell
{
public:
    Shell(Context &);
    void Setup();
    void OnSerial();

private:
    Context &_context;
    String _buffer;

    void _Process();
    void _SetKey(const char *key);
    void _PrintKey();
};
