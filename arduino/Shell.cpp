#include "Shell.h"
#include <Arduino.h>
#include <SerialTerminal.h>

String project = "Arduin-o-punch";
String vers = "v1.0";

void HandleCmd(String msg)
{
	int NbChar = msg.length() - 1;
	msg.remove(NbChar);	// Remove last char (breakout char)

	if (msg.compareTo("help") == 0)
	{
		Serial.print("help msg");
	}
	else
	{
	    Serial.print("unknown command");
	}
}

Shell::Shell(int baud)
{
    term.init(115200, &HandleCmd, &project, &vers);
}
