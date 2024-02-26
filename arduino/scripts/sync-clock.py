#!/usr/bin/env python

# Synchronize Arduin-o-punch clock via serial port

import serial
from datetime import datetime, timedelta, timezone

serial_port = '/dev/ttyUSB0'
baud_rate = 9600


def get_current_time():
    """Get the current time of day in milliseconds"""
    now = datetime.now()
    time_origin = now.replace(hour=0, minute=0, second=0, microsecond=0)
    elapsed_ms = int((now - time_origin).total_seconds() * 1000)
    return elapsed_ms


def format_time(clock):
    seconds = clock / 1000
    time_difference = timedelta(seconds=seconds)
    formatted_time = (datetime(1, 1, 1) + time_difference).strftime("%H:%M:%S")
    return formatted_time


def echo_output(data):
    print(">> " + str(data))


with serial.Serial(serial_port, baud_rate, timeout=1) as ser:
    # Wait for a clear prompt
    ser.write(b'\r')
    while True:
        line = ser.readline()
        echo_output(line)
        if b"Arduin-o-punch> " == line:
            break

    # Get current time before interacting with Arduino
    start = get_current_time()
    # Get Arduin-o-punch clock reading
    ser.write(b'clock\r')
    line = ser.readline()
    echo_output(line)
    arduino_clock = int(line.decode().strip())
    arduino_time = format_time(arduino_clock)
    print(f"Before: clock={arduino_clock} time={arduino_time}")
    # Get current time again
    finish = get_current_time()

    latency_us = 1000 * (finish - start) / 2
    assert latency_us < 1000000
    # Aim at the edge of the second
    while True:
        now_us = datetime.now().microsecond
        dt_us = latency_us + now_us - 1000000
        if dt_us >= 0 and dt_us < 1000:
            break

    now = datetime.now().replace(tzinfo=timezone.utc)
    target_timestamp = int(now.timestamp()) + 1

    # Update Arduino clock
    ser.write(f'timestamp {target_timestamp}\r'.encode())
    line = ser.readline()
    echo_output(line)
    timestamp = int(line.decode().strip())
    dt = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    print(f"After: {dt.strftime('%Y-%m-%d %H:%M:%S')}")
