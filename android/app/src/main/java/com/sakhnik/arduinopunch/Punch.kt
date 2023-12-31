package com.sakhnik.arduinopunch

data class Punch(val station: Int, val timestamp: Long) {

    fun serialize(data: ByteArray, offset: Int) {
        data[offset] = station.toByte()
        // Store timestamp & 0xffffff.
        data[offset + 1] = timestamp.toByte()
        data[offset + 2] = (timestamp shr 8).toByte()
        data[offset + 3] = (timestamp shr 16).toByte()
    }

    constructor(data: ByteArray, offset: Int) : this(getStation(data, offset), getTimestamp(data, offset))

    companion object {
        const val STORAGE_SIZE = 4

        private fun getStation(data: ByteArray, offset: Int): Int {
            return data[offset].toInt() and 0xff
        }

        private fun getTimestamp(data: ByteArray, offset: Int): Long {
            return  (data[offset + 1].toLong() and 0xff) or
                    (data[offset + 2].toLong() and 0xff shl 8) or
                    (data[offset + 3].toLong() and 0xff shl 16)
        }
    }
}
