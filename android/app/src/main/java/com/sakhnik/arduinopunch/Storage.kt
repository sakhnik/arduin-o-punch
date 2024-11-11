package com.sakhnik.arduinopunch

import android.content.Context

class Storage(private val context: Context) {

    private companion object {
        const val CURRENT_VIEW_KEY = "currentView"

        private object Prefs {
            const val NAME = "Prefs"

            const val KEY_STATION_ID = "stationId"
            const val KEY_UPLOAD = "upload"
            const val KEY_UPLOAD_URL = "uploadUrl"
        }
    }

    fun getStationId(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getString(Prefs.KEY_STATION_ID, context.getString(R.string._31)) ?: "31"
    }

    fun updateStationId(stationId: String) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_STATION_ID, stationId)
        editor.apply()
    }

    fun hasUpload(): Boolean {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getBoolean(Prefs.KEY_UPLOAD, false)
    }

    fun updateUpload(hasUpload: Boolean) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putBoolean(Prefs.KEY_UPLOAD, hasUpload)
        editor.apply()
    }

    fun getUploadUrl(): String {
        val prefs = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        val defaultUrl = context.getString(R.string.https_sakhnik_com_qr_o_punch_card)
        return prefs.getString(Prefs.KEY_UPLOAD_URL, defaultUrl) ?: defaultUrl
    }

    fun updateUploadUrl(uploadUrl: String) {
        val editor = context.getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_UPLOAD_URL, uploadUrl)
        editor.apply()
    }
}
