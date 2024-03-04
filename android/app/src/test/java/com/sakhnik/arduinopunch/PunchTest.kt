package com.sakhnik.arduinopunch

import org.junit.Assert.*

import org.junit.Test

class PunchTest {

    @Test
    fun serialize() {
        val data = ByteArray(8)
        val p1 = Punch(1, 3)
        p1.serializeStation(data, 0)
        p1.serializeTimestamp(data, 1)
        assertEquals(1.toByte(), data[0])
        assertEquals(3.toByte(), data[1])
        assertEquals(0.toByte(), data[2])
        assertEquals(0.toByte(), data[3])

        val p2 = Punch(0xfe, 0xdeadbe)
        p2.serializeStation(data, 1)
        p2.serializeTimestamp(data, 2, 0xdeadbe - 0x1234)
        assertEquals(0xfe.toByte(), data[1])
        assertEquals(0x34.toByte(), data[2])
        assertEquals(0x12.toByte(), data[3])
    }

    @Test
    fun deserialize() {
        val data = byteArrayOf(0xde.toByte(), 0xad.toByte(), 0xbe.toByte(), 0x7f.toByte(), 0x12.toByte())
        assertEquals(0xde, Punch.getStation(data, 0))
        assertEquals(0x7fbead, Punch.getTimestamp(data, 1))

        assertEquals(0xad, Punch.getStation(data, 1))
        assertEquals(0x7fbe + 12345, Punch.getTimestamp(data, 2, 12345))
    }

    @Test
    fun punchDecreasingTimestamp() {
        val data = ByteArray(8)
        val p1 = Punch(31, 12345)
        p1.serializeTimestamp(data, 0)
        assertEquals(12345, Punch.getTimestamp(data, 0))
        p1.serializeTimestamp(data, 0, 12340)
        assertEquals(12345, Punch.getTimestamp(data, 0, 12340))
        p1.serializeTimestamp(data, 0, 12350)
        assertEquals(12345, Punch.getTimestamp(data, 0, 12350))
    }
}
