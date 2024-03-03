package com.sakhnik.arduinopunch

import android.content.Context
import java.util.Arrays
import kotlin.random.Random

class PunchCard(private val mifare: IMifare, private val key: ByteArray, private val context: Context) {
    private var authSector = -1

    companion object {
        const val INDEX_SECTOR = 0
        const val HEADER_BLOCK1 = 1
        const val HEADER_BLOCK2 = 2
        const val INDEX_KEY_BLOCK = 3

        // ID is written into KeyB
        const val ID_OFFSET = 10
        const val SECTOR_OFFSET = 12

        const val CHECK_STATION = 1
        const val START_STATION = 10
        const val FINISH_STATION = 255

        const val PUNCHES_PER_BLOCK = 4
        const val PUNCHES_PER_SECTOR = PUNCHES_PER_BLOCK * 3

        const val COUNT_OFFSET = 0
        const val STATION_OFFSET = 1
        const val TIME1_OFFSET = 5
        const val TIME2_OFFSET = 8

        // Default access bits that allow using KeyB for data
        val ACCESS_BITS = byteArrayOf(0xff.toByte(), 0x07.toByte(), 0x80.toByte())
    }

    init {
        assert(key.size == mifare.keyDefault.size)
    }

    fun getMaxPunches(mifare: IMifare): Int {
        return (mifare.sectorCount - 2) * PUNCHES_PER_SECTOR
    }

    private fun authenticate(sector: Int) {
        if (sector != authSector) {
            if (!mifare.authenticateSectorWithKeyA(sector, key)) {
                throw RuntimeException(context.getString(R.string.key_authentication_failure_to_sector, sector))
            }
            authSector = sector
        }
    }

    // Clear the card before the start without changing the format information
    fun clear(progress: Progress = Procedure.NO_PROGRESS) {
        // 1. read startSector
        progress(0, 2)
        authenticate(INDEX_SECTOR)
        val headerKey = mifare.readBlock(INDEX_KEY_BLOCK) as ByteArray
        val startSector = headerKey[SECTOR_OFFSET].toInt()

        progress(1, 2)
        clearPunches(startSector, progress)

        progress(1, 1)
    }

    fun format(id: Int, keysToTry: List<ByteArray>, progress: Progress = Procedure.NO_PROGRESS, startSector: Int = 0) {
        val procedure = Procedure()
        val goodKeys = ArrayList<ByteArray>()

        // Check and configure the card picking KeyA for all sectors
        procedure.add(mifare.sectorCount) { p ->
            pickKeyToSector(keysToTry, goodKeys, p)
        }

        // Choose a start sector for punches to even out card wear
        val chosenStartSector = if (startSector >= 1 && startSector < mifare.sectorCount) {
            startSector
        } else {
            Random.nextInt(1, mifare.sectorCount)
        }

        // Configure KeyA, access bits and write card ID to KeyB
        procedure.add(mifare.sectorCount) { p ->
            writeNewKey(id, goodKeys, p, chosenStartSector)
        }

        procedure.add(3) { p ->
            clearPunches(startSector, p)
        }

        procedure.run(progress)
    }

    private fun clearPunches(startSector: Int, progress: Progress) {
        progress(0, 3)
        authenticate(startSector)
        val headerBlock1 = startSector * 4 + HEADER_BLOCK1
        val headerBlock2 = startSector * 4 + HEADER_BLOCK2
        progress(1, 3)
        val data = ByteArray(16)
        mifare.writeBlock(headerBlock1, data)
        progress(2, 3)
        mifare.writeBlock(headerBlock2, data)
    }

    private fun writeNewKey(id: Int, keys: List<ByteArray>, progress: Progress, startSector: Int) {
        for (sector in 0 until mifare.sectorCount) {
            progress(sector, mifare.sectorCount)
            if (!mifare.authenticateSectorWithKeyA(sector, keys[sector])) {
                throw RuntimeException(context.getString(R.string.no_known_key_for_sector, sector))
            }
            val blockIndex = 3 + 4 * sector
            val trailer = mifare.readBlock(blockIndex) as ByteArray
            key.copyInto(trailer)
            // Restore default access bits to use KeyB for data
            ACCESS_BITS.copyInto(trailer, 6)
            // Write card ID to KeyB
            byteArrayOf(id.toByte(), (id shr 8).toByte()).copyInto(trailer, ID_OFFSET)
            trailer[SECTOR_OFFSET] = startSector.toByte()
            mifare.writeBlock(blockIndex, trailer)
        }
    }

