#include "Settings.hpp"
#include "Buzzer.hpp"
#include "hal/rtc.hpp"
#include "hal/prefs.hpp"
#include "hal/time.hpp"
#include "aop/TZ.hpp"
#include <algorithm>
#include <cstddef>

namespace {

constexpr const size_t EEPROM_SIZE = 32 * 1024;

constexpr const char *const PREF_CONFIG = "config";
constexpr const char *const PREF_ID = "id";
constexpr const char *const PREF_KEY = "key";
constexpr const char *const PREF_KNOWN_KEYS = "known-keys";
constexpr const char *const PREF_T_ACT_M = "t-act";
constexpr const char *const PREF_T_ECO_M = "t-eco";
constexpr const char *const PREF_WIFI_SSID = "wifi-ssid";
constexpr const char *const PREF_WIFI_PASS = "wifi-pass";

} //namespace;

#define ADDROF(field) (Settings::ADDRESS + offsetof(Settings::_Data, field))

Settings::Settings(Buzzer &buzzer)
    : _buzzer{&buzzer}
{
}

void Settings::Setup()
{

    HAL::Prefs prefs(PREF_CONFIG, true);

    _id = prefs.GetU8(PREF_ID, 1);
    if (_id == AOP::PunchCard::CLEAR_STATION) {
        _cardMode = CardMode::Format;
    }

    prefs.GetBytes(PREF_KEY, _key.data(), KEY_SIZE);

    auto prevKeysLen = prefs.GetBytesLength(PREF_KNOWN_KEYS);
    auto prevKeysSize = prevKeysLen / KEY_SIZE;
    _knownKeys.resize(prevKeysSize + 1);
    prefs.GetBytes(PREF_KNOWN_KEYS, reinterpret_cast<uint8_t *>(_knownKeys.data()), prevKeysLen);
    _knownKeys.resize(prevKeysSize);

    _active_minutes = prefs.GetU16(PREF_T_ACT_M, DEFAULT_ACTIVE_MINUTES);
    _active_ms = 60000ul * _active_minutes;
    _eco_minutes = prefs.GetU16(PREF_T_ECO_M, DEFAULT_ECO_MINUTES);
    _eco_ms = 60000ul * _eco_minutes;
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    _wifi_ssid = prefs.GetString(PREF_WIFI_SSID);
    _wifi_pass = prefs.GetString(PREF_WIFI_PASS);
#endif
}

void Settings::SetupLate()
{
    // Prepare to record the punches
    uint32_t now = GetClock(nullptr) / 1000;
    _recorder.Setup(now);
}

bool Settings::IsKeyDefault()
{
    HAL::LockGuard lock{_dataMx};
    for (int i = 0; i < KEY_SIZE; ++i) {
        if (_key[i] != 0xff)
            return false;
    }
    return true;
}

Settings::KeyT Settings::GetKey()
{
    HAL::LockGuard lock{_dataMx};
    return _key;
}

void Settings::SetKey(const KeyT &key)
{
    {
        HAL::LockGuard lock{_dataMx};
        if (_key == key)
            return;
        _key = key;
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutBytes(PREF_KEY, _key.data(), KEY_SIZE);
    }

    NotifyWatchers();
}

const Settings::KeysT& Settings::GetKnownKeys()
{
    return _knownKeys;
}

void Settings::SetKnownKeys(KeysT &&keys)
{
    {
        HAL::LockGuard lock{_dataMx};
        if (_knownKeys == keys)
            return;
        _knownKeys = std::move(keys);
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutBytes(PREF_KNOWN_KEYS, reinterpret_cast<const uint8_t *>(_knownKeys.data()), KEY_SIZE * _knownKeys.size());
    }

    NotifyWatchers();
}

AOP::DateTime Settings::GetDateTime() const
{
    return AOP::TZ::ToLocal(HAL::RTC::Now());
}

uint32_t Settings::GetClock(const AOP::DateTime *date_time) const
{
    if (!date_time) {
        auto dt = GetDateTime();
        return GetClock(&dt);
    }
    // TODO: subsecond resolution
    uint32_t clock = date_time->hour;
    clock = clock * 60 + date_time->minute;
    clock = clock * 60 + date_time->second;
    // We take milliseconds from arduino clock. This is only useful if we copare
    // close clock readings. Otherwise, the milliseconds should be ignored.
    return clock * 1000 + HAL::Millis() % 1000;
}

