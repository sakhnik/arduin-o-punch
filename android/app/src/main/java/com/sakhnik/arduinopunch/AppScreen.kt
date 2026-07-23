package com.sakhnik.arduinopunch

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController

@Composable
fun AppScreen(cardViewModel: CardViewModel) {
    val navController = rememberNavController()

    Scaffold(
        topBar = {
            AppTopBar(cardViewModel)
        },
        bottomBar = {
            when (navController.currentBackStackEntryAsState().value?.destination?.route) {
                "cards" -> CardBottomBar(navController)
                else -> {}
            }
        }
    ) { padding ->
        NavHost(
            navController,
            startDestination = "cards",
            modifier = Modifier.padding(padding)
        ) {
            composable("cards") {
                MainScreen(cardViewModel)
            }

            composable("devices") {
                DeviceScreen()
            }
        }
    }
}
