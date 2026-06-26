#pragma once

#include "ErrorCode.h"

class Settings;
class Operation;
class Buzzer;
class OutMux;

class Puncher
{
public:
    Puncher(Settings &, Operation &, Buzzer &, OutMux &);
    void Setup();
    ErrorCode Punch();

    void AntennaOff();
    void AntennaOn();

private:
    Settings &_settings;
    Operation &_operation;
    Buzzer &_buzzer;
    OutMux &_outMux;

    ErrorCode DoPunch();
    ErrorCode DoReadOut();
    ErrorCode DoFormat();
};
