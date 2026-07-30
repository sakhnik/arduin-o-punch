package com.sakhnik.arduinopunch.card

import android.app.Application
import android.nfc.tech.MifareClassic
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.viewModelScope
import com.sakhnik.arduinopunch.CardRoute
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
import org.yaml.snakeyaml.DumperOptions
import org.yaml.snakeyaml.Yaml
import timber.log.Timber
import java.time.Duration
import java.time.LocalTime
import kotlin.time.Duration.Companion.milliseconds

@OptIn(FlowPreview::class)
open class CardViewModel(private val repository: Repository, application: Application) : AndroidViewModel(application) {
    private val _currentDestination = MutableLiveData<String?>()
    //val currentDestination: LiveData<String?> = _currentDestination
    private val _progress = mutableFloatStateOf(0f)
    val progress: State<Float> = _progress

    private val _readOutCount = mutableIntStateOf(1)
    val readOutCount: State<Int> = _readOutCount
    fun updateReadOutCount(value: Int) {
        _readOutCount.intValue = value.coerceIn(1, 10)
    }

    private val _readOuts: MutableLiveData<List<PunchCard.Info>> = MutableLiveData(listOf())
    val readOuts: LiveData<List<PunchCard.Info>> get() = _readOuts

    fun selectReadOut(index: Int) {
        val info = _readOuts.value?.getOrNull(index) ?: return
        _selectedReadOut.intValue = index
        _readOut.postValue(info)
    }

    private val _selectedReadOut = mutableIntStateOf(0)
    val selectedReadOut: State<Int> = _selectedReadOut

    private val _readOut = MutableLiveData(PunchCard.Info(0, listOf(), debugInfo = null))
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
            .debounce(debounceMillis.milliseconds)
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

    private val _useCardId = MutableStateFlow(false)
    val useCardId: StateFlow<Boolean> = _useCardId

    fun updateUseCardId(use: Boolean) {
        _useCardId.value = use
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
            CardRoute.FORMAT -> formatCard(mifare)
            CardRoute.PUNCH -> punchCard(mifare)
            CardRoute.READ -> readRunner(mifare)
            CardRoute.RESET -> resetCard(mifare)
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
        val newCardId = if (useCardId.value) {
            cardId.value.toInt()
        } else {
            null
        }

        if (newCardId != null) {
            Timber.d("Format $newCardId with key ${key.joinToString("") { "%02X".format(it) }}")
        } else {
            Timber.d("Clear with key ${key.joinToString("") { "%02X".format(it) }}")
        }
        val card = PunchCard(MifareImpl(mifare), key, context)
        card.format(newCardId, getKnownKeys(), this::setProgress)
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
        val runCount = readOutCount.value
        val readOuts = card.readOut(runCount, this::setProgress)
        _readOuts.postValue(readOuts)
        _selectedReadOut.intValue = 0
        _readOut.postValue(readOuts.firstOrNull())

        val doUpload = runBlocking { uploadEnabled.first() }
        if (doUpload && runCount == 1) {
            performUpload()
        }
    }

    fun performUpload() {
        val selected = _readOut.value ?: return
        val uploadUrl = runBlocking { uploadUrl.first() }
        Uploader(this).upload(selected, uploadUrl)
    }

    companion object {
        private const val KEY_KEY_HEX = "keyHex"
        private const val KEY_KNOWN_KEYS = "knownKeys"
        private const val KEY_UPLOAD_URL = "uploadUrl"
    }

    suspend fun settingsToYaml(): String {
        val map = mapOf(
            KEY_KEY_HEX to repository.keyHexFlow.first(),
            KEY_KNOWN_KEYS to repository.knownKeysFlow.first()
                .split(',')
                .map { it.trim() }
                .filter { it.isNotEmpty() },
            KEY_UPLOAD_URL to repository.uploadUrlFlow.first()
        )

        val options = DumperOptions().apply {
            defaultFlowStyle = DumperOptions.FlowStyle.BLOCK
            isPrettyFlow = true
        }

        return Yaml(options).dump(map)
    }

    suspend fun yamlToSettings(yamlText: String) {
        val data = Yaml().load<Map<String, Any>>(yamlText)
        data[KEY_KEY_HEX]?.let {
            val newValue = it.toString()
            _keyHex.value = newValue
            repository.saveKeyHex(newValue)
        }
        data[KEY_KNOWN_KEYS]?.let {
            val value = when (it) {
                is List<*> -> it.joinToString(",") { item -> item.toString() }
                else -> it.toString()
            }
            repository.saveKnownKeys2(value)
        }
        data[KEY_UPLOAD_URL]?.let {
            val newValue = it.toString()
            _uploadUrl.value = newValue
            repository.saveUploadUrl(newValue)
        }
    }
}
