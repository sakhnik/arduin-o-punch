#include "Shell.h"
#include "Context.h"
#include "Buzzer.h"
#include "src/PunchCard.h"
#include <RTClib.h>

namespace {

const uint8_t MAX_SIZE = 32;

void Prompt()
{
    Serial.print(F("Arduin-o-punch> "));
}

} // namespace;

Shell::Shell(Context &context, Buzzer &buzzer)
    : _context{context}
    , _buzzer{buzzer}
{
    _buffer.reserve(MAX_SIZE);
}

void Shell::Setup()
{
    Serial.println();
    Prompt();
}

void Shell::OnSerial()
{
    while (Serial.available()) {
        char ch = Serial.read();
        if (!_buffer.length()) {
            // In the automated communication we don't need the echo, neither the prompt.
            // If timeout elapses since the first character, revert to interactive move.
            _echo_timeout = millis() + 100;
            _echo_idx = 0;
        }
        _buffer += ch;
        if (_buffer.length() >= MAX_SIZE || _buffer.endsWith("\r")) {
            // Echo the rest of the command if necessary
            if (Tick())
                Serial.println();
            _Process();
            _buffer.remove(0, _buffer.length());
            if (Tick())
                Prompt();
        }
    }
}

// Return true if started echoing (interactive mode)
boolean Shell::Tick()
{
    if (_echo_timeout > millis())
        return false;
    while (_echo_idx < _buffer.length())
        Serial.print(_buffer[_echo_idx++]);
    return true;
}

void Shell::_Process()
{
    if (_buffer.startsWith(F("help"))) {
        Serial.println(F("Commands:"));
        Serial.println(F("info              All info"));
        Serial.println(F("id                ID"));
        Serial.print  (F("id 33             Set ID: check="));
        Serial.print(AOP::PunchCard::CHECK_STATION);
        Serial.print(F(" start="));
        Serial.print(AOP::PunchCard::START_STATION);
        Serial.print(F(" finish="));
        Serial.println(AOP::PunchCard::FINISH_STATION);
        Serial.println(F("key               Key"));
        Serial.println(F("key 112233445566  Set key"));
        Serial.println(F("clock             Clock reading (ms)"));
        Serial.println(F("clock 12345000    Set clock (ms)"));
        Serial.println(F("date              Current date"));
        Serial.println(F("time              Current time"));
        Serial.println(F("timestamp         Print UNIX timestamp"));
        Serial.println(F("timestamp 12345   Set date and time with UNIX timestamp"));
        Serial.println(F("recfmt 256 2      Clear/prepare recorder (card count, bits per record)"));
        Serial.println(F("rec               List punched cards"));
        Serial.println(F("rec 123           Print punch count for a card"));
        Serial.println(F("recclr 123        Clear card from the record"));
        Serial.println(F("recdays           How many days to keep the record"));
        Serial.println(F("recdays 1         Clear record after so many days"));
    } else if (_buffer.startsWith(F("info"))) {
        Serial.println(F("version=2.1.0"));
        Serial.print(F("id="));
        _PrintId();
        Serial.print(F("key="));
        _PrintKey();
        auto now = _context.GetDateTime();
        Serial.print(F("date="));
        _PrintDate(now);
        Serial.print(F("time="));
        _PrintTime(now);
        Serial.print(F("rec="));
        Serial.print(_context.GetRecorder().GetSize());
        Serial.print(F(" x "));
        Serial.print(static_cast<int>(_context.GetRecorder().GetBitsPerRecord()));
        Serial.println(F(" bpr"));
        Serial.print(F("recdays="));
        _PrintRecordRetainDays();
    } else if (_buffer.startsWith(F("id "))) {
        _SetId(_buffer.c_str() + 3);
        _PrintId();
    } else if (_buffer.startsWith(F("id"))) {
        _PrintId();
    } else if (_buffer.startsWith(F("key "))) {
        _SetKey(_buffer.c_str() + 4);
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
        _RecorderFormat(_buffer.c_str() + 7);
    } else if (_buffer.startsWith(F("recclr "))) {
        _RecorderClear(_buffer.c_str() + 7);
    } else if (_buffer.startsWith(F("recdays "))) {
        _SetRecordRetainDays(_buffer.c_str() + 9);
    } else if (_buffer.startsWith(F("recdays"))) {
        _PrintRecordRetainDays();
    } else if (_buffer.startsWith(F("rec "))) {
        _RecorderCheck(_buffer.c_str() + 4);
    } else if (_buffer.startsWith(F("rec"))) {
        _RecorderList();
    } else {
        Serial.print(F("Unknown command: "));
        Serial.println(_buffer);
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

void Shell::_SetKey(const char *hex)
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
        Serial.print(buf);
    }
    Serial.println();
}

