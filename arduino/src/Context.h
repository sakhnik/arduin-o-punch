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
    uint32_t GetActiveMinutes();
    void SetActiveMinutes(uint32_t minutes);
    uint32_t GetActiveMs();

    // Polling with light sleep before going to the deep sleep
    uint32_t GetEcoMinutes();
    void SetEcoMinutes(uint32_t minutes);
    uint32_t GetEcoMs();

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

    static constexpr const uint32_t DEFAULT_ACTIVE_MINUTES = 10;
    uint32_t _active_minutes = DEFAULT_ACTIVE_MINUTES;
    uint32_t _active_ms = 60000ul * _active_minutes;
    static constexpr const uint32_t DEFAULT_ECO_MINUTES = 3 * 60;
    uint32_t _eco_minutes = DEFAULT_ECO_MINUTES;
    uint32_t _eco_ms = 60000ul * _eco_minutes;

    uint8_t _record_retain_days = 1;
    std::string _wifi_ssid;
    std::string _wifi_pass;

    AOP::Recorder _recorder;

    void NotifyWatchers();

    friend struct RecorderAccess;
};
