package com.sakhnik.arduinopunch

import android.widget.Toast
import androidx.core.app.ComponentActivity
import okhttp3.Call
import okhttp3.Callback
import okhttp3.MediaType.Companion.toMediaTypeOrNull
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import okhttp3.Response
import org.json.JSONArray
import org.json.JSONObject
import java.io.IOException

class Uploader(private val activity: ComponentActivity) : Callback {
    companion object {
        const val START_HOUR: Int = 0

        const val CHECK_STATION: Int = 1
        const val START_STATION: Int = 10
        const val FINISH_STATION: Int = 255
    }

    fun upload(readOut: PunchCard.Info, url: String) {
        val jsonData = formatJson(readOut)
        val mediaType = "application/json; charset=utf-8".toMediaTypeOrNull()
        val requestBody = jsonData.toRequestBody(mediaType)

        val request = Request.Builder()
            .url(url)
            .post(requestBody)
            .build()

        val client = OkHttpClient()
        client.newCall(request).enqueue(this)
    }

    private fun formatJson(readOut: PunchCard.Info): String {
        if (readOut.punches.size < 2) {
            throw RuntimeException("Not enough punches (start, finish?)")
        }
        val request = JSONObject()
        request.put("stationNumber", 1)
        request.put("cardNumber", readOut.cardNumber)
        val punches = readOut.punches
        var startIdx = 0
        if (punches[startIdx].station == CHECK_STATION) {
            request.put("checkTime", encodeTime(punches[startIdx].timestamp))
            ++startIdx
        }
        if (punches[startIdx].station == START_STATION) {
            val startTime = encodeTime(punches[startIdx].timestamp)
            if (startIdx == 0) {
                request.put("checkTime", startTime)
            }
            request.put("startTime", startTime)
            ++startIdx
        }
        var finishIdx = punches.size - 1
        if (punches[finishIdx].station == FINISH_STATION) {
            request.put("finishTime", encodeTime(punches[finishIdx].timestamp))
            --finishIdx
        }
        val punchesDto = JSONArray()
        request.put("punches", punchesDto)
        for (idx in startIdx .. finishIdx) {
            val punch = JSONObject()
            punchesDto.put(punch)
            punch.put("cardNumber", readOut.cardNumber)
            punch.put("code", punches[idx].station)
            punch.put("time", encodeTime(punches[idx].timestamp))
        }
        return request.toString()
    }

    private fun encodeTime(timestamp: Long): Long {
        return timestamp + START_HOUR * 60 * 60
    }

    override fun onFailure(call: Call, e: IOException) {
        activity.runOnUiThread {
            Toast.makeText(activity, "Failed to upload: ${e.message}", Toast.LENGTH_LONG).show()
        }
    }

    override fun onResponse(call: Call, response: Response) {
        activity.runOnUiThread {
            Toast.makeText(activity, "Uploaded", Toast.LENGTH_SHORT).show()
        }
    }
}
