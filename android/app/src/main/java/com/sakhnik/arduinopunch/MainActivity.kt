package com.sakhnik.arduinopunch

import android.app.Application
import android.app.PendingIntent
import android.content.Intent
import android.media.MediaPlayer
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.MifareClassic
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.sakhnik.arduinopunch.ui.theme.AppTheme
import androidx.activity.viewModels
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Clear
import androidx.compose.material.icons.filled.Create
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.BottomAppBar
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.Scaffold
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import java.io.IOException
import kotlin.concurrent.thread

class MainActivity : ComponentActivity() {
    private val cardViewModel: CardViewModel by viewModels()
    private lateinit var nfcAdapter: NfcAdapter
    private lateinit var pendingIntent: PendingIntent
    private lateinit var okEffectPlayer: MediaPlayer
    private lateinit var failEffectPlayer: MediaPlayer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        nfcAdapter = NfcAdapter.getDefaultAdapter(this)
        val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) PendingIntent.FLAG_MUTABLE else 0
        // Create a PendingIntent that will be used to read NFC tags
        pendingIntent = PendingIntent.getActivity(
            this, 0,
            Intent(this, javaClass).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), flags
        )

        setContent {
            AppTheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    MainScreen(cardViewModel)
                }
            }
        }

        okEffectPlayer = MediaPlayer.create(this, R.raw.ok)
        failEffectPlayer = MediaPlayer.create(this, R.raw.fail)
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
        //savePreferences(currentView)
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
                MifareClassic.get(tag)?.use {
                    thread {
                        handleMifare(it)
                    }
                }
            }
        }
    }

    private fun handleMifare(mifare: MifareClassic) {
        mifare.connect()
        if (mifare.type != MifareClassic.TYPE_CLASSIC || mifare.size != MifareClassic.SIZE_1K) {
            runOnUiThread {
                failEffectPlayer.start()
                Toast.makeText(this,
                    getString(R.string.only_1k_mifare_classic_cards_are_expected), Toast.LENGTH_LONG).show()
            }
            return
        }

        try {
            cardViewModel.handleCard(mifare)

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
            cardViewModel.updateProgress(0f)
        }
        mifare.close()
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainScreen(viewModel: CardViewModel) {
    val progress by viewModel.progress
    val navController = rememberNavController()

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(id = R.string.app_name)) },
                colors = TopAppBarDefaults.smallTopAppBarColors(
                    containerColor = Color.Blue,
                    titleContentColor = Color.White
                )
            )
        },
        bottomBar = {
            BottomAppBar(
                content = {
                    IconButton(onClick = { navController.navigate(DST_FORMAT) }) {
                        Icon(Icons.Default.Create, contentDescription = stringResource(id = R.string.format_card))
                    }
                    Spacer(Modifier.weight(1f, true))
                    IconButton(onClick = { navController.navigate(DST_CLEAR) }) {
                        Icon(Icons.Default.Clear, contentDescription = stringResource(id = R.string.clear_punches))
                    }
                    Spacer(Modifier.weight(1f, true))
                    IconButton(onClick = { navController.navigate(DST_PUNCH) }) {
                        Icon(Icons.Default.Check, contentDescription = stringResource(id = R.string.punch))
                    }
                    Spacer(Modifier.weight(1f, true))
                    IconButton(onClick = { navController.navigate(DST_READ) }) {
                        Icon(Icons.Default.Refresh, contentDescription = stringResource(id = R.string.read_card))
                    }
                    Spacer(Modifier.weight(1f, true))
                    IconButton(onClick = { navController.navigate(DST_RESET) }) {
                        Icon(Icons.Default.Delete, contentDescription = stringResource(id = R.string.reset_card))
                    }
                }
            )
        },
        content = { paddingValues ->
            // Main content area, respecting padding from Scaffold components
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(paddingValues)
            ) {
                Spacer(Modifier.height(8.dp))
                LinearProgressIndicator(modifier = Modifier.fillMaxWidth(), progress = progress)
                Spacer(Modifier.height(8.dp))

                LaunchedEffect(navController) {
                    navController.currentBackStackEntryFlow.collect { backStackEntry ->
                        val destinationRoute = backStackEntry.destination.route
                        viewModel.updateCurrentDestination(destinationRoute)
                    }
                }

                NavHost(navController, startDestination = DST_FORMAT) {
                    composable(DST_FORMAT) { FormatScreen(viewModel) }
                    composable(DST_PUNCH) { PunchScreen() }
                    composable(DST_CLEAR) { ClearScreen() }
                    composable(DST_READ) { ReadScreen() }
                    composable(DST_RESET) { ResetScreen() }
                }
            }
        }
    )
}

