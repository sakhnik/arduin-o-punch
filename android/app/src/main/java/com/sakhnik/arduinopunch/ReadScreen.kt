package com.sakhnik.arduinopunch

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Create
import androidx.compose.material.icons.filled.Lock
import androidx.compose.material3.Button
import androidx.compose.material3.Checkbox
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.sakhnik.arduinopunch.ui.theme.AppTheme
import java.time.DateTimeException
import java.time.LocalTime
import java.time.format.DateTimeFormatter

const val DST_READ = "read"

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
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                val uploadEnabled by cardViewModel.uploadEnabled.collectAsState(initial = false)

                Checkbox(
                    checked = uploadEnabled,
                    onCheckedChange = {
                        cardViewModel.updateUploadEnabled(it)
                    }
                )
                Text(stringResource(id = R.string.upload))
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                val uploadUrl by cardViewModel.uploadUrl.collectAsState(initial = stringResource(id = R.string.https_sakhnik_com_qr_o_punch_card))
                val isLocked = remember { mutableStateOf(true) }

                TextField(
                    value = uploadUrl,
                    onValueChange = {
                        if (!isLocked.value) {
                            cardViewModel.updateUploadUrl(it)
                        }
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    label = { Text(stringResource(id = R.string.url_for_qr_o_punch_upload)) },
                    leadingIcon = {
                        IconButton(onClick = { isLocked.value = !isLocked.value }) {
                            Icon(
                                imageVector = if (isLocked.value) Icons.Filled.Create else Icons.Filled.Lock,
                                contentDescription = if (isLocked.value) "Lock" else "Unlock"
                            )
                        }
                    },
                    enabled = !isLocked.value,
                    singleLine = false
                )
            }

            PunchesTable(viewModel = cardViewModel)
        }
    }
}

@Composable
fun PunchesTable(viewModel: CardViewModel) {
    val readOut by viewModel.readOut.observeAsState()
    val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")
    val clipboardManager = LocalClipboardManager.current

    val selectedRows = remember { mutableStateListOf<Int>() }

    readOut?.let { readout ->
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 4.dp)
                .wrapContentHeight()
        ) {
            // Card ID title
            Text(
                text = "${stringResource(id = R.string.card_id)} ${readout.cardNumber}",
                fontSize = 20.sp,
                fontWeight = FontWeight.SemiBold,
                modifier = Modifier
                    .align(Alignment.CenterHorizontally)
                    .padding(vertical = 4.dp)
            )

            // Table header
            TableRow(
                cells = listOf(stringResource(R.string.index),
                    stringResource(R.string.station), stringResource(R.string.timestamp)
                ),
                isHeader = true
            )

            // Scrollable table content
            LazyColumn(
                modifier = Modifier
                    .weight(1f, fill = false)
                    .fillMaxWidth()
            ) {
                itemsIndexed(readout.punches) { index, punch ->
                    val isSelected = index in selectedRows
                    val station = punch.station.toString()
                    val timestamp = try {
                        LocalTime.ofSecondOfDay(punch.timestamp).format(formatter)
                    } catch (_: DateTimeException) {
                        "???"
                    }
                    TableRow(
                        cells = listOf("${index + 1}", station, timestamp),
                        isSelected = isSelected,
                        isEven = index % 2 == 0,
                        onClick = {
                            if (isSelected) selectedRows.remove(index)
                            else selectedRows.add(index)
                        }
                    )
                }
            }

            // Buttons directly below the table (tight spacing)
            if (readout.punches.isNotEmpty()) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(top = 4.dp),
                    horizontalArrangement = Arrangement.End,
                    verticalAlignment = Alignment.Bottom
                ) {
                    val allSelected = selectedRows.size == readout.punches.size

                    Button(
                        onClick = {
                            if (allSelected) selectedRows.clear()
                            else {
                                selectedRows.clear()
                                selectedRows.addAll(readout.punches.indices)
                            }
                        },
                        contentPadding = PaddingValues(horizontal = 10.dp, vertical = 4.dp)
                    ) {
                        Text(
                            if (allSelected) stringResource(R.string.deselect_all) else stringResource(R.string.select_all),
                            fontSize = 14.sp
                        )
                    }

                    Spacer(modifier = Modifier.width(8.dp))

                    Button(
                        onClick = {
                            val selectedText = selectedRows.joinToString("\n") { index ->
                                val punch = readout.punches[index]
                                val station = punch.station.toString()
                                val timestamp = try {
                                    LocalTime.ofSecondOfDay(punch.timestamp).format(formatter)
                                } catch (_: DateTimeException) {
                                    "???"
                                }
                                "${index + 1}\t$station\t$timestamp"
                            }
                            clipboardManager.setText(AnnotatedString(selectedText))
                        },
                        enabled = selectedRows.isNotEmpty(),
                        contentPadding = PaddingValues(horizontal = 10.dp, vertical = 4.dp)
                    ) {
                        Text(stringResource(R.string.copy), fontSize = 14.sp)
                    }
                }
            }
        }
    }
}

@Composable
fun TableRow(
    cells: List<String>,
    isHeader: Boolean = false,
    isSelected: Boolean = false,
    isEven: Boolean = false,
    onClick: (() -> Unit)? = null
) {
    val background = when {
        isHeader -> MaterialTheme.colorScheme.primary.copy(alpha = 0.1f)
        isSelected -> MaterialTheme.colorScheme.secondary.copy(alpha = 0.2f)
        isEven -> MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.1f)
        else -> Color.Transparent
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(background)
            .then(if (onClick != null) Modifier.clickable(onClick = onClick) else Modifier)
            .padding(vertical = 4.dp, horizontal = 6.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        cells.forEachIndexed { _, cell ->
            Text(
                text = cell,
                modifier = Modifier
                    .weight(1f)
                    .padding(horizontal = 2.dp),
                textAlign = TextAlign.Center,
                fontWeight = if (isHeader) FontWeight.Bold else FontWeight.Normal,
                fontSize = if (isHeader) 14.sp else 13.sp,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
        }
    }
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
        val readOut = PunchCard.Info(cardNumber = 123, punches = punches)
        val mockViewModel = MockCardViewModel().apply {
            setReadOutForPreview(readOut)
        }
        ReadScreen(mockViewModel)
    }
}
