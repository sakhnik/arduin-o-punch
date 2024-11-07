package com.sakhnik.arduinopunch

import android.app.Application
import android.nfc.tech.MifareClassic
import android.util.Log
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableFloatStateOf
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import java.time.Duration
import java.time.LocalTime

open class CardViewModel(application: Application) : AndroidViewModel(application) {
    private val _currentDestination = MutableLiveData<String?>()
    val currentDestination: LiveData<String?> = _currentDestination
    private val _progress = mutableFloatStateOf(0f)
    val progress: State<Float> = _progress
    private val storage = Storage(application.applicationContext)

    fun getStorage(): Storage {
        return storage
    }

    fun updateCurrentDestination(destination: String?) {
        _currentDestination.value = destination
    }

    fun updateProgress(value: Float) {
        _progress.floatValue = value
    }

    private fun setProgress(n: Int, d: Int) {
        updateProgress(n.toFloat() / d.toFloat())
    }

    // Simulate reading data from the card and updating progress
    fun handleCard(mifare: MifareClassic) {
        when (_currentDestination.value) {
            DST_FORMAT -> formatCard(mifare)
            DST_CLEAR -> clearCard(mifare)
            DST_PUNCH -> punchCard(mifare)
            DST_RESET -> resetCard(mifare)
        }
        //when (currentView) {
        //    R.layout.read_runner_view -> readRunner(mifareClassic)
        //}
        //viewModelScope.launch {
        //    for (i in 1..100) {
        //        _progress.value = i / 100f
        //        delay(50) // Simulating work
        //    }
        //}
    }

    private fun formatCard(mifare: MifareClassic) {
        val key = storage.getKey()
        Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")
        storage.updateKnownKeys()

        val context = getApplication<Application>().applicationContext
        val cardId = storage.getCardId().toInt()
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.format(cardId, storage.getKnownKeys(), this::setProgress)
    }

    private fun clearCard(mifareClassic: MifareClassic) {
        val key = storage.getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifareClassic), key, context)
        card.clear(this::setProgress)
    }

    private fun punchCard(mifare: MifareClassic) {
        val key = storage.getKey()
        val context = getApplication<Application>().applicationContext
        val station = storage.getStationId().toInt()
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.punch(Punch(station, getTimestamp()), this::setProgress)
    }

    private fun getTimestamp(): Long {
        return Duration.between(LocalTime.of(0, 0), LocalTime.now()).seconds
    }

    private fun resetCard(mifare: MifareClassic) {
        val key = storage.getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.reset(storage.getKnownKeys(), this::setProgress)
    }
}
