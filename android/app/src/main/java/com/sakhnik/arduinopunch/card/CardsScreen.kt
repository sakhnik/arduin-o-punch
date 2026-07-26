package com.sakhnik.arduinopunch.card

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.ProgressIndicatorDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.sakhnik.arduinopunch.CardRoute
import com.sakhnik.arduinopunch.ui.theme.AppTheme

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CardScreen(viewModel: CardViewModel, cardNav: NavHostController) {
    val progress by viewModel.progress

    Column(
        modifier = Modifier.fillMaxSize().imePadding(),
    ) {
        Spacer(Modifier.height(8.dp))
        LinearProgressIndicator(
            progress = { progress },
            modifier = Modifier.fillMaxWidth(),
            color = ProgressIndicatorDefaults.linearColor,
            trackColor = ProgressIndicatorDefaults.linearTrackColor,
            strokeCap = ProgressIndicatorDefaults.LinearStrokeCap,
        )
        Spacer(Modifier.height(8.dp))

        LaunchedEffect(cardNav) {
            cardNav.currentBackStackEntryFlow.collect { backStackEntry ->
                val destinationRoute = backStackEntry.destination.route
                viewModel.updateCurrentDestination(destinationRoute)
            }
        }

        NavHost(cardNav, startDestination = CardRoute.FORMAT) {
            composable(CardRoute.FORMAT) { FormatScreen(viewModel) }
            composable(CardRoute.PUNCH) { PunchScreen(viewModel) }
            composable(CardRoute.READ) { ReadScreen(viewModel) }
            composable(CardRoute.RESET) { ResetScreen() }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun CardScreenPreview() {
    AppTheme {
        val mockViewModel = remember { MockCardViewModel() }
        CardScreen(mockViewModel, rememberNavController())
    }
}
