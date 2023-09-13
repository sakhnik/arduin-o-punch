package com.sakhnik.arduinopunch

import org.junit.Assert.*

import org.junit.Test

class PunchTest {

    @Test
    fun serialize() {
        val data = ByteArray(Punch.STORAGE_SIZE * 2) { _ -> 0 }
        Punch(1, 3).serialize(data, 0)
        assertEquals(1.toByte(), data[0])
        assertEquals(3.toByte(), data[1])
        assertEquals(0.toByte(), data[2])

        Punch(0xfe, 0xdead).serialize(data, 1)
        assertEquals(0xfe.toByte(), data[1])
        assertEquals(0xad.toByte(), data[2])
        assertEquals(0xde.toByte(), data[3])
    }

    @Test
    fun deserialize() {
        val data = byteArrayOf(0xde.toByte(), 0xad.toByte(), 0xbe.toByte(), 0xef.toByte())
        val p1 = Punch(data, 0)
        assertEquals(0xde, p1.station)
        assertEquals(0xbead, p1.timestamp)

        val p2 = Punch(data, 1)
        assertEquals(0xad, p2.station)
        assertEquals(0xefbe, p2.timestamp)
    }
}
