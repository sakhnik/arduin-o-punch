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
#include <Update.h>

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

namespace {

void handleUpdateEnd()
{
    webServer.sendHeader("Connection", "close");
    if (Update.hasError()) {
        webServer.send(502, "text/plain", Update.errorString());
    } else {
        webServer.sendHeader("Refresh", "10");
        webServer.sendHeader("Location", "/");
        webServer.send(307);
        ESP.restart();
    }
}

void handleUpdate()
{
    size_t fsize = UPDATE_SIZE_UNKNOWN;
    if (webServer.hasArg("size")) {
        fsize = webServer.arg("size").toInt();
    }
    HTTPUpload &upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Receiving Update: %s, Size: %d\n", upload.filename.c_str(), fsize);
        if (!Update.begin(fsize)) {
            //otaDone = 0;
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        } else {
            //otaDone = 100 * Update.progress() / Update.size();
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
        } else {
            Serial.printf("%s\n", Update.errorString());
            //otaDone = 0;
        }
    }
}

} //namespace;

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
    webServer.on("/update", HTTP_POST, handleUpdateEnd, handleUpdate);
    webServer.on("/settings", HTTP_ANY, [this] { _HandleSettings(); });
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

void Network::_HandleGetSettings()
{
    String response;
    const char *nl = "";
    auto addSetting = [&response](const char *key, const String &val) {
        if (response.length())
            response += "\n";
        response += key;
        response += "=";
        response += val;
    };
    addSetting("id", String{static_cast<unsigned>(_context.GetId())});
    const uint8_t *key = _context.GetKey();
    char buf[Context::KEY_SIZE * 2 + 1];
    for (int i = 0; i < Context::KEY_SIZE; ++i) {
        sprintf(buf + i*2, "%02X", static_cast<unsigned>(key[i]));
    }
    buf[Context::KEY_SIZE * 2] = 0;
    addSetting("key", buf);
    addSetting("rec-size", String{_context.GetRecorder().GetSize()});
    addSetting("rec-bits", String{_context.GetRecorder().GetBitsPerRecord()});
    addSetting("rec-days", String{_context.GetRecordRetainDays()});
    webServer.send(200, "text/plain", response);
}

void Network::_HandleSettings()
{
    if (webServer.method() == HTTP_POST) {
        _shell.SetId(webServer.arg("id").c_str());
        _shell.SetKey(webServer.arg("key").c_str());
        String recorder = webServer.arg("rec-size") + " " + webServer.arg("rec-bits");
        _shell.RecorderFormat(recorder.c_str());
        _shell.SetRecordRetainDays(webServer.arg("rec-days").c_str());
        webServer.send(200, "text/html", index_html);
    } else if (webServer.method() == HTTP_GET) {
        _HandleGetSettings();
    } else {
        webServer.send(405);
    }
}

#endif