void Shell::_SetClock(const char *str)
{
    _context.SetClock(ParseNum<uint32_t>(str));
}

void Shell::_PrintClock(const DateTime &time)
{
    Serial.println(_context.GetClock(&time));
}

void Shell::_SetTimestamp(const char *str)
{
    _context.SetDateTime(ParseNum<uint32_t>(str));
    _PrintTimestamp();
}

void Shell::_PrintTimestamp()
{
    Serial.println(_context.GetDateTime().unixtime());
}

namespace {

void PrintDD(uint8_t d)
{
    if (d <= 9)
        Serial.print(F("0"));
    Serial.print(d);
}

} //namespace;

void Shell::_PrintDate(const DateTime &date)
{
    Serial.print(date.year());
    Serial.print(F("-"));
    PrintDD(date.month());
    Serial.print(F("-"));
    PrintDD(date.day());
    Serial.println();
}

void Shell::_PrintTime(const DateTime &time)
{
    PrintDD(time.hour());
    Serial.print(F(":"));
    PrintDD(time.minute());
    Serial.print(F(":"));
    PrintDD(time.second());
    Serial.println();
}

void Shell::_PrintId()
{
    Serial.println(_context.GetId());
}

void Shell::_SetId(const char *str)
{
    _context.SetId(ParseNum<uint8_t>(str));
}

void Shell::_RecorderFormat(const char *str)
{
    uint16_t count = ParseNum<uint16_t>(str);
    uint8_t bits_per_record = ParseNum<uint8_t>(str);

    auto res = _context.GetRecorder().Format(count, bits_per_record, _context.GetDateTime().unixtime());
    if (res < 0) {
        Serial.print(F("Error "));
        Serial.println(-res);
    } else {
        Serial.println(F("OK"));
        Serial.print(F("count="));
        Serial.print(_context.GetRecorder().GetSize());
        Serial.print(F(" bits_per_record="));
        Serial.println(_context.GetRecorder().GetBitsPerRecord());
    }
}

void Shell::_RecorderCheck(const char *str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    uint16_t punch_count = _context.GetRecorder().GetRecordCount(card_id);
    Serial.println(punch_count);
}

void Shell::_RecorderClear(const char *str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    auto ok = _context.GetRecorder().Record(card_id, -1);
    Serial.println(ok ? F("FAIL") : F("OK"));
}

void Shell::_RecorderList()
{
    Serial.print(F("Size="));
    Serial.println(_context.GetRecorder().GetSize());
    struct Printer : AOP::Recorder::IVisitor
    {
        void OnCard(uint16_t card, uint8_t count, void *ctx) override
        {
            Serial.print(' ');
            Serial.print(card);
            Serial.print(':');
            Serial.print(static_cast<uint16_t>(count));
        }
    } printer;
    _context.GetRecorder().List(printer, &printer);
    Serial.println();
}

void Shell::_PrintRecordRetainDays()
{
    Serial.println(_context.GetRecordRetainDays());
}

void Shell::_SetRecordRetainDays(const char *str)
{
    _context.SetRecordRetainDays(ParseNum<uint8_t>(str));
}
