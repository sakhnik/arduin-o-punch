#include "Shell.hpp"
#include "Settings.hpp"
#include "Buzzer.hpp"
#include "OutMux.hpp"
#include "aop/PunchCard.hpp"
#include "Operation.hpp"
#include "hal/time.hpp"

namespace {

const size_t MAX_SIZE = 256;

} // namespace;

Shell::Shell(OutMux &_outMux, Settings &settings, Buzzer &buzzer)
    : _outMux{_outMux}
    , _settings{settings}
    , _buzzer{buzzer}
{
}

void Shell::Setup()
{
    _rxQueue.Setup(128, sizeof(char));

    // run with low priority to avoid deadlock with tinyusb CDC
    _task.Setup(TaskEntry, this, "shell", 4096, 0);

    _outMux.Println();
    _PrintPrompt();
}

void Shell::SetOperation(Operation *operation)
{
    _operation = operation;
}

void Shell::ProcessInput(const uint8_t *data, int size)
{
    while (size--) {
        PutChar(*data++);
    }
}

void Shell::TaskEntry(void* arg)
{
    static_cast<Shell*>(arg)->_Task();
}

void Shell::_Task()
{
    std::string buffer;
    buffer.reserve(MAX_SIZE);
    uint8_t echo_idx = 0;
    uint32_t echo_timeout = 0;

    for (;;) {

        // Compute how long we may block
        uint32_t waitMs = HAL::Queue::WaitForever;

        if (buffer.length()) {
            int32_t remaining = (int32_t)(echo_timeout - HAL::Millis());

            if (remaining <= 0)
                waitMs = 0; // timeout already expired
            else
                waitMs = remaining;
        }

        // Wait for input OR timeout
        char ch;
        bool gotChar = _rxQueue.Receive(&ch, waitMs);

        uint32_t now = HAL::Millis();

        // Timeout handling
        if (buffer.length() && now >= echo_timeout) {
            while (echo_idx < buffer.length()) {
                OUT.Print(buffer[echo_idx++]);
                HAL::Task::Yield();
            }
        }

        if (!gotChar)
            continue;

        if (!buffer.length()) {
            // In the automated communication we don't need the echo, neither the prompt.
            // If timeout elapses since the first character, revert to interactive move.
            echo_timeout = HAL::Millis() + 100;
            echo_idx = 0;
        }

        buffer += ch;

        if (buffer.length() >= MAX_SIZE ||
            buffer.back() == '\r' ||
            buffer.back() == '\n')
        {
            // Echo rest if needed
            if (now >= echo_timeout) {
                OUT.Println();
                HAL::Task::Yield();
            }

            // Trim whitespace from the right
            while (buffer.length() > 0 && (buffer.back() == '\r' || buffer.back() == '\n'))
                buffer.pop_back();

            // Trim leading whitespace
            unsigned idx = 0;
            while (idx < buffer.length() && std::isspace(buffer[idx])) {
                ++idx;
            }

            buffer.erase(0, idx);

            // Execute command
            if (buffer.length() > 0) {
                _Process(buffer);
                buffer.clear();
            }

            if (now >= echo_timeout)
                _PrintPrompt();
        }
    }
}

void Shell::_PrintPrompt()
{
    _outMux.Print(F("AOP> "));
}

void Shell::PutChar(char ch)
{
    // never block producer
    _rxQueue.Send(&ch, 0);
}

namespace {

bool StartsWith(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() &&
           str.substr(0, prefix.size()) == prefix;
}

bool StartsWith(std::string_view str, const __FlashStringHelper *prefix)
{
    return StartsWith(str, reinterpret_cast<const char *>(prefix));
}

} //namespace;

