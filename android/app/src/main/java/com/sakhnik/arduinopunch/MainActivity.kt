package com.sakhnik.arduinopunch

import android.app.PendingIntent
import android.content.Intent
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.sakhnik.arduinopunch.ui.theme.AppTheme
import kotlin.getValue

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

    private val pendingIntent by lazy {
        PendingIntent.getActivity(
            this,
            0,
            Intent(this, javaClass).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
            PendingIntent.FLAG_MUTABLE
        )
    }

    private lateinit var nfcController: NfcController

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        nfcController = NfcController(this, cardViewModel, pendingIntent)

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

        cardViewModel.toastMessage.observe(this) { message ->
            message?.let {
                Toast.makeText(this, it, Toast.LENGTH_LONG).show()
            }
        }

    }

    override fun onDestroy() {
        super.onDestroy()
        nfcController.close()
    }

    override fun onResume() {
        super.onResume()
        nfcController.enable()
    }

    override fun onPause() {
        super.onPause()
        nfcController.disable()
        //savePreferences(currentView)
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        nfcController.handleIntent(intent)
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
            AppTopBar(viewModel)
        },
        bottomBar = {
            CardBottomBar(navController)
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
                    composable(DST_PUNCH) { PunchScreen(viewModel) }
                    composable(DST_READ) { ReadScreen(viewModel) }
                    composable(DST_RESET) { ResetScreen() }
                }
            }
        }
    )
}

@Preview(showBackground = true)
@Composable
fun MainScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        MainScreen(mockViewModel)
    }
}
