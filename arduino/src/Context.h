#pragma once

#include "Recorder.h"
#include <Arduino.h>
#include <RTClib.h>

class Buzzer;

class Context
{
public:
    Context(Buzzer &buzzer);

    int8_t Setup();

    static constexpr const uint8_t KEY_SIZE = 6;
    const uint8_t* GetKey() const
    {
        return _data._key;
    }
    bool IsKeyDefault() const;
    void OnNewKey(const uint8_t *key);

    DateTime GetDateTime() const;
    uint32_t GetClock(const DateTime *date_time) const;
    void SetClock(uint32_t clock);
    void SetDateTime(uint32_t timestamp);

    uint8_t GetId() const
    {
        return _data._id;
    }
    void SetId(uint8_t id);

    AOP::Recorder& GetRecorder()
    {
        return _recorder;
    }

    int8_t GetRecordRetainDays() const
    {
        return _data._record_retain_days;
    }

    void SetRecordRetainDays(uint8_t days);

#ifdef ESP32
    const char *GetWifiSsid() const
    {
        return _data._wifi_ssid;
    }

    void SetWifiSsid(const char *ssid);

    const char *GetWifiPass() const
    {
        return _data._wifi_pass;
    }

    void SetWifiPass(const char *pass);
#endif //ESP32

private:
    Buzzer *_buzzer;

    static constexpr const int ADDRESS = sizeof(Buzzer *);
    struct _Data
    {
        uint8_t _id = 1;
        uint8_t _key[KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        uint8_t _reserved0 = 0xff;
        uint8_t _record_retain_days = 1;
        char _wifi_ssid[16] = {};
        char _wifi_pass[16] = {};
        uint8_t _reserved[7];
    };

    _Data _data;
    AOP::Recorder _recorder;

    friend struct RecorderAccess;
};
