# How to assemble an Arduino station

[Українською](assembly.uk.md)

| Step                                                        | Reference picture                                              |
|-------------------------------------------------------------|----------------------------------------------------------------|
| Solder header pins to the face surface of Arduino Pro Mini  | <img src="images/01-arduino-header-pins.jpg" width="256"/>     |
| Remove power led from Arduino Pro Mini                      | <img src="images/07-cut-power-led-arduino.jpg" width="256"/>   |
| Solder header pins to the face surface of RC522             | <img src="images/02-rc522-header-pins.jpg" width="256"/>       |
| Remove D1 led from RC522                                    | <img src="images/08-cut-off-led-rc522.jpg" width="256"/>       |
| Remove USB output, switch from 18650 power source           | <img src="images/03-cut-off-usb.jpg" width="256"/>             |
| Solder header pins to a 3.3v output of 18650 power source   | <img src="images/04-power-header-pins.jpg" width="256"/>       |
| Solder indicator to the 18650 pads of 18650 power source    | <img src="images/05-charge-indicator.jpg" width="256"/>        |
| Glue the indicator to the side of the 18650 power source    | <img src="images/06-glue-charge-indicator.jpg" width="256"/>   |
| Cut the charging circuit of DS3231 to allow using non-rechargable CR2032 | <img src="images/09-cut-clock-charging.jpg" width="256"/>  |
| Remove power led from DS3231. Be careful not to break the trace from Bat+ to the pin 14 of the IC! | <img src="images/10-remove-clock-power-led.jpg" width="256"/>  |
| Prepare perfboard approximately 10x5 holes                  | <img src="images/11-prepare-perfboard.jpg" width="256"/>       |
| Prepare the required parts: led, active buzzer for 3.3v (5v will also work), 100Ω, 2 5x1 pin headers and a pin header 2x1  | <img src="images/12-prepare-parts.jpg" width="256"/>  |
| Solder the parts onto the perfboard (scheme below)          | <img src="images/13-solder-top.jpg" width="256"/><img src="images/13-solder-bottom.jpg" width="256"/>   |
| Insert backup battery CR2032 into the clock module          | <img src="images/14-insert-clock-battery.jpg" width="256"/>    |
| Connect wires between modules (scheme below)                | <img src="images/15-connect-wires.jpg" width="256"/>           |
| Upload the firmware and test (the procedure below)          |                                                                |
| Drill a hole for LED (I use the swiss knife awl)            | <img src="images/16-drill-hole.jpg" width="256"/>              |
| Glue components to the box                                  | <img src="images/17-glue-parts.jpg" width="256"/>              |

## Soldering scheme

* Two pin headers for negative and positive rails
* Led is placed close to the proximal end on the bottom side, the cathode connected to the negative rail
* Led's anode connected to the resistor 100Ω
* The other end of the resistor to one of the signal pins
* The other signal pin to the positive terminal of the buzzer
* The negative terminal of the buzzer connected to the negative rail

## Wiring scheme

* Clock DS3231: SDA, SCL → A4, A5
* NFC reader RC522: SS, SCK, MOSI, MISO → 10, 13, 11, 12
* Led and buzzer: 4, 5
* All GND to the ground rail of the perfboard with black wires
* All VCC 3.3v to the power rail of the perfboard with red wires
* Power source: GND → ground rail, 3.3v → power rail

<img src="images/wiring.jpg" width="512"/>

The switch the station off, disconnect a wire from the power source.

## Test procedure

* Synchronize the clock
* Configure station id, encryption key
* Punch a test card and verify the punch
* Switch the station off (disconnect from the power source)
* Wait a minute
* Switch the station on
* Check the clock stays set
* Punch a test card and verify the punch
