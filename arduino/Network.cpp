#ifdef ESP32

#include "Network.h"
#include "Context.h"
#include "Shell.h"

Network::Network(OutMux &outMux, Context &context, Shell &shell)
    : _outMux{outMux}
    , _context{context}
    , _shell{shell}
{
}

namespace {

char* PrintNum(uint8_t num, char *buf)
{
    if (!num)
        return buf;
    buf = PrintNum(num / 10, buf);
    *buf = '0' + num % 10;
    return ++buf;
}

} // namespace;

void Network::Setup()
{
}

void Network::SwitchOn()
{
    if (!_is_active) {
        _Start();
        _is_active = true;
    }
}

void Network::SwitchOff()
{
    if (_is_active) {
        _Stop();
        _is_active = false;
    }
}

void Network::Tick()
{
    if (!_is_active) {
        return;
    }
}

bool Network::_Start()
{
    setCpuFrequencyMhz(80);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));

    return true;
}

bool Network::_Stop()
{
    _outMux.SetClient(nullptr);
    setCpuFrequencyMhz(40);
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMhz());
    Serial.println(F(" MHz"));
    return false;
}

void Network::Write(const uint8_t *buffer, size_t size)
{
}

#endif
