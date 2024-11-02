#pragma once

#include "Recorder.h"
#include <Arduino.h>
#include <RTClib.h>

class Buzzer;

class Context
{
public:
    static constexpr const int ADDRESS = 0;

    Context(Buzzer &buzzer);

    int8_t Setup();

    static constexpr const uint8_t KEY_SIZE = 6;
    const uint8_t* GetKey() const
    {
        return _key;
    }
    bool IsKeyDefault() const;
    void OnNewKey(const uint8_t *key);

    DateTime GetDateTime() const;
    uint32_t GetClock(const DateTime *date_time) const;
    void SetClock(uint32_t clock);
    void SetDateTime(uint32_t timestamp);

    uint8_t GetId() const
    {
        return _id;
    }
    void SetId(uint8_t id);

    AOP::Recorder& GetRecorder()
    {
        return _recorder;
    }

    int8_t GetRecordRetainDays() const
    {
        return _record_retain_days;
    }

    void SetRecordRetainDays(uint8_t days);

#ifdef ESP32
    const char *GetWifiSsid() const
    {
        return _wifi_ssid;
    }

    void SetWifiSsid(const char *ssid);

    const char *GetWifiPass() const
    {
        return _wifi_pass;
    }

    void SetWifiPass(const char *pass);
#endif //ESP32

private:
    Buzzer &_buzzer;
    uint8_t _id = 1;
    uint8_t _key[KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t _reserved0 = 0xff;
    uint8_t _record_retain_days = 1;
#ifdef ESP32
    char _wifi_ssid[16] = {};
    char _wifi_pass[16] = {};
#endif //ESP32
    uint8_t _reserved[7];
    AOP::Recorder _recorder;

    friend struct RecorderAccess;
};
