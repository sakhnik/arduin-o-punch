#pragma once

#include "ErrorCode.h"

class Settings;

class Puncher
{
public:
    Puncher(Settings &);
    void Setup();
    ErrorCode Punch();

    void AntennaOff();
    void AntennaOn();

private:
    Settings &_settings;
};
