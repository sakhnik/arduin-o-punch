package com.sakhnik.arduinopunch.card

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Create
import androidx.compose.material.icons.filled.Lock
import androidx.compose.material3.Checkbox
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.sakhnik.arduinopunch.R
import com.sakhnik.arduinopunch.ui.theme.AppTheme

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
