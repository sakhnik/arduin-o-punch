#pragma once

#include <Arduino.h>

class Shell
{
public:
    void Setup();
    void OnSerial();

private:
    String _buffer;

    void _Process();
};
