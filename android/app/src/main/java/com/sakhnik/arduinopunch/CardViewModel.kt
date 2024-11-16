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
import kotlinx.coroutines.FlowPreview
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.distinctUntilChanged
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.launchIn
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.time.Duration
import java.time.LocalTime

@OptIn(FlowPreview::class)
open class CardViewModel(private val repository: Repository, application: Application) : AndroidViewModel(application) {
    private val _currentDestination = MutableLiveData<String?>()
    //val currentDestination: LiveData<String?> = _currentDestination
    private val _progress = mutableFloatStateOf(0f)
    val progress: State<Float> = _progress

    private val _readOut = MutableLiveData(PunchCard.Info(0, listOf()))
    val readOut: LiveData<PunchCard.Info> get() = _readOut
    // Helper method for preview
    fun setReadOutForPreview(readOut: PunchCard.Info) {
        _readOut.value = readOut
    }

    private val _toastMessage = MutableLiveData<String>()
    val toastMessage: LiveData<String> get() = _toastMessage

    fun postToast(message: String) {
        _toastMessage.postValue(message)
    }

    private val _keyHex = MutableStateFlow(runBlocking { repository.keyHexFlow.first() })
    val keyHex: StateFlow<String> get() = _keyHex

    fun updateKeyHex(value: String) {
        _keyHex.value = value
    }

    init {
        _keyHex.debounce(500) // Wait 500 ms after the last text change
            .distinctUntilChanged() // Prevent duplicate saves for the same text
            .onEach { value ->
                viewModelScope.launch {
                    repository.saveKeyHex(value)
                }
            }
            .launchIn(viewModelScope)
    }

    val knownKeys: Flow<String> = repository.knownKeysFlow

    private fun updateKnownKeys() {
        viewModelScope.launch {
            repository.saveKnownKeys()
        }
    }

    val cardId: Flow<String> = repository.cardIdFlow

    fun updateCardId(value: String) {
        viewModelScope.launch {
            repository.saveCardId(value)
        }
    }

    val stationId: Flow<String> = repository.stationIdFlow

    fun updateStationId(value: String) {
        viewModelScope.launch {
            repository.saveStationId(value)
        }
    }

    val uploadEnabled: Flow<Boolean> = repository.uploadEnabledFlow

    fun updateUploadEnabled(value: Boolean) {
        viewModelScope.launch {
            repository.saveUploadEnabled(value)
        }
    }

    val uploadUrl: Flow<String> = repository.uploadUrlFlow

    fun updateUploadUrl(value: String) {
        viewModelScope.launch {
            repository.saveUploadUrl(value)
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

    private fun getKnownKeys(): List<ByteArray> {
        return runBlocking { knownKeys.first() }.split(",").filter { it.isNotEmpty() }.map { RepositoryImpl.parseKey(it) }
    }

    private fun getKey(): ByteArray {
        return runBlocking { RepositoryImpl.parseKey(keyHex.first()) }
    }

    private fun formatCard(mifare: MifareClassic) {
        val key = getKey()
        updateKnownKeys()

        val context = getApplication<Application>().applicationContext
        val cardId = runBlocking { cardId.first() }.toInt()
        Log.d(null, "Format $cardId with key ${key.joinToString("") { "%02X".format(it) }}")
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.format(cardId, getKnownKeys(), this::setProgress)
    }

    private fun clearCard(mifareClassic: MifareClassic) {
        val key = getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifareClassic), key, context)
        card.clear(this::setProgress)
    }

    private fun punchCard(mifare: MifareClassic) {
        val key = getKey()
        val context = getApplication<Application>().applicationContext
        val station = runBlocking { stationId.first() }.toInt()
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.punch(Punch(station, getTimestamp()), this::setProgress)
    }

    private fun getTimestamp(): Long {
        return Duration.between(LocalTime.of(0, 0), LocalTime.now()).seconds
    }

    private fun resetCard(mifare: MifareClassic) {
        val key = getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.reset(getKnownKeys(), this::setProgress)
    }

    private fun readRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        val context = getApplication<Application>().applicationContext
        val card = PunchCard(MifareImpl(mifareClassic), key, context)
        val readOut = card.readOut(this::setProgress)
        _readOut.postValue(readOut)

        val doUpload = runBlocking { uploadEnabled.first() }
        if (doUpload) {
            val uploadUrl = runBlocking { uploadUrl.first() }
            Uploader(this).upload(readOut, uploadUrl)
        }
    }
}
