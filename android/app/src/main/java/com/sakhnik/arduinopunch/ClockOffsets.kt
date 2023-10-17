package com.sakhnik.arduinopunch

import java.util.TreeMap
import kotlin.math.roundToInt

typealias OffsetsT = MutableMap<Int, Long>

class ClockOffsets(private val storage: Storage) {
    private val offsets: OffsetsT = TreeMap()

    fun init() {
        storage.restoreClockOffsets(offsets)
    }

    fun getOffsets(): OffsetsT {
        return offsets
    }

    fun forEach(action: (Int, Long) -> Unit): Unit {
        offsets.forEach(action)
    }

    fun clear() {
        offsets.clear()
        storage.storeClockOffsets(offsets)
    }

    fun process(punches: List<Punch>) {
        var start = 0
        for (i in punches.indices) {
            if (punches[i].station == 0) {
                if (i > start) {
                    val intervals = i - start
                    val delay = punches[i].timestamp - punches[start].timestamp
                    val portion: Float = delay.toFloat() / intervals
                    for (j in start + 1 until i) {
                        val station = punches[j].station
                        offsets[station] = punches[start].timestamp + (portion * (j - start)).roundToInt() - punches[j].timestamp;
                    }
                }
                start = i
            }
        }
        storage.storeClockOffsets(offsets)
    }

    fun applyTo(punches: List<Punch>): List<Punch> {
        return punches.map {
            Punch(it.station, it.timestamp + offsets.getOrDefault(it.station, 0))
        }
    }
}
