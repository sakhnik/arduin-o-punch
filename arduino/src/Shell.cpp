#include "Shell.h"
#include "Context.h"
#include "Buzzer.h"
#include "OutMux.h"
#include "PunchCard.h"
#include <RTClib.h>

namespace {

const uint8_t MAX_SIZE = 32;

} // namespace;

Shell::Shell(OutMux &_outMux, Context &context, Buzzer &buzzer)
    : _outMux{_outMux}
    , _context{context}
    , _buzzer{buzzer}
{
    _buffer.reserve(MAX_SIZE);
}

void Shell::Setup()
{
    _outMux.println();
    _PrintPrompt();
}

void Shell::_PrintPrompt()
{
    _outMux.print(F("Arduin-o-punch> "));
}

void Shell::_ProcessChar(char ch)
{
    if (!_buffer.length()) {
        // In the automated communication we don't need the echo, neither the prompt.
        // If timeout elapses since the first character, revert to interactive move.
        _echo_timeout = millis() + 100;
        _echo_idx = 0;
    }
    _buffer += ch;
    if (_buffer.length() >= MAX_SIZE || _buffer.endsWith("\r") || _buffer.endsWith("\n")) {
        // Echo the rest of the command if necessary
        if (Tick())
            Serial.println();
        // Trim whitespaces from the left
        unsigned idx = 0;
        while (idx < _buffer.length() && isWhitespace(_buffer[idx])) {
            ++idx;
        }
        _buffer.remove(0, idx);
        // Process commands if anything left
        if (_buffer.length() > 0) {
            _Process();
            _buffer.remove(0, _buffer.length());
        }
        if (Tick())
            _PrintPrompt();
    }
}

void Shell::OnSerial()
{
    while (Serial.available()) {
        int ch = Serial.read();
        if (ch >= 0)
            _ProcessChar(ch);
    }
}

#ifdef ESP32

void Shell::ProcessInput(const uint8_t *data, int size)
{
    while (size--) {
        _ProcessChar(*data++);
    }
}

#endif //ESP32

// Return true if started echoing (interactive mode)
boolean Shell::Tick()
{
    if (_echo_timeout > millis())
        return false;
    // Only need to echo to the serial terminal, TCP and BLE show what's being typed by themselves
    while (_echo_idx < _buffer.length())
        Serial.print(_buffer[_echo_idx++]);
    return true;
}

