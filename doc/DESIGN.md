# Arduin-o-punch — custom orienteering punching system built with Arduino

## Components

* MIFARE Classic 1k tags/cards for runners
* Base stations built with Arduino and MFRC 522 RFID reader/writer
* Android application for base station configuration, time calibration and
    start/finish base station

## Operation modes

### Arduino station configuration

Arduino CPU clock is used because there's no real time clock. The base station
must be configured with the number, actual crypto key.

* A service card is programmed with the Android app containing station number
    and marking current timestamp.
* A base station configures itself from the
    service card by assigning itself the next station number and writing the
    number and current timestamp to the card
* The service card is read again in the Android app to map base station's clock
    offset: Tﬆ -> t₁ + ½(t₂ - t₁)

This data should be distributed to perform punch times readout.

### Card cleaning and setup

* Authorize with the default key
* Update the key with the actual value for all the blocks
* Write the initial directory index (1 start punch)
* Confirm operation successful (beep, flash)

### Card punching

In the MIFARE Classic 1k card, there's only 720 bytes available for punches.
Allocating 4 bytes per punch (1 byte station number + 2 bytes for timestamp in
seconds + 1 reserved/service byte) allows storing at most 180 punches. The
actual limit is lower because one block should be used for directory
information.

The Arduino base station records a punch with the following procedure:

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

Android app should be used to authorize with the actual key and reset the card
to the pristine state, set default encryption keys in all blocks, reset access
bits.
The application could also track the configured cards to collect statistics
about card states.

## Data layout

TBD
