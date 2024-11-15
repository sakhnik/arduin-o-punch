package com.sakhnik.arduinopunch

import android.content.Context
import androidx.datastore.preferences.core.booleanPreferencesKey
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

    suspend fun saveStationId(value: String)
    val stationIdFlow: Flow<String>

    suspend fun saveKnownKeys()
    val knownKeysFlow: Flow<String>

    suspend fun saveUploadEnabled(value: Boolean)
    val uploadEnabledFlow: Flow<Boolean>

    suspend fun saveUploadUrl(value: String)
    val uploadUrlFlow: Flow<String>
}

class RepositoryImpl(private val context: Context): Repository {
    companion object {
        private val PREF_KEY = stringPreferencesKey("key")
        private val PREF_CARD_ID = stringPreferencesKey("cardId")
        private val PREF_KNOWN_KEYS = stringPreferencesKey("knownKeys")
        private val PREF_STATION_ID = stringPreferencesKey("stationId")
        private val PREF_UPLOAD_ENABLED = booleanPreferencesKey("upload")
        private val PREF_UPLOAD_URL = stringPreferencesKey("uploadUrl")

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

    override suspend fun saveStationId(value: String) {
        context.dataStore.edit { preferences ->
            preferences[PREF_STATION_ID] = value
        }
    }

    override val stationIdFlow: Flow<String> = context.dataStore.data
        .map { preferences ->
            preferences[PREF_STATION_ID] ?: context.getString(R.string._31)
        }

    override suspend fun saveUploadEnabled(value: Boolean) {
        context.dataStore.edit { preferences ->
            preferences[PREF_UPLOAD_ENABLED] = value
        }
    }

    override val uploadEnabledFlow: Flow<Boolean> = context.dataStore.data
        .map { preferences ->
            preferences[PREF_UPLOAD_ENABLED] ?: false
        }

    override suspend fun saveUploadUrl(value: String) {
        context.dataStore.edit { preferences ->
            preferences[PREF_UPLOAD_URL] = value
        }
    }

    override val uploadUrlFlow: Flow<String> = context.dataStore.data
        .map { preferences ->
            preferences[PREF_UPLOAD_URL] ?: context.getString(R.string.https_sakhnik_com_qr_o_punch_card)
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
