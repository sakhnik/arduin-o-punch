package com.sakhnik.arduinopunch

import android.app.AlertDialog
import android.content.Context
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Lock
import androidx.compose.material.icons.filled.Search
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.sakhnik.arduinopunch.ui.theme.AppTheme
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.runBlocking

const val DST_FORMAT = "format"

@Composable
fun FormatScreen(cardViewModel: CardViewModel) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        ScreenHeader(R.string.format_runner_title, R.string.format_runner_instruction)

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
                    text = stringResource(id = R.string.key),
                    modifier = Modifier.weight(0.25f)
                )

                val keyHex by cardViewModel.keyHex.collectAsState(initial = "0".repeat(12))
                var passwordVisible by remember { mutableStateOf(false) }

                TextField(
                    value = keyHex,
                    onValueChange = {
                        cardViewModel.updateKeyHex(it)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    keyboardOptions = KeyboardOptions.Default.copy(keyboardType = KeyboardType.Password),
                    visualTransformation = if (passwordVisible) VisualTransformation.None else PasswordVisualTransformation(),
                    trailingIcon = {
                        val image = if (passwordVisible) Icons.Default.Lock else Icons.Default.Search
                        val description = if (passwordVisible) "Hide password" else "Show password"

                        IconButton(onClick = { passwordVisible = !passwordVisible }) {
                            Icon(imageVector = image, contentDescription = description)
                        }
                    },
                    //maxLength = 12,
                    singleLine = true
                )

                // Button for showOldKeys
                val context = LocalContext.current
                Button(
                    onClick = { showPreviousKeys(cardViewModel, context) },
                    modifier = Modifier.weight(0.6f)
                ) {
                    Text("🔑🔑")
                }
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(id = R.string.card_id),
                    modifier = Modifier.weight(0.25f)
                )

                // EditText equivalent (TextField) for cardId
                val cardId by cardViewModel.cardId.collectAsState(initial = stringResource(id = R.string._1))

                TextField(
                    value = cardId,
                    onValueChange = {
                        cardViewModel.updateCardId(it)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    keyboardOptions = KeyboardOptions.Default.copy(keyboardType = KeyboardType.Number),
                    label = { Text(stringResource(id = R.string.card_id)) },
                    //maxLength = 5,
                    singleLine = true
                )

                // Button for scanButton
                Button(
                    onClick = { /* Handle click */ },
                    modifier = Modifier.weight(0.6f)
                ) {
                    Text(stringResource(id = R.string.scan_qr))
                }
            }
        }
    }
}

private fun showPreviousKeys(cardViewModel: CardViewModel, context: Context) {
    val keyHex = runBlocking { cardViewModel.keyHex.first() }
    var knownKeys = runBlocking { cardViewModel.knownKeys.first() }
    // knownKeys contains the actual key too, skip it
    if (knownKeys.startsWith(keyHex)) {
        knownKeys = knownKeys.subSequence(keyHex.length, knownKeys.length).toString()
    }
    if (knownKeys.endsWith(",")) {
        knownKeys = knownKeys.subSequence(0, knownKeys.length - 1).toString()
    }
    if (knownKeys.length > 1) {
        val msg = knownKeys.replace(",", "\n")
        val builder = AlertDialog.Builder(context)
        builder.setMessage(msg)
            .setPositiveButton("OK") {_, _ -> }
            .show()
    }
}

@Composable
fun ScreenHeader(titleId: Int, instructionId: Int) {
    Column(modifier = Modifier.fillMaxWidth()) {
        // Title Text
        Text(
            text = stringResource(id = titleId),
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            modifier = Modifier
                .fillMaxWidth()
                .align(Alignment.CenterHorizontally)
        )

        Spacer(modifier = Modifier.height(8.dp))

        // Instruction Text
        Text(
            text = stringResource(id = instructionId),
            modifier = Modifier
                .fillMaxWidth()
                .align(Alignment.Start)
        )

        Spacer(modifier = Modifier.height(16.dp))
    }
}

@Preview(showBackground = true)
@Composable
fun FormatScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        FormatScreen(mockViewModel)
    }
}
