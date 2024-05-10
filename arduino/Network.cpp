#ifdef ESP32

#include "Network.h"
#include "Context.h"
#include "Shell.h"
#include "Buzzer.h"
#include "CpuFreq.h"
#include "src/Utils.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

WiFiServer shellServer{23};
WiFiClient shellClient;
WebServer webServer{80};

extern const char *index_html PROGMEM;

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

    webServer.on("/", [] {
        webServer.send(200, "text/html", index_html);
    });
    webServer.onNotFound([]() {
        webServer.send(200, "text/html", index_html);
    });
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
        shellServer.begin();
        webServer.begin();
    }

    if (!shellClient) {
        shellClient = shellServer.available();
        if (shellClient) {
            _outMux.SetClient(this);
        }
    }
    if (shellClient) {
        if (!shellClient.connected()) {
            shellClient.stop();
            _outMux.SetClient(nullptr);
        } else {
            if (shellClient.available()) {
                uint8_t buf[32];
                int bytesRead = shellClient.read(buf, sizeof(buf));
                if (bytesRead > 0) {
                    _shell.ProcessInput(buf, bytesRead);
                }
            }
        }
    }

    webServer.handleClient();
}

bool Network::_Start()
{
    SetCpuFreq(80);
    char hostname[64] = "aop";
    *AOP::PrintNum(_context.GetId(), hostname + 3) = 0;
    Serial.print("hostname: ");
    Serial.println(hostname);
    MDNS.begin(hostname);
    WiFi.setHostname(hostname);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(_context.GetWifiSsid(), _context.GetWifiPass());
    WiFi.softAP(hostname);
    _last_connecting_dit = millis();
    _connection_signalled = false;
    return true;
}

bool Network::_Stop()
{
    _outMux.SetClient(nullptr);
    shellClient.stop();
    shellServer.end();
    webServer.stop();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    SetCpuFreq(40);
    return false;
}

void Network::Write(const uint8_t *buffer, size_t size)
{
    if (shellClient && shellClient.connected()) {
        shellClient.write(buffer, size);
    }
}

#endif