void Shell::_Process(std::string_view buffer)
{
    if (StartsWith(buffer, F("help"))) {
        _outMux.Println(F("Commands:"));
        _outMux.Println(F("info              All info"));
        _outMux.Println(F("id                ID"));
        _outMux.Print  (F("id 33             Set ID: clear="));
        _outMux.Print(AOP::PunchCard::CLEAR_STATION);
        _outMux.Print(F(" check="));
        _outMux.Print(AOP::PunchCard::CHECK_STATION);
        _outMux.Print(F(" start="));
        _outMux.Print(AOP::PunchCard::START_STATION);
        _outMux.Print(F(" finish="));
        _outMux.Println(AOP::PunchCard::FINISH_STATION);
        _outMux.Println(F("key               Key"));
        _outMux.Println(F("key 112233445566  Set key"));
        _outMux.Println(F("t-act             Active timeout (min)"));
        _outMux.Println(F("t-act 600         Set active timeout (min)"));
        _outMux.Println(F("t-eco             Eco timeout (min)"));
        _outMux.Println(F("t-eco 10800       Set eco timeout (min)"));
        _outMux.Println(F("clock             Clock reading (ms)"));
        _outMux.Println(F("clock 12345000    Set clock (ms)"));
        _outMux.Println(F("date              Current date"));
        _outMux.Println(F("time              Current time"));
        _outMux.Println(F("timestamp         Print UNIX timestamp"));
        _outMux.Println(F("timestamp 12345   Set date and time with UNIX timestamp"));
        _outMux.Println(F("rec               List punched cards"));
        _outMux.Println(F("rec 123           Print punch count for a card"));
#if defined(ENABLE_WIFI) && ENABLE_WIFI
        _outMux.Println(F("wifissid          Print current WiFi SSID"));
        _outMux.Println(F("wifissid ssid     Set WiFi SSID to connect to"));
        _outMux.Println(F("wifipass          Print WiFi password"));
        _outMux.Println(F("wifipass pass     Set WiFi password"));
#endif
        if (_operation) {
            _outMux.Println(F("stats             Print energy consumption stats"));
            _outMux.Println(F("stats-reset       Reset energy consumption stats"));
        }
        _outMux.Println(F("log               Dump log"));
        _outMux.Println(F("card-punch        Activate punch mode"));
        _outMux.Println(F("card-readout cnt  Activate readout mode"));
        _outMux.Println(F("card-format id    Activate card formatting mode"));
        _outMux.Println(F("known-keys        Print known keys to try for formatting"));
        _outMux.Println(F("known-keys <keys> Set list of known keys"));
    } else if (StartsWith(buffer, F("info"))) {
        _outMux.Print(F("version="));
        _outMux.Print(PROJECT_VERSION);
        _outMux.Print("-");
        _outMux.Println(GIT_REVISION);
        _outMux.Print(F("id="));
        _PrintId();
        _outMux.Print(F("t-act="));
        _PrintActive();
        _outMux.Print(F("t-eco="));
        _PrintEco();
        auto now = _settings.GetDateTime();
        _outMux.Print(F("date="));
        _PrintDate(now);
        _outMux.Print(F("time="));
        _PrintTime(now);
#if defined(ENABLE_WIFI) && ENABLE_WIFI
        _outMux.print(F("wifissid="));
        _PrintWifiSsid();
#endif
        _outMux.Print(F("card-mode="));
        switch (_settings.GetActiveCardMode()) {
        case Settings::CardMode::Punch:
            _outMux.Println(F("punch"));
            break;
        case Settings::CardMode::ReadOut:
            _outMux.Print(F("readout "));
            _outMux.Println(_settings.GetCardModeArg().c_str());
            break;
        case Settings::CardMode::Format:
            _outMux.Print(F("format "));
            _outMux.Println(_settings.GetCardModeArg().c_str());
            break;
        }
    } else if (StartsWith(buffer, F("id "))) {
        SetId(buffer.substr(3));
        _PrintId();
    } else if (StartsWith(buffer, F("id"))) {
        _PrintId();
    } else if (StartsWith(buffer, F("t-act "))) {
        SetActive(buffer.substr(6));
        _PrintActive();
    } else if (StartsWith(buffer, F("t-act"))) {
        _PrintActive();
    } else if (StartsWith(buffer, F("t-eco "))) {
        SetEco(buffer.substr(6));
        _PrintEco();
    } else if (StartsWith(buffer, F("t-eco"))) {
        _PrintEco();
    } else if (StartsWith(buffer, F("key "))) {
        SetKey(buffer.substr(4));
        _PrintKey();
        _buzzer.ConfirmPunch();
    } else if (StartsWith(buffer, F("key"))) {
        _PrintKey();
    } else if (StartsWith(buffer, F("clock "))) {
        _SetClock(buffer.substr(6));
        _PrintClock(_settings.GetDateTime());
    } else if (StartsWith(buffer, F("clock"))) {
        _PrintClock(_settings.GetDateTime());
    } else if (StartsWith(buffer, F("timestamp "))) {
        _SetTimestamp(buffer.substr(10));
    } else if (StartsWith(buffer, F("timestamp"))) {
        _PrintTimestamp();
    } else if (StartsWith(buffer, F("time"))) {
        _PrintTime(_settings.GetDateTime());
    } else if (StartsWith(buffer, F("date"))) {
        _PrintDate(_settings.GetDateTime());
    } else if (StartsWith(buffer, F("rec "))) {
        _RecorderCheck(buffer.substr(4));
    } else if (StartsWith(buffer, F("rec"))) {
        _RecorderList();
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    } else if (StartsWith(buffer, "wifissid ")) {
        _SetWifiSsid(buffer.substr(9));
    } else if (StartsWith(buffer, "wifissid")) {
        _PrintWifiSsid();
    } else if (StartsWith(buffer, "wifipass ")) {
        _SetWifiPass(buffer.substr(9));
    } else if (StartsWith(buffer, "wifipass")) {
        _PrintWifiPass();
#endif
    } else if (StartsWith(buffer, "card-punch")) {
        _settings.ActivateCardPunchMode();
    } else if (StartsWith(buffer, "card-readout")) {
        _settings.ActivateCardReadOutMode(buffer.substr(12));
    } else if (StartsWith(buffer, "card-format")) {
        _settings.ActivateCardFormatMode(buffer.substr(11));
    } else if (StartsWith(buffer, "known-keys ")) {
        _SetKnownKeys(buffer.substr(11));
    } else if (StartsWith(buffer, "known-keys")) {
        _PrintKnownKeys();
    } else if (_operation && StartsWith(buffer, "stats-reset")) {
#ifdef ESP32
        _operation->ResetStats();
        auto stats = _operation->GetStats();
        stats.Print(_outMux);
#endif
    } else if (_operation && StartsWith(buffer, "log") || StartsWith(buffer, "stats")) {
#ifdef ESP32
        auto stats = _operation->GetStats();
        stats.Print(_outMux);
#endif
    } else {
        if (buffer[0] != '\r' && buffer[0] != '\n') {
            _outMux.Print(F("Unknown command: "));
            for (int i = 0; i < buffer.length(); ++i) {
                _outMux.Print(' ');
                _outMux.Print((int)buffer[i], 16);
            }
            _outMux.Print(" <");
            for (char ch : buffer)
                _outMux.Print(ch);
            _outMux.Println(">");
        }
    }
    _outMux.Println();
}

