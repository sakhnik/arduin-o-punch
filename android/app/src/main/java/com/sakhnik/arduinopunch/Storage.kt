package com.sakhnik.arduinopunch

import android.content.Context
import android.widget.EditText
import androidx.activity.ComponentActivity

class Storage(private val activity: ComponentActivity) {

    private var key: ByteArray? = null

    fun getKey(): ByteArray {
        if (key != null) {
            return key as ByteArray
        }
        key = getKeyHex().chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
        return key as ByteArray
    }

    fun getKeyHex(): String {
        val filename = "key.txt"

        val editKey = activity.findViewById<EditText>(R.id.editTextKey)
        if (editKey != null) {
            val keyHex: String =
                editKey.text.append("0".repeat(12 - editKey.text.length)).toString()
            activity.applicationContext.openFileOutput(filename, Context.MODE_PRIVATE).use {
                it.write(keyHex.toByteArray())
            }
            return keyHex
        }
        return activity.applicationContext.openFileInput(filename).bufferedReader().readLine()
    }
}
