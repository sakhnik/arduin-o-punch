#include "Settings.h"
#include "Buzzer.h"
#include <RTClib.h>
#include <flex_DST.h>
#include <stddef.h>
#include <Preferences.h>

// Should always keep non-DST time
extern RTC_DS3231 rtc;

namespace {

Preferences prefs;

// EEST DST rules: last Sunday of March to last Sunday of October
flex_DST dst{5, 3, 5, 10};

constexpr const size_t EEPROM_SIZE = 32 * 1024;

struct EepromImpl : AOP::Recorder::IEeprom
{
    uint8_t mem[EEPROM_SIZE];

    uint8_t Read(uint16_t addr) override
    {
        return mem[addr];
    }

    void Write(uint16_t addr, uint8_t val) override
    {
        mem[addr] = val;
    }
} eeprom_impl;

constexpr const char *const PREF_CONFIG = "config";
constexpr const char *const PREF_ID = "id";
constexpr const char *const PREF_KEY = "key";
constexpr const char *const PREF_T_ACT_M = "t-act";
constexpr const char *const PREF_T_ECO_M = "t-eco";
constexpr const char *const PREF_RECDAYS = "recdays";
constexpr const char *const PREF_WIFI_SSID = "wifi-ssid";
constexpr const char *const PREF_WIFI_PASS = "wifi-pass";

} //namespace;

#define ADDROF(field) (Settings::ADDRESS + offsetof(Settings::_Data, field))

Settings::Settings(Buzzer &buzzer)
    : _buzzer{&buzzer}
    , _recorder{eeprom_impl}
{
}

int8_t Settings::Setup()
{
    prefs.begin(PREF_CONFIG, true);
    _id = prefs.getUChar(PREF_ID, 1);
    prefs.getBytes(PREF_KEY, _key.data(), KEY_SIZE);
    _active_minutes = prefs.getUShort(PREF_T_ACT_M, DEFAULT_ACTIVE_MINUTES);
    _active_ms = 60000ul * _active_minutes;
    _eco_minutes = prefs.getUShort(PREF_T_ECO_M, DEFAULT_ECO_MINUTES);
    _eco_ms = 60000ul * _eco_minutes;
    _record_retain_days = prefs.getUChar(PREF_RECDAYS, 1);
    _wifi_ssid = prefs.getString(PREF_WIFI_SSID).c_str();
    _wifi_pass = prefs.getString(PREF_WIFI_PASS).c_str();
    prefs.end();

    // Restore current record
    _recorder.Setup(0, EEPROM_SIZE);

    if (_record_retain_days > 0) {
        // If record is older than the retain period, reformat.
        uint32_t timestamp = rtc.now().unixtime();
        uint16_t cur_day = timestamp / _recorder.SECONDS_IN_DAY;
        if (cur_day - _recorder.GetFormatDay() >= _record_retain_days) {
            _recorder.Format(_recorder.GetSize(), _recorder.GetBitsPerRecord(), timestamp);
        }
    }

    return 0;
}

bool Settings::IsKeyDefault()
{
    LockGuard lock{_dataMx};
    for (int i = 0; i < KEY_SIZE; ++i) {
        if (_key[i] != 0xff)
            return false;
    }
    return true;
}

Settings::KeyT Settings::GetKey()
{
    LockGuard lock{_dataMx};
    Serial.println();
    return _key;
}

void Settings::SetKey(std::string_view skey)
{
    KeyT key = {};
    memcpy(key.data(), skey.data(), std::min(KEY_SIZE, skey.size()));

    {
        LockGuard lock{_dataMx};
        if (_key == key)
            return;
        _key = key;
        prefs.begin(PREF_CONFIG, false);
        prefs.putBytes(PREF_KEY, _key.data(), KEY_SIZE);
        prefs.end();
    }

    NotifyWatchers();
}

DateTime Settings::GetDateTime() const
{
    return dst.calculateTime(rtc.now());
}

uint32_t Settings::GetClock(const DateTime *date_time) const
{
    if (!date_time) {
        auto now = dst.calculateTime(rtc.now());
        return GetClock(&now);
    }
    // TODO: subsecond resolution
    uint32_t clock = date_time->hour();
    clock = clock * 60 + date_time->minute();
    clock = clock * 60 + date_time->second();
    // We take milliseconds from arduino clock. This is only useful if we copare
    // close clock readings. Otherwise, the milliseconds should be ignored.
    return clock * 1000 + millis() % 1000;
}

