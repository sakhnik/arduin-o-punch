#pragma once

#include "Recorder.h"
#include "Mutex.h"

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
    KeyT GetKey();
    void SetKey(std::string_view key);
    bool IsKeyDefault();

    DateTime GetDateTime() const;
    uint32_t GetClock(const DateTime *date_time) const;
    void SetClock(uint32_t clock);
    void SetDateTime(uint32_t timestamp);

    uint8_t GetId();
    void SetId(uint8_t id);

    // Active polling time before going to the light sleep eco mode
    uint16_t GetTActS();
    void SetTActS(uint16_t seconds);
    uint32_t GetTActMs();

    // Polling with light sleep before going to the deep sleep
    uint32_t GetTEcoS();
    void SetTEcoS(uint32_t seconds);
    uint32_t GetTEcoMs();

    AOP::Recorder& GetRecorder() { return _recorder; }

    int8_t GetRecordRetainDays();
    void SetRecordRetainDays(uint8_t days);

    std::string GetWifiSsid();
    void SetWifiSsid(std::string_view ssid);

    std::string GetWifiPass();
    void SetWifiPass(std::string_view pass);

    using OnChangeT = std::function<void(void)>;
    size_t Subscribe(OnChangeT);
    void Unsubscribe(size_t);

private:
    Buzzer *_buzzer;
    Mutex _watchersMx;
    std::array<OnChangeT, 4> watchers = {};

    Mutex _dataMx;
    uint8_t _id = 1;
    KeyT _key = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    static constexpr const uint16_t DEFAULT_ACTIVE_SECONDS = 10 * 60;
    uint16_t _active_seconds = DEFAULT_ACTIVE_SECONDS;
    uint32_t _active_ms = _active_seconds * 1000;
    static constexpr const uint32_t DEFAULT_ECO_SECONDS = 3 * 60 * 60;
    uint32_t _eco_seconds = DEFAULT_ECO_SECONDS;
    uint32_t _eco_ms = _eco_seconds * 1000;
    uint8_t _record_retain_days = 1;
    std::string _wifi_ssid;
    std::string _wifi_pass;

    AOP::Recorder _recorder;

    void NotifyWatchers();

    friend struct RecorderAccess;
};
