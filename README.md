# IRremote Arduino Library

[![Build Status](https://travis-ci.org/z3t0/Arduino-IRremote.svg?branch=master)](https://travis-ci.org/z3t0/Arduino-IRremote)

[![Join the chat at https://gitter.im/z3t0/Arduino-IRremote](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/z3t0/Arduino-IRremote?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

This library enables you to send and receive using infra-red signals on an Arduino.

Tutorials and more information will be made available on [the official homepage](http://z3t0.github.io/Arduino-IRremote/).

## Version - 2.4.0b

## Installation
1. Navigate to the [Releases](https://github.com/z3t0/Arduino-IRremote/releases) page.
2. Download the latest release.
3. Extract the zip file
4. Move the "IRremote" folder that has been extracted to your libraries directory.
5. Make sure to delete Arduino_Root/libraries/RobotIRremote. Where Arduino_Root refers to the install directory of Arduino. The library RobotIRremote has similar definitions to IRremote and causes errors.


## FAQ
- IR does not work right when I use Neopixels (aka WS2811/WS2812/WS2812B)  
Whether you use the Adafruit Neopixel lib, or FastLED, interrupts get disabled on many lower end CPUs like the basic arduinos. In turn, this stops the IR interrupt handler from running when it needs to. There are some solutions to this on some processors, [see this page from Marc MERLIN](http://marc.merlins.org/perso/arduino/post_2017-04-03_Arduino-328P-Uno-Teensy3_1-ESP8266-ESP32-IR-and-Neopixels.html)


## Supported Boards
- Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano etc.
- Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / Teensy-LC; Credits: @PaulStoffregen (Teensy Team)
- Sanguino
- ATmega8, 48, 88, 168, 328
- ATmega8535, 16, 32, 164, 324, 644, 1284,
- ATmega64, 128
- ATtiny 84 / 85
- ESP32 (receive only)
- ESP8266 is supported in a fork based on an old codebase that isn't as recent, but it works reasonably well given that perfectly timed sub millisecond interrupts are different on that chip. See https://github.com/markszabo/IRremoteESP8266

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.

### Hardware specifications

| Board/CPU                                                                | Send Pin            | Timers            |
|--------------------------------------------------------------------------|---------------------|-------------------|
| [ATtiny84](https://github.com/SpenceKonde/ATTinyCore)                    | **6**               | **1**             |
| [ATtiny85](https://github.com/SpenceKonde/ATTinyCore)                    | **1**               | **TINY0**         |
| [ATmega8](https://github.com/MCUdude/MiniCore)                           | **9**               | **1**             |
| Atmega32u4                                                               | 5, 9, **13**        | 1, 3, **4**       |
| [ATmega48, ATmega88, ATmega168, ATmega328](https://github.com/MCUdude/MiniCore) | **3**, 9     | 1, **2**          |
| [ATmega1284](https://github.com/MCUdude/MightyCore)                      | 13, 14, 6           | 1, **2**, 3       |
| [ATmega164, ATmega324, ATmega644](https://github.com/MCUdude/MightyCore) | 13, **14**          | 1, **2**          |
| [ATmega8535 ATmega16, ATmega32](https://github.com/MCUdude/MightyCore)   | **13**              | **1**             |
| [ATmega64, ATmega128](https://github.com/MCUdude/MegaCore)               | **13**              | **1**             |
| ATmega1280, ATmega2560                                                   | 5, 6, **9**, 11, 46 | 1, **2**, 3, 4, 5 |
| [ESP32](http://esp32.net/)                                               | N/A (not supported) | **1**             |
| [Teensy 1.0](https://www.pjrc.com/teensy/)                               | **17**              | **1**             |
| [Teensy 2.0](https://www.pjrc.com/teensy/)                               | 9, **10**, 14       | 1, 3, **4_HS**    |
| [Teensy++ 1.0 / 2.0](https://www.pjrc.com/teensy/)                       | **1**, 16, 25       | 1, **2**, 3       |
| [Teensy 3.0 / 3.1](https://www.pjrc.com/teensy/)                         | **5**               | **CMT**           |
| [Teensy-LC](https://www.pjrc.com/teensy/)                                | **16**              | **TPM1**          |


### Experimental patches
The following are strictly community supported patches that have yet to make it into mainstream. If you have issues feel free to ask here. If it works well then let us know!

[Arduino 101](https://github.com/z3t0/Arduino-IRremote/pull/481#issuecomment-311243146)

The table above lists the currently supported timers and corresponding send pins, many of these can have additional pins opened up and we are open to requests if a need arises for other pins.

## Usage
- TODO (Check examples for now)

## Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library
- Contribute new protocols

Check [here](Contributing.md) for some guidelines.

## Contact
Email: zetoslab@gmail.com
Please only email me if it is more appropriate than creating an Issue / PR. I **will** not respond to requests for adding support for particular boards, unless of course you are the creator of the board and would like to cooperate on the project. I will also **ignore** any emails asking me to tell you how to implement your ideas. However, if you have a private inquiry that you would only apply to you and you would prefer it to be via email, by all means.

## Contributors
Check [here](Contributors.md)

## Copyright
Copyright 2009-2012 Ken Shirriff