    private fun pickKeyToSector(keysToTry: List<ByteArray>, goodKeys: MutableList<ByteArray>, progress: Progress) {
        for (sector in 0 until mifare.sectorCount) {
            progress(sector, mifare.sectorCount)
            // Try the previous sector's key first for better luck
            if (sector > 0 && mifare.authenticateSectorWithKeyA(sector, goodKeys[sector - 1])) {
                goodKeys.add(goodKeys[sector - 1])
            }
            else if (mifare.authenticateSectorWithKeyA(sector, mifare.keyDefault)) {
                goodKeys.add(mifare.keyDefault)
            } else {
                val k = keysToTry.find {
                    mifare.authenticateSectorWithKeyA(sector, it)
                }
                    ?: throw RuntimeException(context.getString(R.string.no_known_key_for_sector, sector))
                goodKeys.add(k)
            }
        }
    }

    fun reset(keysToTry: List<ByteArray>, progress: Progress = Procedure.NO_PROGRESS) {
        val procedure = Procedure()
        val goodKeys = ArrayList<ByteArray>()

        // Check and configure the card setting KeyA into all sectors
        procedure.add(mifare.sectorCount) { p ->
            pickKeyToSector(keysToTry, goodKeys, p)
        }

        procedure.add(mifare.sectorCount) { p ->
            val sectorCount = mifare.sectorCount
            // Restore the card to its pristine state
            for (sector in 0 until sectorCount) {
                p(sector, sectorCount)
                if (goodKeys[sector].contentEquals(mifare.keyDefault))
                    continue
                if (!mifare.authenticateSectorWithKeyA(sector, goodKeys[sector])) {
                    throw RuntimeException(context.getString(R.string.key_authentication_failure_to_sector, sector))
                }
                val blockIndex = 3 + 4 * sector
                val trailer = mifare.readBlock(blockIndex) as ByteArray
                mifare.keyDefault.copyInto(trailer)
                mifare.writeBlock(blockIndex, trailer)
            }
        }

        procedure.run(progress)
    }

    private fun getId(header: ByteArray): Int {
        return (header[ID_OFFSET].toInt() and 0xff) or ((header[ID_OFFSET + 1].toInt() and 0xff) shl 8)
    }

    private fun recoverHeader(startSector: Int, progressStages: Int, progress: Progress): ByteArray {
        val headerBlock1 = startSector * 4 + HEADER_BLOCK1
        val headerBlock2 = startSector * 4 + HEADER_BLOCK2

        // 2. read header 1
        progress(2, progressStages)
        authenticate(startSector)
        val header1 = mifare.readBlock(headerBlock1) as ByteArray
        // 3. read header 2
        progress(3, progressStages)
        val header2 = mifare.readBlock(headerBlock2) as ByteArray

        // 4. decide the right one
        // 4.1 write the missing one back
        progress(4, progressStages)
        val header1IsOk = header1[0].toInt() and 0xff != 0xff
        val header2IsOk = header2[0].toInt() and 0xff != 0xff
        if (!header1IsOk && !header2IsOk) {
            throw RuntimeException(context.getString(R.string.data_is_corrupted))
        }
        return if (header1IsOk) {
            mifare.writeBlock(headerBlock2, header1)
            header1
        } else {
            mifare.writeBlock(headerBlock1, header2)
            header2
        }
    }

