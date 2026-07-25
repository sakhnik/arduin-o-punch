package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.sakhnik.arduinopunch.card.CardBottomBar
import com.sakhnik.arduinopunch.card.CardScreen
import com.sakhnik.arduinopunch.card.CardViewModel
import com.sakhnik.arduinopunch.card.MockCardViewModel
import com.sakhnik.arduinopunch.station.StationScreen
import com.sakhnik.arduinopunch.ui.theme.AppTheme

@Composable
fun AppScreen(cardViewModel: CardViewModel) {
    val rootNav = rememberNavController()
    val cardNav = rememberNavController()

    val currentRoot by rootNav.currentBackStackEntryAsState()

    Scaffold(
        topBar = {
            AppTopBar(rootNav, cardViewModel)
        },
        bottomBar = {
            if (currentRoot?.destination?.route == RootRoute.CARD) {
                CardBottomBar(cardNav)
            }
        }
    ) { padding ->
        NavHost(
            rootNav,
            startDestination = RootRoute.CARD,
            modifier = Modifier.padding(padding),
        ) {
            composable(RootRoute.CARD) {
                CardScreen(cardViewModel, cardNav)
            }

            composable(RootRoute.STATION) {
                StationScreen()
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun AppScreenPreview() {
    AppTheme {
        val mockViewModel = MockCardViewModel()
        AppScreen(mockViewModel)
    }
}
