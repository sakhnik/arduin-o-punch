#include "CpuFreq.h"
#include <Arduino.h>

void SetCpuFreq(int mhz)
{
    setCpuFrequencyMhz(mhz);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));
}
