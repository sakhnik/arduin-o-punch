package com.sakhnik.arduinopunch

import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.navigation.NavController
import androidx.navigation.compose.currentBackStackEntryAsState
import kotlinx.coroutines.runBlocking

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppTopBar(
    rootNav: NavController,
    viewModel: CardViewModel
) {
    var expanded by remember { mutableStateOf(false) }
    val context = LocalContext.current

    val exportLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.CreateDocument("text/yaml")
    ) { uri ->
        uri?.let {
            val yamlString = runBlocking { viewModel.settingsToYaml() }
            context.contentResolver.openOutputStream(uri)?.use { it.write(yamlString.toByteArray()) }
            Toast.makeText(context,
                context.getString(R.string.settings_exported), Toast.LENGTH_SHORT).show()
        }
    }

    val importLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { uri ->
        uri?.let {
            context.contentResolver.openInputStream(uri)?.use { stream ->
                val yamlText = stream.bufferedReader().readText()
                runBlocking { viewModel.yamlToSettings(yamlText) }
            }
            Toast.makeText(context,
                context.getString(R.string.settings_imported), Toast.LENGTH_SHORT).show()
        }
    }


    TopAppBar(
        title = { Text(stringResource(id = R.string.app_name)) },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = Color.Blue,
            titleContentColor = Color.White
        ),
        actions = {
            IconButton(onClick = { expanded = true }) {
                Icon(Icons.Default.Menu, contentDescription = "Menu", tint = Color.White)
            }

            var showAboutDialog by remember { mutableStateOf(false) }
            val backStackEntry by rootNav.currentBackStackEntryAsState()
            val currentRoute = backStackEntry?.destination?.route

            DropdownMenu(
                expanded = expanded,
                onDismissRequest = { expanded = false }
            ) {
                val showingStation = currentRoute == "station"
                val destination = if (showingStation) "card" else "station"
                val title = if (showingStation) "Card operations" else "Station manager"

                DropdownMenuItem(
                    text = { Text(title) },
                    onClick = {
                        expanded = false
                        rootNav.navigate(destination) {
                            popUpTo(rootNav.graph.startDestinationId)
                            launchSingleTop = true
                            restoreState = true
                        }
                    }
                )

                DropdownMenuItem(
                    text = { Text(stringResource(R.string.export_settings)) },
                    onClick = {
                        expanded = false
                        exportLauncher.launch("aop.yaml")
                    }
                )
                DropdownMenuItem(
                    text = { Text(stringResource(R.string.import_settings)) },
                    onClick = {
                        expanded = false
                        importLauncher.launch(arrayOf("*/*"))
                    }
                )
                DropdownMenuItem(
                    text = { Text(stringResource(R.string.about_app)) }, // Add string to strings.xml
                    onClick = {
                        expanded = false
                        showAboutDialog = true
                    }
                )
            }

            if (showAboutDialog) {
                AlertDialog(
                    onDismissRequest = { showAboutDialog = false },
                    confirmButton = {
                        Button(onClick = { showAboutDialog = false }) {
                            Text("OK")
                        }
                    },
                    title = { Text(stringResource(R.string.about_app)) },
                    text = {

                        Text(
                            stringResource(R.string.version, BuildConfig.VERSION_NAME) +
                                stringResource(R.string.git_revision, BuildConfig.GIT_REVISION) +
                                stringResource(R.string.build_type, BuildConfig.BUILD_TYPE)
                        )
                    }
                )
            }
        }
    )
}
