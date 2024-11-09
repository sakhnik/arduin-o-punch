package com.sakhnik.arduinopunch

import android.app.AlertDialog
import android.content.Context
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.sakhnik.arduinopunch.ui.theme.AppTheme

const val DST_FORMAT = "format"

@Composable
fun FormatScreen(cardViewModel: CardViewModel) {
    Column(
        modifier = Modifier.fillMaxSize().padding(16.dp)
    ) {
        // Title Text
        Text(
            text = stringResource(id = R.string.format_runner_title),
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.fillMaxWidth().align(Alignment.CenterHorizontally)
        )

        Spacer(modifier = Modifier.height(8.dp))

        // Instruction Text
        Text(
            text = stringResource(id = R.string.format_runner_instruction),
            modifier = Modifier.fillMaxWidth().align(Alignment.Start)
        )

        Spacer(modifier = Modifier.height(16.dp))

        LazyVerticalGrid(
            columns = GridCells.Fixed(3),
            modifier = Modifier.fillMaxWidth()
        ) {
            item {
                Text(
                    text = stringResource(id = R.string.key),
                    modifier = Modifier.align(Alignment.CenterHorizontally)
                )
            }
            item {
                // EditText equivalent (TextField)
                //var text by remember { mutableStateOf("") }
                var text by rememberSaveable { mutableStateOf("Text") }
                TextField(
                    value = text,
                    onValueChange = { text = it },
                    modifier = Modifier.fillMaxWidth(),
                    keyboardOptions = KeyboardOptions.Default.copy(keyboardType = KeyboardType.Password),
                    visualTransformation = PasswordVisualTransformation(),
                    //maxLength = 12,
                    singleLine = true
                )
            }
            item {
                // Button for showOldKeys
                val context = LocalContext.current
                Button(
                    onClick = { showPreviousKeys(cardViewModel, context) },
                    modifier = Modifier.align(Alignment.CenterHorizontally)
                ) {
                    Text("🔑🔑")
                }
            }
            item {
                Text(
                    text = stringResource(id = R.string.card_id),
                    modifier = Modifier.align(Alignment.CenterHorizontally)
                )
            }
            item {
                // EditText equivalent (TextField) for cardId
                var cardId by remember { mutableStateOf("") }
                TextField(
                    value = cardId,
                    onValueChange = { cardId = it },
                    modifier = Modifier.fillMaxWidth(),
                    keyboardOptions = KeyboardOptions.Default.copy(keyboardType = KeyboardType.Number),
                    label = { Text(stringResource(id = R.string.card_id)) },
                    //maxLength = 5,
                    singleLine = true
                )
            }
            item {
                // Button for scanButton
                Button(
                    onClick = { /* Handle click */ },
                    modifier = Modifier.align(Alignment.CenterHorizontally)
                ) {
                    Text(stringResource(id = R.string.scan_qr))
                }
            }
        }
    }
    //AndroidView(
    //    factory = { context ->
    //        val storage = cardViewModel.getStorage()
    //        val view = LayoutInflater.from(context).inflate(R.layout.format_view, null)
    //        val etKey = view.findViewById<EditText>(R.id.editKey)
    //        etKey.setText(storage.getKeyHex())
    //        etKey.setOnEditorActionListener { _, actionId, _ ->
    //            if (actionId == EditorInfo.IME_ACTION_DONE) {
    //                storage.updateKey(etKey.text.toString())
    //                true
    //            } else {
    //                false
    //            }
    //        }
    //        val etId = view.findViewById<EditText>(R.id.editCardId)
    //        etId.setText(storage.getCardId())
    //        etId.setOnEditorActionListener { _, actionId, _ ->
    //            if (actionId == EditorInfo.IME_ACTION_DONE) {
    //                storage.updateCardId(etId.text.toString())
    //                true // Return true to consume the action
    //            } else {
    //                false
    //            }
    //        }

    //        val showOldKeys = view.findViewById<Button>(R.id.showOldKeys)
    //        showOldKeys.setOnClickListener {
    //            showPreviousKeys(cardViewModel, context)
    //        }
    //        view // Return the view to display in Compose
    //    },
    //    modifier = Modifier.fillMaxWidth()
    //)
}

private fun showPreviousKeys(cardViewModel: CardViewModel, context: Context) {
    val storage = cardViewModel.storage
    val keyHex = storage.getKeyHex()
    var knownKeys = storage.getKnownKeysStr()
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

@Preview(showBackground = true)
@Composable
fun FormatScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        MainScreen(mockViewModel)
    }
}
