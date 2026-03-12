#include "Context.h"
#include "Buzzer.h"
#include <ESP32Time.h>
#include <flex_DST.h>
#include <stddef.h>
#include <Preferences.h>

namespace {

Preferences prefs;

// Should always keep non-DST time
ESP32Time rtc{3600 * 2};  // GMT+2
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
constexpr const char *const PREF_RECDAYS = "recdays";
constexpr const char *const PREF_WIFI_SSID = "wifi-ssid";
constexpr const char *const PREF_WIFI_PASS = "wifi-pass";

} //namespace;

#define ADDROF(field) (Context::ADDRESS + offsetof(Context::_Data, field))

Context::Context(Buzzer &buzzer)
    : _buzzer{&buzzer}
    , _recorder{eeprom_impl}
{
}

int8_t Context::Setup()
{
    prefs.begin(PREF_CONFIG, true);
    _id = prefs.getUChar(PREF_ID, 1);
    prefs.getBytes(PREF_KEY, _key, KEY_SIZE);
    _id = prefs.getUChar(PREF_RECDAYS, 1);
    _wifi_ssid = prefs.getString(PREF_WIFI_SSID);
    _wifi_pass = prefs.getString(PREF_WIFI_PASS);
    prefs.end();

    // Restore current record
    _recorder.Setup(0, EEPROM_SIZE);

    if (_record_retain_days > 0) {
        // If record is older than the retain period, reformat.
        uint32_t timestamp = rtc.getEpoch();
        uint16_t cur_day = timestamp / _recorder.SECONDS_IN_DAY;
        if (cur_day - _recorder.GetFormatDay() >= _record_retain_days) {
            _recorder.Format(_recorder.GetSize(), _recorder.GetBitsPerRecord(), timestamp);
        }
    }

    return 0;
}

bool Context::IsKeyDefault() const
{
    for (int i = 0; i < KEY_SIZE; ++i) {
        if (_key[i] != 0xff)
            return false;
    }
    return true;
}

void Context::OnNewKey(const uint8_t *key)
{
    memcpy(_key, key, sizeof(_key));
    prefs.begin(PREF_CONFIG, false);
    prefs.putBytes(PREF_KEY, _key, KEY_SIZE);
    prefs.end();
}

DateTime Context::GetDateTime() const
{
    return dst.calculateTime(DateTime{rtc.getEpoch()});
}

uint32_t Context::GetClock(const DateTime *date_time) const
{
    if (!date_time) {
        auto now = GetDateTime();
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

void Context::SetClock(uint32_t clock)
{
    auto now = GetDateTime();
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
    rtc.setTime(adjusted.unixtime());
}

void Context::SetDateTime(uint32_t timestamp)
{
    DateTime adjusted{timestamp};
    if (dst.checkDST(adjusted)) {
        adjusted = adjusted.unixtime() - 3600;
    }
    rtc.setTime(adjusted.unixtime());
}

void Context::SetId(uint8_t id)
{
    _id = id;
    prefs.begin(PREF_CONFIG, false);
    prefs.putUChar(PREF_ID, _id);
    prefs.end();
}

void Context::SetRecordRetainDays(uint8_t days)
{
    _record_retain_days = days;
    prefs.begin(PREF_CONFIG, false);
    prefs.putUChar(PREF_RECDAYS, days);
    prefs.end();
}

void Context::SetWifiSsid(const char *ssid)
{
    _wifi_ssid = ssid;
    prefs.begin(PREF_CONFIG, false);
    prefs.putString(PREF_WIFI_SSID, _wifi_ssid);
    prefs.end();
}

void Context::SetWifiPass(const char *pass)
{
    _wifi_pass = pass;
    prefs.begin(PREF_CONFIG, false);
    prefs.putString(PREF_WIFI_PASS, _wifi_pass);
    prefs.end();
}
