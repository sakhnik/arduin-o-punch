#pragma once

#include "aop/DateTime.hpp"
#include "hal/queue.hpp"
#include "hal/task.hpp"
#include <string_view>

class Settings;
class Buzzer;
class Operation;
class DateTime;
class OutMux;

class Shell
{
public:
    Shell(OutMux &, Settings &, Buzzer &);

    void Setup();
    void SetOperation(Operation *);

    void ProcessInput(const uint8_t *data, int size);
    void PutChar(char ch);

    void SetId(std::string_view);
    void SetActive(std::string_view);
    void SetEco(std::string_view);
    void SetKey(std::string_view hex);

private:
    OutMux &_outMux;
    Settings &_settings;
    Buzzer &_buzzer;
    Operation *_operation = nullptr;

    HAL::Queue _rxQueue;
    HAL::Task _task;

    static void TaskEntry(void *arg);
    void _Task();

    void _PrintPrompt();
    void _Process(std::string_view);
    void _PrintId();
    void _PrintActive();
    void _PrintEco();
    void _PrintKey();
    void _SetClock(std::string_view);
    void _PrintClock(const AOP::DateTime &time);
    void _SetTimestamp(std::string_view);
    void _PrintTimestamp();
    void _PrintDate(const AOP::DateTime &time);
    void _PrintTime(const AOP::DateTime &time);
    void _RecorderCheck(std::string_view);
    void _RecorderList();
    void _PrintDD(uint8_t d);
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    void _SetWifiSsid(std::string_view);
    void _PrintWifiSsid();
    void _SetWifiPass(std::string_view);
    void _PrintWifiPass();
#endif
    void _PrintKnownKeys();
    void _SetKnownKeys(std::string_view);
};
