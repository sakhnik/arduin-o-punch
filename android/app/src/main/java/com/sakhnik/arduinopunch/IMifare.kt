package com.sakhnik.arduinopunch

interface IMifare {
    val sectorCount: Int
    val blockCount: Int
    val keyDefault: ByteArray

    fun blockToSector(blockIndex: Int): Int

    fun authenticateSectorWithKeyA(sectorIndex: Int, key: ByteArray?): Boolean
    fun readBlock(blockIndex: Int): ByteArray?
    fun writeBlock(blockIndex: Int, data: ByteArray?)
}
