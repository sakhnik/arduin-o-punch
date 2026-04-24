#pragma once

#include "ErrorCode.h"

class Settings;
class Operation;

class Puncher
{
public:
    Puncher(Settings &, Operation &);
    void Setup();
    ErrorCode Punch();

    void AntennaOff();
    void AntennaOn();

private:
    Settings &_settings;
    Operation &_operation;
};
