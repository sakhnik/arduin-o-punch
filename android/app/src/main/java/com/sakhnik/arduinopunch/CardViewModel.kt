package com.sakhnik.arduinopunch

import android.app.Application
import android.nfc.tech.MifareClassic
import android.util.Log
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableFloatStateOf
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

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
            DST_FORMAT -> formatRunner(mifare)
        }
        //when (currentView) {
        //    R.layout.clear_view -> clearRunner(mifareClassic)
        //    R.layout.punch_view -> punchRunner(mifareClassic)
        //    R.layout.read_runner_view -> readRunner(mifareClassic)
        //    R.layout.reset_view -> resetRunner(mifareClassic)
        //}
        //viewModelScope.launch {
        //    for (i in 1..100) {
        //        _progress.value = i / 100f
        //        delay(50) // Simulating work
        //    }
        //}
    }

    private fun formatRunner(mifare: MifareClassic) {
        val key = storage.getKey()
        Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")
        storage.updateKnownKeys()

        val context = getApplication<Application>().applicationContext
        val cardId = storage.getCardId().toInt()
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.format(cardId, storage.getKnownKeys(), this::setProgress)
    }

}
