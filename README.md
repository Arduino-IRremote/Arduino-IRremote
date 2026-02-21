# IRremote Arduino Library

> **Note**: This is a fork of the [Arduino-IRremote library](https://github.com/Arduino-IRremote/Arduino-IRremote) for personal Arduino projects and experimentation with infrared remote control functionality.

[![Build Status](https://travis-ci.org/z3t0/Arduino-IRremote.svg?branch=master)](https://travis-ci.org/z3t0/Arduino-IRremote)

[![Join the chat at https://gitter.im/z3t0/Arduino-IRremote](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/z3t0/Arduino-IRremote?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Overview

This library enables you to **send and receive infrared signals** on Arduino and compatible boards. Perfect for building remote controls, IR-based communication systems, and learning about infrared protocols.

### Key Features
- 📡 **Send and receive IR signals** with various protocols
- 🎮 **Decode remote controls** from TVs, air conditioners, and more
- 🔧 **Supports multiple boards** including Arduino, ESP32, Teensy
- 📚 **Extensive protocol library** (NEC, Sony, RC5, RC6, and more)
- ⚡ **Easy to use** API for quick prototyping

### Common Use Cases
- Build custom universal remotes
- Create IR-controlled projects (robots, home automation)
- Learn about infrared communication protocols
- Decode and clone existing remote controls
- Implement IR-based data communication

## Version - 2.2.3

## Quick Start

### Installation
1. Navigate to the [Releases](https://github.com/z3t0/Arduino-IRremote/releases) page.
2. Download the latest release.
3. Extract the zip file
4. Move the "IRremote" folder that has been extracted to your libraries directory.
5. **Important**: Delete `Arduino_Root/libraries/RobotIRremote` if it exists, as it has conflicting definitions.

### Basic Example - Receiving IR Signals

```cpp
#include <IRremote.h>

int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
}
```

### Basic Example - Sending IR Signals

```cpp
#include <IRremote.h>

IRsend irsend;

void setup() {
  Serial.begin(9600);
}

void loop() {
  irsend.sendNEC(0xFFE01F, 32); // Send NEC protocol code
  delay(2000);
}
```

## Supported Boards
- ✅ Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano
- ✅ Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / Teensy-LC (Credits: @PaulStoffregen)
- ✅ Sanguino
- ✅ ATmega series: 8, 48, 88, 168, 328, 8535, 16, 32, 164, 324, 644, 1284, 64, 128
- ✅ ATtiny 84 / 85
- ✅ ESP32 (receive only)
- ✅ ESP8266 ([separate fork](https://github.com/markszabo/IRremoteESP8266))
- ✅ Sparkfun Pro Micro

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.

### Hardware Specifications

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
| [Sparkfun Pro Micro](https://www.sparkfun.com/products/12640)            | 9, **5**, 5         | 1, **3**, 4_HS    |
| [Teensy 1.0](https://www.pjrc.com/teensy/)                               | **17**              | **1**             |
| [Teensy 2.0](https://www.pjrc.com/teensy/)                               | 9, **10**, 14       | 1, 3, **4_HS**    |
| [Teensy++ 1.0 / 2.0](https://www.pjrc.com/teensy/)                       | **1**, 16, 25       | 1, **2**, 3       |
| [Teensy 3.0 / 3.1](https://www.pjrc.com/teensy/)                         | **5**               | **CMT**           |
| [Teensy-LC](https://www.pjrc.com/teensy/)                                | **16**              | **TPM1**          |

**Bold** indicates the recommended/default pin for each board.

## Supported IR Protocols

The library supports many common IR protocols:
- NEC
- Sony SIRC
- RC5
- RC6
- DISH Network
- Sharp
- Panasonic
- JVC
- Samsung
- Whynter
- LG
- Denon
- And more!

## Documentation

### API Reference
- `IRrecv::enableIRIn()` - Initialize the IR receiver
- `IRrecv::decode(decode_results *results)` - Decode an IR signal
- `IRrecv::resume()` - Resume receiving after decode
- `IRsend::sendNEC(unsigned long data, int nbits)` - Send NEC protocol
- `IRsend::sendSony(unsigned long data, int nbits)` - Send Sony protocol
- `IRsend::sendRaw(unsigned int buf[], int len, int hz)` - Send raw timing data

For comprehensive documentation and tutorials, visit [the official homepage](http://z3t0.github.io/Arduino-IRremote/).

## Examples

The library includes several example sketches:
- **IRrecvDemo** - Basic IR receiver
- **IRsendDemo** - Basic IR transmitter
- **IRrelay** - IR relay/repeater
- **IRrecord** - Record and playback IR signals
- **IRtest** - Test IR functionality

Find them in: `File → Examples → IRremote`

## FAQ

**Q: IR does not work right when I use Neopixels (WS2811/WS2812/WS2812B)**  
A: Whether you use the Adafruit Neopixel lib, or FastLED, interrupts get disabled on many lower end CPUs like the basic Arduinos. In turn, this stops the IR interrupt handler from running when it needs to. See [this page from Marc MERLIN](http://marc.merlins.org/perso/arduino/post_2017-04-03_Arduino-328P-Uno-Teensy3_1-ESP8266-ESP32-IR-and-Neopixels.html) for solutions.

**Q: Why do I need to delete RobotIRremote?**  
A: The RobotIRremote library has similar definitions to IRremote and causes compilation errors. It must be removed.

**Q: Can I use this with ESP8266?**  
A: ESP8266 is supported in a separate fork: [IRremoteESP8266](https://github.com/markszabo/IRremoteESP8266)

**Q: How do I decode my specific remote?**  
A: Use the IRrecvDemo example to capture the codes your remote sends, then use those codes in your projects.

## Troubleshooting

- **No IR signal received**: Check your wiring, ensure receiver is powered and connected to the correct pin
- **Erratic behavior**: Keep IR receiver away from sunlight and fluorescent lights
- **Compilation errors**: Remove RobotIRremote library, check board selection
- **Inconsistent decoding**: Increase receive buffer or filter out noise

## Hardware Setup

### IR Receiver
```
IR Receiver (VS1838B or similar)
- VCC → 5V
- GND → GND  
- OUT → Digital Pin 11 (or your chosen pin)
```

### IR LED (Transmitter)
```
IR LED + Transistor
- IR LED Anode → Collector (2N2222)
- IR LED Cathode → GND
- Base → 220Ω resistor → Pin 3 (default send pin)
- Emitter → GND
```

## Experimental Patches
The following are strictly community supported patches:

[Arduino 101](https://github.com/z3t0/Arduino-IRremote/pull/481#issuecomment-311243146)

## Contributing

If you want to contribute to this project:
- 🐛 Report bugs and errors
- ✨ Ask for enhancements
- 📝 Create issues and pull requests
- 📣 Tell other people about this library
- 🔧 Contribute new protocols

Check the [Contributing Guidelines](Contributing.md) for more details.

For the main project contributions, please visit the [upstream repository](https://github.com/Arduino-IRremote/Arduino-IRremote).

## Resources

- [Official Homepage](http://z3t0.github.io/Arduino-IRremote/)
- [GitHub Issues](https://github.com/z3t0/Arduino-IRremote/issues)
- [Gitter Chat](https://gitter.im/z3t0/Arduino-IRremote)
- [Upstream Project](https://github.com/Arduino-IRremote/Arduino-IRremote)

## Contact

**Original Library:**
Email: zetoslab@gmail.com

Please only email for private inquiries. For bugs, features, and support, use GitHub Issues on the main repository.

## License

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

## Contributors

Check [Contributors.md](Contributors.md) for the list of contributors to the original project.

## Copyright

Copyright 2009-2012 Ken Shirriff

---

**About This Fork**: This is a personal fork for Arduino IR remote projects and experimentation. For the latest features and updates, please refer to the [main Arduino-IRremote repository](https://github.com/Arduino-IRremote/Arduino-IRremote).
