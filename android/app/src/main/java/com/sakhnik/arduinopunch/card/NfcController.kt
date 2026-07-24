package com.sakhnik.arduinopunch.card

import android.app.PendingIntent
import android.content.Intent
import android.media.MediaPlayer
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.MifareClassic
import android.os.Build
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.lifecycle.lifecycleScope
import com.sakhnik.arduinopunch.R
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class NfcController(
    private val activity: ComponentActivity,
    private val viewModel: CardViewModel,
    private val pendingIntent: PendingIntent
) {
    private var nfcAdapter: NfcAdapter = NfcAdapter.getDefaultAdapter(activity)
    private var okEffectPlayer: MediaPlayer = MediaPlayer.create(activity, R.raw.ok)
    private var failEffectPlayer: MediaPlayer = MediaPlayer.create(activity, R.raw.fail)

    fun close() {
        okEffectPlayer.release()
        failEffectPlayer.release()
    }

    fun enable() {
        nfcAdapter.enableForegroundDispatch(activity, pendingIntent, null, null)
    }

    fun disable() {
        nfcAdapter.disableForegroundDispatch(activity)
    }

    fun handleIntent(intent: Intent) {
        if (NfcAdapter.ACTION_TAG_DISCOVERED == intent.action) {
            // A new NFC tag was discovered
            val tag: Tag? =
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    intent.getParcelableExtra(NfcAdapter.EXTRA_TAG, Tag::class.java)
                } else {
                    @Suppress("DEPRECATION")
                    intent.getParcelableExtra(NfcAdapter.EXTRA_TAG)
                }

            // Handle MIFARE Classic 1K cards
            if (tag != null) {
                MifareClassic.get(tag).also { card ->
                    activity.lifecycleScope.launch(Dispatchers.IO) {
                        card.use {
                            it.connect()
                            handleMifare(it)
                        }
                    }
                }
            }
        }
    }

    private fun handleMifare(mifare: MifareClassic) {
        if (mifare.type != MifareClassic.TYPE_CLASSIC || mifare.size != MifareClassic.SIZE_1K) {
            activity.runOnUiThread {
                failEffectPlayer.start()
                Toast.makeText(activity,
                    activity.getString(R.string.only_1k_mifare_classic_cards_are_expected), Toast.LENGTH_LONG).show()
            }
            return
        }

        try {
            viewModel.handleCard(mifare)

            activity.runOnUiThread {
                okEffectPlayer.start()
            }
        } catch (ex: Exception) {
            Log.e("NfcController", "Card error $ex")
            activity.runOnUiThread {
                failEffectPlayer.start()
                Toast.makeText(activity, "${ex.message}", Toast.LENGTH_LONG).show()
            }
        } finally {
            viewModel.updateProgress(0f)
        }
    }
}
