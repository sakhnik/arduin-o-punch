package com.sakhnik.arduinopunch

import android.app.PendingIntent
import android.content.Intent
import android.media.MediaPlayer
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.MifareClassic
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.Button
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
    private val storage = Storage(this)
    private val clockOffsets = ClockOffsets()

    private val menuToLayout = mapOf(
        R.id.menu_item_format to R.layout.format_view,
        R.id.menu_item_clear to R.layout.clear_view,
        R.id.menu_item_punch to R.layout.punch_view,
        R.id.menu_item_read_runner to R.layout.read_runner_view,
        R.id.menu_item_reset to R.layout.reset_view,
        R.id.menu_item_format_service to R.layout.format_service_view,
        R.id.menu_item_punch_service to R.layout.punch_service_view,
        R.id.menu_item_read_service to R.layout.read_service_view,
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
        if (viewId == R.layout.format_view || viewId == R.layout.format_service_view) {
            findViewById<EditText>(R.id.editTextKey).setText(storage.getKeyHex())
        } else if (viewId == R.layout.read_service_view) {
            showClockOffsets()
            findViewById<Button>(R.id.button_clear_clocks).setOnClickListener {
                clockOffsets.offsets.clear()
                showClockOffsets()
            }
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

            try {
                when (currentView) {
                    R.layout.format_view -> formatRunner(mifareClassic)
                    R.layout.punch_view -> punchRunner(mifareClassic)
                    R.layout.read_runner_view -> readRunner(mifareClassic)
                    R.layout.reset_view -> resetRunner(mifareClassic)
                    R.layout.format_service_view -> formatService(mifareClassic)
                    R.layout.punch_service_view -> punchService(mifareClassic)
                    R.layout.read_service_view -> readService(mifareClassic)
                }

                runOnUiThread {
                    okEffectPlayer.start()
                }
            } catch (ex: IOException) {
                Log.e(null, "IO exception $ex")
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this, "IOException ${ex.message}", Toast.LENGTH_LONG).show()
                }
            } catch (ex: RuntimeException) {
                Log.e(null, "Runtime exception $ex")
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this, "RuntimeException ${ex.message}", Toast.LENGTH_LONG).show()
                }
            } finally {
                findViewById<ProgressBar>(R.id.progressBar).progress = 0
            }
            mifareClassic.close()
        }
    }

    private fun setProgress(n: Int, d: Int) {
        val progress = findViewById<ProgressBar>(R.id.progressBar)
        progress.max = d
        progress.progress = n
    }

    private fun resetRunner(mifareClassic: MifareClassic) {
        val key = storage.getKey()
        val runnerCard = PunchCard(MifareImpl(mifareClassic), key)
        runnerCard.reset(this::setProgress)
    }

    private fun readRunner(mifareClassic: MifareClassic) {
        val key = storage.getKey()
        val runnerCard = PunchCard(MifareImpl(mifareClassic), key)
        val readOut = runnerCard.readOut(this::setProgress)
        runOnUiThread {
            findViewById<TextView>(R.id.textViewStation).text = readOut.station.toString()

            val tableLayout = findViewById<TableLayout>(R.id.tableLayout)
            for (i in tableLayout.childCount - 1 downTo 1) {
                tableLayout.removeViewAt(i)
            }

            for (punch in readOut.punches) {
                val offset = clockOffsets.offsets.getOrDefault(punch.station, 0)
                val timestamp = punch.timestamp + offset
                val tableRow = TableRow(this)
                val cell1 = TextView(this)
                cell1.text = punch.station.toString()
                tableRow.addView(cell1)
                val cell2 = TextView(this)
                cell2.text = timestamp.toString()
                tableRow.addView(cell2)
                tableLayout.addView(tableRow)
            }
        }
    }

    private fun readService(mifareClassic: MifareClassic) {
        val serviceCard = PunchCard(MifareImpl(mifareClassic), MifareClassic.KEY_DEFAULT)

        val readOut = serviceCard.readOut(this::setProgress)
        clockOffsets.process(readOut.punches)
        runOnUiThread {
            showClockOffsets()
        }
    }

    private fun showClockOffsets() {
        val tableLayout = findViewById<TableLayout>(R.id.tableLayout)
        for (i in tableLayout.childCount - 1 downTo 1) {
            tableLayout.removeViewAt(i)
        }

        for ((station, offset) in clockOffsets.offsets) {
            val tableRow = TableRow(this)
            val cell1 = TextView(this)
            cell1.text = station.toString()
            tableRow.addView(cell1)
            val cell2 = TextView(this)
            cell2.text = offset.toString()
            tableRow.addView(cell2)
            tableLayout.addView(tableRow)
        }
    }

    private fun punchRunner(mifareClassic: MifareClassic) {
        val key = storage.getKey()
        val station = findViewById<EditText>(R.id.editTextStation).text.toString().toInt()
        if (station != 0) {
            val runnerCard = PunchCard(MifareImpl(mifareClassic), key)
            runnerCard.punch(Punch(station, getTimestamp()), this::setProgress)
        }
    }

    private fun punchService(mifareClassic: MifareClassic) {
        val serviceCard = PunchCard(MifareImpl(mifareClassic), MifareClassic.KEY_DEFAULT)
        serviceCard.punch(Punch(0, getTimestamp()), this::setProgress)
    }

    private fun formatRunner(mifareClassic: MifareClassic) {
        val key = storage.getKey()
        Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")

        val etId = findViewById<EditText>(R.id.editTextId)
        val id = etId.text.toString().toInt()
        if (id != 0) {
            val runnerCard = PunchCard(MifareImpl(mifareClassic), key)
            runnerCard.prepareRunner(id, getTimestamp(), this::setProgress)
        }
    }

    private fun formatService(mifareClassic: MifareClassic) {
        val key = storage.getKey()
        Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")

        val serviceCard = PunchCard(MifareImpl(mifareClassic), MifareClassic.KEY_DEFAULT)
        serviceCard.prepareService(key, getTimestamp(), this::setProgress)
    }
}
