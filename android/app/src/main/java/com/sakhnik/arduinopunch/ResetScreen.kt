package com.sakhnik.arduinopunch

import android.view.LayoutInflater
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

const val DST_RESET = "reset"

@Composable
fun ResetScreen() {
    AndroidView(
        factory = { context ->
            val view = LayoutInflater.from(context).inflate(R.layout.reset_view, null)
            view // Return the view to display in Compose
        },
        modifier = Modifier.fillMaxWidth()
    )
}
