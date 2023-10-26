#include "Shell.h"
#include "Context.h"
#include "Buzzer.h"

namespace {

const uint8_t MAX_SIZE = 64;

void Prompt()
{
    Serial.print(F("Arduin-o-punch> "));
}

} // namespace;

void Shell::Setup()
{
    Serial.println();
    Prompt();
}

void Shell::OnSerial()
{
    while (Serial.available())
    {
        char ch = Serial.read();
        if (!_buffer.length())
        {
            // In the automated communication we don't need the echo, neither the prompt.
            // If timeout elapses since the first character, revert to interactive move.
            _echo_timeout = millis() + 100;
            _echo_idx = 0;
        }
        _buffer += ch;
        if (_buffer.length() >= MAX_SIZE || _buffer.endsWith("\r"))
        {
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
    if (_buffer.startsWith(F("help")))
    {
        Serial.println(F("Commands:"));
        Serial.println(F("info              Print all info"));
        Serial.println(F("id                Print ID"));
        Serial.println(F("id 33             Set ID"));
        Serial.println(F("key               Print key"));
        Serial.println(F("key 112233445566  Set key"));
        Serial.println(F("clock             Print clock reading (ms)"));
        Serial.println(F("clock 12345000    Set clock (ms)"));
    }
    else if (_buffer.startsWith(F("info")))
    {
        Serial.print(F("id="));
        _PrintId();
        Serial.print(F("key="));
        _PrintKey();
        Serial.print(F("clock="));
        _PrintClock();
    }
    else if (_buffer.startsWith(F("id ")))
    {
        _SetId(_buffer.c_str() + 3);
        _PrintId();
    }
    else if (_buffer.startsWith(F("id")))
    {
        _PrintId();
    }
    else if (_buffer.startsWith(F("key ")))
    {
        _SetKey(_buffer.c_str() + 4);
        _PrintKey();
        _buzzer.ConfirmPunch();
    }
    else if (_buffer.startsWith(F("key")))
    {
        _PrintKey();
    }
    else if (_buffer.startsWith(F("clock ")))
    {
        _SetClock(_buffer.c_str() + 6);
        _PrintClock();
    }
    else if (_buffer.startsWith(F("clock")))
    {
        _PrintClock();
    }
    else
    {
        Serial.print(F("Unknown command: "));
        Serial.println(_buffer);
    }
}

namespace {

int16_t FromHex(char digit)
{
    if (!digit) return -1;
    switch (digit)
    {
    case '0'...'9': return digit - '0';
    case 'A'...'F': return digit - 'A' + 10;
    case 'a'...'f': return digit - 'a' + 10;
    default: return -1;
    }
}

} //namespace;

void Shell::_SetKey(const char *hex)
{
    uint8_t key[Context::KEY_SIZE] = {0};
    for (uint8_t i = 0; i < sizeof(key); ++i)
    {
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
    for (uint8_t i = 0; i < Context::KEY_SIZE; ++i)
    {
        char buf[3];
        sprintf(buf, "%02X", key[i]);
        Serial.print(buf);
    }
    Serial.println();
}

void Shell::_SetClock(const char *str)
{
    uint32_t clock = 0;
    while (char ch = *str++)
    {
        if (ch < '0' || ch > '9')
            break;
        clock = clock * 10 + (ch - '0');
    }
    _context.SetClock(clock);
}

void Shell::_PrintClock()
{
    Serial.println(_context.GetClock());
}

void Shell::_PrintId()
{
    Serial.println(_context.GetId());
}

void Shell::_SetId(const char *str)
{
    uint8_t id = 0;
    while (char ch = *str++)
    {
        if (ch < '0' || ch > '9')
            break;
        id = id * 10 + (ch - '0');
    }
    _context.SetId(id);
}
