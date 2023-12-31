# Arduin-o-punch — custom orienteering punching system built with Arduino

## Components

* MIFARE Classic 1k tags/cards for runners
* Base stations built with Arduino and MFRC 522 RFID reader/writer
* Configuration station: base station + serial interface + PC application
* Android application for card formatting, readout and punches upload:
    [guide](https://developer.android.com/guide/topics/connectivity/nfc/nfc).

## Operation modes

### Arduino station configuration

A high precision RTC clock DS323a is used backed with CR2032 battery. A station
must be configured with the number, actual crypto key using built in CLI via UART.
Clock is also synchronized with CLI using a scripted application taking into account
communication latency.

The crypto key should ideally be changed on every station before competition, the clocks
are to be synchronized too.

### Signaling

Morse code is used by the base station for signaling. See `Buzzer.cpp` for the actual
list of messages. Most notable are the following ones:

- `RTC` upon startup if communication with RTC fails
- `KEY` upon startup if the default encryption key is used (needs changing)
- `.-` successful punch
- `..-` successful punch of two cards at once
- `...-` successful punch of three cards at once
- `..-.` (Morse `F`) when the card can't be punched because it's full

### Card cleaning and setup

* Authorize with the default key
* Update the key with the actual value for all the blocks
* Write the initial directory index (1 start punch)
* Confirm operation successful (beep, flash)

### Card punching

In the MIFARE Classic 1k card, there's only 720 bytes available for punches.
Allocating 4 bytes per punch (1 byte station number + 3 bytes for timestamp in
seconds) allows storing at most 180 punches. The actual limit is lower because
one block should be used for directory information.

The Arduino station records a punch with the following procedure:

* Read the directory to get the block and offset to write to
* Read the previous punch (station, time)
* If station corresponds to the actual station, just confirm, nothing to write
    (beep, flash)
* Punch the card (station number, timestamp)
* Update the directory (index + 1)
* Confirm the successful punching (beep, flash)

The same way Android punching should function for the data consistency.

### Card readout

* Read the directory to find out the number of punches
* Read all the punches
* Calculate timestamps taking the map from timestamp calibration (see Arduino
    configuration)
* Prepare JSON in the qr-o-punch format, upload to Quick Event
* Confirm successful upload (beep, flash)

### Card reset

Configurator should be used to authorize with the actual key and reset the card
to the pristine state, set default encryption keys in all blocks, reset access
bits.
The application could also track the configured cards to collect statistics
about card states.

## Data layout

See `src/PunchCard.h` for the directory layout, and `src/Punch.h` for the punch data.
