package com.sakhnik.arduinopunch

import android.content.Context

class Storage(private val context: Context) {

    private companion object {
        const val CURRENT_VIEW_KEY = "currentView"

        private object Prefs {
            const val NAME = "Prefs"

            const val KEY_KNOWN_KEYS = "knownKeys"
            const val KEY_STATION_ID = "stationId"
            const val KEY_UPLOAD = "upload"
            const val KEY_UPLOAD_URL = "uploadUrl"

            const val KNOWN_KEYS_HISTORY_SIZE = 4
        }

        private fun parseKey(hex: String): ByteArray {
            return hex.chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
        }
    }

    // Take the current key and make sure it's in the list of known keys
    fun updateKnownKeys() {
        // TODO
        val keyHex = ""  // runBlocking { keyHexFlow.first() }
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        val knownKeys = prefs.getString(Prefs.KEY_KNOWN_KEYS, "")!!
        if (knownKeys.startsWith(keyHex)) {
            // It's already there
            return
        }
        val knownKeysList = knownKeys.split(",")
        val newList = ArrayList<String>()
        newList.add(keyHex)
        // Limit the history of the known keys
        if (knownKeysList.size > Prefs.KNOWN_KEYS_HISTORY_SIZE)
            newList.addAll(knownKeysList.subList(0, Prefs.KNOWN_KEYS_HISTORY_SIZE))
        else
            newList.addAll(knownKeysList)
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_KNOWN_KEYS, newList.joinToString(","))
        editor.apply()
    }

    fun getKnownKeys(): List<ByteArray> {
        return getKnownKeysStr().split(",").filter { it.isNotEmpty() }.map { parseKey(it) }
    }

    fun getKnownKeysStr(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getString(Prefs.KEY_KNOWN_KEYS, "")!!
    }

    fun getStationId(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getString(Prefs.KEY_STATION_ID, context.getString(R.string._31)) ?: "31"
    }

    fun updateStationId(stationId: String) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_STATION_ID, stationId)
        editor.apply()
    }

    fun hasUpload(): Boolean {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getBoolean(Prefs.KEY_UPLOAD, false)
    }

    fun updateUpload(hasUpload: Boolean) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putBoolean(Prefs.KEY_UPLOAD, hasUpload)
        editor.apply()
    }

    fun getUploadUrl(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        val defaultUrl = context.getString(R.string.https_sakhnik_com_qr_o_punch_card)
        return prefs.getString(Prefs.KEY_UPLOAD_URL, defaultUrl) ?: defaultUrl
    }

    fun updateUploadUrl(uploadUrl: String) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_UPLOAD_URL, uploadUrl)
        editor.apply()
    }
}
