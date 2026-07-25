package com.sakhnik.arduinopunch.station

import android.Manifest
import android.bluetooth.le.ScanResult
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.clickable
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

@Composable
fun StationScreen() {
    val context = LocalContext.current
    val viewModel: StationViewModel = viewModel()

    val peripherals = remember { mutableStateListOf<BluetoothPeripheral>() }

    val permissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        // Check permissions and start scanning
    }

    val handler = remember {
        Handler(Looper.getMainLooper())
    }

    lateinit var central: BluetoothCentralManager

    val centralCallback = remember {
        object : BluetoothCentralManagerCallback() {

            override fun onDisconnected(peripheral: BluetoothPeripheral, status: HciStatus) {
                Log.i(null, "*** Disconnected ${peripheral.name}")
                viewModel.setPeripheral(null)
                central.scanForPeripheralsWithServices(setOf(SHELL_SERVICE_UUID));
            }

            override fun onDiscovered(peripheral: BluetoothPeripheral, scanResult: ScanResult) {
                val name = peripheral.name
                Log.i(null, "onDiscovered $name")
                if (!name.startsWith("AOP ")) return

                val index = peripherals.indexOfFirst {
                    it.address == peripheral.address
                }

                if (index >= 0) {
                    peripherals[index] = peripheral
                } else {
                    peripherals += peripheral
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

    DisposableEffect(Unit) {
        central.scanForPeripheralsWithServices(setOf(SHELL_SERVICE_UUID));

        onDispose {
            central.stopScan()
        }
    }

    if (viewModel.connectedPeripheral == null) {

        LazyColumn {
            items(
                peripherals,
                key = { it.address }
            ) { peripheral ->
                ListItem(
                    headlineContent = {
                        Text(peripheral.name)
                    },
                    supportingContent = {
                        Text(peripheral.address)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable {
                            central.stopScan()
                            viewModel.setPeripheral(peripheral)
                            central.connect(peripheral, viewModel.peripheralCallback)
                        }
                )
            }
        }

    } else {
        ShellScreen(viewModel, central)
    }
}
