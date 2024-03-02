package com.sakhnik.arduinopunch

data class Punch(val station: Int, val timestamp: Long) {

    fun serializeStation(data: ByteArray, offset: Int) {
        data[offset] = station.toByte()
    }

    fun serializeTimestamp(data: ByteArray, offset: Int) {
        // Store timestamp & 0xffffff.
        data[offset] = timestamp.toByte()
        data[offset + 1] = (timestamp shr 8).toByte()
        data[offset + 2] = (timestamp shr 16).toByte()
    }

    fun serializeTimestamp(data: ByteArray, offset: Int, timestamp0: Long) {
        val dt = timestamp - timestamp0
        data[offset] = dt.toByte()
        data[offset + 1] = (dt shr 8).toByte()
    }

    companion object {
        fun getStation(data: ByteArray, offset: Int): Int {
            return data[offset].toInt() and 0xff
        }

        fun getTimestamp(data: ByteArray, offset: Int): Long {
            return  (data[offset].toLong() and 0xff) or
                    (data[offset + 1].toLong() and 0xff shl 8) or
                    (data[offset + 2].toLong() and 0xff shl 16)
        }

        fun getTimestamp(data: ByteArray, offset: Int, timestamp0: Long): Long {
            return timestamp0 +
                ((data[offset].toLong() and 0xff) or
                (data[offset + 1].toLong() and 0xff shl 8))
        }
    }
}
