# Arduin-o-punch — custom orienteering punching system built with Arduino

## Components

* MIFARE Classic 1k tags/cards for runners
* Base stations built with Arduino and MFRC 522 RFID reader/writer
* Configuration station: base station + serial interface + PC application
* Android application for card formatting, readout and punches upload:
    [guide](https://developer.android.com/guide/topics/connectivity/nfc/nfc).

## Operation modes

### Arduino station configuration

A high precision RTC clock DS3231 is used backed with CR2032 battery. A station
must be configured with the number, actual crypto key using built in CLI via UART.
Clock is also synchronized with CLI using a scripted application taking into account
communication latency.

The crypto key should ideally be changed on every station before competition, the clocks
are to be synchronized too.

### Signaling

Morse code is used by the base station for signaling. See `Buzzer.cpp` for the actual
list of messages. Most notable are the following ones:

- `OK` upon successful startup
- `RTC` upon startup if communication with RTC fails
- `KEY` upon startup if the default encryption key is used (needs changing)
- `.-` successful punch
- `..-` successful punch of two cards at once
- `...-` successful punch of three cards at once
- `..-.` (Morse `F`) when the card can't be punched because it's full

### Card cleaning and setup

* Authorize with the default key
* Update the key with the actual value for all the blocks
* Write the initial directory index
* Confirm operation successful (beep, flash)

### Card punching

In the MIFARE Classic 1k card, there's only 720 bytes available for punches.
Allocating 4 bytes per punch (1 byte station number + 3 bytes for timestamp in
seconds) allows storing at most 180 punches. The actual limit is lower because
one block should be used for directory information.

It turned out that writing to some cards isn't atomic, the block may be left reset
to all `F`, and the information lost. This prompted for a development of a more
resilient data structure.

The Arduino station records a punch with the following procedure:

- One sector is allocated for a write-ahead log (WAL)
- The log doubles information of the most recent punches (two blocks are used)
- Before punching, the WAL blocks are compared with each other to figure out
  the actual state
- If one of the WAL blocks is missing, it gets restored and written back from
  the healthy one
- When the WAL gets full with 4 punches, the next one triggers copying the punches
  to free storage block (1 copy).
- Add a new punch to the WAL
- Write WAL to the two WAL blocks
- Confirm successful punch

If punching is interrupted at any stage, the data remains consistent. Either
the punching succeeded and the runner got a confirmation, or no confirmation
for her to retry.

The same way Android punching should function for the data consistency.

### Card readout

* Read WAL to find out the actual number of punches
* Read all the punches
* Append the most recent punches from the WAL
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
