package com.sakhnik.arduinopunch

import android.app.AlertDialog
import android.content.Context
import android.view.LayoutInflater
import android.view.inputmethod.EditorInfo
import android.widget.Button
import android.widget.EditText
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

const val DST_FORMAT = "format"

@Composable
fun FormatScreen(cardViewModel: CardViewModel) {
    AndroidView(
        factory = { context ->
            val storage = cardViewModel.getStorage()
            val view = LayoutInflater.from(context).inflate(R.layout.format_view, null)
            val etKey = view.findViewById<EditText>(R.id.editKey)
            etKey.setText(storage.getKeyHex())
            etKey.setOnEditorActionListener { _, actionId, _ ->
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    storage.updateKey(etKey.text.toString())
                    true
                } else {
                    false
                }
            }
            val etId = view.findViewById<EditText>(R.id.editCardId)
            etId.setText(storage.getCardId())
            etId.setOnEditorActionListener { _, actionId, _ ->
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    storage.updateCardId(etId.text.toString())
                    true // Return true to consume the action
                } else {
                    false
                }
            }

            val showOldKeys = view.findViewById<Button>(R.id.showOldKeys)
            showOldKeys.setOnClickListener {
                showPreviousKeys(cardViewModel, context)
            }
            view // Return the view to display in Compose
        },
        modifier = Modifier.fillMaxWidth()
    )
}

private fun showPreviousKeys(cardViewModel: CardViewModel, context: Context) {
    val storage = cardViewModel.getStorage()
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
