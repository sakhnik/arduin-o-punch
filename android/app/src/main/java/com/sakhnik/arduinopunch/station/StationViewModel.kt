package com.sakhnik.arduinopunch.station

import android.app.Application
import android.bluetooth.BluetoothGattCharacteristic
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import com.welie.blessed.BluetoothPeripheral
import com.welie.blessed.BluetoothPeripheralCallback
import com.welie.blessed.GattStatus
import com.welie.blessed.WriteType
import java.util.UUID

val SHELL_SERVICE_UUID: UUID = UUID.fromString("16404bac-eab0-422c-955f-fb13799c00fa")
private val SHELL_STDIN_UUID = UUID.fromString("16404bac-eab1-422c-955f-fb13799c00fa")
private val SHELL_STDOUT_UUID = UUID.fromString("16404bac-eab2-422c-955f-fb13799c00fa")

class StationViewModel(application: Application) : AndroidViewModel(application) {

    private val handler = Handler(Looper.getMainLooper())

    private val _lines = mutableStateListOf<String>()
    val lines: List<String> get() = _lines

    private val rxBuffer = mutableListOf<Byte>()
    private val responseBuffer = StringBuilder()

    var connectedPeripheral by mutableStateOf<BluetoothPeripheral?>(null)
        private set

    fun setPeripheral(peripheral: BluetoothPeripheral?) {
        connectedPeripheral = peripheral
        _lines.clear()
    }

    val peripheralCallback = object : BluetoothPeripheralCallback() {

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
                    Log.i("BLE", "Complete response:\n$responseBuffer")
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
        Log.i(null, "Send $command")
        connectedPeripheral?.writeCharacteristic(
            SHELL_SERVICE_UUID,
            SHELL_STDIN_UUID,
            "$command\r\n".toByteArray(),
            WriteType.WITH_RESPONSE
        )
    }
}
