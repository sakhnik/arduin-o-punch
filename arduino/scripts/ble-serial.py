#!/usr/bin/env python

from ble_connection import BleConnection


def handle_stdout(data):
    try:
        print(data.decode(), end="", flush=True)
    except UnicodeDecodeError:
        print(data)


def main():
    with BleConnection(on_receive=handle_stdout) as conn:
        print("=== Connected. Type commands ===")

        while True:
            line = input()

            if line.strip().lower() in ("exit", "quit"):
                break

            conn.write((line + "\n").encode())


if __name__ == "__main__":
    main()
