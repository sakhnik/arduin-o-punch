#pragma once

#ifdef ESP32

#include "OutMux.h"

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
    void Tick();

private:
    OutMux &_outMux;
    Context &_context;
    Shell &_shell;
    Buzzer &_buzzer;
    bool _is_active = false;
    unsigned long _last_connecting_dit = 0;
    bool _connection_signalled = false;

    bool _Start();
    bool _Stop();

    // MuxOut::IClient
    void Write(const uint8_t *buffer, size_t size) override;

    void _HandleGetSettings();
    void _HandleSettings();
    void _HandleRecord();
};

#endif //ESP32
