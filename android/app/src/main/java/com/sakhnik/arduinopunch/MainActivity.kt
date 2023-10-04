package com.sakhnik.arduinopunch

import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.media.MediaPlayer
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.MifareClassic
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.ProgressBar
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.navigation.NavigationView
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.io.IOException
import java.time.Duration
import java.time.LocalTime

class MainActivity : AppCompatActivity() {

    private lateinit var nfcAdapter: NfcAdapter
    private lateinit var pendingIntent: PendingIntent
    private lateinit var okEffectPlayer: MediaPlayer
    private lateinit var failEffectPlayer: MediaPlayer
    private var currentView: Int = R.layout.format_view
    private var key: ByteArray? = null

    private val menuToLayout = mapOf(
        R.id.menu_item_format to R.layout.format_view,
        R.id.menu_item_clear to R.layout.clear_view,
        R.id.menu_item_punch to R.layout.punch_view,
        R.id.menu_item_read to R.layout.read_view,
        R.id.menu_item_reset to R.layout.reset_view,
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val navigationView = findViewById<NavigationView>(R.id.navigationView)

        navigationView.setNavigationItemSelectedListener { menuItem ->
            val viewId = menuToLayout.getOrDefault(menuItem.itemId, currentView)
            currentView = viewId
            setActiveView(viewId)
            true
        }

        okEffectPlayer = MediaPlayer.create(this, R.raw.ok)
        failEffectPlayer = MediaPlayer.create(this, R.raw.fail)

        nfcAdapter = NfcAdapter.getDefaultAdapter(this)

        val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) PendingIntent.FLAG_MUTABLE else 0
        // Create a PendingIntent that will be used to read NFC tags
        pendingIntent = PendingIntent.getActivity(
            this, 0,
            Intent(this, javaClass).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), flags
        )
    }

    private fun setActiveView(viewId: Int) {
        val container = findViewById<FrameLayout>(R.id.container)
        container.removeAllViews()
        val view = layoutInflater.inflate(viewId, container, false)
        container.addView(view)
        if (viewId == R.layout.format_view) {
            findViewById<EditText>(R.id.editTextKey).setText(getKeyHex())
        }
    }

    override fun onDestroy() {
        okEffectPlayer.release()
        failEffectPlayer.release()
        super.onDestroy()
    }

    override fun onResume() {
        super.onResume()
        nfcAdapter.enableForegroundDispatch(this, pendingIntent, null, null)
    }

    override fun onPause() {
        super.onPause()
        nfcAdapter.disableForegroundDispatch(this)
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
                    failEffectPlayer.start()
                    Toast.makeText(this, "Only 1k MIFARE Classic cards are expected", Toast.LENGTH_LONG).show()
                }
                return
            }

            val key = getKey()
            assert(key.size == 6)
            val punchCard = PunchCard(MifareImpl(mifareClassic), key)
            Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")

            val progress = findViewById<ProgressBar>(R.id.progressBar)
            val progressCb = { n: Int, d: Int ->
                progress.max = d
                progress.progress = n
            }
            try {
                when (currentView) {
                    R.layout.format_view -> {
                        val etId = findViewById<EditText>(R.id.editTextId)
                        val id = etId.text.toString().toInt()
                        punchCard.prepare(id, getTimestamp(), progressCb)
                    }
                    R.layout.punch_view -> {
                        val station = findViewById<EditText>(R.id.editTextStation).text.toString().toInt()
                        punchCard.punch(Punch(station, getTimestamp()), progressCb)
                    }
                    R.layout.read_view -> {
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
                    R.layout.reset_view -> {
                        punchCard.reset(progressCb)
                    }
                }

                runOnUiThread {
                    okEffectPlayer.start()
                }
            } catch (ex: IOException) {
                Log.e(null, "IO exception $ex")
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this, "IOException $ex", Toast.LENGTH_LONG).show()
                }
            } catch (ex: RuntimeException) {
                Log.e(null, "Runtime exception $ex")
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this, "RuntimeException $ex", Toast.LENGTH_LONG).show()
                }
            } finally {
                progressCb(0, 1)
            }
            mifareClassic.close()
        }
    }

    private fun getKey(): ByteArray {
        if (key != null) {
            return key as ByteArray
        }
        key = getKeyHex().chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
        return key as ByteArray
    }
    private fun getKeyHex(): String {
        val filename = "key.txt"

        val editKey = findViewById<EditText>(R.id.editTextKey)
        if (editKey != null) {
            val keyHex: String =
                editKey.text.append("0".repeat(12 - editKey.text.length)).toString()
            applicationContext.openFileOutput(filename, Context.MODE_PRIVATE).use {
                it.write(keyHex.toByteArray())
            }
            return keyHex
        }
        return applicationContext.openFileInput(filename).bufferedReader().readLine()
    }
}