class MockCardViewModel : CardViewModel(Application()) {
    init {
        // Initialize with sample progress for preview
        updateProgress(0.75f)
    }
}

@Preview(showBackground = true)
@Composable
fun MainScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        MainScreen(mockViewModel)
    }
}

//class MainActivity : AppCompatActivity() {
//
//    private var currentView: Int = R.layout.format_view
//
//    private val menuToLayout = mapOf(
//        R.id.menu_item_format to R.layout.format_view,
//        R.id.menu_item_clear to R.layout.clear_view,
//        R.id.menu_item_punch to R.layout.punch_view,
//        R.id.menu_item_read to R.layout.read_runner_view,
//        R.id.menu_item_reset to R.layout.reset_view,
//    )
//
//    private val barcodeLauncher: ActivityResultLauncher<ScanOptions> =
//        registerForActivityResult(ScanContract()) { result ->
//            if (result.contents == null) {
//                Toast.makeText(this@MainActivity, getString(R.string.cancelled), Toast.LENGTH_SHORT).show()
//            } else {
//                val regex = Regex("""SetStartNumber (\d+)""")
//                val number = regex.find(result.contents)?.groupValues?.get(1)
//                if (number != null) {
//                    findViewById<EditText>(R.id.editCardId)?.setText(number)
//                } else {
//                    Toast.makeText(this@MainActivity,
//                        getString(R.string.not_from_qr_o_punch), Toast.LENGTH_LONG).show()
//                }
//            }
//        }
//
//    override fun onCreate(savedInstanceState: Bundle?) {
//        super.onCreate(savedInstanceState)
//        //setContentView(R.layout.activity_main)
//        setContent {
//            MainScreenWithDrawer()
//        }
//
//        //supportActionBar?.apply {
//        //    setDisplayHomeAsUpEnabled(true)
//        //    setHomeAsUpIndicator(R.drawable.ic_menu)
//        //}
//        //val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
//        //val toggle = ActionBarDrawerToggle(this, drawerLayout, R.string.navigation_drawer_open, R.string.navigation_drawer_close)
//        //drawerLayout.addDrawerListener(toggle)
//        //toggle.syncState()
//
//        //val navigationView = findViewById<NavigationView>(R.id.navigation_view)
//        //navigationView.setNavigationItemSelectedListener { menuItem ->
//        //    if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
//        //        drawerLayout.closeDrawer(GravityCompat.START)
//        //    }
//        //    savePreferences(currentView)
//        //    val viewId = menuToLayout.getOrDefault(menuItem.itemId, currentView)
//        //    currentView = viewId
//        //    setActiveView(viewId)
//        //    true
//        //}
//
//        //if (savedInstanceState != null) {
//        //    currentView = savedInstanceState.getInt(CURRENT_VIEW_KEY)
//        //    if (currentView == 0) {
//        //        currentView = R.layout.format_view
//        //    }
//        //}
//        //setActiveView(currentView)
//
//
//    }
//
//    override fun onSaveInstanceState(outState: Bundle) {
//        outState.putInt(CURRENT_VIEW_KEY, currentView)
//        super.onSaveInstanceState(outState)
//    }
//
//    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
//        currentView = savedInstanceState.getInt(CURRENT_VIEW_KEY)
//        setActiveView(currentView)
//        super.onRestoreInstanceState(savedInstanceState)
//    }
//
//    override fun onOptionsItemSelected(item: MenuItem): Boolean {
//        val id = item.itemId
//        if (id == android.R.id.home) {
//            val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
//            if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
//                drawerLayout.closeDrawer(GravityCompat.START)
//            } else {
//                drawerLayout.openDrawer(GravityCompat.START)
//            }
//            return true
//        }
//        return super.onOptionsItemSelected(item)
//    }
//
//    private fun setActiveView(viewId: Int) {
//        val container = findViewById<FrameLayout>(R.id.main_content)
//        container.removeAllViews()
//        val view = layoutInflater.inflate(viewId, container, false)
//        container.addView(view)
//        loadPreferences(viewId)
//
//        when (viewId) {
//            R.layout.format_view -> {
//                val scanButton = findViewById<Button>(R.id.scanButton)
//                if (!scanButton.hasOnClickListeners()) {
//                    scanButton.setOnClickListener {
//                        barcodeLauncher.launch(ScanOptions().setOrientationLocked(false))
//                    }
//                }
//            }
//            R.layout.read_runner_view -> {
//                val toggleEditUrl = findViewById<ToggleButton>(toggleUrlEditing)
//                toggleEditUrl.setOnCheckedChangeListener{_, isChecked ->
//                    findViewById<EditText>(R.id.editUploadUrl).isEnabled = isChecked
//                }
//            }
//        }
//    }
//
//    private fun savePreferences(viewId: Int) {
//        val editor = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE).edit()
//        when (viewId) {
//            R.layout.format_view -> {
//                editor.putString(Prefs.KEY_CARD_ID, findViewById<EditText>(R.id.editCardId).text.toString())
//                editor.putString(Prefs.KEY_KEY, findViewById<EditText>(R.id.editKey).text.toString())
//            }
//            R.layout.punch_view -> {
//                editor.putString(Prefs.KEY_STATION_ID, findViewById<EditText>(R.id.editStationId).text.toString())
//            }
//            R.layout.read_runner_view -> {
//                editor.putBoolean(Prefs.KEY_UPLOAD, findViewById<CheckBox>(R.id.checkBoxUpload).isChecked)
//                editor.putString(Prefs.KEY_UPLOAD_URL, findViewById<EditText>(R.id.editUploadUrl).text.toString())
//            }
//        }
//        editor.apply()
//    }
//
//    private fun loadPreferences(viewId: Int) {
//        val prefs = getSharedPreferences(Prefs.NAME, Context.MODE_PRIVATE)
//        when (viewId) {
//            R.layout.format_view -> {
//                findViewById<EditText>(R.id.editCardId).setText(prefs.getString(Prefs.KEY_CARD_ID, getString(R.string._1)))
//                findViewById<EditText>(R.id.editKey).setText(prefs.getString(Prefs.KEY_KEY, ""))
//            }
//            R.layout.punch_view -> {
//                findViewById<EditText>(R.id.editStationId).setText(prefs.getString(Prefs.KEY_STATION_ID, getString(R.string._31)))
//            }
//            R.layout.read_runner_view -> {
//                findViewById<CheckBox>(R.id.checkBoxUpload).isChecked = prefs.getBoolean(Prefs.KEY_UPLOAD, false)
//                findViewById<EditText>(R.id.editUploadUrl).setText(prefs.getString(Prefs.KEY_UPLOAD_URL, getString(R.string.https_sakhnik_com_qr_o_punch_card)))
//            }
//        }
//    }
//
//    private fun getTimestamp(): Long {
//        return Duration.between(LocalTime.of(0, 0), LocalTime.now()).seconds
//    }
//
//    private fun resetRunner(mifareClassic: MifareClassic) {
//        val key = getKey()
//        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
//        card.reset(getKnownKeys(), this::setProgress)
//    }
//
//    private fun readRunner(mifareClassic: MifareClassic) {
//        val key = getKey()
//        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
//        val readOut = card.readOut(this::setProgress)
//
//        if (findViewById<CheckBox>(R.id.checkBoxUpload).isChecked) {
//            val url = findViewById<EditText>(R.id.editUploadUrl).text.toString()
//            Uploader(this).upload(readOut, url)
//        }
//
//        runOnUiThread {
//            findViewById<TextView>(R.id.textViewCardNumber).text = getString(R.string.card_id_report, readOut.cardNumber)
//
//            val tableLayout = findViewById<TableLayout>(R.id.tableLayout)
//            for (i in tableLayout.childCount - 1 downTo 1) {
//                tableLayout.removeViewAt(i)
//            }
//
//            // Format LocalTime to "HH:mm:ss"
//            val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")
//
//            for ((index, punch) in readOut.punches.withIndex()) {
//                val tableRow = TableRow(this)
//                val cell1 = TextView(this)
//                val strNumber = "${index + 1}"
//                cell1.text = strNumber
//                tableRow.addView(cell1)
//                val cell2 = TextView(this)
//                cell2.text = punch.station.toString()
//                tableRow.addView(cell2)
//                val cell3 = TextView(this)
//                try {
//                val localTime = LocalTime.ofSecondOfDay(punch.timestamp)
//                cell3.text = localTime.format(formatter)
//                } catch (ex: DateTimeException) {
//                    cell3.text = "???"
//                }
//                tableRow.addView(cell3)
//                tableLayout.addView(tableRow)
//            }
//        }
//    }
//
//    private fun punchRunner(mifareClassic: MifareClassic) {
//        val key = getKey()
//        val station = findViewById<EditText>(R.id.editStationId).text.toString().toInt()
//        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
//        card.punch(Punch(station, getTimestamp()), this::setProgress)
//    }
//
//    private fun clearRunner(mifareClassic: MifareClassic) {
//        val key = getKey()
//        val card = PunchCard(MifareImpl(mifareClassic), key, applicationContext)
//        card.clear(this::setProgress)
//    }
//
//
//}
