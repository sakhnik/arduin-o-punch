#pragma once

#ifdef ARDUINO_AVR_PRO
#define LED_CONFIRM_PIN  4
#define BUZZER_PIN 5
#define RFID_SS_PIN 10
#endif //ARDUINO_AVR_PRO

#ifdef ESP32
#define LED_CONFIRM_PIN  2
#define BUZZER_PIN 3
#define RFID_SS_PIN 7
#endif //ESP32
