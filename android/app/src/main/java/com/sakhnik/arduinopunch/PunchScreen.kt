package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.sakhnik.arduinopunch.ui.theme.AppTheme

const val DST_PUNCH = "punch"

@Composable
fun PunchScreen(cardViewModel: CardViewModel) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        ScreenHeader(R.string.punch_runner_title, R.string.punch_runner_instruction)

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
                Text(
                    text = stringResource(id = R.string.station_id),
                    modifier = Modifier.weight(0.4f)
                )

                val stationId by cardViewModel.stationId.collectAsState(initial = stringResource(id = R.string._31))

                TextField(
                    value = stationId,
                    onValueChange = {
                        cardViewModel.updateStationId(it)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    keyboardOptions = KeyboardOptions.Default.copy(keyboardType = KeyboardType.Number),
                    label = { Text(stringResource(id = R.string.station_id)) },
                    //maxLength = 5,
                    singleLine = true
                )

                Spacer(
                    modifier = Modifier.weight(0.6f)
                )
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun PunchScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        PunchScreen(mockViewModel)
    }
}
