package com.sakhnik.arduinopunch

import android.nfc.tech.MifareClassic
import kotlin.experimental.xor

const val HEADER_BLOCK = 1

class PunchCard(private val mifare: IMifare, private val key: ByteArray) {
    private var authSector = -1

    companion object {
        const val DESC_OFFSET: Int = 0
        const val DESC_RUNNER: Byte = 1
        const val DESC_SERVICE: Byte = 2

        const val ID_OFFSET: Int = 1
        const val SERVICE_ID: Int = 0

        const val TIMESTAMP_OFFSET: Int = 3
        const val XOR_OFFSET: Int = 7   // Xor sum of the previous header bytes to make the service card more distinctive
        const val INDEX_OFFSET = 8
        const val KEY_OFFSET = 9
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

    // Clear the card before the start without changing the format information
    fun clear(progress: Progress = noProgress) {
        val stages = 3
        progress(0, stages)

        // 1. read the header
        val headerSector = mifare.blockToSector(HEADER_BLOCK)
        authenticate(headerSector)
        progress(1, stages)
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        if (calculateXorSum(header, XOR_OFFSET + 1) != 0.toByte()) {
            throw RuntimeException("Bad checksum")
        }
        progress(2, stages)

        // Reset the index
        header[INDEX_OFFSET] = 1

        // Write the header back
        mifare.writeBlock(HEADER_BLOCK, header)
        progress(3, stages)
    }

    fun prepareRunner(id: Int, timestamp: Long, progress: Progress = noProgress) {
        val procedure = Procedure()

        // Check and configure the card setting KeyA into all sectors
        procedure.add(mifare.sectorCount) { p ->
            checkDefaultKeyUsable(p)
        }

        // If the desired key is different from the default, update the key
        if (!key.contentEquals(mifare.keyDefault)) {
            procedure.add(mifare.sectorCount) { p ->
                writeNewKey(p)
            }
        }

        procedure.add(1) { p ->
            writeInitialPunch(timestamp, p)
        }

        procedure.add(1) { p ->
            writeRunnerHeader(p, id, timestamp)
        }

        procedure.run(progress)
    }

    fun prepareService(newKey: ByteArray, timestamp: Long, progress: Progress = noProgress) {
        assert(newKey.size == mifare.keyDefault.size)

        // If the desired key is different from the default, error out
        if (!key.contentEquals(mifare.keyDefault)) {
            throw RuntimeException("Service card should use the default key")
        }

        val procedure = Procedure()

        // Check and configure the card setting KeyA into all sectors
        procedure.add(mifare.sectorCount) { p ->
            checkDefaultKeyUsable(p)
        }

        procedure.add(1) { p ->
            writeInitialPunch(timestamp, p)
        }

        procedure.add(1) { p ->
            writeServiceHeader(newKey, timestamp, p)
        }

        procedure.run(progress)
    }

    private fun writeRunnerHeader(progress: Progress, id: Int, timestamp: Long) {
        progress(0, 1)
        authenticate(mifare.blockToSector(HEADER_BLOCK))
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        // Format the index
        header[DESC_OFFSET] = DESC_RUNNER  // descriptor
        header[ID_OFFSET] = id.toByte()
        header[ID_OFFSET + 1] = (id shr 8).toByte()
        for (i in 0..3) header[TIMESTAMP_OFFSET + i] = (timestamp shr (i * 8)).toByte()
        header[XOR_OFFSET] = calculateXorSum(header, XOR_OFFSET)
        header[INDEX_OFFSET] = 1
        mifare.keyDefault.copyInto(header, KEY_OFFSET)
        mifare.writeBlock(HEADER_BLOCK, header)
    }

    private fun writeServiceHeader(newKey: ByteArray, timestamp: Long, progress: Progress) {
        progress(0, 1)
        authenticate(mifare.blockToSector(HEADER_BLOCK))
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        // Format the index
        header[DESC_OFFSET] = DESC_SERVICE
        header[ID_OFFSET] = SERVICE_ID.toByte()
        header[ID_OFFSET + 1] = (SERVICE_ID shr 8).toByte()
        for (i in 0..3) header[TIMESTAMP_OFFSET + i] = (timestamp shr (i * 8)).toByte()
        header[XOR_OFFSET] = calculateXorSum(header, XOR_OFFSET)
        header[INDEX_OFFSET] = 1
        newKey.copyInto(header, KEY_OFFSET)
        mifare.writeBlock(HEADER_BLOCK, header)
    }

    private fun calculateXorSum(header: ByteArray, count: Int): Byte {
        var res: Byte = 0
        for (i in 0 until count) {
            res = res xor header[i]
        }
        return res
    }

    private fun writeInitialPunch(timestamp: Long, progress: Progress) {
        progress(0, 1)
        val (block, offset) = getPunchAddr(0)
        authenticate(mifare.blockToSector(block))
        val punchBlock = mifare.readBlock(block)
        // Current real time
        Punch(0xff, timestamp).serialize(punchBlock as ByteArray, offset)
        mifare.writeBlock(block, punchBlock)
    }

    private fun writeNewKey(progress: Progress) {
        for (sector in 0 until mifare.sectorCount) {
            progress(sector, mifare.sectorCount)
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
    }

    private fun checkDefaultKeyUsable(progress: Progress) {
        for (sector in 0 until mifare.sectorCount) {
            progress(sector, mifare.sectorCount)
            if (!mifare.authenticateSectorWithKeyA(sector, mifare.keyDefault)
                && !mifare.authenticateSectorWithKeyA(sector, key)
            ) {
                throw RuntimeException("Can't authenticate to sector $sector with either default or our key")
            }
        }
    }

    fun reset(progress: Progress = noProgress) {
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

    private fun getId(header: ByteArray): Int {
        return header[ID_OFFSET].toInt() or (header[ID_OFFSET + 1].toInt() shl 8)
    }

    fun punch(newPunch: Punch, progress: Progress = noProgress): Boolean {
        val stages = 8
        progress(0, stages)

        // 1. read the index
        val headerSector = mifare.blockToSector(HEADER_BLOCK)
        authenticate(headerSector)
        progress(1, stages)
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        if (calculateXorSum(header, XOR_OFFSET + 1) != 0.toByte()) {
            throw RuntimeException("Bad checksum")
        }
        progress(2, stages)
        val desc = header[DESC_OFFSET]
        val id = getId(header)
        val index = header[INDEX_OFFSET].toInt() and 0xff
        val (block, offset) = getPunchAddr(index - 1)
        authenticate(mifare.blockToSector(block))
        progress(3, stages)
        val punchBlock = mifare.readBlock(block) as ByteArray
        progress(4, stages)

        // 2. read the last record
        val prevPunch = Punch(punchBlock, offset)
        //   a. check the station is different or is a service card
        if (prevPunch.station == newPunch.station &&
            (desc != DESC_SERVICE || id == 0)) {
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

    data class Info(val cardNumber: Int, val punches: List<Punch>)

    fun readOut(progress: Progress = noProgress): Info {
        progress(0, 1)
        // Read everything
        // Reconstruct timestamps
        val headerSector = mifare.blockToSector(HEADER_BLOCK)
        authenticate(headerSector)
        val header = mifare.readBlock(HEADER_BLOCK) as ByteArray
        if (calculateXorSum(header, XOR_OFFSET + 1) != 0.toByte()) {
            throw RuntimeException("Bad checksum")
        }
        val id = getId(header)
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
        return Info(id, punches)
    }
}
