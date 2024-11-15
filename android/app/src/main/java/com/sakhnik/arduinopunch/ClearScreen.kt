package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.sakhnik.arduinopunch.ui.theme.AppTheme

const val DST_CLEAR = "clear"

@Composable
fun ClearScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        // Title Text
        Text(
            text = stringResource(id = R.string.clear_runner_title),
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            modifier = Modifier
                .fillMaxWidth()
                .align(Alignment.CenterHorizontally)
        )

        Spacer(modifier = Modifier.height(8.dp))

        // Instruction Text
        Text(
            text = stringResource(id = R.string.clear_runner_instruction),
            modifier = Modifier
                .fillMaxWidth()
                .align(Alignment.Start)
        )
    }
}

@Preview(showBackground = true)
@Composable
fun ClearScreenPreview() {
    AppTheme {
        ClearScreen()
    }
}
