package com.sakhnik.arduinopunch.station

import android.app.Application
import android.bluetooth.BluetoothGattCharacteristic
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.compose.runtime.mutableStateListOf
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

    private val rxBuffer = StringBuilder()

    private var connectedPeripheral: BluetoothPeripheral? = null

    fun setPeripheral(peripheral: BluetoothPeripheral?) {
        connectedPeripheral = peripheral
    }

    val peripheralCallback = object : BluetoothPeripheralCallback() {

        override fun onServicesDiscovered(peripheral: BluetoothPeripheral) {
            Log.i(null, "*** onServicesDiscovered")
            peripheral.startNotify(SHELL_SERVICE_UUID, SHELL_STDOUT_UUID)
            send("info")
        }

        override fun onCharacteristicUpdate(
            peripheral: BluetoothPeripheral,
            value: ByteArray,
            characteristic: BluetoothGattCharacteristic,
            status: GattStatus
        ) {
            Log.i(null, "** onCharacteristicUpdate")
            rxBuffer.append(value.toString(Charsets.UTF_8))

            while (true) {
                val end = rxBuffer.indexOf("\r\n\r\n")
                if (end < 0) break

                _lines += rxBuffer.substring(0, end)
                Log.i(null, "** lines=$lines")
                rxBuffer.delete(0, end + 4)
            }
        }
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
