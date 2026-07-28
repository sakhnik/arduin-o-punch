package com.sakhnik.arduinopunch.card

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.CloudOff
import androidx.compose.material.icons.filled.CloudUpload
import androidx.compose.material.icons.filled.Lock
import androidx.compose.material.icons.filled.LockOpen
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExposedDropdownMenuAnchorType
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuDefaults.TrailingIcon
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.sakhnik.arduinopunch.R
import com.sakhnik.arduinopunch.ui.theme.AppTheme
import java.time.DateTimeException
import java.time.LocalTime
import java.time.format.DateTimeFormatter
import kotlin.collections.emptyList
import kotlin.math.roundToInt

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ReadScreen(cardViewModel: CardViewModel) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        ScreenHeader(R.string.read_title, R.string.read_instruction)

        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(4.dp)
        ) {
            val uploadEnabled by cardViewModel.uploadEnabled.collectAsState(initial = false)
            val uploadUrl by cardViewModel.uploadUrl.collectAsState(initial = stringResource(id = R.string.https_sakhnik_com_qr_o_punch_card))
            val isLocked = remember { mutableStateOf(true) }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                IconButton(
                    onClick = {
                        cardViewModel.updateUploadEnabled(!uploadEnabled)
                    }
                ) {
                    Icon(
                        imageVector = if (uploadEnabled) Icons.Default.CloudUpload else Icons.Default.CloudOff,
                        contentDescription = stringResource(R.string.upload)
                    )
                }

                TextField(
                    value = uploadUrl,
                    onValueChange = {
                        if (!isLocked.value) {
                            cardViewModel.updateUploadUrl(it)
                        }
                    },
                    modifier = Modifier
                        .weight(1f)
                        .padding(start = 8.dp),
                    label = { Text(stringResource(R.string.url_for_qr_o_punch_upload)) },
                    leadingIcon = {
                        IconButton(onClick = { if (uploadEnabled) { isLocked.value = !isLocked.value } } ) {
                            Icon(
                                imageVector = if (isLocked.value) Icons.Default.Lock else Icons.Default.LockOpen,
                                contentDescription = null
                            )
                        }
                    },
                    enabled = uploadEnabled && !isLocked.value,
                    singleLine = false
                )
            }

            val readOutCount by cardViewModel.readOutCount
            val readOuts by cardViewModel.readOuts.observeAsState(emptyList())
            val selected by cardViewModel.selectedReadOut

            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(R.string.runs_to_read),
                    modifier = Modifier.weight(1f)
                )

                Text(
                    text = readOutCount.toString(),
                    style = MaterialTheme.typography.bodyLarge
                )
            }

            Slider(
                value = readOutCount.toFloat(),
                onValueChange = {
                    cardViewModel.updateReadOutCount(it.roundToInt())
                },
                valueRange = 1f..10f,
                steps = 8
            )

            if (readOutCount > 1 && readOuts.isNotEmpty()) {
                var expanded by remember { mutableStateOf(false) }

                ExposedDropdownMenuBox(
                    expanded = expanded,
                    onExpandedChange = { expanded = !expanded }
                ) {
                    OutlinedTextField(
                        modifier = Modifier
                            .menuAnchor(
                                ExposedDropdownMenuAnchorType.PrimaryNotEditable,
                                enabled = true
                            )
                            .fillMaxWidth(),
                        readOnly = true,
                        value = formatReadOut(readOuts[selected]),
                        onValueChange = {},
                        label = { Text(stringResource(R.string.run)) },
                        trailingIcon = {
                            TrailingIcon(expanded)
                        }
                    )

                    ExposedDropdownMenu(
                        expanded = expanded,
                        onDismissRequest = { expanded = false }
                    ) {
                        readOuts.forEachIndexed { index, info ->
                            DropdownMenuItem(
                                text = {
                                    Text(formatReadOut(info))
                                },
                                onClick = {
                                    cardViewModel.selectReadOut(index)
                                    expanded = false
                                }
                            )
                        }
                    }
                }
            }

            PunchesTable(viewModel = cardViewModel)
        }
    }
}

private val timeFormatter = DateTimeFormatter.ofPattern("HH:mm")

private fun formatReadOut(info: PunchCard.Info): String {
    val timestamp = info.punches.firstOrNull()?.let { punch ->
        try {
            LocalTime.ofSecondOfDay(punch.timestamp).format(timeFormatter)
        } catch (_: DateTimeException) {
            "--:--"
        }
    } ?: "--:--"

    return "${info.cardNumber}, $timestamp, x${info.punches.size}"
}

@Preview(showBackground = true)
@Composable
fun ReadScreenPreview() {
    AppTheme {
        val punches = listOf(
            Punch(station = 31, timestamp = 3600L),
            Punch(station = 32, timestamp = 7200L),
            Punch(station = 33, timestamp = 10800L),
            Punch(station = 100, timestamp = 10900L),
        )
        val readOut = PunchCard.Info(cardNumber = 123, punches = punches, debugInfo = null)
        val mockViewModel = remember {
            MockCardViewModel().apply {
                setReadOutForPreview(readOut)
            }
        }
        ReadScreen(mockViewModel)
    }
}
