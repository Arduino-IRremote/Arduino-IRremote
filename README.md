# IRremote Arduino Library
Available as Arduino library "IRremote"

### [Version 2.5.0](https://github.com/z3t0/Arduino-IRremote/releases)

[![License: GPL v2](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://www.gnu.org/licenses/gpl-2.0)
[![Installation instructions](https://www.ardu-badge.com/badge/IRremote.svg?)](https://www.ardu-badge.com/IRremote)
[![Join the chat at https://gitter.im/z3t0/Arduino-IRremote](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/z3t0/Arduino-IRremote?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![LibraryBuild](https://github.com/z3t0/Arduino-IRremote/workflows/LibraryBuild/badge.svg)](https://github.com/z3t0/Arduino-IRremote/actions)

This library enables you to send and receive using infra-red signals on an Arduino.

Tutorials and more information will be made available on [the official homepage](http://z3t0.github.io/Arduino-IRremote/).

# Installation
Click on the LibraryManager badge above to see the instructions.

# FAQ
- IR does not work right when I use Neopixels (aka WS2811/WS2812/WS2812B)
Whether you use the Adafruit Neopixel lib, or FastLED, interrupts get disabled on many lower end CPUs like the basic arduinos. In turn, this stops the IR interrupt handler from running when it needs to. There are some solutions to this on some processors, [see this page from Marc MERLIN](http://marc.merlins.org/perso/arduino/post_2017-04-03_Arduino-328P-Uno-Teensy3_1-ESP8266-ESP32-IR-and-Neopixels.html)
- The default IR timer on AVR's is timer 2. Since the Arduino Tone library as well as analogWrite() for pin 3 and pin 11 requires timer 2, this functionality cannot be used simultaneously.
- You can use **multiple IR receiver** by just connecting the output pins of several IR receivers together. The IR receivers use an NPN transistor as output device with just a 30k resistor to VCC. This is almost "open collector" and allows connecting of several output pins.

# Supported IR Protocols
Aiwa, BoseWave, Denon, Dish, JVC, Lego, LG, MagiQuest, Mitsubishi, Panasonic, RC5, RC6, Samsung, Sanyo, Sharp, Sony, Whynter, (Pronto).

## Handling unknown Protocols
### Disclaimer
This library was never designed to handle long codes like the ones used by air conditioners. See [Recording long Infrared Remote control signals with Arduino](https://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino).<br/>
The main reason is, that it was designed to fit inside MCUs with relatively low levels of resources and was intended to work as a library together with other applications which also require some resources of the MCU to operate.

If you do not know which protocol your IR transmitter uses, you have several choices.
- Use the [IRrecvDumpV2 example](examples/IRrecvDumpV2) to dump out the IR timing. You can then reproduce/send this timing with the [IRsendRawDemo example](examples/IRsendRawDemo). For **long codes** like from air conditioners, you can **change the length of the input buffer** in [IRremoteInt.h](src/private/IRremoteInt.h#L30).
- If you have a bigger Arduino board at hand (> 100 kByte program space) you can try the [IRremoteDecode example](https://github.com/bengtmartensson/Arduino-DecodeIR/blob/master/examples/IRremoteDecode/IRremoteDecode.ino) of the Arduino library DecodeIR.
- Use [IrScrutinizer](http://www.harctoolbox.org/IrScrutinizer.html). It can automatically generate a send sketch for your protocol by exporting as "Arduino Raw". It supports IRremote, the old [IRLib](https://github.com/cyborg5/IRLib) and [Infrared4Arduino](https://github.com/bengtmartensson/Infrared4Arduino).
- Use the [IRMP AllProtocol example](https://github.com/ukw100/IRMP#allprotocol-example) and check the serial output if the protocol is one of the 40 supported protocols.

# Supported Boards
- Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano etc.
- Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / Teensy-LC; Credits: @PaulStoffregen (Teensy Team)
- Sanguino
- ATmega8, 48, 88, 168, 328
- ATmega8535, 16, 32, 164, 324, 644, 1284,
- ATmega64, 128
- ATtiny84, 85
- SAMD21 (receive only)
- ESP32
- ESP8266 is supported in a fork based on an old codebase that isn't as recent, but it works reasonably well given that perfectly timed sub millisecond interrupts are different on that chip. See https://github.com/markszabo/IRremoteESP8266
- Sparkfun Pro Micro

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.

## Hardware specifications
The timer and the pin usage can be adjusted in [IRremoteBoardDefs.h](src/private/IRremoteBoardDefs.h#L154)

| Board/CPU                                                                | Send Pin            | Timers            |
|--------------------------------------------------------------------------|---------------------|-------------------|
| [ATtiny84](https://github.com/SpenceKonde/ATTinyCore)                    | **6**               | **1**             |
| [ATtiny85](https://github.com/SpenceKonde/ATTinyCore)                    | **1**               | **TINY0**         |
| [ATmega8](https://github.com/MCUdude/MiniCore)                           | **9**               | **1**             |
| [ATmega48, ATmega88, ATmega168, ATmega328](https://github.com/MCUdude/MiniCore) | **3**, 9     | 1, **2**          |
| [ATmega1284](https://github.com/MCUdude/MightyCore)                      | 13, 14, 6           | 1, **2**, 3       |
| [ATmega164, ATmega324, ATmega644](https://github.com/MCUdude/MightyCore) | 13, **14**          | 1, **2**          |
| [ATmega8535 ATmega16, ATmega32](https://github.com/MCUdude/MightyCore)   | **13**              | **1**             |
| [ATmega64, ATmega128, ATmega1281, ATmega2561](https://github.com/MCUdude/MegaCore) | **13**              | **1**             |
| [ATmega8515, ATmega162](https://github.com/MCUdude/MajorCore)            | **13**              | **1**             |
| ATmega1280, ATmega2560                                                   | 5, 6, **9**, 11, 46 | 1, **2**, 3, 4, 5 |
| Leonardo (Atmega32u4)                                                    | 5, **9**, 13        | 1, 3, **4_HS**    |
| [ESP32](http://esp32.net/)                                               | **4**, all pins     | **1**             |
| [Sparkfun Pro Micro](https://www.sparkfun.com/products/12640)            | **5**, 9, 13        | 1, **3**, 4_HS    |
| [Teensy 1.0](https://www.pjrc.com/teensy/)                               | **17**              | **1**             |
| [Teensy 2.0](https://www.pjrc.com/teensy/)                               | **9**, 10, 14       | 1, **3**, 4_HS    |
| [Teensy++ 1.0 / 2.0](https://www.pjrc.com/teensy/)                       | **1**, 16, 25       | 1, **2**, 3       |
| [Teensy 3.0 / 3.1](https://www.pjrc.com/teensy/)                         | **5**               | **CMT**           |
| [Teensy-LC](https://www.pjrc.com/teensy/)                                | **16**              | **TPM1**          |

## Usage
- TODO (Check examples for now)

## Revision History
Please see [changelog.md](https://github.com/z3t0/Arduino-IRremote/blob/master/changelog.md).

## API documentation
This project documents the library API using [Doxygen](http://www.doxygen.org).
It is planned to make generated and up-to-date API documentation available online.

To generate the API documentation,
Doxygen, as well as [Graphviz](http://www.graphviz.org/) should be installed.
(Note that on Windows, it may be necessary to add the Graphviz binary directory
(something like `C:\Program Files\Graphviz2.38\bin`)
to the `PATH` variable manually.)
With Doxygen and Graphviz installed, issue the command
`doxygen` from the command line in the main project directory, which will
generate the API documentation in HTML format.
The just generated `api-doc/index.html` can now be opened in a browser.

## Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library
- Contribute new protocols

Check [here](https://github.com/z3t0/Arduino-IRremote/blob/master/Contributing.md) for some guidelines.

## Contact
Email: zetoslab@gmail.com
Please only email me if it is more appropriate than creating an Issue / PR. I **will** not respond to requests for adding support for particular boards, unless of course you are the creator of the board and would like to cooperate on the project. I will also **ignore** any emails asking me to tell you how to implement your ideas. However, if you have a private inquiry that you would only apply to you and you would prefer it to be via email, by all means.

## Contributors
Check [here](https://github.com/z3t0/Arduino-IRremote/blob/master/Contributors.md)

## Copyright
Copyright 2009-2012 Ken Shirriff
Copyright (c) 2016 Rafi Khan