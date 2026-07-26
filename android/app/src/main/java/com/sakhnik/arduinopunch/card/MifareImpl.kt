package com.sakhnik.arduinopunch.card

import android.nfc.tech.MifareClassic
import timber.log.Timber

class MifareImpl(private val mifare: MifareClassic) : IMifare {
    private val debug = false

    override val sectorCount: Int
        get() = mifare.sectorCount
    override val blockCount: Int
        get() = mifare.blockCount

    override fun blockToSector(blockIndex: Int): Int {
        return mifare.blockToSector(blockIndex)
    }

    override val keyDefault: ByteArray
        get() = MifareClassic.KEY_DEFAULT

    private fun ByteArray.toHexString(): String =
        joinToString(" ") { "%02X".format(it.toInt() and 0xFF) }

    override fun authenticateSectorWithKeyA(sectorIndex: Int, key: ByteArray?): Boolean {
        val keyString = key?.toHexString() ?: "null"
        val result = mifare.authenticateSectorWithKeyA(sectorIndex, key)
        if (debug) {
            Timber.i("AUTH sector=$sectorIndex key=[$keyString] result=$result")
        }
        return result
    }

    override fun readBlock(blockIndex: Int): ByteArray? {
        val data = mifare.readBlock(blockIndex)
        if (debug) {
            Timber.i("READ block=$blockIndex data=${data?.toHexString() ?: "null"}")
        }
        return data
    }

    override fun writeBlock(blockIndex: Int, data: ByteArray) {
        if (debug) {
            Timber.i("WRITE block=$blockIndex data=${data.toHexString()}")
        }
        mifare.writeBlock(blockIndex, data)

        if (debug) {
            // Verify what was actually written.
            val verify = mifare.readBlock(blockIndex)
            Timber.i("VERIFY block=$blockIndex data=${verify?.toHexString() ?: "null"}")
        }
    }
}
