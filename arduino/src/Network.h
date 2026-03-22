#pragma once

#include "OutMux.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Context;
class Shell;
class Buzzer;

class Network
    : private OutMux::IClient
{
public:
    Network(OutMux &, Context &, Shell &, Buzzer &);

    void Setup();
    void SwitchOn();
    void SwitchOff();

private:
    OutMux &_outMux;
    Context &_context;
    Shell &_shell;
    Buzzer &_buzzer;
    unsigned long _last_connecting_dit = 0;
    bool _connection_signalled = false;
    TaskHandle_t _taskHandle = nullptr;
    SemaphoreHandle_t _wakeSignal = nullptr;

    bool _Start();
    bool _Stop();

    // MuxOut::IClient
    void Write(const uint8_t *buffer, size_t size) override;

    static void _TaskEntry(void* arg);
    void _Task();
    void _HandleGetSettings();
    void _HandleSettings();
    void _HandleRecord();
    void _HandleClock();
};
