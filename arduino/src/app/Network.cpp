#include "Network.hpp"
#ifdef ESP32
# include "../esp32/Network.hpp"
#elif defined(NATIVE)
# include "../native/Network.hpp"
#else
# include <climits>
#endif

namespace {

#if defined(ESP32)
using ImplT = Esp32::Network;
#elif defined(NATIVE)
using ImplT = Native::Network;
#else

struct Impl
{
    Impl(OutMux &, Settings &, Shell &, Buzzer &) { }

    void Setup() { }
    void SwitchOn() { }
    void SwitchOff() { }
    unsigned GetInactivitySeconds() { return UINT_MAX; }
};
using ImplT = Impl;

#endif

} //namespace;

Network::Network(OutMux &outMux, Settings &settings, Shell &shell, Buzzer &buzzer)
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    : _impl{new ImplT(outMux, settings, shell, buzzer)}
#endif
{
}

Network::~Network()
{
    delete static_cast<ImplT *>(_impl);
}

#if defined(ENABLE_WIFI) && ENABLE_WIFI

void Network::Setup()
{
    static_cast<ImplT *>(_impl)->Setup();
}

void Network::SwitchOn()
{
    static_cast<ImplT *>(_impl)->SwitchOn();
}

void Network::SwitchOff()
{
    static_cast<ImplT *>(_impl)->SwitchOff();
}

unsigned Network::GetInactivitySeconds()
{
    return static_cast<ImplT *>(_impl)->GetInactivitySeconds();
}

#endif
