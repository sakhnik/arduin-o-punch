#pragma once

#include "Recorder.h"
#include <RTClib.h>
#include <array>
#include <functional>
#include <string_view>

class Buzzer;

class Context
{
public:
    Context(Buzzer &buzzer);

    Buzzer& GetBuzzer() const { return *_buzzer; }

    int8_t Setup();

    static constexpr const size_t KEY_SIZE = 6;
    using KeyT = std::array<uint8_t, KEY_SIZE>;
    const KeyT& GetKey() const { return _key; }
    bool IsKeyDefault() const;
    void OnNewKey(std::string_view key);

    DateTime GetDateTime() const;
    uint32_t GetClock(const DateTime *date_time) const;
    void SetClock(uint32_t clock);
    void SetDateTime(uint32_t timestamp);

    uint8_t GetId() const { return _id; }
    void SetId(uint8_t id);

    AOP::Recorder& GetRecorder() { return _recorder; }

    int8_t GetRecordRetainDays() const { return _record_retain_days; }
    void SetRecordRetainDays(uint8_t days);

    const char *GetWifiSsid() const { return _wifi_ssid.c_str(); }
    void SetWifiSsid(std::string_view ssid);

    const char *GetWifiPass() const { return _wifi_pass.c_str(); }
    void SetWifiPass(std::string_view pass);

    using OnChangeT = std::function<void(void)>;
    size_t Subscribe(OnChangeT);
    void Unsubscribe(size_t);

private:
    Buzzer *_buzzer;
    std::array<OnChangeT, 4> watchers = {};

    uint8_t _id = 1;
    KeyT _key = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t _record_retain_days = 1;
    std::string _wifi_ssid;
    std::string _wifi_pass;

    AOP::Recorder _recorder;

    void NotifyWatchers();

    friend struct RecorderAccess;
};
