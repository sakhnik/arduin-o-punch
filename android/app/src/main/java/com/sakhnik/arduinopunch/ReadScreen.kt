package com.sakhnik.arduinopunch

import android.util.Log
import android.view.LayoutInflater
import android.view.inputmethod.EditorInfo
import android.widget.CheckBox
import android.widget.EditText
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView
import java.time.DateTimeException
import java.time.LocalTime
import java.time.format.DateTimeFormatter

const val DST_READ = "read"

@Composable
fun ReadScreen(cardViewModel: CardViewModel) {
    AndroidView(
        factory = { context ->
            val storage = cardViewModel.storage
            val view = LayoutInflater.from(context).inflate(R.layout.read_runner_view, null)

            val cbUpload = view.findViewById<CheckBox>(R.id.checkBoxUpload)
            cbUpload.isChecked = storage.hasUpload()
            cbUpload.setOnCheckedChangeListener { _, b -> storage.updateUpload(b) }

            val etUrl = view.findViewById<EditText>(R.id.editUploadUrl)
            etUrl.setText(storage.getUploadUrl())
            etUrl.setOnEditorActionListener { _, actionId, _ ->
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    storage.updateUploadUrl(etUrl.text.toString())
                    true
                } else {
                    false
                }
            }

            view // Return the view to display in Compose
        },
        update = { view ->
            val readOut = cardViewModel.readOut.value
            view.findViewById<TextView>(R.id.textViewCardNumber).text = readOut?.cardNumber.toString()

            val tableLayout = view.findViewById<TableLayout>(R.id.tableLayout)
            for (i in tableLayout.childCount - 1 downTo 1) {
                tableLayout.removeViewAt(i)
            }

            if (readOut != null) {
                Log.d(null, "read out punches")
                // Format LocalTime to "HH:mm:ss"
                val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")

                for ((index, punch) in readOut.punches.withIndex()) {
                    val tableRow = TableRow(view.context)
                    val cell1 = TextView(view.context)
                    val strNumber = "${index + 1}"
                    cell1.text = strNumber
                    tableRow.addView(cell1)
                    val cell2 = TextView(view.context)
                    cell2.text = punch.station.toString()
                    tableRow.addView(cell2)
                    val cell3 = TextView(view.context)
                    try {
                        val localTime = LocalTime.ofSecondOfDay(punch.timestamp)
                        cell3.text = localTime.format(formatter)
                    } catch (ex: DateTimeException) {
                        cell3.text = "???"
                    }
                    tableRow.addView(cell3)
                    tableLayout.addView(tableRow)
                }
            }
        },

        modifier = Modifier.fillMaxWidth()
    )
}
