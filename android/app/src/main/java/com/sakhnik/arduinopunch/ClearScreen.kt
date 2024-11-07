package com.sakhnik.arduinopunch

import android.view.LayoutInflater
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

const val DST_CLEAR = "clear"

@Composable
fun ClearScreen() {
    AndroidView(
        factory = { context ->
            // Inflate the XML layout
            val view = LayoutInflater.from(context).inflate(R.layout.clear_view, null)

            view // Return the view to display in Compose
        },
        modifier = Modifier.fillMaxWidth()
    )
}
