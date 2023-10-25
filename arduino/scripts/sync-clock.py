#!/usr/bin/env python

# Synchronize Arduin-o-punch clock via serial port

import serial
from datetime import datetime

serial_port = '/dev/ttyUSB0'
baud_rate = 9600


def get_current_time():
    """Get the current time in milliseconds since 8:00."""
    now = datetime.now()
    eight_am = now.replace(hour=8, minute=0, second=0, microsecond=0)
    elapsed_ms = int((now - eight_am).total_seconds() * 1000)
    return elapsed_ms


with serial.Serial(serial_port, baud_rate, timeout=1) as ser:
    # Get current time before interacting with Arduino
    start = get_current_time()
    # Get Arduin-o-punch clock reading
    ser.write(b'clock\r')
    arduino_clock = int(ser.readline().decode().strip())
    print(f"Before: clock={arduino_clock}")
    # Get current time again
    finish = get_current_time()

    latency = (finish - start) // 2
    clock_offset = start + latency - arduino_clock

    # Update Arduino clock
    ser.write(f'clock {finish + latency}\r'.encode())
    arduino_clock = int(ser.readline().decode().strip())
    print(f"After: clock={arduino_clock}")
