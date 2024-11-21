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

    private fun <T> trackAndSave(
        flow: StateFlow<T>,
        saveAction: suspend (T) -> Unit,
        debounceMillis: Long = 500L
    ) {
        flow
            .debounce(debounceMillis)
            .distinctUntilChanged()
            .onEach { value -> saveAction(value) }
            .launchIn(viewModelScope)
    }

    val knownKeys: Flow<String> = repository.knownKeysFlow

    private fun updateKnownKeys() {
        viewModelScope.launch {
            repository.saveKnownKeys()
        }
    }

    private val _cardId = MutableStateFlow(runBlocking { repository.cardIdFlow.first() })
    val cardId: StateFlow<String> get() = _cardId

    fun updateCardId(value: String) {
        _cardId.value = value
    }

    private val _stationId = MutableStateFlow(runBlocking { repository.stationIdFlow.first() })
    val stationId: StateFlow<String> get() = _stationId

    fun updateStationId(value: String) {
        _stationId.value = value
    }

    val uploadEnabled: Flow<Boolean> = repository.uploadEnabledFlow

    fun updateUploadEnabled(value: Boolean) {
        viewModelScope.launch {
            repository.saveUploadEnabled(value)
        }
    }

    private val _uploadUrl = MutableStateFlow(runBlocking { repository.uploadUrlFlow.first() })
    val uploadUrl: StateFlow<String> get() = _uploadUrl

    fun updateUploadUrl(value: String) {
        _uploadUrl.value = value
    }

    init {
        trackAndSave(_keyHex, { repository.saveKeyHex(it) })
        trackAndSave(_cardId, { repository.saveCardId(it) })
        trackAndSave(_stationId, { repository.saveStationId(it) })
        trackAndSave(_uploadUrl, { repository.saveUploadUrl(it) })
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
