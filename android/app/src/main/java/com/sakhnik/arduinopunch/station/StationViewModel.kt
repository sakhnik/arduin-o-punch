package com.sakhnik.arduinopunch.station

import android.app.Application
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.le.ScanResult
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.welie.blessed.BluetoothCentralManager
import com.welie.blessed.BluetoothCentralManagerCallback
import com.welie.blessed.BluetoothPeripheral
import com.welie.blessed.BluetoothPeripheralCallback
import com.welie.blessed.GattStatus
import com.welie.blessed.HciStatus
import com.welie.blessed.WriteType
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.util.UUID
import kotlin.collections.plusAssign
import kotlin.math.abs
import kotlin.time.Duration.Companion.milliseconds

val SHELL_SERVICE_UUID: UUID = UUID.fromString("16404bac-eab0-422c-955f-fb13799c00fa")
private val SHELL_STDIN_UUID = UUID.fromString("16404bac-eab1-422c-955f-fb13799c00fa")
private val SHELL_STDOUT_UUID = UUID.fromString("16404bac-eab2-422c-955f-fb13799c00fa")


data class DiscoveredPeripheral(
    val peripheral: BluetoothPeripheral,
    val rssi: Int,
    var lastSeen: Long,
    val lastRssiUpdate: Long
)

class StationViewModel(application: Application) : AndroidViewModel(application) {

    private val handler = Handler(Looper.getMainLooper())

    private val _peripherals = mutableStateListOf<DiscoveredPeripheral>()
    val peripherals: List<DiscoveredPeripheral> get() = _peripherals

    private val centralCallback = object : BluetoothCentralManagerCallback() {

        override fun onDisconnected(peripheral: BluetoothPeripheral, status: HciStatus) {
            setPeripheral(null)
            startScan()
        }

        override fun onDiscovered(peripheral: BluetoothPeripheral, scanResult: ScanResult) {
            val name = peripheral.name
            if (!name.startsWith("AOP ")) return

            val now = SystemClock.elapsedRealtime()

            val index = peripherals.indexOfFirst {
                it.peripheral.address == peripheral.address
            }

            if (index >= 0) {
                val old = peripherals[index]

                _peripherals[index] = if (now - old.lastRssiUpdate >= 500 && abs(scanResult.rssi - old.rssi) >= 5) {
                    old.copy(
                        peripheral = peripheral,
                        rssi = scanResult.rssi,
                        lastSeen = now,
                        lastRssiUpdate = now
                    )
                } else {
                    old.copy(
                        peripheral = peripheral,
                        lastSeen = now
                    )
                }
            } else {
                _peripherals += DiscoveredPeripheral(
                    peripheral = peripheral,
                    rssi = scanResult.rssi,
                    lastSeen = now,
                    lastRssiUpdate = now
                )
            }
        }
    }

    private val central = BluetoothCentralManager(
        getApplication(),
        centralCallback,
        handler
    )


    init {
        viewModelScope.launch {
            while (isActive) {
                delay(1000.milliseconds)

                val now = SystemClock.elapsedRealtime()
                _peripherals.removeAll {
                    now - it.lastSeen > 5000
                }
            }
        }
    }

    private val _lines = mutableStateListOf<String>()
    val lines: List<String> get() = _lines

    private val rxBuffer = mutableListOf<Byte>()
    private val responseBuffer = StringBuilder()

    var connectedPeripheral by mutableStateOf<BluetoothPeripheral?>(null)
        private set

    fun startScan() {
        central.scanForPeripheralsWithServices(setOf(SHELL_SERVICE_UUID))
    }

    fun stopScan() {
        central.stopScan()
    }

    fun connect(item: DiscoveredPeripheral) {
        stopScan()
        setPeripheral(item.peripheral)
        central.connect(item.peripheral, peripheralCallback)
    }

    fun disconnect() {
        connectedPeripheral?.let {
            central.cancelConnection(it)
        }
    }

    fun setPeripheral(peripheral: BluetoothPeripheral?) {
        connectedPeripheral = peripheral
        _lines.clear()
    }

    private val peripheralCallback = object : BluetoothPeripheralCallback() {

        override fun onServicesDiscovered(peripheral: BluetoothPeripheral) {
            peripheral.startNotify(SHELL_SERVICE_UUID, SHELL_STDOUT_UUID)
            send("info")
        }

        override fun onCharacteristicUpdate(
            peripheral: BluetoothPeripheral,
            value: ByteArray,
            characteristic: BluetoothGattCharacteristic,
            status: GattStatus
        ) {
            rxBuffer += value.toList()

            while (true) {
                val end = findCrlf(rxBuffer)
                if (end < 0) break

                val line = rxBuffer.subList(0, end).toByteArray().decodeToString()

                _lines += line
                responseBuffer.append(line).append('\n')

                rxBuffer.subList(0, end + 2).clear()

                // Blank line -> end of response
                if (line.isEmpty()) {
                    responseBuffer.clear()
                }
            }
        }
    }

    private fun findCrlf(buffer: List<Byte>): Int {
        for (i in 0 until buffer.size - 1) {
            if (buffer[i] == '\r'.code.toByte() &&
                buffer[i + 1] == '\n'.code.toByte()
            ) {
                return i
            }
        }
        return -1
    }

    fun send(command: String) {
        connectedPeripheral?.writeCharacteristic(
            SHELL_SERVICE_UUID,
            SHELL_STDIN_UUID,
            "$command\r\n".toByteArray(),
            WriteType.WITH_RESPONSE
        )
    }
}