void Settings::SetClock(uint32_t clock)
{
    auto now = dst.calculateTime(rtc.now());
    //auto ms = clock % 1000;
    clock /= 1000;
    auto sec = clock % 60;
    clock /= 60;
    auto min = clock % 60;
    clock /= 60;
    DateTime adjusted(now.year(), now.month(), now.day(), clock, min, sec);
    if (dst.checkDST(adjusted)) {
        adjusted = adjusted.unixtime() - 3600;
    }
    rtc.adjust(adjusted);
}

void Settings::SetDateTime(uint32_t timestamp)
{
    DateTime adjusted{timestamp};
    if (dst.checkDST(adjusted)) {
        adjusted = adjusted.unixtime() - 3600;
    }
    rtc.adjust(adjusted);
}

uint8_t Settings::GetId()
{
    LockGuard lock{_dataMx};
    return _id;
}

void Settings::SetId(uint8_t id)
{
    {
        LockGuard lock{_dataMx};
        if (_id == id)
            return;
        _id = id;
        prefs.begin(PREF_CONFIG, false);
        prefs.putUChar(PREF_ID, _id);
        prefs.end();
    }

    NotifyWatchers();
}

uint32_t Settings::GetActiveMinutes()
{
    LockGuard lock{_dataMx};
    return _active_minutes;
}

uint32_t Settings::GetActiveMs()
{
    LockGuard lock{_dataMx};
    return _active_ms;
}

void Settings::SetActiveMinutes(uint32_t minutes)
{
    {
        LockGuard lock{_dataMx};
        if (_active_minutes == minutes)
            return;
        _active_minutes = minutes;
        _active_ms = 60000ul * _active_minutes;
        prefs.begin(PREF_CONFIG, false);
        prefs.putUShort(PREF_T_ACT_M, _active_minutes);
        prefs.end();
    }

    NotifyWatchers();
}

uint32_t Settings::GetEcoMinutes()
{
    LockGuard lock{_dataMx};
    return _eco_minutes;
}

uint32_t Settings::GetEcoMs()
{
    LockGuard lock{_dataMx};
    return _eco_ms;
}

void Settings::SetEcoMinutes(uint32_t minutes)
{
    {
        LockGuard lock{_dataMx};
        if (_eco_minutes == minutes)
            return;
        _eco_minutes = minutes;
        _eco_ms = 60000ul * _eco_minutes;
        prefs.begin(PREF_CONFIG, false);
        prefs.putUShort(PREF_T_ECO_M, _eco_minutes);
        prefs.end();
    }

    NotifyWatchers();
}

int8_t Settings::GetRecordRetainDays()
{
    LockGuard lock{_dataMx};
    return _record_retain_days;
}

void Settings::SetRecordRetainDays(uint8_t days)
{
    {
        LockGuard lock{_dataMx};
        if (_record_retain_days == days)
            return;
        _record_retain_days = days;
        prefs.begin(PREF_CONFIG, false);
        prefs.putUChar(PREF_RECDAYS, days);
        prefs.end();
    }

    NotifyWatchers();
}

std::string Settings::GetWifiSsid()
{
    LockGuard lock{_dataMx};
    return _wifi_ssid;
}

void Settings::SetWifiSsid(std::string_view ssid)
{
    {
        LockGuard lock{_dataMx};
        if (_wifi_ssid == ssid)
            return;
        _wifi_ssid = ssid;
        prefs.begin(PREF_CONFIG, false);
        prefs.putString(PREF_WIFI_SSID, _wifi_ssid.c_str());
        prefs.end();
    }

    NotifyWatchers();
}

std::string Settings::GetWifiPass()
{
    LockGuard lock{_dataMx};
    return _wifi_pass;
}

void Settings::SetWifiPass(std::string_view pass)
{
    {
        LockGuard lock{_dataMx};
        if (_wifi_pass == pass)
            return;
        _wifi_pass = pass;
        prefs.begin(PREF_CONFIG, false);
        prefs.putString(PREF_WIFI_PASS, _wifi_pass.c_str());
        prefs.end();
    }

    NotifyWatchers();
}

size_t Settings::Subscribe(OnChangeT on_change)
{
    LockGuard lock{_watchersMx};
    auto it = std::find_if(watchers.begin(), watchers.end(), [](OnChangeT w) { return !w; });
    if (it != watchers.end()) {
        *it = on_change;
    }
    return it - watchers.begin();
}

void Settings::Unsubscribe(size_t idx)
{
    LockGuard lock{_watchersMx};
    if (idx < watchers.size())
        watchers[idx] = {};
}

void Settings::NotifyWatchers()
{
    LockGuard lock{_watchersMx};
    for (const auto &w : watchers) {
        if (w) {
            w();
        }
    }
}
