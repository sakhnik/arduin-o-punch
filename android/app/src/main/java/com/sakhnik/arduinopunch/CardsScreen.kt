package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.sakhnik.arduinopunch.ui.theme.AppTheme

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CardScreen(viewModel: CardViewModel) {
    val cardNav = rememberNavController()
    val progress by viewModel.progress

    Scaffold(
        bottomBar = {
            CardBottomBar(cardNav)
        }
    ) { padding ->
        Column(
            modifier = Modifier.fillMaxSize(),
        ) {
            Spacer(Modifier.height(8.dp))
            LinearProgressIndicator(modifier = Modifier.fillMaxWidth(), progress = progress)
            Spacer(Modifier.height(8.dp))

            LaunchedEffect(cardNav) {
                cardNav.currentBackStackEntryFlow.collect { backStackEntry ->
                    val destinationRoute = backStackEntry.destination.route
                    viewModel.updateCurrentDestination(destinationRoute)
                }
            }

            NavHost(cardNav, startDestination = DST_FORMAT) {
                composable(DST_FORMAT) { FormatScreen(viewModel) }
                composable(DST_PUNCH) { PunchScreen(viewModel) }
                composable(DST_READ) { ReadScreen(viewModel) }
                composable(DST_RESET) { ResetScreen() }
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun CardScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        CardScreen(mockViewModel)
    }
}
