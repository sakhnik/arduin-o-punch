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
import androidx.activity.enableEdgeToEdge
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
import androidx.compose.foundation.layout.imePadding
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
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import java.io.IOException
import kotlin.concurrent.thread

class MainActivity : ComponentActivity() {
    private val cardViewModel: CardViewModel by viewModels {
        object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                if (modelClass.isAssignableFrom(CardViewModel::class.java)) {
                    @Suppress("UNCHECKED_CAST")
                    return CardViewModel(RepositoryImpl(applicationContext), application) as T
                }
                throw IllegalArgumentException("Unknown ViewModel class")
            }
        }
    }
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

        enableEdgeToEdge()

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

        cardViewModel.toastMessage.observe(this) { message ->
            message?.let {
                Toast.makeText(this, it, Toast.LENGTH_LONG).show()
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
            mifare.close()
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
        modifier = Modifier.imePadding(), // This modifier moves the BottomAppBar above the keyboard
        topBar = {
            TopAppBar(
                title = { Text(stringResource(id = R.string.app_name)) },
                colors = TopAppBarDefaults.topAppBarColors(
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
                    composable(DST_CLEAR) { ClearScreen() }
                    composable(DST_PUNCH) { PunchScreen(viewModel) }
                    composable(DST_READ) { ReadScreen(viewModel) }
                    composable(DST_RESET) { ResetScreen() }
                }
            }
        }
    )
}

class MockRepository : Repository {
    private val mockKeyFlow = MutableStateFlow("0".repeat(12))
    private val mockCardIdFlow = MutableStateFlow("1")

    override val keyHexFlow: Flow<String> = mockKeyFlow
    override val cardIdFlow: Flow<String> = mockCardIdFlow

    override suspend fun saveKeyHex(value: String) {
        mockKeyFlow.value = value
    }

    override fun getKey(): ByteArray {
        TODO("Not yet implemented")
    }

    override suspend fun saveCardId(value: String) {
        mockCardIdFlow.value = value
    }
}

class MockCardViewModel : CardViewModel(MockRepository(), Application()) {
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
//        }
//    }
//
//}
