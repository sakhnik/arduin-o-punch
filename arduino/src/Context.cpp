#include "Context.h"
#include "Buzzer.h"
#include <extEEPROM.h>
#include <RTClib.h>
#include <flex_DST.h>
#include <stddef.h>
#include <Preferences.h>

namespace {

Preferences prefs;

// Should always keep non-DST time
RTC_DS3231 rtc;
// EEST DST rules: last Sunday of March to last Sunday of October
flex_DST dst{5, 3, 5, 10};

// AT24C32 onboard DS3231 with default address
extEEPROM eeprom{eeprom_size_t::kbits_32, 1, 32, 0x57};

struct EepromImpl : AOP::Recorder::IEeprom
{
    uint8_t Read(uint16_t addr) override
    {
        return eeprom.read(addr);
    }

    void Write(uint16_t addr, uint8_t val) override
    {
        eeprom.write(addr, val);
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

struct RecorderAccess
{
    // Choose a non-aligned region for the punch record. This will spread eeprom wear.
    static constexpr int RECORD_START = Context::ADDRESS;
    // Just start with an odd address
    static constexpr int RECORD_ADJUSTED_START = (RECORD_START & 1) == 1 ? RECORD_START : RECORD_START + 1;
};

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

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        _buzzer->SignalRTCFail();
        return -1;
    }

    // Initialize external EEPROM with default speed, default I2C bus
    eeprom.begin();

    // Restore current record
    _recorder.Setup(RecorderAccess::RECORD_ADJUSTED_START,
                    static_cast<uint16_t>(static_cast<uint16_t>(eeprom.length() / 8) - RecorderAccess::RECORD_ADJUSTED_START));

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
    return dst.calculateTime(rtc.now());
}

uint32_t Context::GetClock(const DateTime *date_time) const
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

void Context::SetClock(uint32_t clock)
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

void Context::SetDateTime(uint32_t timestamp)
{
    DateTime adjusted{timestamp};
    if (dst.checkDST(adjusted)) {
        adjusted = adjusted.unixtime() - 3600;
    }
    rtc.adjust(adjusted);
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
