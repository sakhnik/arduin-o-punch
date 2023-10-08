package com.sakhnik.arduinopunch

import java.util.TreeMap
import kotlin.math.roundToInt

class ClockOffsets {
    val offsets: MutableMap<Int, Long> = TreeMap()

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
    }

    fun applyTo(punches: List<Punch>): List<Punch> {
        return punches.map {
            Punch(it.station, it.timestamp + offsets.getOrDefault(it.station, 0))
        }
    }
}
