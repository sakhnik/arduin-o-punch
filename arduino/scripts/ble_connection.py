import asyncio
import threading

from bleak import BleakClient, BleakScanner

from connection import Connection


SERVICE_UUID = "16404bac-eab0-422c-955f-fb13799c00fa"
STDIN_UUID   = "16404bac-eab1-422c-955f-fb13799c00fa"
STDOUT_UUID  = "16404bac-eab2-422c-955f-fb13799c00fa"

ADAPTER = "hci0"


class BleConnection(Connection):
    def __init__(self, on_receive=None):
        self._on_receive = on_receive
        self._loop = asyncio.new_event_loop()
        self._thread = threading.Thread(target=self._loop.run_forever, daemon=True)
        self._thread.start()

        self._client = None
        self._rx = bytearray()
        self._rx_event = threading.Event()

        asyncio.run_coroutine_threadsafe(self._connect(), self._loop).result()

    async def _connect(self):
        scanner = BleakScanner(bluez={"adapter": ADAPTER})
        devices = await scanner.discover(return_adv=True)

        target = next(
            (
                device
                for device, adv in devices.values()
                if SERVICE_UUID.lower() in [
                    u.lower() for u in (adv.service_uuids or [])
                ]
            ),
            None,
        )

        if not target:
            raise RuntimeError("BLE device not found")

        print(f"Connecting to {target.name} ({target.address})...")

        self._client = BleakClient(target.address, bluez={"adapter": ADAPTER})
        await self._client.connect()

        service = self._client.services.get_service(SERVICE_UUID)
        if service is None:
            raise RuntimeError(f"Service {SERVICE_UUID} not found")

        stdout = next(c for c in service.characteristics if c.uuid.lower() == STDOUT_UUID.lower())

        await self._client.start_notify(stdout, self._handle_stdout)

        print("Connected")

    def _handle_stdout(self, _, data: bytearray):
        if self._on_receive:
            self._on_receive(data)

        self._rx.extend(data)
        self._rx_event.set()

    def write(self, data: bytes):
        data = data.replace(b"\r", b"\n")

        asyncio.run_coroutine_threadsafe(self._client.write_gatt_char(STDIN_UUID, data), self._loop).result()

    def read(self) -> bytes:
        while True:
            pos = self._rx.find(b"\n")

            if pos >= 0:
                line = bytes(self._rx[:pos + 1])
                del self._rx[:pos + 1]
                return line

            self._rx_event.clear()
            self._rx_event.wait()

    def close(self):
        if self._client:
            asyncio.run_coroutine_threadsafe(self._disconnect(), self._loop).result()

        self._loop.call_soon_threadsafe(self._loop.stop)
        self._thread.join()

    async def _disconnect(self):
        await self._client.stop_notify(STDOUT_UUID)
        await self._client.disconnect()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()
