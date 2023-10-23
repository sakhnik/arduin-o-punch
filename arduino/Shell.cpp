#include "Shell.h"
#include "Context.h"

namespace {
const uint8_t MAX_SIZE = 64;

void Prompt()
{
    Serial.print(F("Arduin-o-punch> "));
}

} // namespace;

Shell::Shell(Context &context)
    : _context{context}
{
}

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
        Serial.print(ch);
        _buffer += ch;
        if (_buffer.length() >= MAX_SIZE || _buffer.endsWith("\r"))
        {
            Serial.println();
            _Process();
            _buffer.remove(0, _buffer.length());
            Prompt();
        }
	}
}

void Shell::_Process()
{
    if (_buffer.startsWith(F("help")))
    {
        Serial.println(F("Help message"));
    }
    else if (_buffer.startsWith(F("key ")))
    {
        _SetKey(_buffer.c_str() + 4);
        _PrintKey();
    }
    else if (_buffer.startsWith(F("key")))
    {
        _PrintKey();
    }
    else
    {
        Serial.println("Unknown command");
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
    Serial.print(F("Key="));
    for (uint8_t i = 0; i < Context::KEY_SIZE; ++i)
    {
        char buf[3];
        sprintf(buf, "%02X", key[i]);
        Serial.print(buf);
    }
    Serial.println();
}
