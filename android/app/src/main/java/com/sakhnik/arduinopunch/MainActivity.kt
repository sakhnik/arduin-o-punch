package com.sakhnik.arduinopunch

import android.app.AlertDialog
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
import android.widget.Button
import android.widget.CheckBox
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.ProgressBar
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import android.widget.ToggleButton
import androidx.activity.result.ActivityResultLauncher
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.navigation.NavigationView
import com.journeyapps.barcodescanner.ScanContract
import com.journeyapps.barcodescanner.ScanOptions
import com.sakhnik.arduinopunch.R.id.toggleUrlEditing
import java.io.IOException
import java.time.DateTimeException
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

    private val barcodeLauncher: ActivityResultLauncher<ScanOptions> =
        registerForActivityResult(ScanContract()) { result ->
            if (result.contents == null) {
                Toast.makeText(this@MainActivity, getString(R.string.cancelled), Toast.LENGTH_SHORT).show()
            } else {
                val regex = Regex("""SetStartNumber (\d+)""")
                val number = regex.find(result.contents)?.groupValues?.get(1)
                if (number != null) {
                    findViewById<EditText>(R.id.editCardId)?.setText(number)
                } else {
                    Toast.makeText(this@MainActivity,
                        getString(R.string.not_from_qr_o_punch), Toast.LENGTH_LONG).show()
                }
            }
        }

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

        if (savedInstanceState != null) {
            currentView = savedInstanceState.getInt(CURRENT_VIEW_KEY)
            if (currentView == 0) {
                currentView = R.layout.format_view
            }
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

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putInt(CURRENT_VIEW_KEY, currentView)
        super.onSaveInstanceState(outState)
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        currentView = savedInstanceState.getInt(CURRENT_VIEW_KEY)
        setActiveView(currentView)
        super.onRestoreInstanceState(savedInstanceState)
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

        when (viewId) {
            R.layout.format_view -> {
                val scanButton = findViewById<Button>(R.id.scanButton)
                if (!scanButton.hasOnClickListeners()) {
                    scanButton.setOnClickListener {
                        barcodeLauncher.launch(ScanOptions().setOrientationLocked(false))
                    }
                }
                val showOldKeys = findViewById<Button>(R.id.showOldKeys)
                showOldKeys.setOnClickListener {
                    showPreviousKeys()
                }
            }
            R.layout.read_runner_view -> {
                val toggleEditUrl = findViewById<ToggleButton>(toggleUrlEditing)
                toggleEditUrl.setOnCheckedChangeListener{_, isChecked ->
                    findViewById<EditText>(R.id.editUploadUrl).isEnabled = isChecked
                }
            }
        }
    }

    private companion object {
        const val CURRENT_VIEW_KEY = "currentView"

        private object Prefs {
            const val NAME = "Prefs"
            const val KEY_CARD_ID = "cardId"
            const val KEY_KEY = "key"
            const val KEY_KNOWN_KEYS = "knownKeys"
            const val KEY_STATION_ID = "stationId"
            const val KEY_UPLOAD = "upload"
            const val KEY_UPLOAD_URL = "uploadUrl"

            const val KNOWN_KEYS_HISTORY_SIZE = 4
        }

        private fun parseKey(hex: String): ByteArray {
            return hex.chunked(2) { it.toString().toInt(16).toByte() }.toByteArray()
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

    private fun getKeyHex(): String {
        val prefs = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        val keyRaw = prefs.getString(Prefs.KEY_KEY, null) ?: ""
        return keyRaw + "0".repeat(12 - keyRaw.length)
    }

    private fun getKey(): ByteArray {
        return parseKey(getKeyHex())
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
            val tag: Tag? =
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    intent.getParcelableExtra(NfcAdapter.EXTRA_TAG, Tag::class.java)
                } else {
                    @Suppress("DEPRECATION")
                    intent.getParcelableExtra(NfcAdapter.EXTRA_TAG)
                }

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
        return Duration.between(LocalTime.of(0, 0), LocalTime.now()).seconds
    }

    private fun handleMifare(mifare: MifareClassic?) {
        mifare?.use { mifareClassic ->
            mifareClassic.connect()
            if (mifareClassic.type != MifareClassic.TYPE_CLASSIC || mifareClassic.size != MifareClassic.SIZE_1K) {
                runOnUiThread {
                    failEffectPlayer.start()
                    Toast.makeText(this,
                        getString(R.string.only_1k_mifare_classic_cards_are_expected), Toast.LENGTH_LONG).show()
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
        card.reset(getKnownKeys(), this::setProgress)
    }

    private fun readRunner(mifareClassic: MifareClassic) {
        val key = getKey()
        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
        val readOut = card.readOut(this::setProgress)

        if (findViewById<CheckBox>(R.id.checkBoxUpload).isChecked) {
            val url = findViewById<EditText>(R.id.editUploadUrl).text.toString()
            Uploader(this).upload(readOut, url)
        }

        runOnUiThread {
            findViewById<TextView>(R.id.textViewCardNumber).text = getString(R.string.card_id_report, readOut.cardNumber)

            val tableLayout = findViewById<TableLayout>(R.id.tableLayout)
            for (i in tableLayout.childCount - 1 downTo 1) {
                tableLayout.removeViewAt(i)
            }

            // Format LocalTime to "HH:mm:ss"
            val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")

            for ((index, punch) in readOut.punches.withIndex()) {
                val tableRow = TableRow(this)
                val cell1 = TextView(this)
                val strNumber = "${index + 1}"
                cell1.text = strNumber
                tableRow.addView(cell1)
                val cell2 = TextView(this)
                cell2.text = punch.station.toString()
                tableRow.addView(cell2)
                val cell3 = TextView(this)
                try {
                val localTime = LocalTime.ofSecondOfDay(punch.timestamp)
                cell3.text = localTime.format(formatter)
                } catch (ex: DateTimeException) {
                    cell3.text = "???"
                }
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
        updateKnownKeys()

        val etId = findViewById<EditText>(R.id.editCardId)
        val id = etId.text.toString().toInt()
        if (id != 0) {
            val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
            card.format(id, getKnownKeys(), this::setProgress)
        }
    }

    private fun getKnownKeys(): List<ByteArray> {
        val prefs = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        return prefs.getString(Prefs.KEY_KNOWN_KEYS, "")!!.split(",").filter { it.isNotEmpty() }.map { parseKey(it) }
    }

    private fun showPreviousKeys() {
        val keyHex = getKeyHex()
        val prefs = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        var knownKeys = prefs.getString(Prefs.KEY_KNOWN_KEYS, "")!!
        // knownKeys contains the actual key too, skip it
        if (knownKeys.startsWith(keyHex)) {
            knownKeys = knownKeys.subSequence(keyHex.length, knownKeys.length).toString()
        }
        if (knownKeys.endsWith(",")) {
            knownKeys = knownKeys.subSequence(0, knownKeys.length - 1).toString()
        }
        if (knownKeys.length > 1) {
            val msg = knownKeys.replace(",", "\n")
            val builder = AlertDialog.Builder(this)
            builder.setMessage(msg)
                .setPositiveButton("OK") {_, _ -> }
                .show()
        }
    }

    // Take the current key and make sure it's in the list of known keys
    private fun updateKnownKeys() {
        val keyHex = getKeyHex()
        val prefs = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
        val knownKeys = prefs.getString(Prefs.KEY_KNOWN_KEYS, "")!!
        if (knownKeys.startsWith(keyHex)) {
            // It's already there
            return
        }
        val knownKeysList = knownKeys.split(",")
        val newList = ArrayList<String>()
        newList.add(keyHex)
        // Limit the history of the known keys
        if (knownKeysList.size > Prefs.KNOWN_KEYS_HISTORY_SIZE)
            newList.addAll(knownKeysList.subList(0, Prefs.KNOWN_KEYS_HISTORY_SIZE))
        else
            newList.addAll(knownKeysList)
        val editor = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
        editor.putString(Prefs.KEY_KNOWN_KEYS, newList.joinToString(","))
        editor.apply()
    }
}
