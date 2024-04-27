#pragma once

#include <Arduino.h>

class Context;
class Buzzer;
class DateTime;

class Shell
{
public:
    Shell(Context &, Buzzer &);

    void Setup();
    void OnSerial();
    boolean Tick();

private:
    Context &_context;
    Buzzer &_buzzer;
    String _buffer;
    uint8_t _echo_idx = 0;
    uint32_t _echo_timeout = 0;

    void _PrintPrompt();
    void _Process();
    void _PrintId();
    void _SetId(const char *param);
    void _SetKey(const char *hex);
    void _PrintKey();
    void _SetClock(const char *str);
    void _PrintClock(const DateTime &time);
    void _SetTimestamp(const char *str);
    void _PrintTimestamp();
    void _PrintDate(const DateTime &time);
    void _PrintTime(const DateTime &time);
    void _RecorderFormat(const char *str);
    void _RecorderClear(const char *str);
    void _RecorderCheck(const char *str);
    void _RecorderList();
    void _PrintRecordRetainDays();
    void _SetRecordRetainDays(const char *str);
};
