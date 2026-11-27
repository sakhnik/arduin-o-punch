#ifdef ESP32
#include "Shell.hpp"
#include "Settings.hpp"
#include "Buzzer.hpp"
#include "OutMux.hpp"
#include "aop/PunchCard.hpp"
#include "Operation.hpp"
#include <RTClib.h>

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
    _rxQueue = xQueueCreate(128, sizeof(char));

    // run with low priority to avoid deadlock with tinyusb CDC
    xTaskCreatePinnedToCore(TaskEntry, "shell", 4096, this, 1, nullptr, 0);

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
    String buffer;
    buffer.reserve(MAX_SIZE);
    uint8_t echo_idx = 0;
    uint32_t echo_timeout = 0;

    for (;;) {

        // Compute how long we may block
        TickType_t wait = portMAX_DELAY;

        if (buffer.length()) {
            int32_t remaining = (int32_t)(echo_timeout - millis());

            if (remaining <= 0)
                wait = 0; // timeout already expired
            else
                wait = pdMS_TO_TICKS(remaining);
        }

        // Wait for input OR timeout
        char ch;
        bool gotChar = xQueueReceive(_rxQueue, &ch, wait) == pdTRUE;

        uint32_t now = millis();

        // Timeout handling
        if (buffer.length() && now >= echo_timeout) {
            while (echo_idx < buffer.length()) {
                Serial.print(buffer[echo_idx++]);
                taskYIELD();
            }
        }

        if (!gotChar)
            continue;

        if (!buffer.length()) {
            // In the automated communication we don't need the echo, neither the prompt.
            // If timeout elapses since the first character, revert to interactive move.
            echo_timeout = millis() + 100;
            echo_idx = 0;
        }

        buffer += ch;

        if (buffer.length() >= MAX_SIZE ||
            buffer.endsWith("\r") ||
            buffer.endsWith("\n"))
        {
            // Echo rest if needed
            if (now >= echo_timeout) {
                Serial.println();
                taskYIELD();
            }

            // Trim whitespace from the right
            while (buffer.length() > 0 && (buffer.endsWith("\r") || buffer.endsWith("\n")))
                buffer.remove(buffer.length() - 1);

            // Trim leading whitespace
            unsigned idx = 0;
            while (idx < buffer.length() &&
                   isWhitespace(buffer[idx])) {
                ++idx;
            }

            buffer.remove(0, idx);

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
    _outMux.Print(F("Arduin-o-punch> "));
}

void Shell::PutChar(char ch)
{
    if (!_rxQueue) return;

    // never block producer
    xQueueSend(_rxQueue, &ch, 0);
}

void Shell::_Process(const String &buffer)
{
    if (buffer.startsWith(F("help"))) {
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
    } else if (buffer.startsWith(F("info"))) {
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
    } else if (buffer.startsWith(F("id "))) {
        SetId(buffer.c_str() + 3);
        _PrintId();
    } else if (buffer.startsWith(F("id"))) {
        _PrintId();
    } else if (buffer.startsWith(F("t-act "))) {
        SetActive(buffer.c_str() + 6);
        _PrintActive();
    } else if (buffer.startsWith(F("t-act"))) {
        _PrintActive();
    } else if (buffer.startsWith(F("t-eco "))) {
        SetEco(buffer.c_str() + 6);
        _PrintEco();
    } else if (buffer.startsWith(F("t-eco"))) {
        _PrintEco();
    } else if (buffer.startsWith(F("key "))) {
        SetKey(buffer.c_str() + 4);
        _PrintKey();
        _buzzer.ConfirmPunch();
    } else if (buffer.startsWith(F("key"))) {
        _PrintKey();
    } else if (buffer.startsWith(F("clock "))) {
        _SetClock(buffer.c_str() + 6);
        _PrintClock(_settings.GetDateTime());
    } else if (buffer.startsWith(F("clock"))) {
        _PrintClock(_settings.GetDateTime());
    } else if (buffer.startsWith(F("timestamp "))) {
        _SetTimestamp(buffer.c_str() + 10);
    } else if (buffer.startsWith(F("timestamp"))) {
        _PrintTimestamp();
    } else if (buffer.startsWith(F("time"))) {
        _PrintTime(_settings.GetDateTime());
    } else if (buffer.startsWith(F("date"))) {
        _PrintDate(_settings.GetDateTime());
    } else if (buffer.startsWith(F("rec "))) {
        _RecorderCheck(buffer.c_str() + 4);
    } else if (buffer.startsWith(F("rec"))) {
        _RecorderList();
#if defined(ENABLE_WIFI) && ENABLE_WIFI
    } else if (buffer.startsWith("wifissid ")) {
        _SetWifiSsid(buffer.c_str() + 9);
    } else if (buffer.startsWith("wifissid")) {
        _PrintWifiSsid();
    } else if (buffer.startsWith("wifipass ")) {
        _SetWifiPass(buffer.c_str() + 9);
    } else if (buffer.startsWith("wifipass")) {
        _PrintWifiPass();
#endif
    } else if (buffer.startsWith("card-punch")) {
        _settings.ActivateCardPunchMode();
    } else if (buffer.startsWith("card-readout")) {
        _settings.ActivateCardReadOutMode(buffer.c_str() + 12);
    } else if (buffer.startsWith("card-format")) {
        _settings.ActivateCardFormatMode(buffer.c_str() + 11);
    } else if (buffer.startsWith("known-keys ")) {
        _SetKnownKeys(buffer.c_str() + 11);
    } else if (buffer.startsWith("known-keys")) {
        _PrintKnownKeys();
    } else if (_operation && buffer.startsWith("stats-reset")) {
        _operation->ResetStats();
        auto stats = _operation->GetStats();
        stats.Print(_outMux);
    } else if (buffer.startsWith("log") || buffer.startsWith("stats")) {
        auto stats = _operation->GetStats();
        stats.Print(_outMux);
    } else {
        if (buffer[0] != '\r' && buffer[0] != '\n') {
            _outMux.Print(F("Unknown command: "));
            for (int i = 0; i < buffer.length(); ++i) {
                _outMux.Print(' ');
                _outMux.Print((int)buffer[i], 16);
            }
            _outMux.Print(" <");
            _outMux.Print(buffer);
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
T ParseNum(const char *&str)
{
    T num = 0;
    while (char ch = *str++) {
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

void Shell::SetKey(const char *hex)
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

void Shell::_SetClock(const char *str)
{
    _settings.SetClock(ParseNum<uint32_t>(str));
}

void Shell::_PrintClock(const AOP::DateTime &time)
{
    _outMux.Println(_settings.GetClock(&time));
}

void Shell::_SetTimestamp(const char *str)
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

void Shell::SetId(const char *str)
{
    _settings.SetId(ParseNum<uint8_t>(str));
}

void Shell::_PrintActive()
{
    _outMux.Println(_settings.GetActiveMinutes());
}

void Shell::SetActive(const char *str)
{
    _settings.SetActiveMinutes(ParseNum<uint32_t>(str));
}

void Shell::_PrintEco()
{
    _outMux.Println(_settings.GetEcoMinutes());
}

void Shell::SetEco(const char *str)
{
    _settings.SetEcoMinutes(ParseNum<uint32_t>(str));
}

void Shell::_RecorderCheck(const char *str)
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

void Shell::_SetKnownKeys(const char *str)
{
    Settings::KeysT keys;

    while (*str) {
        while (*str && (std::isspace(*str) || *str == ',')) {
            ++str;
        }

        if (!*str)
            break;

        const char *begin = str;
        while (*str && !std::isspace(*str) && *str != ',')
            ++str;

        keys.push_back(ParseKey(std::string_view(begin, str - begin)));
    }

    _settings.SetKnownKeys(std::move(keys));
}
#endif //ESP32
