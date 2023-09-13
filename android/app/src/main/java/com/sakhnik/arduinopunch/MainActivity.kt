package com.sakhnik.arduinopunch

import android.app.PendingIntent
import android.content.Intent
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.MifareClassic
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.EditText
import android.widget.ProgressBar
import android.widget.RadioButton
import android.widget.SeekBar
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.io.IOException
import java.time.Duration
import java.time.LocalTime

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

        val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) PendingIntent.FLAG_MUTABLE else 0
        // Create a PendingIntent that will be used to read NFC tags
        pendingIntent = PendingIntent.getActivity(
            this, 0,
            Intent(this, javaClass).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), flags
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
                GlobalScope.launch(Dispatchers.IO) {
                    handleMifare(mifare)
                }
            }
        }
    }

    private fun getTimestamp(): Long {
        return Duration.between(LocalTime.of(8, 0), LocalTime.now()).seconds
    }

    private fun handleMifare(mifare: MifareClassic?) {
        mifare?.use { mifareClassic ->
            mifareClassic.connect()
            if (mifareClassic.type != MifareClassic.TYPE_CLASSIC || mifareClassic.size != MifareClassic.SIZE_1K) {
                runOnUiThread {
                    Toast.makeText(this, "Only 1k MIFARE Classic cards are expected", Toast.LENGTH_LONG).show()
                }
                return
            }
            val editKey = findViewById<EditText>(R.id.editTextKey)
            val keyHex: String =
                editKey.text.append("0".repeat(12 - editKey.text.length)).toString()
            val key =
                keyHex.chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
            assert(key.size == 6)
            val punchCard = PunchCard(MifareImpl(mifareClassic), key)
            Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")

            val progress = findViewById<ProgressBar>(R.id.progressBar)
            val progressCb = { n: Int, d: Int ->
                progress.max = d
                progress.progress = n
            }
            try {
                if (findViewById<RadioButton>(R.id.radioButtonPrepare).isChecked) {
                    punchCard.prepare(getTimestamp(), progressCb)
                } else if (findViewById<RadioButton>(R.id.radioButtonReset).isChecked) {
                    punchCard.reset(progressCb)
                } else if (findViewById<RadioButton>(R.id.radioButtonPunch).isChecked) {
                    val station = findViewById<EditText>(R.id.editTextStation).text.toString().toInt()
                    punchCard.punch(Punch(station, getTimestamp()), progressCb)
                } else if (findViewById<RadioButton>(R.id.radioButtonReadOut).isChecked) {
                    val readOut = punchCard.readOut(progressCb)
                    runOnUiThread {
                        val tableLayout = findViewById<TableLayout>(R.id.tableLayout)
                        for (i in tableLayout.childCount - 1 downTo 1) {
                            tableLayout.removeViewAt(i)
                        }

                        for (punch in readOut) {
                            val tableRow = TableRow(this)
                            val cell1 = TextView(this)
                            cell1.text = punch.station.toString()
                            tableRow.addView(cell1)
                            val cell2 = TextView(this)
                            cell2.text = punch.timestamp.toString()
                            tableRow.addView(cell2)
                            tableLayout.addView(tableRow)
                        }
                    }
                }

            } catch (ex: IOException) {
                Log.e(null, "IO exception $ex")
                runOnUiThread {
                    Toast.makeText(this, "IOException $ex", Toast.LENGTH_LONG).show()
                }
            } catch (ex: RuntimeException) {
                Log.e(null, "Runtime exception $ex")
                runOnUiThread {
                    Toast.makeText(this, "RuntimeException $ex", Toast.LENGTH_LONG).show()
                }
            } finally {
                progressCb(0, 1)
            }
            mifareClassic.close()
        }
    }
}
