#pragma once

#ifdef ESP32

#include "OutMux.h"

class Context;
class Shell;

class Network
    : private OutMux::IClient
{
public:
    Network(OutMux &, Context &, Shell &);

    void Setup();
    void SwitchOn();
    void SwitchOff();
    void Tick();

private:
    OutMux &_outMux;
    Context &_context;
    Shell &_shell;
    bool _is_active = false;

    bool _Start();
    bool _Stop();

    // MuxOut::IClient
    void Write(const uint8_t *buffer, size_t size) override;
};

#endif //ESP32
