package com.sakhnik.arduinopunch

import android.app.PendingIntent
import android.content.Intent
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import com.sakhnik.arduinopunch.card.CardViewModel
import com.sakhnik.arduinopunch.card.MockCardViewModel
import com.sakhnik.arduinopunch.card.NfcController
import com.sakhnik.arduinopunch.card.RepositoryImpl
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
                    AppScreen(cardViewModel)
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

@Preview(showBackground = true)
@Composable
fun MainActivityPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        AppScreen(mockViewModel)
    }
}
