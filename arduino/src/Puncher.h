#pragma once

#include "ErrorCode.h"

class Context;

class Puncher
{
public:
    Puncher(Context &context);
    void Setup();
    ErrorCode Punch();

    void AntennaOff();
    void AntennaOn();

private:
    Context &_context;
};
