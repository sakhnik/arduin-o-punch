#pragma once

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include "src/ErrorCode.h"

class Context;

class Puncher
{
public:
    Puncher(Context &context);
    void Setup();
    ErrorCode Punch();

private:
    Context &_context;

    MFRC522DriverPinSimple ss_pin = 10; // Configurable, see typical pin layout above.
    MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
    MFRC522 mfrc522{driver};  // Create MFRC522 instance.
};
