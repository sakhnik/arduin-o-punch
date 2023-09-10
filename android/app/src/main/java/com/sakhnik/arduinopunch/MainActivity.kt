package com.sakhnik.arduinopunch

import android.app.PendingIntent
import android.content.Intent
import android.content.IntentFilter
import android.nfc.NdefMessage
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.MifareClassic
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import java.io.IOException

class MainActivity : AppCompatActivity() {

    private var nfcAdapter: NfcAdapter? = null
    private var pendingIntent: PendingIntent? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        nfcAdapter = NfcAdapter.getDefaultAdapter(this)
        if (nfcAdapter == null) {
            finish()
            return
        }

        // Create a PendingIntent that will be used to read NFC tags
        pendingIntent = PendingIntent.getActivity(
            this, 0,
            Intent(this, javaClass).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), PendingIntent.FLAG_MUTABLE
        )
    }

    override fun onResume() {
        super.onResume()
        nfcAdapter?.enableForegroundDispatch(this, pendingIntent, null, null)
    }

    override fun onPause() {
        super.onPause()
        nfcAdapter?.disableForegroundDispatch(this)
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        if (NfcAdapter.ACTION_TAG_DISCOVERED == intent.action) {
            // A new NFC tag was discovered
            val tag: Tag? = intent.getParcelableExtra(NfcAdapter.EXTRA_TAG)

            // Handle MIFARE Classic 1K cards
            if (tag != null) {
                val mifare = MifareClassic.get(tag)

                mifare?.use { mifareClassic ->
                    mifareClassic.connect()

                    try {
                        //val key = ByteArray(MifareClassic.KEY_NFC_FORUM) { 0xFF.toByte() }
                        if (mifare.authenticateSectorWithKeyA(0, MifareClassic.KEY_DEFAULT)) {
                            // Authentication successful
                            Toast.makeText(this, "Authentication Successful", Toast.LENGTH_SHORT).show()
                            // Assuming you want to read block 0, you can change this block number
                            val blockNumber = 0
                            val block = mifareClassic.readBlock(blockNumber)
                            Log.d(null, "block: $block")
                        } else {
                            // Authentication failed
                            Toast.makeText(this, "Authentication Failed", Toast.LENGTH_SHORT).show()
                        }

                        // Handle the data in the 'block' byte array
                        // Example: val dataAsString = String(block)

                    } catch (ex: IOException) {
                        Log.w(null, "IOException $ex")
                    }

                    mifareClassic.close()
                }
            }
        }
    }
}
