package com.sakhnik.arduinopunch.card

import androidx.compose.foundation.ExperimentalFoundationApi
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController

@OptIn(ExperimentalFoundationApi::class)
@Composable
fun NavButton(
    action: String,
    icon: ImageVector,
    description: String,
    selectedAction: String?,
    navController: NavHostController,
    setSelected: (String?) -> Unit
) {
    val isSelected = selectedAction == action
    val enabled = selectedAction == null || isSelected

    val iconColor = when {
        !enabled -> MaterialTheme.colorScheme.onSurface.copy(alpha = 0.4f)
        isSelected -> MaterialTheme.colorScheme.primary
        else -> MaterialTheme.colorScheme.onSurface
    }

    Icon(
        imageVector = icon,
        contentDescription = description,
        tint = iconColor,
        modifier = Modifier
            .combinedClickable(
                enabled = enabled,
                onClick = { navController.navigate(action) },
                onLongClick = {
                    navController.navigate(action)
                    toggleSelection(action, selectedAction, setSelected)
                }
            )
            .padding(8.dp)
    )
}

fun toggleSelection(action: String, current: String?, set: (String?) -> Unit) {
    set(if (current == action) null else action)
}
