package com.sakhnik.arduinopunch.card

import android.media.MediaPlayer
import android.nfc.NfcAdapter
import android.nfc.tech.MifareClassic
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.lifecycle.lifecycleScope
import com.sakhnik.arduinopunch.R
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import timber.log.Timber

class NfcController(
    private val activity: ComponentActivity,
    private val viewModel: CardViewModel,
) {
    private var nfcAdapter: NfcAdapter = NfcAdapter.getDefaultAdapter(activity)
    private var okEffectPlayer: MediaPlayer = MediaPlayer.create(activity, R.raw.ok)
    private var failEffectPlayer: MediaPlayer = MediaPlayer.create(activity, R.raw.fail)

    fun close() {
        okEffectPlayer.release()
        failEffectPlayer.release()
    }

    fun enable() {
        nfcAdapter.enableReaderMode(
            activity,
            { tag ->
                MifareClassic.get(tag).also { card ->
                    activity.lifecycleScope.launch(Dispatchers.IO) {
                        card.use {
                            it.connect()
                            handleMifare(it)
                        }
                    }
                }
            },
            NfcAdapter.FLAG_READER_NFC_A or
                NfcAdapter.FLAG_READER_NFC_B or
                NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,
            null
        )
    }

    fun disable() {
        nfcAdapter.disableReaderMode(activity)
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
            Timber.e("Card error $ex")
            activity.runOnUiThread {
                failEffectPlayer.start()
                Toast.makeText(activity, "${ex.message}", Toast.LENGTH_LONG).show()
            }
        } finally {
            viewModel.updateProgress(0f)
        }
    }
}
