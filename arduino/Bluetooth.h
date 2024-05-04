#pragma once

#ifdef ESP32

#include "OutMux.h"
#include "src/RingBuffer.h"

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
    void Tick();

private:
    OutMux &_outMux;
    Context &_context;
    Shell &_shell;
    bool _is_active = false;
    AOP::RingBuffer<1024> _outBuffer;
    unsigned long _last_write_time = 0;

    bool _Start();
    bool _Stop();

    // MuxOut::IClient
    void Write(const uint8_t *buffer, size_t size) override;
};

#endif //ESP32
