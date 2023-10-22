#include "Shell.h"

namespace {
const int MAX_SIZE = 64;

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
	if (Serial.available())
	{
	    char ch = Serial.read();
	    Serial.print(ch);
		_buffer += ch;
        if (_buffer.length() >= MAX_SIZE || _buffer.endsWith("\r"))
        {
            Serial.println();
            int rem = _buffer.length();
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
    else
    {
        Serial.println("Unknown command");
    }
}
