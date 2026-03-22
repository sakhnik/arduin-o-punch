#pragma once

#include "OutMux.h"
#include "RingBuffer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Context;
class Shell;

class Bluetooth
    : private OutMux::IClient
{
public:
    Bluetooth(OutMux &, Context &, Shell &);

    void Setup();
    void SwitchOn();
    void SwitchOff();

private:
    OutMux &_outMux;
    Context &_context;
    Shell &_shell;
    bool _is_active = false;
    AOP::RingBuffer<1024> _outBuffer;
    unsigned long _last_write_time = 0;
    SemaphoreHandle_t _txMutex = nullptr;
    SemaphoreHandle_t _txSignal = nullptr;
    TaskHandle_t _taskHandle = nullptr;

    static void _TaskEntry(void *arg);
    void _Task();
    bool _Start();
    bool _Stop();

    // MuxOut::IClient
    void Write(const uint8_t *buffer, size_t size) override;
};
