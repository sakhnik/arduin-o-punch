package com.sakhnik.arduinopunch

import android.content.Context
import junit.framework.TestCase.assertEquals
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
        punchCard.prepareRunner(12345, 0.toLong(), listOf())
        assertEquals(12345, punchCard.readOut().cardNumber)
        punchCard.reset()
    }

    @Test
    fun punch() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, byteArrayOf(0x12, 0x34, 0x56, 0x78, 0x90.toByte(), 0xab.toByte()), context)
        punchCard.prepareRunner(42, 0.toLong(), listOf())
        assertEquals(0, punchCard.readOut().punches.size)
        val punches = listOf(Punch(31, 100), Punch(32, 130), Punch(33, 221))
        for (i in punches.indices) {
            punchCard.punch(punches[i])
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
        punchCard.prepareRunner(42, 0.toLong(), listOf())

        val testPunch = {i: Int -> Punch(i + 1, 12345.toLong() * (i + 1))}

        val maxPunches: Int = 16 * (16 * 3 - 2) / Punch.STORAGE_SIZE - 1
        for (i in 0 until maxPunches) {
            punchCard.punch(testPunch(i + 10))
        }
        // No more space
        assertThrows(RuntimeException::class.java) {
            punchCard.punch(Punch(31, 65000))
        }

        val readOut = punchCard.readOut()
        for (i in 0 until maxPunches) {
            assertEquals(testPunch(i + 10), readOut.punches[i])
        }

        punchCard.reset()
    }

    @Test
    fun prepareWithPreviousKeys() {
        val mifare = TestMifare()
        val key1 = byteArrayOf(0x12, 0x34, 0x56, 0x78, 0x90.toByte(), 0xab.toByte())
        val key2 = byteArrayOf(0x11, 0x22, 0x33, 0x44, 0x55, 0x66)
        PunchCard(mifare, key1, context).prepareRunner(42, 0.toLong(), listOf())
        val punchCard2 = PunchCard(mifare, key2, context)
        punchCard2.prepareRunner(43, 0.toLong(), listOf(key2, key1))
        assertThrows(RuntimeException::class.java) {
            // Mind the cached authSector in PunchCard
            PunchCard(mifare, key1, context).punch(Punch(31, 123.toLong()))
        }
        assertThrows(RuntimeException::class.java) {
            PunchCard(mifare, key1, context).readOut()
        }
        punchCard2.punch(Punch(32, 123.toLong()))
        val readOut = punchCard2.readOut()
        assertEquals(43, readOut.cardNumber)
        assertEquals(1, readOut.punches.size)
        assertEquals(Punch(32, 123.toLong()), readOut.punches[0])

        assertThrows(RuntimeException::class.java) {
            PunchCard(mifare, key1, context).reset()
        }
        punchCard2.reset()
    }
}
