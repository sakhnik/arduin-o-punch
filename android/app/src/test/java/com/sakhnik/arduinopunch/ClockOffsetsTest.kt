package com.sakhnik.arduinopunch

import androidx.activity.ComponentActivity
import org.junit.Assert.*
import org.junit.Test
import org.mockito.Mockito

class ClockOffsetsTest {

    private class TestStorage : Storage(Mockito.mock(ComponentActivity::class.java)) {
        override fun restoreClockOffsets(offsets: OffsetsT) {
        }

        override fun storeClockOffsets(offsets: OffsetsT) {
        }
    }

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
        val offsets = ClockOffsets(TestStorage())
        offsets.process(punches)
        assertEquals(101L, offsets.getOffsets()[31])
        assertEquals(101L, offsets.getOffsets()[32])
        assertEquals(1003L - 13, offsets.getOffsets()[33])
    }
}
