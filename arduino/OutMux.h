#pragma once

#include <Arduino.h>

class OutMux : public HardwareSerial
{
public:
    OutMux();

private:
    size_t write(const uint8_t *buffer, size_t size) override;
};