void Shell::_Process()
{
    if (_buffer.startsWith(F("help"))) {
        _outMux.println(F("Commands:"));
        _outMux.println(F("info              All info"));
        _outMux.println(F("id                ID"));
        _outMux.print  (F("id 33             Set ID: check="));
        _outMux.print(AOP::PunchCard::CHECK_STATION);
        _outMux.print(F(" start="));
        _outMux.print(AOP::PunchCard::START_STATION);
        _outMux.print(F(" finish="));
        _outMux.println(AOP::PunchCard::FINISH_STATION);
        _outMux.println(F("key               Key"));
        _outMux.println(F("key 112233445566  Set key"));
        _outMux.println(F("clock             Clock reading (ms)"));
        _outMux.println(F("clock 12345000    Set clock (ms)"));
        _outMux.println(F("date              Current date"));
        _outMux.println(F("time              Current time"));
        _outMux.println(F("timestamp         Print UNIX timestamp"));
        _outMux.println(F("timestamp 12345   Set date and time with UNIX timestamp"));
        _outMux.println(F("recfmt 256 2      Clear/prepare recorder (card count, bits per record)"));
        _outMux.println(F("rec               List punched cards"));
        _outMux.println(F("rec 123           Print punch count for a card"));
        _outMux.println(F("recclr 123        Clear card from the record"));
        _outMux.println(F("recdays           How many days to keep the record"));
        _outMux.println(F("recdays 1         Clear record after so many days"));
#ifdef ESP32
        _outMux.println(F("wifissid          Print current WiFi SSID"));
        _outMux.println(F("wifissid ssid     Set WiFi SSID to connect to"));
        _outMux.println(F("wifipass          Print WiFi password"));
        _outMux.println(F("wifipass pass     Set WiFi password"));
#endif //ESP32
    } else if (_buffer.startsWith(F("info"))) {
        _outMux.print(F("version="));
        _outMux.print(PROJECT_VERSION);
        _outMux.print("-");
        _outMux.println(GIT_REVISION);
        _outMux.print(F("id="));
        _PrintId();
        auto now = _context.GetDateTime();
        _outMux.print(F("date="));
        _PrintDate(now);
        _outMux.print(F("time="));
        _PrintTime(now);
        _outMux.print(F("rec="));
        _outMux.print(_context.GetRecorder().GetSize());
        _outMux.print(F(" x "));
        _outMux.print(static_cast<int>(_context.GetRecorder().GetBitsPerRecord()));
        _outMux.println(F(" bpr"));
        _outMux.print(F("recdays="));
        _PrintRecordRetainDays();
#ifdef ESP32
        _outMux.print(F("wifissid="));
        _PrintWifiSsid();
#endif //ESP32
    } else if (_buffer.startsWith(F("id "))) {
        SetId(_buffer.c_str() + 3);
        _PrintId();
    } else if (_buffer.startsWith(F("id"))) {
        _PrintId();
    } else if (_buffer.startsWith(F("key "))) {
        SetKey(_buffer.c_str() + 4);
        _PrintKey();
        _buzzer.ConfirmPunch();
    } else if (_buffer.startsWith(F("key"))) {
        _PrintKey();
    } else if (_buffer.startsWith(F("clock "))) {
        _SetClock(_buffer.c_str() + 6);
        _PrintClock(_context.GetDateTime());
    } else if (_buffer.startsWith(F("clock"))) {
        _PrintClock(_context.GetDateTime());
    } else if (_buffer.startsWith(F("timestamp "))) {
        _SetTimestamp(_buffer.c_str() + 10);
    } else if (_buffer.startsWith(F("timestamp"))) {
        _PrintTimestamp();
    } else if (_buffer.startsWith(F("time"))) {
        _PrintTime(_context.GetDateTime());
    } else if (_buffer.startsWith(F("date"))) {
        _PrintDate(_context.GetDateTime());
    } else if (_buffer.startsWith(F("recfmt "))) {
        RecorderFormat(_buffer.c_str() + 7);
    } else if (_buffer.startsWith(F("recclr "))) {
        _RecorderClear(_buffer.c_str() + 7);
    } else if (_buffer.startsWith(F("recdays "))) {
        SetRecordRetainDays(_buffer.c_str() + 9);
    } else if (_buffer.startsWith(F("recdays"))) {
        _PrintRecordRetainDays();
    } else if (_buffer.startsWith(F("rec "))) {
        _RecorderCheck(_buffer.c_str() + 4);
    } else if (_buffer.startsWith(F("rec"))) {
        _RecorderList();
#ifdef ESP32
    } else if (_buffer.startsWith("wifissid ")) {
        _SetWifiSsid(_buffer.c_str() + 9);
    } else if (_buffer.startsWith("wifissid")) {
        _PrintWifiSsid();
    } else if (_buffer.startsWith("wifipass ")) {
        _SetWifiPass(_buffer.c_str() + 9);
    } else if (_buffer.startsWith("wifipass")) {
        _PrintWifiPass();
#endif //ESP32
    } else {
        if (_buffer[0] != '\r' && _buffer[0] != '\n') {
            _outMux.print(F("Unknown command: "));
            for (int i = 0; i < _buffer.length(); ++i) {
                _outMux.print(' ');
                _outMux.print((int)_buffer[i], HEX);
            }
            _outMux.print(" <");
            _outMux.print(_buffer);
            _outMux.println(">");
        }
    }
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

} //namespace;

void Shell::SetKey(const char *hex)
{
    uint8_t key[Context::KEY_SIZE] = {0};
    for (uint8_t i = 0; i < sizeof(key); ++i) {
        auto d1 = FromHex(*hex++);
        if (d1 == -1)
            break;
        auto d2 = FromHex(*hex++);
        if (d2 == -1)
            break;
        key[i] = (d1 << 4) | d2;
    }
    _context.OnNewKey(key);
}

