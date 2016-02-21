# IRremote Arduino Library

[![Build Status](https://travis-ci.org/z3t0/Arduino-IRremote.svg?branch=master)](https://travis-ci.org/z3t0/Arduino-IRremote)

[![Join the chat at https://gitter.im/z3t0/Arduino-IRremote](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/z3t0/Arduino-IRremote?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

This library enables you to send and receive using infra-red signals on an Arduino.

Check [here](http://z3t0.github.io/Arduino-IRremote/) for tutorials and more information.

## Version - 2.1.0

## Installation
1. Navigate to the [Releases](https://github.com/z3t0/Arduino-IRremote/releases) page.
2. Download the latest release.
3. Extract the zip file
4. Move the "IRremote" folder that has been extracted to your libraries directory.
5. Make sure to delete Arduino_Root/libraries/RobotIRremote. Where Arduino_Root refers to the install directory of Arduino. The library RobotIRremote has similar definitions to IRremote and causes errors.

## Supported Boards
- Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano etc.
- Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / Teensy-LC; Credits: @PaulStoffregen (Teensy Team)
- Sanguino
- Atmega8
- ATtiny 84 / 85

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.

### Hardware specifications

| Board/CPU                                | Send Pin            | Timers            |
|------------------------------------------|---------------------|-------------------|
| Arduino Mega / ATmega 1280 / ATmega 2560 | 5, 6, **9**, 11, 46 | 1, **2**, 3, 4, 5 |
| Teensy 1.0                               | **17**              | **1**             |
| Teensy 2.0                               | 9, **10**, 14       | 1, 3, **4_HS**    |
| Teensy++ 1.0 / 2.0                       | **1**, 16, 25       | 1, **2**, 3       |
| Teensy 3.0 / 3.1                         | **5**               | **CMT**           |
| Teensy-LC                                | **16**              | **TPM1**          |
| Sanguino                                 | 13, **14**          | 1, **2**          |
| Atmega8                                  | **9**               | **1**             |
| ATtiny84                                 | **6**               | **1**             |
| ATtiny85                                 | **1**               | **TINY0**         |
| Arduino Duemilanove, UNO etc.            | **3**, 9            | 1, **2**          |

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
The only way to contact me at the moment is by email: zetoslab@gmail.com
I am not currently monitoring any PRs or Issues due to other issues but will respond to all emails. If anyone wants contributor access, feel free to email me. Or if you find any Issues/PRs to be of importance that my attention is needed please email me.

## Contributors
Check [here](Contributors.md)

## Copyright
Copyright 2009-2012 Ken Shirriff
