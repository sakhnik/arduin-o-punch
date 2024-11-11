package com.sakhnik.arduinopunch

import android.app.Application
import android.nfc.tech.MifareClassic
import android.util.Log
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableFloatStateOf
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.time.Duration
import java.time.LocalTime

open class CardViewModel(private val repository: Repository, application: Application) : AndroidViewModel(application) {
    private val _currentDestination = MutableLiveData<String?>()
    //val currentDestination: LiveData<String?> = _currentDestination
    private val _progress = mutableFloatStateOf(0f)
    val progress: State<Float> = _progress
    val storage: Storage by lazy {
        Storage(application.applicationContext)
    }
    private val _readOut = MutableLiveData(PunchCard.Info(0, listOf()))
    val readOut: LiveData<PunchCard.Info> get() = _readOut
    private val _toastMessage = MutableLiveData<String>()
    val toastMessage: LiveData<String> get() = _toastMessage

    fun postToast(message: String) {
        _toastMessage.postValue(message)
    }

    val keyHex: Flow<String> = repository.keyHexFlow

    fun updateKeyHex(value: String) {
        viewModelScope.launch {
            repository.saveKeyHex(value)
        }
    }

    val cardId: Flow<String> = repository.cardIdFlow

    fun updateCardId(value: String) {
        Log.d(null, "** updateCardId $value")
        viewModelScope.launch {
            repository.saveCardId(value)
        }
    }

    fun getStringFromResources(resourceId: Int): String {
        return getApplication<Application>().getString(resourceId)
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

    fun handleCard(mifare: MifareClassic) {
        when (_currentDestination.value) {
            DST_FORMAT -> formatCard(mifare)
            DST_CLEAR -> clearCard(mifare)
            DST_PUNCH -> punchCard(mifare)
            DST_READ -> readRunner(mifare)
            DST_RESET -> resetCard(mifare)
        }
    }

    private fun formatCard(mifare: MifareClassic) {
        val key = repository.getKey()
        storage.updateKnownKeys()

        val context = getApplication<Application>().applicationContext
        val cardId = runBlocking { cardId.first() }.toInt()
        Log.d(null, "Format $cardId with key ${key.joinToString("") { "%02X".format(it) }}")
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.format(cardId, storage.getKnownKeys(), this::setProgress)
    }

    private fun clearCard(mifareClassic: MifareClassic) {
        val key = repository.getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifareClassic), key, context)
        card.clear(this::setProgress)
    }

    private fun punchCard(mifare: MifareClassic) {
        val key = repository.getKey()
        val context = getApplication<Application>().applicationContext
        val station = storage.getStationId().toInt()
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.punch(Punch(station, getTimestamp()), this::setProgress)
    }

    private fun getTimestamp(): Long {
        return Duration.between(LocalTime.of(0, 0), LocalTime.now()).seconds
    }

    private fun resetCard(mifare: MifareClassic) {
        val key = repository.getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.reset(storage.getKnownKeys(), this::setProgress)
    }

    private fun readRunner(mifareClassic: MifareClassic) {
        val key = repository.getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifareClassic), key, context)
        val readOut = card.readOut(this::setProgress)
        _readOut.postValue(readOut)

        if (storage.hasUpload()) {
            Uploader(this).upload(readOut, storage.getUploadUrl())
        }
    }
}
