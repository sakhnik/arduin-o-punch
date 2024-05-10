#!/usr/bin/env python

# Synchronize Arduin-o-punch clock via serial port or TCP

from datetime import datetime, timedelta, timezone
import argparse
from connection import Connection, SerialConnection, TcpConnection

serial_port = '/dev/ttyUSB0'
baud_rate = 9600

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help=f"serial port {serial_port}",
                    type=str)
parser.add_argument("-t", "--tcp", help="TCP connection", type=str)
parser.add_argument("-s", "--script", help="file with additional commands",
                    type=str)
parser.add_argument("--id", help="set station id", type=int)
args = parser.parse_args()


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


def run(conn: Connection):
    # Wait for a clear prompt
    conn.write(b'\r')
    idle_count = 0
    while True:
        line = conn.read()
        echo_output(line)
        if b'Arduin-o-punch> ' == line:
            break
        if line:
            idle_count = 0
            continue
        idle_count += 1
        if idle_count == 3:
            break

    # Get current time before interacting with Arduino
    start = get_current_time()
    # Get Arduin-o-punch clock reading
    conn.write(b'clock\r')
    line = conn.read()
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
    conn.write(f'timestamp {target_timestamp}\r'.encode())
    line = conn.read()
    echo_output(line)
    timestamp = int(line.decode().strip())
    dt = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    print(f"After: {dt.strftime('%Y-%m-%d %H:%M:%S')}")

    if args.id is not None:
        conn.write(f'id {args.id}\r'.encode())
        resp = conn.read()
        echo_output(resp)

    if args.script:
        with open(args.script, 'r') as f:
            for line in f.readlines():
                print(line.strip())
                conn.write(f'{line.strip()}\r'.encode())
                resp = conn.read()
                echo_output(resp)


if args.tcp:
    with TcpConnection(args.tcp) as conn:
        run(conn)
else:
    if args.port:
        serial_port = args.port
    with SerialConnection(serial_port, baud_rate) as conn:
        run(conn)
