package com.sakhnik.arduinopunch

import android.content.Context
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map

private val Context.dataStore by preferencesDataStore(name = "Prefs")

interface Repository {
    suspend fun saveKeyHex(value: String)
    val keyHexFlow: Flow<String>

    suspend fun saveCardId(value: String)
    val cardIdFlow: Flow<String>

    suspend fun saveKnownKeys()
    val knownKeysFlow: Flow<String>
}

class RepositoryImpl(private val context: Context): Repository {
    companion object {
        private val PREF_KEY = stringPreferencesKey("key")
        private val PREF_CARD_ID = stringPreferencesKey("cardId")
        private val PREF_KNOWN_KEYS = stringPreferencesKey("knownKeys")

        const val KNOWN_KEYS_HISTORY_SIZE = 4

        fun parseKey(hex: String): ByteArray {
            return hex.chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
        }
    }

    override suspend fun saveKeyHex(value: String) {
        context.dataStore.edit { preferences ->
            preferences[PREF_KEY] = value + "0".repeat(12 - value.length)
        }
    }

    override val keyHexFlow: Flow<String> = context.dataStore.data
        .map { preferences ->
            val value = preferences[PREF_KEY] ?: ""
            value + "0".repeat(12 - value.length)
        }

    override suspend fun saveCardId(value: String) {
        context.dataStore.edit { preferences ->
            preferences[PREF_CARD_ID] = value
        }
    }

    override val cardIdFlow: Flow<String> = context.dataStore.data
        .map { preferences ->
            preferences[PREF_CARD_ID] ?: context.getString(R.string._1)
        }

    override suspend fun saveKnownKeys() {
        val keyHex = keyHexFlow.first()
        context.dataStore.edit { preferences ->
            val knownKeys = preferences[PREF_KNOWN_KEYS]
            if (knownKeys?.startsWith(keyHex) == true) {
                // It's already there
                return@edit
            }
            val knownKeysList = knownKeys?.split(",") ?: arrayListOf()
            val newList = ArrayList<String>()
            newList.add(keyHex)
            // Limit the history of the known keys
            if (knownKeysList.size > KNOWN_KEYS_HISTORY_SIZE)
                newList.addAll(knownKeysList.subList(0, KNOWN_KEYS_HISTORY_SIZE))
            else
                newList.addAll(knownKeysList)
            preferences[PREF_KNOWN_KEYS] = newList.joinToString(",")
        }
    }

    override val knownKeysFlow: Flow<String> = context.dataStore.data
        .map { preferences ->
            preferences[PREF_KNOWN_KEYS] ?: ""
        }
}
