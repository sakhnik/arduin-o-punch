#pragma once

#ifdef ESP32

#include "aop/ErrorCode.hpp"

class Settings;
class Operation;
class Buzzer;
class OutMux;

class Puncher
{
public:
    Puncher(Settings &, Operation &, Buzzer &, OutMux &);
    int Setup();
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

#endif //ESP32
