#pragma once

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include "ErrorCode.h"
#include "defs.h"

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

    MFRC522DriverPinSimple ss_pin = RFID_SS_PIN;
    MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
    MFRC522 mfrc522{driver};  // Create MFRC522 instance.
};
