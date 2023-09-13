package com.sakhnik.arduinopunch

import android.nfc.tech.MifareClassic

class MifareImpl(private val mifare: MifareClassic) : IMifare {
    override val sectorCount: Int
        get() = mifare.sectorCount
    override val blockCount: Int
        get() = mifare.blockCount

    override fun blockToSector(blockIndex: Int): Int {
        return mifare.blockToSector(blockIndex)
    }

    override val keyDefault: ByteArray
        get() = MifareClassic.KEY_DEFAULT

    override fun authenticateSectorWithKeyA(sectorIndex: Int, key: ByteArray?): Boolean {
        return mifare.authenticateSectorWithKeyA(sectorIndex, key)
    }

    override fun readBlock(blockIndex: Int): ByteArray? {
        return mifare.readBlock(blockIndex)
    }

    override fun writeBlock(blockIndex: Int, data: ByteArray?) {
        mifare.writeBlock(blockIndex, data)
    }
}
