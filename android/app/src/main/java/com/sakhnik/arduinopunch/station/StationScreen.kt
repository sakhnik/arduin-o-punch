package com.sakhnik.arduinopunch.station

import android.Manifest
import android.bluetooth.le.ScanResult
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.ListItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.viewmodel.compose.viewModel
import com.welie.blessed.BluetoothCentralManager
import com.welie.blessed.BluetoothCentralManagerCallback
import com.welie.blessed.BluetoothPeripheral
import com.welie.blessed.HciStatus
import kotlinx.coroutines.delay
import kotlin.math.abs
import kotlin.time.Duration.Companion.milliseconds

@Composable
fun StationScreen() {
    val context = LocalContext.current
    val viewModel: StationViewModel = viewModel()

    data class DiscoveredPeripheral(
        val peripheral: BluetoothPeripheral,
        val rssi: Int,
        var lastSeen: Long,
        val lastRssiUpdate: Long
    )

    val peripherals = remember { mutableStateListOf<DiscoveredPeripheral>() }

    val permissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) {
    }

    val handler = remember {
        Handler(Looper.getMainLooper())
    }

    lateinit var central: BluetoothCentralManager

    val centralCallback = remember {
        object : BluetoothCentralManagerCallback() {

            override fun onDisconnected(peripheral: BluetoothPeripheral, status: HciStatus) {
                viewModel.setPeripheral(null)
                central.scanForPeripheralsWithServices(setOf(SHELL_SERVICE_UUID))
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

                    peripherals[index] = if (now - old.lastRssiUpdate >= 500 && abs(scanResult.rssi - old.rssi) >= 5) {
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
                    peripherals += DiscoveredPeripheral(
                        peripheral = peripheral,
                        rssi = scanResult.rssi,
                        lastSeen = now,
                        lastRssiUpdate = now
                    )
                }
            }
        }
    }

    central = remember {
        BluetoothCentralManager(
            context,
            centralCallback,
            handler
        )
    }

    LaunchedEffect(Unit) {
        permissionLauncher.launch(
            arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
            )
        )
    }

    LaunchedEffect(Unit) {
        while (true) {
            delay(1000.milliseconds)

            val now = SystemClock.elapsedRealtime()
            peripherals.removeAll {
                now - it.lastSeen > 5000   // disappear after 5 seconds
            }
        }
    }

    DisposableEffect(Unit) {
        central.scanForPeripheralsWithServices(setOf(SHELL_SERVICE_UUID))

        onDispose {
            central.stopScan()
        }
    }

    if (viewModel.connectedPeripheral == null) {

        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
        ) {
            items(peripherals, key = { it.peripheral.address }) { item ->
                ListItem(
                    headlineContent = {
                        Text(item.peripheral.name)
                    },
                    supportingContent = {
                        Text("${item.peripheral.address}    ${item.rssi} dBm")
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable {
                            central.stopScan()
                            viewModel.setPeripheral(item.peripheral)
                            central.connect(item.peripheral, viewModel.peripheralCallback)
                        }
                )
            }
        }

    } else {
        ShellScreen(viewModel, central)
    }
}
