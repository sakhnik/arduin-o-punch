package com.sakhnik.arduinopunch.card

import android.app.Application

class MockCardViewModel : CardViewModel(MockRepository(), Application()) {
    init {
        // Initialize with sample progress for preview
        updateProgress(0.75f)
    }
}
