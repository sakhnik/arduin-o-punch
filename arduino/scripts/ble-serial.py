#!/usr/bin/env python

import asyncio
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "16404bac-eab0-422c-955f-fb13799c00fa"
STDIN_UUID = "16404bac-eab1-422c-955f-fb13799c00fa"
STDOUT_UUID = "16404bac-eab2-422c-955f-fb13799c00fa"


async def main():
    print("Scanning...")
    scanner = BleakScanner(adapter='hci1')
    devices = await scanner.discover(return_adv=True)

    target = None
    for d, adv in devices.values():
        uuids = adv.service_uuids or []
        if SERVICE_UUID.lower() in [u.lower() for u in uuids]:
            target = d
            break

    if not target:
        print("Device not found")
        return

    print(f"Connecting to {target.address}...")

    async with BleakClient(target.address, adapter='hci1') as client:
        print("Connected")

        # callback для stdout
        def handle_stdout(_, data: bytearray):
            try:
                print(data.decode(), end="", flush=True)
            except:
                print(data)

        await client.start_notify(STDOUT_UUID, handle_stdout)

        print("=== Connected. Type commands ===")

        loop = asyncio.get_event_loop()

        while True:
            # читаємо з stdin без блокування event loop
            line = await loop.run_in_executor(None, input)

            if line.strip().lower() in ("exit", "quit"):
                break

            await client.write_gatt_char(STDIN_UUID, (line + "\n").encode())

        await client.stop_notify(STDOUT_UUID)


if __name__ == "__main__":
    asyncio.run(main())
