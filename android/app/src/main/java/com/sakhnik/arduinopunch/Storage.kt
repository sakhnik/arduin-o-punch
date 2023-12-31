package com.sakhnik.arduinopunch

import android.content.Context
import android.widget.EditText
import androidx.activity.ComponentActivity
import java.io.FileNotFoundException

open class Storage(private val activity: ComponentActivity) {

    private var key: ByteArray? = null

    companion object {
        private const val KEY_FILENAME = "key.txt"
    }

    fun getKey(): ByteArray {
        if (key != null) {
            return key as ByteArray
        }
        key = getKeyHex().chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
        return key as ByteArray
    }

    fun getKeyHex(): String {
        try {
            activity.applicationContext.openFileInput(KEY_FILENAME).bufferedReader().use {
                return it.readLine()
            }
        } catch (_: FileNotFoundException) {
        }
        return "0".repeat(12)
    }

    fun checkKeyUpdate(): ByteArray {
        val editKey = activity.findViewById<EditText>(R.id.editTextKey)
        val keyFromInput = editKey.text.append("0".repeat(12 - editKey.text.length)).toString()
        if (keyFromInput != getKeyHex()) {
            key = null
            activity.applicationContext.openFileOutput(KEY_FILENAME, Context.MODE_PRIVATE).use {
                it.write(keyFromInput.toByteArray())
            }
        }
        return getKey()
    }
}
