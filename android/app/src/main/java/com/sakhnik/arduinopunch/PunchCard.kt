package com.sakhnik.arduinopunch

import android.nfc.tech.MifareClassic
import android.util.Log

class PunchCard(mifareClassic: MifareClassic, key: ByteArray) {
    val mifareClassic: MifareClassic = mifareClassic
    val key: ByteArray = key

    init {
        assert(key.size == MifareClassic.KEY_DEFAULT.size)
    }

    fun prepare(progress: (Int, Int) -> Unit = {_, _ -> }) {
        val sectorCount = mifareClassic.sectorCount
        val stages = 2 * sectorCount
        // Check and configure the card setting KeyA into all sectors
        for (sector in 0 until mifareClassic.sectorCount) {
            progress(sector, stages)
            if (!mifareClassic.authenticateSectorWithKeyA(sector, MifareClassic.KEY_DEFAULT)) {
                throw RuntimeException("Can't authenticate to sector $sector with default key")
            }
        }
        for (sector in 0 until mifareClassic.sectorCount) {
            progress(sectorCount + sector, stages)
            if (!mifareClassic.authenticateSectorWithKeyA(sector, MifareClassic.KEY_DEFAULT)) {
                throw RuntimeException("Can't authenticate to sector $sector with default key")
            }
            val blockIndex = 3 + 4 * sector
            var trailer = mifareClassic.readBlock(blockIndex);
            key.copyInto(trailer)
            mifareClassic.writeBlock(blockIndex, trailer)
        }
        progress(stages, stages)

        // Format the index
        // Write current real time
    }

    fun reset(progress: (Int, Int) -> Unit = {_, _ -> }) {
        val sectorCount = mifareClassic.sectorCount
        // Restore the card to its pristine state
        for (sector in 0 until mifareClassic.sectorCount) {
            progress(sector, sectorCount)
            if (!mifareClassic.authenticateSectorWithKeyA(sector, key)) {
                // Try with the default key too
                if (!mifareClassic.authenticateSectorWithKeyA(sector, MifareClassic.KEY_DEFAULT)) {
                    throw RuntimeException("Can't authenticate to sector $sector with our key")
                } else {
                    continue
                }
            }
            val blockIndex = 3 + 4 * sector
            var trailer = mifareClassic.readBlock(blockIndex);
            MifareClassic.KEY_DEFAULT.copyInto(trailer)
            mifareClassic.writeBlock(blockIndex, trailer)
        }
        progress(sectorCount, sectorCount)
    }

    fun punch(station: Byte, timestamp: Int) {
        // Store timestamp & 0xffff. The recording is consecutive, and it's safe to assume
        // that the overflow can only happen by incrementing the next digit by 1.
        // 1. read the index
        // 2. read the last record
        //   a. check the station is different
        // 3. write the next record
        // 4. update the index
    }

    fun readOut() {
        // Read everything
        // Reconstruct timestamps
    }
}
