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
import android.view.MenuItem
import android.widget.CheckBox
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.ProgressBar
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.navigation.NavigationView
import java.io.IOException
import java.time.Duration
import java.time.LocalTime
import java.time.format.DateTimeFormatter
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {

    private lateinit var nfcAdapter: NfcAdapter
    private lateinit var pendingIntent: PendingIntent
    private lateinit var okEffectPlayer: MediaPlayer
    private lateinit var failEffectPlayer: MediaPlayer
    private var currentView: Int = R.layout.format_view

    private val menuToLayout = mapOf(
        R.id.menu_item_format to R.layout.format_view,
        R.id.menu_item_clear to R.layout.clear_view,
        R.id.menu_item_punch to R.layout.punch_view,
        R.id.menu_item_read to R.layout.read_runner_view,
        R.id.menu_item_reset to R.layout.reset_view,
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        supportActionBar?.apply {
            setDisplayHomeAsUpEnabled(true)
            setHomeAsUpIndicator(R.drawable.ic_menu)
        }
        val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
        val toggle = ActionBarDrawerToggle(this, drawerLayout, R.string.navigation_drawer_open, R.string.navigation_drawer_close)
        drawerLayout.addDrawerListener(toggle)
        toggle.syncState()

        val navigationView = findViewById<NavigationView>(R.id.navigation_view)
        navigationView.setNavigationItemSelectedListener { menuItem ->
            if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
                drawerLayout.closeDrawer(GravityCompat.START)
            }
            savePreferences(currentView)
            val viewId = menuToLayout.getOrDefault(menuItem.itemId, currentView)
            currentView = viewId
            setActiveView(viewId)
            true
        }
        setActiveView(currentView)

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

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        val id = item.itemId
        if (id == android.R.id.home) {
            val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
            if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
                drawerLayout.closeDrawer(GravityCompat.START)
            } else {
                drawerLayout.openDrawer(GravityCompat.START)
            }
            return true
        }
        return super.onOptionsItemSelected(item)
    }

    private fun setActiveView(viewId: Int) {
        val container = findViewById<FrameLayout>(R.id.main_content)
        container.removeAllViews()
        val view = layoutInflater.inflate(viewId, container, false)
        container.addView(view)
        loadPreferences(viewId)
    }

    private companion object {
        private object Prefs {
            const val NAME = "Prefs"
            const val KEY_CARD_ID = "cardId"
            const val KEY_KEY = "key"
            const val KEY_STATION_ID = "stationId"
            const val KEY_UPLOAD = "upload"
            const val KEY_UPLOAD_URL = "uploadUrl"
        }
    }

    private fun savePreferences(viewId: Int) {
        val editor = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        when (viewId) {
            R.layout.format_view -> {
                editor.putString(Prefs.KEY_CARD_ID, findViewById<EditText>(R.id.editCardId).text.toString())
                editor.putString(Prefs.KEY_KEY, findViewById<EditText>(R.id.editKey).text.toString())
            }
            R.layout.punch_view -> {
                editor.putString(Prefs.KEY_STATION_ID, findViewById<EditText>(R.id.editStationId).text.toString())
            }
            R.layout.read_runner_view -> {
                editor.putBoolean(Prefs.KEY_UPLOAD, findViewById<CheckBox>(R.id.checkBoxUpload).isChecked)
                editor.putString(Prefs.KEY_UPLOAD_URL, findViewById<EditText>(R.id.editUploadUrl).text.toString())
            }
        }
        editor.apply()
    }

    private fun loadPreferences(viewId: Int) {
        val prefs = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        when (viewId) {
            R.layout.format_view -> {
                findViewById<EditText>(R.id.editCardId).setText(prefs.getString(Prefs.KEY_CARD_ID, getString(R.string._1)))
                findViewById<EditText>(R.id.editKey).setText(prefs.getString(Prefs.KEY_KEY, ""))
            }
            R.layout.punch_view -> {
                findViewById<EditText>(R.id.editStationId).setText(prefs.getString(Prefs.KEY_STATION_ID, getString(R.string._31)))
            }
            R.layout.read_runner_view -> {
                findViewById<CheckBox>(R.id.checkBoxUpload).isChecked = prefs.getBoolean(Prefs.KEY_UPLOAD, false)
                findViewById<EditText>(R.id.editUploadUrl).setText(prefs.getString(Prefs.KEY_UPLOAD_URL, getString(R.string.https_sakhnik_com_qr_o_punch_card)))
            }
        }
    }

    private fun getKey(): ByteArray {
        val prefs = getSharedPreferences("Prefs", Context.MODE_PRIVATE)
        val keyRaw = prefs.getString("key", null) ?: ""
        val keyHex = keyRaw + "0".repeat(12 - keyRaw.length)
        return keyHex.chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
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
        savePreferences(currentView)
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        if (NfcAdapter.ACTION_TAG_DISCOVERED == intent.action) {
            // A new NFC tag was discovered
            val tag: Tag? = intent.getParcelableExtra(NfcAdapter.EXTRA_TAG)

            // Handle MIFARE Classic 1K cards
            if (tag != null) {
                val mifare = MifareClassic.get(tag)
                thread {
                    handleMifare(mifare)
                }
            }
        }
    }

    private fun getTimestamp(): Long {
        return Duration.between(LocalTime.of(Uploader.START_HOUR, 0), LocalTime.now()).seconds
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
                    R.layout.clear_view -> clearRunner(mifareClassic)
                    R.layout.punch_view -> punchRunner(mifareClassic)
                    R.layout.read_runner_view -> readRunner(mifareClassic)
                    R.layout.reset_view -> resetRunner(mifareClassic)
                }

                runOnUiThread {
                    okEffectPlayer.start()
                }
            } catch (ex: IOException) {
                Log.e(null, "IO exception $ex")
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this, "${ex.message}", Toast.LENGTH_LONG).show()
                }
            } catch (ex: RuntimeException) {
                Log.e(null, "Runtime exception $ex")
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this, "${ex.message}", Toast.LENGTH_LONG).show()
                }
            } finally {
                findViewById<ProgressBar>(R.id.progressBar).progress = 0
            }
            mifareClassic.close()
        }
    }

    private fun setProgress(n: Int, d: Int) {
        runOnUiThread {
            val progress = findViewById<ProgressBar>(R.id.progressBar)
            progress.max = d
            progress.progress = n
        }
    }

    private fun resetRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
        card.reset(this::setProgress)
    }

    private fun readRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
        var readOut = card.readOut(this::setProgress)
        // Adjust timestamps from the arduino stations
        readOut = PunchCard.Info(readOut.cardNumber, readOut.punches)

        if (findViewById<CheckBox>(R.id.checkBoxUpload).isChecked) {
            val url = findViewById<EditText>(R.id.editUploadUrl).text.toString()
            Uploader(this).upload(readOut, url)
        }

        runOnUiThread {
            findViewById<TextView>(R.id.textViewCardNumber).text = readOut.cardNumber.toString()

            val tableLayout = findViewById<TableLayout>(R.id.tableLayout)
            for (i in tableLayout.childCount - 1 downTo 1) {
                tableLayout.removeViewAt(i)
            }

            // Format LocalTime to "HH:mm:ss"
            val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")

            for (punch in readOut.punches) {
                val tableRow = TableRow(this)
                val cell1 = TextView(this)
                cell1.text = punch.station.toString()
                tableRow.addView(cell1)
                val cell2 = TextView(this)
                cell2.text = punch.timestamp.toString()
                tableRow.addView(cell2)
                val cell3 = TextView(this)
                val localTime = LocalTime.ofSecondOfDay(punch.timestamp)
                cell3.text = localTime.format(formatter)
                tableRow.addView(cell3)
                tableLayout.addView(tableRow)
            }
        }
    }

    private fun punchRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        val station = findViewById<EditText>(R.id.editStationId).text.toString().toInt()
        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
        card.punch(Punch(station, getTimestamp()), this::setProgress)
    }

    private fun clearRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
        card.clear(this::setProgress)
    }

    private fun formatRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        Log.d(null, "Key is ${key.joinToString("") { "%02X".format(it) }}")

        val etId = findViewById<EditText>(R.id.editCardId)
        val id = etId.text.toString().toInt()
        if (id != 0) {
            val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
            card.prepareRunner(id, getTimestamp(), this::setProgress)
        }
    }
}
