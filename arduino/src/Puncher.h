#pragma once

#include "ErrorCode.h"

class Settings;
class Operation;
class Buzzer;

class Puncher
{
public:
    Puncher(Settings &, Operation &, Buzzer &);
    void Setup();
    ErrorCode Punch();

    void AntennaOff();
    void AntennaOn();

private:
    Settings &_settings;
    Operation &_operation;
    Buzzer &_buzzer;

    ErrorCode DoPunch();
    ErrorCode DoReadOut();
    ErrorCode DoFormat();
};
