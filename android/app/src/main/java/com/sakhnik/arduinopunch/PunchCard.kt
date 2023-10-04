package com.sakhnik.arduinopunch

import android.nfc.tech.MifareClassic

const val HEADER_BLOCK = 1

class PunchCard(private val mifare: IMifare, private val key: ByteArray) {
    private var authSector = -1

    companion object {
        val MAGIC_OFFSET: Int = 0
        val ID_OFFSET: Int = 1
        val TIMESTAMP_OFFSET: Int = 3
        val INDEX_OFFSET = 7
    }

    init {
        assert(key.size == mifare.keyDefault.size)
    }

    private fun authenticate(sector: Int) {
        if (sector != authSector) {
            if (!mifare.authenticateSectorWithKeyA(sector, key)) {
                throw RuntimeException("Failed to authenticate to sector $sector with the key")
            }
            authSector = sector
        }
    }

    fun prepare(id: Int, timestamp: Long, progress: (Int, Int) -> Unit = {_, _ -> }) {
        val sectorCount = mifare.sectorCount
        val stages = 2 * sectorCount + 2

        // Check and configure the card setting KeyA into all sectors
        for (sector in 0 until mifare.sectorCount) {
            progress(sector, stages)
            if (!mifare.authenticateSectorWithKeyA(sector, mifare.keyDefault)
                && !mifare.authenticateSectorWithKeyA(sector, key)) {
                throw RuntimeException("Can't authenticate to sector $sector with either default or our key")
            }
        }
        for (sector in 0 until mifare.sectorCount) {
            progress(sectorCount + sector, stages)
            if (!mifare.authenticateSectorWithKeyA(sector, mifare.keyDefault)) {
                if (!mifare.authenticateSectorWithKeyA(sector, key)) {
                    throw RuntimeException("Failed to authenticate to sector $sector with either default or our key")
                }
            }
            val blockIndex = 3 + 4 * sector
            val trailer = mifare.readBlock(blockIndex)
            key.copyInto(trailer as ByteArray)
            mifare.writeBlock(blockIndex, trailer)
        }

        progress(stages - 2, stages)
        val (block, offset) = getPunchAddr(0)
        authenticate(mifare.blockToSector(block))
        val punchBlock = mifare.readBlock(block)
        // Current real time
        Punch(0, timestamp).serialize(punchBlock as ByteArray, offset)
        mifare.writeBlock(block, punchBlock)

        progress(stages - 1, stages)
        authenticate(mifare.blockToSector(HEADER_BLOCK))
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        // Format the index
        header[MAGIC_OFFSET] = 1  // magic
        header[ID_OFFSET] = id.toByte()
        header[ID_OFFSET+1] = (id shr 8).toByte()
        for (i in 0..3) header[TIMESTAMP_OFFSET + i] = (timestamp shr (i*8)).toByte()
        header[INDEX_OFFSET] = 1
        mifare.writeBlock(HEADER_BLOCK, header)
        progress(stages, stages)
    }

    fun reset(progress: (Int, Int) -> Unit = {_, _ -> }) {
        val sectorCount = mifare.sectorCount
        // Restore the card to its pristine state
        for (sector in 0 until mifare.sectorCount) {
            progress(sector, sectorCount)
            if (!mifare.authenticateSectorWithKeyA(sector, key)) {
                // Try with the default key too
                if (!mifare.authenticateSectorWithKeyA(sector, mifare.keyDefault)) {
                    throw RuntimeException("Can't authenticate to sector $sector with our key")
                } else {
                    continue
                }
            }
            val blockIndex = 3 + 4 * sector
            val trailer = mifare.readBlock(blockIndex) as ByteArray
            mifare.keyDefault.copyInto(trailer)
            mifare.writeBlock(blockIndex, trailer)
        }
        progress(sectorCount, sectorCount)
    }

    private fun getPunchAddr(index: Int): Pair<Int, Int> {
        val byteAddr = index * Punch.STORAGE_SIZE
        val offset = byteAddr % MifareClassic.BLOCK_SIZE
        val blockNumber = byteAddr / MifareClassic.BLOCK_SIZE

        // Map onto free blocks skipping through the trailer blocks
        // 0 -> 2    1 -> 4, 2 -> 5, 3 -> 6   4 -> 8 ...
        val block = (blockNumber + 2) / 3 * 4 + (blockNumber + 2) % 3
        return Pair(block, offset)
    }

    fun punch(newPunch: Punch, progress: (Int, Int) -> Unit = {_, _ -> }): Boolean {
        val stages = 8
        progress(0, stages)

        // 1. read the index
        val headerSector = mifare.blockToSector(HEADER_BLOCK)
        authenticate(headerSector)
        progress(1, stages)
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        progress(2, stages)
        val index = header[INDEX_OFFSET].toInt() and 0xff
        val (block, offset) = getPunchAddr(index - 1)
        authenticate(mifare.blockToSector(block))
        progress(3, stages)
        val punchBlock = mifare.readBlock(block) as ByteArray
        progress(4, stages)

        // 2. read the last record
        val prevPunch = Punch(punchBlock, offset)
        //   a. check the station is different
        if (prevPunch.station == newPunch.station) {
            progress(stages, stages)
            return true
        }

        // 3. write the next record
        val (newBlock, newOffset) = getPunchAddr(index)
        if (newBlock == block) {
            newPunch.serialize(punchBlock, newOffset)
            mifare.writeBlock(newBlock, punchBlock)
        } else {
            if (newBlock >= mifare.blockCount) {
                throw RuntimeException("The card is full")
            }
            assert(newOffset == 0)
            authenticate(mifare.blockToSector(newBlock))
            progress(5, stages)
            val newPunchBlock = mifare.readBlock(newBlock) as ByteArray
            progress(6, stages)
            newPunch.serialize(newPunchBlock, newOffset)
            mifare.writeBlock(newBlock, newPunchBlock)
        }
        progress(7, stages)

        // 4. update the index
        header[INDEX_OFFSET] = (index + 1).toByte()
        authenticate(headerSector)
        mifare.writeBlock(HEADER_BLOCK, header)
        progress(stages, stages)
        return true
    }

    fun readOut(progress: (Int, Int) -> Unit = {_, _ -> }): List<Punch> {
        progress(0, 1)
        // Read everything
        // Reconstruct timestamps
        val headerSector = mifare.blockToSector(HEADER_BLOCK)
        authenticate(headerSector)
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        val count = header[INDEX_OFFSET].toInt() and 0xff

        var punchBlock: ByteArray? = null
        var prevBlock = -1

        val punches = ArrayList<Punch>()
        for (index in 1 until count) {
            progress(index, count)
            val (block, offset) = getPunchAddr(index)
            if (block != prevBlock) {
                prevBlock = block
                authenticate(mifare.blockToSector(block))
                punchBlock = mifare.readBlock(block)
            }
            punches.add(Punch(punchBlock as ByteArray, offset))
        }
        progress(count, count)
        return punches
    }
}
