#pragma once

class Context;
class Shell;

class Bluetooth
{
public:
    Bluetooth(Context &, Shell &);

    void Setup();
    void Toggle();
    void Tick();

private:
    Context &_context;
    Shell &_shell;
    bool _is_active = false;

    bool _Start();
    bool _Stop();
};
