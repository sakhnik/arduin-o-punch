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
        assertEquals(0.toByte(), data[3])

        Punch(0xfe, 0xdeadbe).serialize(data, 1)
        assertEquals(0xfe.toByte(), data[1])
        assertEquals(0xbe.toByte(), data[2])
        assertEquals(0xad.toByte(), data[3])
        assertEquals(0xde.toByte(), data[4])
    }

    @Test
    fun deserialize() {
        val data = byteArrayOf(0xde.toByte(), 0xad.toByte(), 0xbe.toByte(), 0xef.toByte(), 0x12.toByte())
        val p1 = Punch(data, 0)
        assertEquals(0xde, p1.station)
        assertEquals(0xefbead, p1.timestamp)

        val p2 = Punch(data, 1)
        assertEquals(0xad, p2.station)
        assertEquals(0x12efbe, p2.timestamp)
    }
}
