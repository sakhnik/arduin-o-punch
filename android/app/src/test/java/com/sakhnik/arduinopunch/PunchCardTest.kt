package com.sakhnik.arduinopunch

import android.content.Context
import junit.framework.TestCase.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertThrows
import org.junit.Assert.assertTrue
import org.junit.Test
import org.mockito.Mockito
import java.util.Arrays
import kotlin.random.Random

class PunchCardTest {

    private val context: Context = Mockito.mock(Context::class.java)

    class TestMifare : IMifare {
        private val blocks: ArrayList<ByteArray> = ArrayList()
        private val defKey = ByteArray(6) { _ -> 0xff.toByte() }
        private var authSector = -1
        // Schedule subsequent writes to fail this many times
        private var failWrites = 0

        init {
            repeat(blockCount) {
                blocks.add(ByteArray(16))
            }
            repeat(sectorCount) {
                defKey.copyInto(blocks[it * 4 + 3])
            }
        }

        fun setFailWrites(count: Int) {
            failWrites = count
        }

        fun getFailWrites(): Int {
            return failWrites
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
            authSector = sectorIndex
            return true
        }

        override fun readBlock(blockIndex: Int): ByteArray {
            if (authSector != blockToSector(blockIndex)) {
                throw RuntimeException("Auth failure")
            }
            return blocks[blockIndex]
        }

        override fun writeBlock(blockIndex: Int, data: ByteArray) {
            if (authSector != blockToSector(blockIndex)) {
                throw RuntimeException("Auth failure")
            }
            if (failWrites > 0) {
                // Mimic the behaviour of some cheap cards than can lose data
                Arrays.fill(blocks[blockIndex], 0xff.toByte())
                --failWrites
                throw RuntimeException("Timeout")
            } else {
                data.copyInto(blocks[blockIndex])
            }
        }
    }

    companion object {
        val TEST_KEY = byteArrayOf(0x12, 0x34, 0x56, 0x78, 0x90.toByte(), 0xab.toByte())
    }

