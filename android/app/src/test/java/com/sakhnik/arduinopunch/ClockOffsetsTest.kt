package com.sakhnik.arduinopunch

import org.junit.Assert.*
import org.junit.Test

class ClockOffsetsTest {

    @Test
    fun testProcess() {
        val punches = listOf(
            Punch(0, 100),
            Punch(31, 0),
            Punch(32, 1),
            Punch(0, 103),
            Punch(0, 1000),
            Punch(33, 13),
            Punch(0, 1005)
        )
        val offsets = ClockOffsets()
        offsets.process(punches)
        assertEquals(101L, offsets.offsets[31])
        assertEquals(101L, offsets.offsets[32])
        assertEquals(1003L - 13, offsets.offsets[33])
    }
}