namespace {

int16_t FromHex(char digit)
{
    if (!digit) return -1;
    switch (digit) {
    case '0'...'9':
        return digit - '0';
    case 'A'...'F':
        return digit - 'A' + 10;
    case 'a'...'f':
        return digit - 'a' + 10;
    default:
        return -1;
    }
}

template <typename T>
T ParseNum(std::string_view &str)
{
    T num = 0;
    for (; !str.empty(); str = str.substr(1)) {
        char ch = str.front();
        if (ch < '0' || ch > '9')
            break;
        num = num * 10 + (ch - '0');
    }
    return num;
}

Settings::KeyT ParseKey(std::string_view s)
{
    Settings::KeyT key{};

    std::size_t nibble = 0;
    for (char c : s) {
        int v = FromHex(c);
        if (v < 0)
            continue;

        if (nibble >= key.size() * 2)
            break;

        auto &b = key[nibble / 2];
        if ((nibble & 1) == 0)
            b = static_cast<uint8_t>(v << 4);
        else
            b |= static_cast<uint8_t>(v);

        ++nibble;
    }

    return key;
}

} //namespace;

void Shell::SetKey(std::string_view hex)
{
    auto key = ParseKey(hex);
    _settings.SetKey(key);
}

void Shell::_PrintKey()
{
    for (uint8_t b : _settings.GetKey()) {
        char buf[3];
        sprintf(buf, "%02X", static_cast<unsigned>(b));
        _outMux.Print(buf);
    }
    _outMux.Println();
}

void Shell::_SetClock(std::string_view str)
{
    _settings.SetClock(ParseNum<uint32_t>(str));
}

void Shell::_PrintClock(const AOP::DateTime &time)
{
    _outMux.Println(_settings.GetClock(&time));
}

void Shell::_SetTimestamp(std::string_view str)
{
    _settings.SetDateTime(ParseNum<uint32_t>(str));
    _PrintTimestamp();
}

void Shell::_PrintTimestamp()
{
    _outMux.Println(_settings.GetDateTime().ToUnix());
}

