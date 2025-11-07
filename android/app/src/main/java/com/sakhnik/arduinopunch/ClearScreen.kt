package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.sakhnik.arduinopunch.ui.theme.AppTheme

const val DST_CLEAR = "clear"

@Composable
fun ClearScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        ScreenHeader(titleId = R.string.clear_title, instructionId = R.string.clear_instruction)
    }
}

@Preview(showBackground = true)
@Composable
fun ClearScreenPreview() {
    AppTheme {
        ClearScreen()
    }
}
