package com.sakhnik.arduinopunch

import android.view.LayoutInflater
import android.view.inputmethod.EditorInfo
import android.widget.EditText
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

const val DST_PUNCH = "punch"

@Composable
fun PunchScreen(cardViewModel: CardViewModel) {
    AndroidView(
        factory = { context ->
            val storage = cardViewModel.storage

            val view = LayoutInflater.from(context).inflate(R.layout.punch_view, null)

            val etStation = view.findViewById<EditText>(R.id.editStationId)
            etStation.setText(storage.getStationId())
            etStation.setOnEditorActionListener { _, actionId, _ ->
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    storage.updateStationId(etStation.text.toString())
                    true // Return true to consume the action
                } else {
                    false
                }
            }

            view // Return the view to display in Compose
        },
        modifier = Modifier.fillMaxWidth()
    )
}
