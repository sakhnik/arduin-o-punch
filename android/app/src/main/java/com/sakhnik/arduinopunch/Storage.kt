package com.sakhnik.arduinopunch

import android.content.Context

class Storage(private val context: Context) {

    private companion object {
        const val CURRENT_VIEW_KEY = "currentView"

        private object Prefs {
            const val NAME = "Prefs"
            const val KEY_CARD_ID = "cardId"
            const val KEY_KEY = "key"
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

    fun getKeyHex(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        val keyRaw = prefs.getString(Prefs.KEY_KEY, null) ?: ""
        return keyRaw + "0".repeat(12 - keyRaw.length)
    }

    fun getKey(): ByteArray {
        return parseKey(getKeyHex())
    }

    fun updateKey(key: String) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_KEY, key)
        editor.apply()
    }

    // Take the current key and make sure it's in the list of known keys
    fun updateKnownKeys() {
        val keyHex = getKeyHex()
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

    fun getCardId(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getString(Prefs.KEY_CARD_ID, context.getString(R.string._1)) ?: "1"
    }

    fun updateCardId(cardId: String) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_CARD_ID, cardId)
        editor.apply()
    }

//        when (viewId) {
//            R.layout.format_view -> {
//                findViewById<EditText>(R.id.editCardId).setText())
//                findViewById<EditText>(R.id.editKey).setText(prefs.getString(Prefs.KEY_KEY, ""))

}
