package com.sakhnik.arduinopunch.station

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.consumeWindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import com.welie.blessed.BluetoothCentralManager

@Composable
fun ShellScreen(viewModel: StationViewModel, central: BluetoothCentralManager) {
    var command by rememberSaveable { mutableStateOf("") }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .consumeWindowInsets(PaddingValues())
            .padding(16.dp)
    ) {

        Text(
            text = viewModel.connectedPeripheral!!.name,
            style = MaterialTheme.typography.titleMedium
        )

        Spacer(Modifier.height(8.dp))

        LazyColumn(
            modifier = Modifier
                .weight(1f)
                .fillMaxWidth(),
            reverseLayout = true
        ) {
            items(viewModel.lines.reversed()) { line ->
                Text(
                    line,
                    modifier = Modifier.padding(vertical = 2.dp)
                )
            }
        }

        Spacer(Modifier.height(8.dp))

        Row {
            OutlinedTextField(
                value = command,
                onValueChange = { command = it },
                modifier = Modifier.weight(1f),
                singleLine = true,
                label = { Text(text = "Command") },
                keyboardOptions = KeyboardOptions(
                    imeAction = ImeAction.Send
                ),
                keyboardActions = KeyboardActions(
                    onSend = {
                        if (command.isNotBlank()) {
                            viewModel.send(command)
                            command = ""
                        }
                    }
                )
            )

            Spacer(Modifier.width(8.dp))

            Button(
                onClick = {
                    if (command.isNotBlank()) {
                        viewModel.send(command)
                        command = ""
                    }
                }
            ) {
                Text("Send")
            }
        }

        Spacer(Modifier.height(8.dp))

        OutlinedButton(
            onClick = {
                viewModel.connectedPeripheral?.let {
                    central.cancelConnection(it)
                }
            },
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("Disconnect")
        }
    }
}