    fun punch(newPunch: Punch, progress: Progress = Procedure.NO_PROGRESS) {
        val stages = 9
        progress(0, stages)

        // 1. read card id
        progress(1, stages)
        authenticate(INDEX_SECTOR)
        val headerKey = mifare.readBlock(INDEX_KEY_BLOCK) as ByteArray
        //val cardId = getId(headerKey)
        val startSector = headerKey[SECTOR_OFFSET].toInt()

        val header = recoverHeader(startSector, stages, progress)

        // 5. calculate the offset in the header
        // If this is the start station punching, clear all the previous punches
        if (newPunch.station == START_STATION) {
            Arrays.fill(header, 0)
        }
        val count = header[0].toInt() and 0xff
        val offsetInBlock = count % PUNCHES_PER_BLOCK

        //   a. check the station is different and not the start station.
        // allow punching the start station many times because the index is reset
        val prevStationId = if (count == 0) {
            -1
        } else {
            val blockOffset = (count - 1) % PUNCHES_PER_BLOCK
            header[STATION_OFFSET + blockOffset].toInt()
        }
        if (newPunch.station != START_STATION && prevStationId == newPunch.station) {
            progress(stages, stages)
            return
        }

        if (count == getMaxPunches(mifare)) {
            throw RuntimeException(context.getString(R.string.the_card_is_full))
        }

        // 6. if the offset is 4, store the completed row to the proper block
        if (count != 0 && offsetInBlock == 0) {
            progress(6, stages)
            val block = getPunchBlock(count - PUNCHES_PER_BLOCK, startSector)
            authenticate(mifare.blockToSector(block))
            mifare.writeBlock(block, header)
            Arrays.fill(header, STATION_OFFSET, header.size, 0)
        }
        header[COUNT_OFFSET] = (count + 1).toByte()

        // 7. add data to the header
        val blockOffset = count % PUNCHES_PER_BLOCK
        newPunch.serializeStation(header, STATION_OFFSET + blockOffset)
        if (blockOffset == 0) {
            // Store full timestamp for the first punch in the block
            newPunch.serializeTimestamp(header, TIME1_OFFSET)
        } else {
            // Store two-byte difference from the first punch timestamp in the block
            val timestamp0 = Punch.getTimestamp(header, TIME1_OFFSET)
            newPunch.serializeTimestamp(header, TIME2_OFFSET + blockOffset * 2, timestamp0)
        }

        // 8. write header 2
        progress(7, stages)
        val headerBlock2 = startSector * 4 + HEADER_BLOCK2
        authenticate(startSector)
        mifare.writeBlock(headerBlock2, header)

        // 9. write header 1
        progress(8, stages)
        val headerBlock1 = startSector * 4 + HEADER_BLOCK1
        authenticate(startSector)
        mifare.writeBlock(headerBlock1, header)

        progress(stages, stages)
    }

    private fun getPunchBlock(index: Int, startSector: Int): Int {
        var sector = (index / PUNCHES_PER_SECTOR + startSector + 1)
        if (sector >= mifare.sectorCount) {
            sector -= mifare.sectorCount - 1
        }
        val sectorOffset = index % PUNCHES_PER_SECTOR / PUNCHES_PER_BLOCK
        return sector * 4 + sectorOffset
    }

    data class Info(val cardNumber: Int, val punches: List<Punch>)

    fun readOut(progress: Progress = Procedure.NO_PROGRESS): Info {
        val stages = 8
        progress(0, 1)

        // 1. read card id
        progress(1, stages)
        authenticate(INDEX_SECTOR)
        val headerKey = mifare.readBlock(INDEX_KEY_BLOCK) as ByteArray
        val cardId = getId(headerKey)
        val startSector = headerKey[SECTOR_OFFSET].toInt()

        // 2. Recover the header
        val header = recoverHeader(startSector, stages, progress)
        val count = header[COUNT_OFFSET].toInt() and 0xff

        // 5. loop over the punches
        val punches = ArrayList<Punch>()

        if (count > 0) {
            var tail = count % PUNCHES_PER_BLOCK
            if (tail == 0) {
                tail = PUNCHES_PER_BLOCK
            }
            val wholeBlocks = (count - tail) / PUNCHES_PER_BLOCK
            for (i in 0 until wholeBlocks) {
                val block = getPunchBlock(i * PUNCHES_PER_BLOCK, startSector)
                progress(i, wholeBlocks)
                authenticate(mifare.blockToSector(block))
                val punchBlock = mifare.readBlock(block)
                readPunchesFromBlock(4, punchBlock!!, punches)
            }
            readPunchesFromBlock(tail, header, punches)
        }

        progress(1, 1)
        return Info(cardId, punches)
    }

    private fun readPunchesFromBlock(count: Int, data: ByteArray, punches: ArrayList<Punch>) {
        var timestamp0: Long = 0
        for (offset in 0 until count) {
            val stationId = Punch.getStation(data, STATION_OFFSET + offset)
            val punch = if (offset == 0) {
                timestamp0 = Punch.getTimestamp(data, TIME1_OFFSET)
                Punch(stationId, timestamp0)
            } else {
                val timestamp = Punch.getTimestamp(data, TIME2_OFFSET + 2 * offset, timestamp0)
                Punch(stationId, timestamp)
            }
            punches.add(punch)
        }
    }
}
