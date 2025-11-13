# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Service card to cycle through operation, Bluetooth and WiFi modes
- Bluetooth server for configuration
- TCP server on port 23 over WiFi for configuration
- Web app for configuration and OTA firmware update
- Signal last octet of IP address in Morse
- Android: long tap on the selector locks the view
- Android: allow selecting and copying punches to the clipboard
- Android: export/import settings
- Android: about dialog with version and git revision

### Changed

- Building firmware with platformio
- Ported Android app to Jetpack Compose
- Arduino: display the bytes of unknown command
- Android: remove the word "runner" from every page
- Arduino: soft reset MFRC522 periodically

### Removed

- Key display in `info`
- Navigation menu from Android app, using icon buttons instead

## [2.0.0] - 2024.04.29

### Added

- Continuous integration in GitHub workflow
- Web app to test the card layout (built from the actual code with emsdk)
- Automatic DST adjustment for EEST rules
- Tested programming and configuration in Windows, added instructions
- Added arguments to sync-clock.py to change serial port and to execute command script
- OpenSCAD model of the station and suggested a mount

### Changed

- **New data structure** resilient to information loss due to failed writes on some cards
- Use external EEPROM AT24C32 onboard DS3231 RTC
- Fix latency calculation in sync_clock.py, wait for the prompt explicitly
- Fix layout in Android app for smaller screens
- Keep current view when the screen is rotated
- Fix the capacity number
- Fix crash when garbage in the timestamp
- Allow entering key with lower case hex digits too
- Don't record a duplicate punch on the station

## [1.1.0] - 2024.02.24

### Added

- Try previous keys in Android to facilitate card preparation and resetting
- Lock upload URL to prevent accidental modification
- Successful startup confirmation (OK = --- -.-)
- Set both date and time with UNIX timestamp
- Record retaining period in days

### Changed

- Fix finish control uploading: shouldn't be uploaded, the number should be 900 in QuickEvent
- Fix start time by the *last* start control punch
- Prototype casing with DC input, switch and programmator connector
- Fix DNF when finish wasn't punched (finishTime = 0xEEEE)
- Use arduino clock for millisecond measurement of clock during synchronization

### Removed

- Batteries from the BOM (can use recycled ones from wapes)
- Time measurement from custom point, always from midnight
- Sleeping on timeout in favour of the power off button

## [1.0] - 2024-01-27

### Added

- First 10 prototypes with open casing

[Unreleased]: https://github.com/sakhnik/arduin-o-punch/compare/v2.0.0...HEAD
[2.0.0]: https://github.com/sakhnik/arduin-o-punch/compare/v1.1.0...v2.0.0
[1.1.0]: https://github.com/sakhnik/arduin-o-punch/compare/v1.0...v1.1.0
[1.0]: https://github.com/sakhnik/arduin-o-punch/releases/tag/v1.0
