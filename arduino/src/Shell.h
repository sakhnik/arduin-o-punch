#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class Context;
class Buzzer;
class DateTime;
class OutMux;

class Shell
{
public:
    Shell(OutMux &, Context &, Buzzer &);

    void Setup();

    void ProcessInput(const uint8_t *data, int size);
    void PutChar(char ch);

    void SetId(const char *param);
    void SetKey(const char *hex);
    void RecorderFormat(const char *str);
    void SetRecordRetainDays(const char *str);

private:
    OutMux &_outMux;
    Context &_context;
    Buzzer &_buzzer;

    QueueHandle_t _rxQueue;

    static void TaskEntry(void *arg);
    void _Task();

    void _PrintPrompt();
    void _Process(const String &);
    void _PrintId();
    void _PrintKey();
    void _SetClock(const char *str);
    void _PrintClock(const DateTime &time);
    void _SetTimestamp(const char *str);
    void _PrintTimestamp();
    void _PrintDate(const DateTime &time);
    void _PrintTime(const DateTime &time);
    void _RecorderClear(const char *str);
    void _RecorderCheck(const char *str);
    void _RecorderList();
    void _PrintRecordRetainDays();
    void _PrintDD(uint8_t d);
    void _SetWifiSsid(const char *str);
    void _PrintWifiSsid();
    void _SetWifiPass(const char *str);
    void _PrintWifiPass();
};
