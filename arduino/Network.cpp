#ifdef ESP32

#include "Network.h"
#include "Context.h"
#include "Shell.h"
#include "Buzzer.h"
#include "CpuFreq.h"
#include <WiFi.h>

Network::Network(OutMux &outMux, Context &context, Shell &shell, Buzzer &buzzer)
    : _outMux{outMux}
    , _context{context}
    , _shell{shell}
    , _buzzer{buzzer}
{
}

void Network::Setup()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
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

    // Signal with dit every second until connected to the network
    if (WiFi.status() != WL_CONNECTED) {
        _connection_signalled = false;
        auto now = millis();
        if (now - _last_connecting_dit > 1000) {
            _last_connecting_dit = now;
            _buzzer.SignalDit();
        }
        return;
    }

    // Confirm connection with dah
    if (!_connection_signalled) {
        _connection_signalled = true;
        _buzzer.SignalDah();
        Serial.println("Connected");
        Serial.print("Local IP: ");
        Serial.println(WiFi.localIP());
    }
}

bool Network::_Start()
{
    SetCpuFreq(80);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_context.GetWifiSsid(), _context.GetWifiPass());
    _last_connecting_dit = millis();
    _connection_signalled = false;
    return true;
}

bool Network::_Stop()
{
    _outMux.SetClient(nullptr);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    SetCpuFreq(40);
    return false;
}

void Network::Write(const uint8_t *buffer, size_t size)
{
}

#endif
