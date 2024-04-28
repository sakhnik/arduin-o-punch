#pragma once

#include "OutMux.h"

class Context;
class Shell;

class Bluetooth
    : private OutMux::IClient
{
public:
    Bluetooth(OutMux &, Context &, Shell &);

    void Setup();
    void Toggle();
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
