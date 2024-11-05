package com.sakhnik.arduinopunch

import android.view.LayoutInflater
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

const val DST_FORMAT = "format"

@Composable
fun FormatScreen() {
    AndroidView(
        factory = { context ->
            // Inflate the XML layout
            val view = LayoutInflater.from(context).inflate(R.layout.format_view, null)

            // Access the views in the XML layout
            //val textView = view.findViewById<TextView>(R.id.exampleText)
            //val button = view.findViewById<Button>(R.id.exampleButton)

            //// Set up interactions or modify the views if needed
            //button.setOnClickListener {
            //    textView.text = "Button Clicked!"
            //}

            view // Return the view to display in Compose
        },
        modifier = Modifier.fillMaxWidth()
    )
}
