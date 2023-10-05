package com.sakhnik.arduinopunch

import org.junit.Assert.*

import org.junit.Test

class ProcedureTest {

    @Test
    fun run() {
        val stepBuilder = StringBuilder()

        val proc = Procedure()
        proc.add(3) { p ->
            stepBuilder.append("a")
            p(0, 3)
            p(1, 3)
            p(2, 3)
        }
        proc.add(2) { p ->
            stepBuilder.append("b")
            p(0, 2)
            p(1, 2)
        }

        val progressBuilder = StringBuilder()
        val progress: Progress = { n, d -> progressBuilder.append(' ').append(n).append('/').append(d) }
        proc.run(progress)
        assertEquals(" 0/5 1/5 2/5 3/5 4/5 5/5", progressBuilder.toString())
        assertEquals("ab", stepBuilder.toString())
    }
}
