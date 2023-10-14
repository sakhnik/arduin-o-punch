#pragma once

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include "src/ErrorCode.h"

class Puncher
{
public:
    Puncher(uint8_t *key_receiver = nullptr);
    void Setup();
    ErrorCode Punch(const uint8_t *key);

private:
    uint8_t *_key_receiver;

    MFRC522DriverPinSimple ss_pin = 10; // Configurable, see typical pin layout above.
    MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
    MFRC522 mfrc522{driver};  // Create MFRC522 instance.
};
