from __future__ import annotations

import asyncio
import struct
from pathlib import Path
from typing import Callable

from bleak import BleakClient, BleakScanner
from bleak.backends.device import BLEDevice


OTA_SVC = "00008018-0000-1000-8000-00805f9b34fb"
RECV_CHAR = "00008020-0000-1000-8000-00805f9b34fb"
CMD_CHAR = "00008022-0000-1000-8000-00805f9b34fb"

DIS_SVC = "0000180a-0000-1000-8000-00805f9b34fb"
DIS_MODEL = "00002a24-0000-1000-8000-00805f9b34fb"
DIS_SERIAL = "00002a25-0000-1000-8000-00805f9b34fb"
DIS_FW = "00002a26-0000-1000-8000-00805f9b34fb"
DIS_HW = "00002a27-0000-1000-8000-00805f9b34fb"
DIS_MANUFACTURER = "00002a29-0000-1000-8000-00805f9b34fb"

CMD_FLASH = 0x0001
CMD_STOP = 0x0002
CMD_ACK = 0x0003
CMD_SPIFFS = 0x0004

SECTOR_SIZE = 4096
DEFAULT_PAYLOAD = 507

MTU_CANDIDATES = [510, 247, 185, 122, 23]


# -----------------------------------------------------------------------------
# CRC16 CCITT
# -----------------------------------------------------------------------------


def crc16(data: bytes | bytearray | memoryview, init: int = 0) -> int:
    crc = init

    for b in data:
        crc ^= b << 8

        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

    return crc & 0xFFFF


def crc16_incremental(previous: int, chunk: bytes) -> int:
    return crc16(chunk, previous)


# -----------------------------------------------------------------------------
# Packet builders
# -----------------------------------------------------------------------------


def build_command(cmd: int, payload: bytes = b"") -> bytes:
    """
    20-byte command packet.

        uint16 command
        uint8  payload[16]
        uint16 crc
    """

    packet = bytearray(20)

    struct.pack_into("<H", packet, 0, cmd)

    payload = payload[:16]
    packet[2:2 + len(payload)] = payload

    crc = crc16(packet[:18])

    struct.pack_into("<H", packet, 18, crc)

    return bytes(packet)


def build_sector_packets(
    sector: bytes,
    sector_index: int,
    mtu_payload: int,
) -> list[bytes]:

    packets: list[bytes] = []

    crc = 0
    offset = 0
    sequence = 0

    while offset < len(sector):

        size = min(mtu_payload, len(sector) - offset)

        chunk = sector[offset:offset + size]
        offset += size

        crc = crc16_incremental(crc, chunk)

        last = offset >= len(sector)

        if last:
            pkt = bytearray(3 + len(chunk) + 2)
        else:
            pkt = bytearray(3 + len(chunk))

        struct.pack_into("<H", pkt, 0, sector_index)

        pkt[2] = 0xFF if last else sequence

        pkt[3:3 + len(chunk)] = chunk

        if last:
            struct.pack_into("<H", pkt, 3 + len(chunk), crc)

        packets.append(bytes(pkt))

        sequence += 1

    return packets


# -----------------------------------------------------------------------------
# Client
# -----------------------------------------------------------------------------


