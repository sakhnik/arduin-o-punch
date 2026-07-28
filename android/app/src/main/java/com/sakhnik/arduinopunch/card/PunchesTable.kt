package com.sakhnik.arduinopunch.card

import android.content.ClipData
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalClipboard
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.sakhnik.arduinopunch.R
import kotlinx.coroutines.launch
import java.time.DateTimeException
import java.time.LocalTime
import java.time.format.DateTimeFormatter


@Composable
fun PunchesTable(viewModel: CardViewModel) {
    val readOut by viewModel.readOut.observeAsState()
    val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")
    val clipboard = LocalClipboard.current

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
                            if (allSelected) stringResource(R.string.deselect_all) else stringResource(
                                R.string.select_all),
                            fontSize = 14.sp
                        )
                    }

                    Spacer(modifier = Modifier.width(8.dp))

                    val scope = rememberCoroutineScope()

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
                            scope.launch {
                                clipboard.nativeClipboard.setPrimaryClip(
                                    ClipData.newPlainText("text", selectedText)
                                )
                            }
                        },
                        enabled = selectedRows.isNotEmpty(),
                        contentPadding = PaddingValues(horizontal = 10.dp, vertical = 4.dp)
                    ) {
                        Text(stringResource(R.string.copy), fontSize = 14.sp)
                    }
                }
            }

            // Debug info section
            readout.debugInfo?.let { debug ->

                Spacer(modifier = Modifier.height(12.dp))

                Text(
                    text = "Debug info",
                    fontWeight = FontWeight.SemiBold,
                    fontSize = 16.sp,
                    modifier = Modifier.padding(vertical = 4.dp)
                )

                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .background(MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.2f))
                        .padding(8.dp)
                ) {
                    DebugStatsFormatter.format(debug).forEach { line ->
                        Text(
                            text = line,
                            fontSize = 13.sp
                        )
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