    @Test
    fun prepareReset() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, TEST_KEY, context)
        punchCard.format(12345, listOf())
        assertEquals(12345, punchCard.readOut().cardNumber)
        punchCard.reset(listOf(TEST_KEY))
    }

    @Test
    fun punch() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, TEST_KEY, context)
        punchCard.format(42, listOf())
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

        punchCard.reset(listOf(TEST_KEY))
    }

    @Test
    fun punchAsynchronous() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, TEST_KEY, context)
        punchCard.format(42, listOf())
        assertEquals(0, punchCard.readOut().punches.size)
        // Stations could potentially be not synchronized, the timestamps can go down occasionally.
        val punches = listOf(Punch(31, 200), Punch(32, 130), Punch(33, 100))
        for (i in punches.indices) {
            punchCard.punch(punches[i])
            val readOut = punchCard.readOut().punches
            assertEquals(i + 1, readOut.size)
            assertEquals(punches[i], readOut[i])
        }

        val readOut = punchCard.readOut()
        assertEquals(punches.size, readOut.punches.size)
        assertEquals(punches, readOut.punches)

        punchCard.reset(listOf(TEST_KEY))
    }

    @Test
    fun maxPunches() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, TEST_KEY, context)

        for (k in 0 until 16) {
            // Test repetitive formatting doesn't corrupt the card
            punchCard.format(42, listOf(TEST_KEY))

            val testPunch = {i: Int -> Punch(i + PunchCard.START_STATION, (10000 + i * 100).toLong())}

            val maxPunches = punchCard.getMaxPunches(mifare)
            for (i in 0 until maxPunches) {
                punchCard.punch(testPunch(i))
                val readOut = punchCard.readOut().punches
                assertEquals(i + 1, readOut.size)
                for (j in 0 .. i) {
                    assertEquals(testPunch(j), readOut[j])
                }
            }
            // No more space
            assertThrows(RuntimeException::class.java) {
                punchCard.punch(Punch(31, 65000))
            }

            val readOut = punchCard.readOut()
            for (i in 0 until maxPunches) {
                assertEquals(testPunch(i), readOut.punches[i])
            }
        }

        punchCard.reset(listOf(TEST_KEY))
    }

    @Test
    fun maxRepeatedPunches() {
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, TEST_KEY, context)

        // Test repetitive formatting doesn't corrupt the card
        punchCard.format(42, listOf(TEST_KEY))

        val testPunch = {i: Int -> Punch(i + PunchCard.START_STATION, (10000 + i * 100).toLong())}

        val maxPunches = punchCard.getMaxPunches(mifare)
        for (i in 0 until maxPunches) {
            // The start station can punch many times, the last timestamp counts
            if (i == 0) {
                val p = testPunch(i)
                p.timestamp -= 100
                punchCard.punch(p)
            }
            punchCard.punch(testPunch(i))
            // Only the first timestamp counts for the rest of the stations
            if (i > 0) {
                val p = testPunch(i)
                p.timestamp += 100
                punchCard.punch(p)
            }
            val readOut = punchCard.readOut().punches
            assertEquals(i + 1, readOut.size)
        }
        // No more space
        assertThrows(RuntimeException::class.java) {
            punchCard.punch(Punch(31, 65000))
        }

        val readOut = punchCard.readOut()
        for (i in 0 until maxPunches) {
            assertEquals(testPunch(i), readOut.punches[i])
        }

        punchCard.reset(listOf(TEST_KEY))
    }

    @Test
    fun prepareWithPreviousKeys() {
        val mifare = TestMifare()
        val key2 = byteArrayOf(0x11, 0x22, 0x33, 0x44, 0x55, 0x66)
        PunchCard(mifare, TEST_KEY, context).format(42, listOf())
        val punchCard2 = PunchCard(mifare, key2, context)
        punchCard2.format(43, listOf(key2, TEST_KEY))
        assertThrows(RuntimeException::class.java) {
            // Mind the cached authSector in PunchCard
            PunchCard(mifare, TEST_KEY, context).punch(Punch(31, 123.toLong()))
        }
        assertThrows(RuntimeException::class.java) {
            PunchCard(mifare, TEST_KEY, context).readOut()
        }
        punchCard2.punch(Punch(32, 123.toLong()))
        val readOut = punchCard2.readOut()
        assertEquals(43, readOut.cardNumber)
        assertEquals(1, readOut.punches.size)
        assertEquals(Punch(32, 123.toLong()), readOut.punches[0])

        assertThrows(RuntimeException::class.java) {
            PunchCard(mifare, TEST_KEY, context).reset(listOf())
        }
        punchCard2.reset(listOf(key2, TEST_KEY))
    }

    @Test
    fun recoverFromFailedWrite() {
        // Some cheap cards may lose data when timeout occurs. The puncher should
        // be resilient and never lose ability to continue punching even after
        // occasional data loss in one block because of unsuccessful write operation.
        val mifare = TestMifare()
        val punchCard = PunchCard(mifare, TEST_KEY, context)
        punchCard.format(42, listOf())
        assertEquals(0, punchCard.readOut().punches.size)

        val testPunch = {i: Int -> Punch(i + PunchCard.START_STATION, (10000 + i * 100).toLong())}
        val success = ArrayList<Int>()

        for (i in 0 until 100) {
            var exceptionAnticipated = false
            try {
                if (Random.nextInt(0, 100) < 10) {
                    mifare.setFailWrites(Random.nextInt(0, 4))
                }
                if (mifare.getFailWrites() > 0) {
                    exceptionAnticipated = true
                }
                punchCard.punch(testPunch(i))
                success.add(i)
            } catch (e: Exception) {
                assertTrue(exceptionAnticipated)
            }
        }

        assertFalse(success.isEmpty())

        mifare.setFailWrites(0)
        val readOut = punchCard.readOut()
        assertEquals(success.size, readOut.punches.size)
        for (i in 0 until success.size) {
            assertEquals(testPunch(success[i]), readOut.punches[i])
        }

        punchCard.reset(listOf(TEST_KEY))
    }
}