class BleOtaClient:

    def __init__(
        self,
        device: str | BLEDevice,
        adapter='hci1',
        *,
        progress: Callable[[int, int], None] | None = None,
    ):
        self._device = device
        self._adapter = adapter
        self._progress = progress

        self.client: BleakClient | None = None

        self.mtu_payload = DEFAULT_PAYLOAD

        #
        # Command ACK state
        #

        self._cmd_future: asyncio.Future[int] | None = None
        self._expected_cmd = 0

        #
        # Firmware ACK state
        #

        self._fw_future: asyncio.Future[bool] | None = None
        self._expected_sector = 0

    #
    # ------------------------------------------------------------------
    #

    async def __aenter__(self):
        await self.connect()
        return self

    async def __aexit__(self, *_):
        await self.disconnect()

    #
    # ------------------------------------------------------------------
    #

    async def connect(self):
        if isinstance(self._device, str):
            device = await BleakScanner.find_device_by_name(self._device)

            if device is None:
                raise RuntimeError(f"Device '{self._device}' not found")
        else:
            device = self._device

        self.client = BleakClient(device, adapter=self._adapter)

        await self.client.connect()
        await self.client.start_notify(CMD_CHAR, self._on_command_notification,)
        await self.client.start_notify(RECV_CHAR, self._on_firmware_notification,)
        await self.probe_mtu()

    async def disconnect(self):
        if self.client is None:
            return

        try:
            await self.client.stop_notify(CMD_CHAR)
        except Exception:
            pass

        try:
            await self.client.stop_notify(RECV_CHAR)
        except Exception:
            pass

        await self.client.disconnect()

    #
    # ------------------------------------------------------------------
    #

    async def read_dis(self) -> dict[str, str]:

        assert self.client is not None

        chars = {
            "model": DIS_MODEL,
            "serial": DIS_SERIAL,
            "firmware": DIS_FW,
            "hardware": DIS_HW,
            "manufacturer": DIS_MANUFACTURER,
        }

        values = {}

        for name, uuid in chars.items():

            try:
                raw = await self.client.read_gatt_char(uuid)
                values[name] = raw.decode("utf-8").replace("\0", "").strip()
            except Exception:
                values[name] = ""

        return values

    #
    # ------------------------------------------------------------------
    #

    def _on_command_notification(self, _: object, data: bytearray):

        if len(data) < 20:
            return

        crc_received = struct.unpack_from("<H", data, 18)[0]

        if crc16(data[:18]) != crc_received:
            return

        cmd = struct.unpack_from("<H", data, 0)[0]

        if cmd != CMD_ACK:
            return

        for_cmd = struct.unpack_from("<H", data, 2)[0]
        answer = struct.unpack_from("<H", data, 4)[0]

        if for_cmd != self._expected_cmd:
            return

        if self._cmd_future is None:
            return

        if self._cmd_future.done():
            return

        self._cmd_future.set_result(answer)

    #
    # ------------------------------------------------------------------
    #

    def _on_firmware_notification(self, _: object, data: bytearray):

        if len(data) < 4:
            return

        sector, ack = struct.unpack_from("<HH", data)

        if sector != self._expected_sector:
            return

        if self._fw_future is None:
            return

        if self._fw_future.done():
            return

        self._fw_future.set_result(ack == 0)

    #
    # ------------------------------------------------------------------
    #

    async def probe_mtu(self):
        """
        Probe the largest payload accepted by the peripheral.

        Like the Web Bluetooth implementation, Bleak does not expose the
        negotiated ATT MTU on every backend, so we determine it by trial.
        """

        assert self.client is not None

        #
        # BlueZ exposes the negotiated ATT MTU.
        #

        mtu = getattr(self.client, "mtu_size", None)

        if mtu and mtu > 23:
            #
            # ATT header = 3 bytes
            # OTA packet header = 3 bytes
            #

            self.mtu_payload = mtu - 6
            return self.mtu_payload

        #
        # Fallback for platforms that don't expose MTU.
        #

        for candidate in MTU_CANDIDATES:
            try:
                await self.client.write_gatt_char(RECV_CHAR, bytes(candidate), response=False,)
                self.mtu_payload = candidate - 3
                return self.mtu_payload

            except Exception:
                pass

        self.mtu_payload = 20
        return self.mtu_payload

    #
    # ------------------------------------------------------------------
    #

    async def _wait_for_command(
        self,
        timeout: float,
    ) -> int:

        if self._cmd_future is None:
            raise RuntimeError("No command pending")

        try:
            return await asyncio.wait_for(self._cmd_future, timeout,)
        finally:
            self._cmd_future = None

    #
    # ------------------------------------------------------------------
    #

    async def _wait_for_sector(
        self,
        timeout: float,
    ) -> bool:

        if self._fw_future is None:
            raise RuntimeError("No sector pending")

        try:
            return await asyncio.wait_for(self._fw_future, timeout,)
        finally:
            self._fw_future = None

    #
    # ------------------------------------------------------------------
    #

    async def _send_command(
        self,
        command: int,
        payload: bytes = b"",
        timeout: float = 15,
    ):

        assert self.client is not None

        self._expected_cmd = command
        self._cmd_future = asyncio.get_running_loop().create_future()

        await self.client.write_gatt_char(CMD_CHAR, build_command(command, payload), response=False,)

        answer = await self._wait_for_command(timeout)

        #
        # JavaScript:
        #
        #   0 = OK
        #   3 = signature verification failed
        #   anything else = generic NACK
        #

        if answer == 0:
            return

        if answer == 3:
            raise RuntimeError("Firmware signature verification failed")

        raise RuntimeError(f"Command 0x{command:04x} rejected (0x{answer:04x})")

    #
    # ------------------------------------------------------------------
    #

    async def _send_sector_once(
        self,
        sector: bytes,
        sector_index: int,
    ) -> bool:

        assert self.client is not None

        packets = build_sector_packets(sector, sector_index, self.mtu_payload,)

        self._expected_sector = sector_index
        self._fw_future = asyncio.get_running_loop().create_future()

        #
        # Burst transmission.
        #
        pending = []

        for packet in packets:

            pending.append(
                asyncio.create_task(
                    self.client.write_gatt_char(RECV_CHAR, packet, response=False,)
                )
            )

            #
            # Limit number of outstanding writes.
            #

            if len(pending) >= 8:
                await asyncio.gather(*pending)
                pending.clear()

        if pending:
            await asyncio.gather(*pending)

        return await self._wait_for_sector(15)

    #
    # ------------------------------------------------------------------
    #

    async def _send_sector(
        self,
        sector: bytes,
        sector_index: int,
        retries: int = 3,
    ):

        for attempt in range(retries):

            if await self._send_sector_once(sector, sector_index,):
                return

            if attempt + 1 == retries:
                break

            #
            # Same pause as the JavaScript client.
            #

            await asyncio.sleep(0.2)

        raise RuntimeError(f"Sector {sector_index} failed after {retries} attempts")

    #
    # ------------------------------------------------------------------
    #

    @staticmethod
    def _iter_sectors(data: bytes):
        offset = 0
        while offset < len(data):
            yield (offset // SECTOR_SIZE, data[offset:offset + SECTOR_SIZE],)
            offset += SECTOR_SIZE

    #
    # ------------------------------------------------------------------
    #

    async def flash(
        self,
        firmware: str | Path,
        *,
        spiffs: bool = False,
    ):
        """
        Upload firmware using the BLEOTA protocol.
        """

        firmware = Path(firmware)
        image = firmware.read_bytes()

        total = len(image)
        command = CMD_SPIFFS if spiffs else CMD_FLASH

        #
        # START command
        #

        payload = struct.pack("<I", total)

        await self._send_command(command, payload, timeout=15,)

        written = 0

        #
        # Send sectors
        #

        for sector_index, sector in self._iter_sectors(image):
            await self._send_sector(sector, sector_index,)
            written += len(sector)

            if self._progress is not None:
                self._progress(written, total,)

        #
        # STOP command.
        #
        # Firmware signature verification (secure OTA) happens here.
        #

        await self._send_command(CMD_STOP, timeout=15,)

        #
        # Device will reboot shortly.
        #

        await asyncio.sleep(0.5)

        try:
            await self.disconnect()
        except Exception:
            pass


# -----------------------------------------------------------------------------
# CLI
# -----------------------------------------------------------------------------


async def discover() -> list[BLEDevice]:

    devices = await BleakScanner(bluez={"adapter": "hci1"}).discover(return_adv=True)
    print(devices)

    return [d for name, d in devices.values() if OTA_SVC in [u.lower() for u in d.service_uuids]]


async def flash(
    device: str,
    firmware: str,
):

    def progress(done: int, total: int):

        pct = done * 100 // total

        print(f"\r{pct:3d}%  {done}/{total} bytes", end="", flush=True,)

    async with BleOtaClient(
        device,
        'hci1',
        progress=progress,
    ) as ota:

        print("Connected")

        try:
            dis = await ota.read_dis()

            print("Manufacturer :", dis["manufacturer"])
            print("Model        :", dis["model"])
            print("Firmware     :", dis["firmware"])
            print("Hardware     :", dis["hardware"])
            print()

        except Exception:
            pass

        print(f"Uploading {firmware}...")

        await ota.flash(firmware)

        print("\nUpload complete.")


async def main():

    import argparse

    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd", required=True,)
    sub.add_parser("scan")
    p = sub.add_parser("flash")
    p.add_argument("device", help="BLE device name",)
    p.add_argument("firmware",)

    args = parser.parse_args()

    if args.cmd == "scan":

        devices = await discover()

        if not devices:
            print("No BLEOTA devices found.")
            return

        for d in devices:
            print(f"{d.name or '<unnamed>':20s}  {d.address}")

        return

    if args.cmd == "flash":

        await flash(args.device, args.firmware,)


if __name__ == "__main__":
    asyncio.run(main())
