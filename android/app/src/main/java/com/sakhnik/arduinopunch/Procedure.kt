package com.sakhnik.arduinopunch

typealias Progress = (Int, Int) -> Unit
typealias Step = (Progress) -> Unit

// Complex procedure consisting of smaller steps to simplify progress tracking
class Procedure {
    private var totalProgress: Int = 0
    private var progressIncrements: List<Int> = ArrayList()
    private var steps: List<Step> = ArrayList()

    companion object {
        val NO_PROGRESS: Progress = {_, _ -> }
    }

    // Add another step with expected progress increment
    fun add(progress: Int, step: Step) {
        totalProgress += progress
        progressIncrements += progress
        steps += step
    }

    // Run all the steps and track the progress
    fun run(progress: Progress): Unit {
        var currentProgress = 0

        val accumulativeProgress: Progress = { cur, _ ->
            progress(currentProgress + cur, totalProgress)
        }

        for (i in steps.indices) {
            steps[i](accumulativeProgress)
            currentProgress += progressIncrements[i]
        }
        progress(totalProgress, totalProgress)
    }
}
