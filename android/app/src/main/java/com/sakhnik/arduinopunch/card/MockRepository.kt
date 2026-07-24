package com.sakhnik.arduinopunch.card

import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.first

class MockRepository : Repository {
    private val mockKeyFlow = MutableStateFlow("0".repeat(12))
    private val mockCardIdFlow = MutableStateFlow("1")
    private val mockStationIdFlow = MutableStateFlow("31")
    private val mockKnownKeysFlow = MutableStateFlow("F".repeat(12))
    private val mockUploadEnabledFlow = MutableStateFlow(false)
    private val mockUploadUrlFlow = MutableStateFlow("https://example.com/upload")

    override val keyHexFlow: Flow<String> = mockKeyFlow
    override val cardIdFlow: Flow<String> = mockCardIdFlow
    override val stationIdFlow: Flow<String> = mockStationIdFlow
    override val knownKeysFlow: Flow<String> = mockKnownKeysFlow
    override val uploadEnabledFlow: Flow<Boolean> = mockUploadEnabledFlow
    override val uploadUrlFlow: Flow<String> = mockUploadUrlFlow

    override suspend fun saveUploadEnabled(value: Boolean) {
        mockUploadEnabledFlow.value = value
    }

    override suspend fun saveUploadUrl(value: String) {
        mockUploadUrlFlow.value = value
    }

    override suspend fun saveKeyHex(value: String) {
        mockKeyFlow.value = value
    }

    override suspend fun saveCardId(value: String) {
        mockCardIdFlow.value = value
    }

    override suspend fun saveStationId(value: String) {
        mockStationIdFlow.value = value
    }

    override suspend fun saveKnownKeys() {
        mockKnownKeysFlow.value = "${mockKeyFlow.first()},${mockKnownKeysFlow.first()}"
    }

    override suspend fun saveKnownKeys2(value: String) {
        mockKnownKeysFlow.value = value
    }
}