void Shell::_PrintDD(uint8_t d)
{
    if (d <= 9)
        _outMux.Print(F("0"));
    _outMux.Print(d);
}

void Shell::_PrintDate(const AOP::DateTime &date)
{
    _outMux.Print(date.year);
    _outMux.Print(F("-"));
    _PrintDD(date.month);
    _outMux.Print(F("-"));
    _PrintDD(date.day);
    _outMux.Println();
}

void Shell::_PrintTime(const AOP::DateTime &time)
{
    _PrintDD(time.hour);
    _outMux.Print(F(":"));
    _PrintDD(time.minute);
    _outMux.Print(F(":"));
    _PrintDD(time.second);
    _outMux.Println();
}

void Shell::_PrintId()
{
    _outMux.Println(_settings.GetId());
}

void Shell::SetId(std::string_view str)
{
    _settings.SetId(ParseNum<uint8_t>(str));
}

void Shell::_PrintActive()
{
    _outMux.Println(_settings.GetActiveMinutes());
}

void Shell::SetActive(std::string_view str)
{
    _settings.SetActiveMinutes(ParseNum<uint32_t>(str));
}

void Shell::_PrintEco()
{
    _outMux.Println(_settings.GetEcoMinutes());
}

void Shell::SetEco(std::string_view str)
{
    _settings.SetEcoMinutes(ParseNum<uint32_t>(str));
}

void Shell::_RecorderCheck(std::string_view str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    auto timestamps = _settings.GetRecorder().GetTimestamps(card_id);
    for (auto ts : timestamps) {
        auto s = ts % 60;
        ts /= 60;
        auto m = ts % 60;
        ts /= 60;
        _outMux.Print(ts);
        _outMux.Print(':');
        if (m < 10) _outMux.Print('0');
        _outMux.Print(m);
        _outMux.Print(':');
        if (s < 10) _outMux.Print('0');
        _outMux.Print(s);
        _outMux.Println();
    }
}

void Shell::_RecorderList()
{
    _outMux.Print("count=");
    _outMux.Println(_settings.GetRecorder().GetCount());

    struct Printer : AOP::Recorder::IVisitor
    {
        OutMux &_outMux;
        Printer(OutMux &outMux) : _outMux{outMux} { }

        void OnCard(uint16_t card, uint32_t ts, void *ctx) override
        {
            auto s = ts % 60;
            ts /= 60;
            auto m = ts % 60;
            ts /= 60;
            _outMux.Print(card);
            _outMux.Print(' ');
            _outMux.Print(ts);
            _outMux.Print(':');
            if (m < 10) _outMux.Print('0');
            _outMux.Print(m);
            _outMux.Print(':');
            if (s < 10) _outMux.Print('0');
            _outMux.Print(s);
            _outMux.Println();
        }
    } printer{_outMux};

    _settings.GetRecorder().List(printer, &printer);
}

#if defined(ENABLE_WIFI) && ENABLE_WIFI
void Shell::_SetWifiSsid(const char *str)
{
    _settings.SetWifiSsid(str);
    _PrintWifiSsid();
}

void Shell::_PrintWifiSsid()
{
    _outMux.println(_settings.GetWifiSsid().c_str());
}

void Shell::_SetWifiPass(const char *str)
{
    _settings.SetWifiPass(str);
    _PrintWifiPass();
}

void Shell::_PrintWifiPass()
{
    _outMux.println(_settings.GetWifiPass().c_str());
}
#endif

void Shell::_PrintKnownKeys()
{
    const char *comma = "";
    for (const auto &key : _settings.GetKnownKeys()) {
        _outMux.Print(comma);
        comma = ",";
        for (auto b : key) {
            char buf[3];
            sprintf(buf, "%02X", static_cast<unsigned>(b));
            _outMux.Print(buf);
        }
    }
    _outMux.Println();
}

void Shell::_SetKnownKeys(std::string_view str)
{
    Settings::KeysT keys;

    while (!str.empty()) {
        while (!str.empty() && (std::isspace(static_cast<unsigned char>(str.front())) || str.front() == ',')) {
            str.remove_prefix(1);
        }

        if (str.empty())
            break;

        auto end = str.find_first_of(" \t\n\r\f\v,");
        auto token = str.substr(0, end);

        keys.push_back(ParseKey(token));

        if (end == std::string_view::npos)
            break;

        str.remove_prefix(end);
    }

    _settings.SetKnownKeys(std::move(keys));
}
