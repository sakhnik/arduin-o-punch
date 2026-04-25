package com.sakhnik.arduinopunch

import java.nio.ByteBuffer
import java.nio.ByteOrder

data class DebugInfo(
    val version: Int,
    val bootCount: Int,
    val lastResetReason: Int,
    val timeStats: IntArray
) {

    companion object {
        const val SIZE = 24  // enforce contract

        fun parse(data: ByteArray): DebugInfo {
            require(data.size >= SIZE) {
                "DebugInfo requires at least $SIZE bytes, got ${data.size}"
            }

            val buffer = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN)

            val version = buffer.get().toInt() and 0xFF
            val bootCount = buffer.short.toInt() and 0xFFFF
            val lastResetReason = buffer.get().toInt() and 0xFF

            val count = (SIZE - 1 - 2 - 1) / 4
            val timeStats = IntArray(count) { buffer.int }

            return DebugInfo(version, bootCount, lastResetReason, timeStats)
        }
    }
}

object DebugStatsFormatter {

    enum class Mode {
        Active, Eco, BLE, WiFi, Sleep
    }

    private fun current(mode: Mode): Double = when (mode) {
        Mode.Active -> 20.0
        Mode.Eco -> 10.0
        Mode.BLE -> 100.0
        Mode.WiFi -> 100.0
        Mode.Sleep -> 0.05
    }

    private fun toMah(seconds: Int, current: Double): Double {
        return current * seconds / 3600.0
    }

    fun format(info: DebugInfo): List<String> {

        val lines = mutableListOf<String>()

        // -------------------------
        // HEADER
        // -------------------------
        lines += "Version: ${info.version}"
        lines += "Boot count: ${info.bootCount}"
        lines += "Reset reason: ${resetReasonText(info.lastResetReason)}"
        lines += ""

        // -------------------------
        // STATS
        // -------------------------
        val snapshot = info.timeStats

        var totalTime = 0
        var totalEnergy = 0.0

        fun line(label: String, mode: Mode, time: Int): String {
            val cur = current(mode)
            totalTime += time
            val mah = toMah(time, cur)
            totalEnergy += mah

            val h = time / 3600
            val m = (time / 60) % 60
            val s = time % 60

            return "%-7s %8d (%02d:%02d:%02d) %6.1f mAh".format(
                label, time, h, m, s, mah
            )
        }

        lines += line("Active:", Mode.Active, snapshot[0])
        lines += line("Eco:",    Mode.Eco,    snapshot[1])
        lines += line("BLE:",    Mode.BLE,    snapshot[2])
        lines += line("WiFi:",   Mode.WiFi,   snapshot[3])
        lines += line("Sleep:",  Mode.Sleep,  snapshot[4])

        val avgCurrent = if (totalTime > 0)
            totalEnergy * 3600.0 / totalTime
        else 0.0

        val h = totalTime / 3600
        val m = (totalTime / 60) % 60
        val s = totalTime % 60

        lines += ""
        lines += "Total:   %8d (%02d:%02d:%02d) %.1f mAh  I≈%.2f mA".format(
            totalTime, h, m, s, totalEnergy, avgCurrent
        )

        return lines
    }

    private fun resetReasonText(r: Int): String = when (r) {
        0 -> "UNKNOWN"      // ESP_RST_UNKNOWN
        1 -> "POWERON"      // ESP_RST_POWERON
        2 -> "EXT"          // ESP_RST_EXT
        3 -> "SW"           // ESP_RST_SW
        4 -> "PANIC"        // ESP_RST_PANIC
        5 -> "INT_WDT"      // ESP_RST_INT_WDT
        6 -> "TASK_WDT"     // ESP_RST_TASK_WDT
        7 -> "WDT"          // ESP_RST_WDT
        8 -> "DEEPSLEEP"    // ESP_RST_DEEPSLEEP
        9 -> "BROWNOUT"     // ESP_RST_BROWNOUT
        10 -> "SDIO"        // ESP_RST_SDIO
        else -> "UNKNOWN($r)"
    }
}
