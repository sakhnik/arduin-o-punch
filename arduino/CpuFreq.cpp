#ifdef ESP32

#include "CpuFreq.h"
#include <Arduino.h>

void SetCpuFreq(int mhz)
{
    setCpuFrequencyMhz(40);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));
}

#endif //ESP32
