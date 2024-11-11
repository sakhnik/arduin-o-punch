package com.sakhnik.arduinopunch

import android.content.Context
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.runBlocking

private val Context.dataStore by preferencesDataStore(name = "Prefs")

interface Repository {
    suspend fun saveKeyHex(value: String)
    val keyHexFlow: Flow<String>
    fun getKey(): ByteArray

    suspend fun saveCardId(value: String)
    val cardIdFlow: Flow<String>
}

class RepositoryImpl(private val context: Context): Repository {
    private companion object {
        private val PREF_KEY = stringPreferencesKey("key")
        private val PREF_CARD_ID = stringPreferencesKey("cardId")

        private fun parseKey(hex: String): ByteArray {
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

    override fun getKey(): ByteArray {
        return runBlocking { parseKey(keyHexFlow.first()) }
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
}
