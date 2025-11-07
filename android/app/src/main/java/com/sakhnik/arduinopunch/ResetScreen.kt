package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.sakhnik.arduinopunch.ui.theme.AppTheme

const val DST_RESET = "reset"

@Composable
fun ResetScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        ScreenHeader(R.string.reset_title, R.string.reset_instruction)
    }
}

@Preview(showBackground = true)
@Composable
fun ResetScreenPreview() {
    AppTheme {
        ResetScreen()
    }
}