void Shell::_PrintKey()
{
    const uint8_t *key = _context.GetKey();
    for (uint8_t i = 0; i < Context::KEY_SIZE; ++i) {
        char buf[3];
        sprintf(buf, "%02X", key[i]);
        _outMux.print(buf);
    }
    _outMux.println();
}

void Shell::_SetClock(const char *str)
{
    _context.SetClock(ParseNum<uint32_t>(str));
}

void Shell::_PrintClock(const DateTime &time)
{
    _outMux.println(_context.GetClock(&time));
}

void Shell::_SetTimestamp(const char *str)
{
    _context.SetDateTime(ParseNum<uint32_t>(str));
    _PrintTimestamp();
}

void Shell::_PrintTimestamp()
{
    _outMux.println(_context.GetDateTime().unixtime());
}

void Shell::_PrintDD(uint8_t d)
{
    if (d <= 9)
        _outMux.print(F("0"));
    _outMux.print(d);
}

void Shell::_PrintDate(const DateTime &date)
{
    _outMux.print(date.year());
    _outMux.print(F("-"));
    _PrintDD(date.month());
    _outMux.print(F("-"));
    _PrintDD(date.day());
    _outMux.println();
}

void Shell::_PrintTime(const DateTime &time)
{
    _PrintDD(time.hour());
    _outMux.print(F(":"));
    _PrintDD(time.minute());
    _outMux.print(F(":"));
    _PrintDD(time.second());
    _outMux.println();
}

void Shell::_PrintId()
{
    _outMux.println(_context.GetId());
}

void Shell::SetId(const char *str)
{
    _context.SetId(ParseNum<uint8_t>(str));
}

void Shell::RecorderFormat(const char *str)
{
    uint16_t count = ParseNum<uint16_t>(str);
    uint8_t bits_per_record = ParseNum<uint8_t>(str);

    auto res = _context.GetRecorder().Format(count, bits_per_record, _context.GetDateTime().unixtime());
    if (res < 0) {
        _outMux.print(F("Error "));
        _outMux.println(-res);
    } else {
        _outMux.println(F("OK"));
        _outMux.print(F("count="));
        _outMux.print(_context.GetRecorder().GetSize());
        _outMux.print(F(" bits_per_record="));
        _outMux.println(_context.GetRecorder().GetBitsPerRecord());
    }
}

void Shell::_RecorderCheck(const char *str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    uint16_t punch_count = _context.GetRecorder().GetRecordCount(card_id);
    _outMux.println(punch_count);
}

void Shell::_RecorderClear(const char *str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    auto ok = _context.GetRecorder().Record(card_id, -1);
    _outMux.println(ok ? F("FAIL") : F("OK"));
}

void Shell::_RecorderList()
{
    _outMux.print(F("Size="));
    _outMux.println(_context.GetRecorder().GetSize());
    struct Printer : AOP::Recorder::IVisitor
    {
        OutMux &_outMux;
        Printer(OutMux &outMux) : _outMux{outMux} { }

        void OnCard(uint16_t card, uint8_t count, void *ctx) override
        {
            _outMux.print(' ');
            _outMux.print(card);
            _outMux.print(':');
            _outMux.print(static_cast<uint16_t>(count));
        }
    } printer{_outMux};
    _context.GetRecorder().List(printer, &printer);
    _outMux.println();
}

void Shell::_PrintRecordRetainDays()
{
    _outMux.println(_context.GetRecordRetainDays());
}

void Shell::SetRecordRetainDays(const char *str)
{
    _context.SetRecordRetainDays(ParseNum<uint8_t>(str));
}

#ifdef ESP32

void Shell::_SetWifiSsid(const char *str)
{
    _context.SetWifiSsid(str);
    _PrintWifiSsid();
}

void Shell::_PrintWifiSsid()
{
    _outMux.println(_context.GetWifiSsid());
}

void Shell::_SetWifiPass(const char *str)
{
    _context.SetWifiPass(str);
    _PrintWifiPass();
}

void Shell::_PrintWifiPass()
{
    _outMux.println(_context.GetWifiPass());
}

#endif //ESP32
