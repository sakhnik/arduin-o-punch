package com.sakhnik.arduinopunch.station

import android.Manifest
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
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel

@Composable
fun StationScreen() {
    val viewModel: StationViewModel = viewModel()

    val permissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) {
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
        viewModel.startScan()

        onDispose {
            viewModel.stopScan()
        }
    }

    if (viewModel.connectedPeripheral == null) {

        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
        ) {
            items(viewModel.peripherals, key = { it.peripheral.address }) { item ->
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
                            viewModel.connect(item)
                        }
                )
            }
        }

    } else {
        ShellScreen(viewModel)
    }
}