void Settings::SetClock(uint32_t clock)
{
    auto now = GetDateTime();
    //auto ms = clock % 1000;
    clock /= 1000;
    auto sec = clock % 60;
    clock /= 60;
    auto min = clock % 60;
    clock /= 60;
    AOP::DateTime adjusted(now.year, now.month, now.day, clock, min, sec);
    adjusted = AOP::TZ::ToStandard(adjusted);
    HAL::RTC::Adjust(adjusted);
}

void Settings::SetDateTime(uint32_t timestamp)
{
    auto adjusted = AOP::DateTime::FromUnix(timestamp);
    adjusted = AOP::TZ::ToStandard(adjusted);
    HAL::RTC::Adjust(adjusted);
}

uint8_t Settings::GetId()
{
    HAL::LockGuard lock{_dataMx};
    return _id;
}

void Settings::SetId(uint8_t id)
{
    {
        HAL::LockGuard lock{_dataMx};
        if (_id == id)
            return;
        _id = id;
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutU8(PREF_ID, _id);
    }

    NotifyWatchers();
}

uint32_t Settings::GetActiveMinutes()
{
    HAL::LockGuard lock{_dataMx};
    return _active_minutes;
}

uint32_t Settings::GetActiveMs()
{
    HAL::LockGuard lock{_dataMx};
    return _active_ms;
}

void Settings::SetActiveMinutes(uint32_t minutes)
{
    {
        HAL::LockGuard lock{_dataMx};
        if (_active_minutes == minutes)
            return;
        _active_minutes = minutes;
        _active_ms = 60000ul * _active_minutes;
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutU16(PREF_T_ACT_M, _active_minutes);
    }

    NotifyWatchers();
}

uint32_t Settings::GetEcoMinutes()
{
    HAL::LockGuard lock{_dataMx};
    return _eco_minutes;
}

uint32_t Settings::GetEcoMs()
{
    HAL::LockGuard lock{_dataMx};
    return _eco_ms;
}

void Settings::SetEcoMinutes(uint32_t minutes)
{
    {
        HAL::LockGuard lock{_dataMx};
        if (_eco_minutes == minutes)
            return;
        _eco_minutes = minutes;
        _eco_ms = 60000ul * _eco_minutes;
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutU16(PREF_T_ECO_M, _eco_minutes);
    }

    NotifyWatchers();
}

#if defined(ENABLE_WIFI) && ENABLE_WIFI
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
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutString(PREF_WIFI_SSID, _wifi_ssid.c_str());
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
        HAL::Prefs prefs(PREF_CONFIG, false);
        prefs.PutString(PREF_WIFI_PASS, _wifi_pass.c_str());
    }

    NotifyWatchers();
}
#endif

size_t Settings::Subscribe(OnChangeT on_change)
{
    HAL::LockGuard lock{_watchersMx};
    auto it = std::find_if(watchers.begin(), watchers.end(), [](OnChangeT w) { return !w; });
    if (it != watchers.end()) {
        *it = on_change;
    }
    return it - watchers.begin();
}

void Settings::Unsubscribe(size_t idx)
{
    HAL::LockGuard lock{_watchersMx};
    if (idx < watchers.size())
        watchers[idx] = {};
}

void Settings::NotifyWatchers()
{
    HAL::LockGuard lock{_watchersMx};
    for (const auto &w : watchers) {
        if (w) {
            w();
        }
    }
}

void Settings::ActivateCardReadOutMode(std::string_view arg)
{
    _cardMode = CardMode::ReadOut;
    while (!arg.empty() && !std::isdigit(arg.front())) {
        arg = arg.substr(1);
    }
    if (arg.empty())
        arg = "1";
    _cardModeArg = arg;
}

void Settings::ActivateCardFormatMode(std::string_view arg)
{
    _cardMode = CardMode::Format;
    while (!arg.empty() && !std::isdigit(arg.front())) {
        arg = arg.substr(1);
    }
    _cardModeArg = arg;
}
