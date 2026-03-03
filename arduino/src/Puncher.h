#pragma once

#include <MFRC522v2.h>
#include <MFRC522DriverI2C.h>
#include <MFRC522DriverPinSimple.h>
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

    MFRC522DriverI2C driver{0x3c, Wire};
    MFRC522 mfrc522{driver};  // Create MFRC522 instance.
};
