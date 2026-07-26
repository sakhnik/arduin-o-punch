package com.sakhnik.arduinopunch.card

import androidx.compose.foundation.layout.Spacer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.List
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Create
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.BottomAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.navigation.NavHostController
import com.sakhnik.arduinopunch.CardRoute
import com.sakhnik.arduinopunch.R

@Composable
fun CardBottomBar(
    cardNav: NavHostController
) {

    var selectedAction by remember { mutableStateOf<String?>(null) }

    BottomAppBar {

        NavButton(
            action = CardRoute.FORMAT,
            icon = Icons.Default.Create,
            description = stringResource(id = R.string.format_card),
            selectedAction = selectedAction,
            cardNav
        ) {
            selectedAction = it
        }
        Spacer(Modifier.weight(1f, true))

        NavButton(
            action = CardRoute.PUNCH,
            icon = Icons.Default.Add,
            description = stringResource(id = R.string.punch),
            selectedAction = selectedAction,
            cardNav
        ) {
            selectedAction = it
        }
        Spacer(Modifier.weight(1f, true))

        NavButton(
            action = CardRoute.READ,
            icon = Icons.AutoMirrored.Filled.List,
            description = stringResource(id = R.string.read_card),
            selectedAction = selectedAction,
            cardNav
        ) {
            selectedAction = it
        }
        Spacer(Modifier.weight(1f, true))

        NavButton(
            action = CardRoute.RESET,
            icon = Icons.Default.Delete,
            description = stringResource(id = R.string.reset_card),
            selectedAction = selectedAction,
            cardNav
        ) {
            selectedAction = it
        }
    }
}
