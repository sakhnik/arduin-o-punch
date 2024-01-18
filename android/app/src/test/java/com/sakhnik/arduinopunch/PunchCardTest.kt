package com.sakhnik.arduinopunch

import android.content.Context
import junit.framework.TestCase.assertEquals
import junit.framework.TestCase.assertTrue
import org.junit.Assert.assertThrows
import org.junit.Test
import org.mockito.Mockito

class PunchCardTest {

    private val context: Context = Mockito.mock(Context::class.java)

    class TestMifare : IMifare {
        private val blocks: ArrayList<ByteArray> = ArrayList()
        private val defKey = ByteArray(6) { _ -> 0xff.toByte() }

        init {
            repeat(blockCount) {
                blocks.add(ByteArray(16))
            }
            repeat(sectorCount) {
                defKey.copyInto(blocks[it * 4 + 3])
            }
        }

        override val sectorCount: Int
            get() = 16

        override val blockCount: Int
            get() = 64

        override fun blockToSector(blockIndex: Int): Int {
            return blockIndex / 4
        }

        override val keyDefault: ByteArray
            get() = defKey

        override fun authenticateSectorWithKeyA(sectorIndex: Int, key: ByteArray?): Boolean {
            val bytes = blocks[sectorIndex * 4 + 3]
            repeat(6) {
                if ((key as ByteArray)[it] != bytes[it]) {
                    return false
                }
            }
            return true
        }

        override fun readBlock(blockIndex: Int): ByteArray {
            return blocks[blockIndex]
        }

        override fun writeBlock(blockIndex: Int, data: ByteArray?) {
            blocks[blockIndex] = data as ByteArray
        }

    }


    @Test
    fun prepareReset() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, byteArrayOf(0x12, 0x34, 0x56, 0x78, 0x90.toByte(), 0xab.toByte()), context)
        punchCard.prepareRunner(12345, 0.toLong())
        assertEquals(12345, punchCard.readOut().cardNumber)
        punchCard.reset()
    }

    @Test
    fun punch() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, byteArrayOf(0x12, 0x34, 0x56, 0x78, 0x90.toByte(), 0xab.toByte()), context)
        punchCard.prepareRunner(42, 0.toLong())
        assertEquals(0, punchCard.readOut().punches.size)
        val punches = listOf(Punch(31, 100), Punch(32, 130), Punch(33, 221))
        for (i in punches.indices) {
            assertTrue(punchCard.punch(punches[i]))
            val readOut = punchCard.readOut().punches
            assertEquals(i + 1, readOut.size)
            assertEquals(punches[i], readOut[i])
        }

        val readOut = punchCard.readOut()
        assertEquals(punches.size, readOut.punches.size)
        assertEquals(punches, readOut.punches)

        punchCard.reset()
    }

    @Test
    fun maxPunches() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, byteArrayOf(0x12, 0x34, 0x56, 0x78, 0x90.toByte(), 0xab.toByte()), context)
        punchCard.prepareRunner(42, 0.toLong())

        val testPunch = {i: Int -> Punch(i + 1, 12345.toLong() * (i + 1))}

        val maxPunches: Int = 16 * (16 * 3 - 2) / Punch.STORAGE_SIZE - 1
        for (i in 0 until maxPunches) {
            assertTrue(punchCard.punch(testPunch(i)))
        }
        // No more space
        assertThrows(RuntimeException::class.java) {
            punchCard.punch(Punch(1, 65000))
        }

        val readOut = punchCard.readOut()
        for (i in 0 until maxPunches) {
            assertEquals(testPunch(i), readOut.punches[i])
        }

        punchCard.reset()
    }
}
