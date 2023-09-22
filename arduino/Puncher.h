#pragma once

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

class Puncher
{
public:
    void Setup();
    uint8_t Punch();

    enum ErrorCodes : uint8_t
    {
        E_OK = 0,

        E_NO_CARD = 0x80,
        E_NO_SERIAL,
        E_WRONG_CARD,
    };

private:
    MFRC522DriverPinSimple ss_pin = 10; // Configurable, see typical pin layout above.
    MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
    MFRC522 mfrc522{driver};  // Create MFRC522 instance.
};
