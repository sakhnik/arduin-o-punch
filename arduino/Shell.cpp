#include "Shell.h"
#include "Context.h"
#include "Buzzer.h"
#include "OutMux.h"
#include "src/PunchCard.h"
#include <RTClib.h>

namespace {

const uint8_t MAX_SIZE = 32;

OutMux outMux;

} // namespace;

Shell::Shell(Context &context, Buzzer &buzzer)
    : _context{context}
    , _buzzer{buzzer}
{
    _buffer.reserve(MAX_SIZE);
}

void Shell::Setup()
{
    outMux.println();
    _PrintPrompt();
}

void Shell::_PrintPrompt()
{
    outMux.print(F("Arduin-o-punch> "));
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
                outMux.println();
            _Process();
            _buffer.remove(0, _buffer.length());
            if (Tick())
                _PrintPrompt();
        }
    }
}

// Return true if started echoing (interactive mode)
boolean Shell::Tick()
{
    if (_echo_timeout > millis())
        return false;
    while (_echo_idx < _buffer.length())
        outMux.print(_buffer[_echo_idx++]);
    return true;
}

void Shell::_Process()
{
    if (_buffer.startsWith(F("help"))) {
        outMux.println(F("Commands:"));
        outMux.println(F("info              All info"));
        outMux.println(F("id                ID"));
        outMux.print  (F("id 33             Set ID: check="));
        outMux.print(AOP::PunchCard::CHECK_STATION);
        outMux.print(F(" start="));
        outMux.print(AOP::PunchCard::START_STATION);
        outMux.print(F(" finish="));
        outMux.println(AOP::PunchCard::FINISH_STATION);
        outMux.println(F("key               Key"));
        outMux.println(F("key 112233445566  Set key"));
        outMux.println(F("clock             Clock reading (ms)"));
        outMux.println(F("clock 12345000    Set clock (ms)"));
        outMux.println(F("date              Current date"));
        outMux.println(F("time              Current time"));
        outMux.println(F("timestamp         Print UNIX timestamp"));
        outMux.println(F("timestamp 12345   Set date and time with UNIX timestamp"));
        outMux.println(F("recfmt 256 2      Clear/prepare recorder (card count, bits per record)"));
        outMux.println(F("rec               List punched cards"));
        outMux.println(F("rec 123           Print punch count for a card"));
        outMux.println(F("recclr 123        Clear card from the record"));
        outMux.println(F("recdays           How many days to keep the record"));
        outMux.println(F("recdays 1         Clear record after so many days"));
    } else if (_buffer.startsWith(F("info"))) {
        outMux.println(F("version=2.0.0"));
        outMux.print(F("id="));
        _PrintId();
        outMux.print(F("key="));
        _PrintKey();
        auto now = _context.GetDateTime();
        outMux.print(F("date="));
        _PrintDate(now);
        outMux.print(F("time="));
        _PrintTime(now);
        outMux.print(F("rec="));
        outMux.print(_context.GetRecorder().GetSize());
        outMux.print(F(" x "));
        outMux.print(static_cast<int>(_context.GetRecorder().GetBitsPerRecord()));
        outMux.println(F(" bpr"));
        outMux.print(F("recdays="));
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
        outMux.print(F("Unknown command: "));
        outMux.println(_buffer);
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
        outMux.print(buf);
    }
    outMux.println();
}

void Shell::_SetClock(const char *str)
{
    _context.SetClock(ParseNum<uint32_t>(str));
}

void Shell::_PrintClock(const DateTime &time)
{
    outMux.println(_context.GetClock(&time));
}

void Shell::_SetTimestamp(const char *str)
{
    _context.SetDateTime(ParseNum<uint32_t>(str));
    _PrintTimestamp();
}

void Shell::_PrintTimestamp()
{
    outMux.println(_context.GetDateTime().unixtime());
}

namespace {

void PrintDD(uint8_t d)
{
    if (d <= 9)
        outMux.print(F("0"));
    outMux.print(d);
}

} //namespace;

void Shell::_PrintDate(const DateTime &date)
{
    outMux.print(date.year());
    outMux.print(F("-"));
    PrintDD(date.month());
    outMux.print(F("-"));
    PrintDD(date.day());
    outMux.println();
}

void Shell::_PrintTime(const DateTime &time)
{
    PrintDD(time.hour());
    outMux.print(F(":"));
    PrintDD(time.minute());
    outMux.print(F(":"));
    PrintDD(time.second());
    outMux.println();
}

void Shell::_PrintId()
{
    outMux.println(_context.GetId());
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
        outMux.print(F("Error "));
        outMux.println(-res);
    } else {
        outMux.println(F("OK"));
        outMux.print(F("count="));
        outMux.print(_context.GetRecorder().GetSize());
        outMux.print(F(" bits_per_record="));
        outMux.println(_context.GetRecorder().GetBitsPerRecord());
    }
}

void Shell::_RecorderCheck(const char *str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    uint16_t punch_count = _context.GetRecorder().GetRecordCount(card_id);
    outMux.println(punch_count);
}

void Shell::_RecorderClear(const char *str)
{
    uint16_t card_id = ParseNum<uint16_t>(str);
    auto ok = _context.GetRecorder().Record(card_id, -1);
    outMux.println(ok ? F("FAIL") : F("OK"));
}

void Shell::_RecorderList()
{
    outMux.print(F("Size="));
    outMux.println(_context.GetRecorder().GetSize());
    struct Printer : AOP::Recorder::IVisitor
    {
        void OnCard(uint16_t card, uint8_t count, void *ctx) override
        {
            outMux.print(' ');
            outMux.print(card);
            outMux.print(':');
            outMux.print(static_cast<uint16_t>(count));
        }
    } printer;
    _context.GetRecorder().List(printer, &printer);
    outMux.println();
}

void Shell::_PrintRecordRetainDays()
{
    outMux.println(_context.GetRecordRetainDays());
}

void Shell::_SetRecordRetainDays(const char *str)
{
    _context.SetRecordRetainDays(ParseNum<uint8_t>(str));
}
