# IRremote Arduino Library
Available as Arduino library "IRremote"

### [Version 3.0.0](https://github.com/z3t0/Arduino-IRremote/archive/master.zip) - work in progress

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Commits since latest](https://img.shields.io/github/commits-since/z3t0/Arduino-IRremote/latest)](https://github.com/z3t0/Arduino-IRremote/commits/master)
[![Installation instructions](https://www.ardu-badge.com/badge/IRremote.svg?)](https://www.ardu-badge.com/IRremote)
[![Join the chat at https://gitter.im/z3t0/Arduino-IRremote](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/z3t0/Arduino-IRremote?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![LibraryBuild](https://github.com/z3t0/Arduino-IRremote/workflows/LibraryBuild/badge.svg)](https://github.com/z3t0/Arduino-IRremote/actions)

This library enables you to send and receive using infra-red signals on an Arduino.

Tutorials and more information will be made available on [the official homepage](https://arduino-irremote.github.io/Arduino-IRremote/).

# Installation
Click on the LibraryManager badge above to see the [instructions](https://www.ardu-badge.com/IRremote/zip).

# Supported IR Protocols
Denon, JVC, LG,  NEC, Panasonic / Kaseikyo, RC5, RC6, Samsung, Sharp, Sony, (Pronto), BoseWave, Lego, Whynter, MagiQuest.<br/>
Protocols can be switched off and on by changing the lines in *IRremote.h*:

```
#define DECODE_<PROTOCOL_NAME>  1
#define SEND_<PROTOCOL_NAME>    1
```
# [Wiki](https://github.com/z3t0/Arduino-IRremote/wiki)
This is a quite old but maybe useful wiki for this library.

# Converting your program to the 3.x version
- Now there is  an **IRreceiver** and **IRsender** object like the well known Arduino **Serial** object.
- Just remove the line `IRrecv IrReceiver(IR_RECEIVE_PIN);` and/or `IRsend IrSender;` in your program, and replace all occurences of `IRrecv.` or `irrecv.` with `IrReceiver`.
- Like for the Serial object, call [`IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/IRreceiveDemo/IRreceiveDemo.ino#L38) or `IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);` instead of the `IrReceiver.enableIRIn();` or `irrecv.enableIRIn();` in setup().
- Old `decode(decode_results *aResults)` function is replaced by simple `decode()`. So if you have a statement `if(irrecv.decode(&results))` replace it with `if (IrReceiver.decode())`.
- `results.value` moved to `IrReceiver.decodedIRData.decodedRawData`.
- `results.decode_type` moved to `IrReceiver.decodedIRData.decodedRawData`.
- Overflow, Repeat and other flags are now in [`IrReceiver.receivedIRData.flags`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremote.h#L126).
- Seldomly used: `results.rawbuf` and `results.rawlen` moved to `IrReceiver.decodedIRData.rawDataPtr->rawbuf` and `IrReceiver.decodedIRData.rawDataPtr->rawlen`.

If you discover more changes, which should be documented, please send me a mail to armin.arduino@gmail.com.

# FAQ
- IR does not work right when I use Neopixels (aka WS2811/WS2812/WS2812B)<br/>
 Whether you use the Adafruit Neopixel lib, or FastLED, interrupts get disabled on many lower end CPUs like the basic Arduinos for longer than 50 µs.
In turn, this stops the IR interrupt handler from running when it needs to. There are some solutions to this on some processors,
 [see this page from Marc MERLIN](http://marc.merlins.org/perso/arduino/post_2017-04-03_Arduino-328P-Uno-Teensy3_1-ESP8266-ESP32-IR-and-Neopixels.html)
- The default IR timer on AVR's is timer 2. Since the **Arduino Tone library** as well as **analogWrite() for pin 3 and pin 11** requires timer 2,
 this functionality cannot be used simultaneously. You can use tone() but after the tone has stopped, you must call IrReceiver.enableIRIn() to restore the timer settings for receive.<br/>
If you can live with the NEC protocol, you can try the MinimalReceiver example, it requires no timer.
- You can use **multiple IR receiver** by just connecting the output pins of several IR receivers together.
 The IR receivers use an NPN transistor as output device with just a 30k resistor to VCC.
 This is almost "open collector" and allows connecting of several output pins to one Arduino input pin.

# Minimal version
For applications only requiring NEC protocol, there is a receiver which has very **small codesize and does NOT require any timer**. See the MinimalReceiver and IRDispatcherDemo example how to use it.

# Handling unknown Protocols
## Disclaimer
This library was never designed to handle long codes like the ones used by air conditioners.
See [Recording long Infrared Remote control signals with Arduino](https://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino).<br/>
The main reason is, that it was designed to fit inside MCUs with relatively low levels of resources and was intended to work as a library together with other applications which also require some resources of the MCU to operate.

## Hints
If you do not know which protocol your IR transmitter uses, you have several choices.
- Use the [IRreceiveDump example](examples/IRreceiveDump) to dump out the IR timing.
 You can then reproduce/send this timing with the [IRsendRawDemo example](examples/IRsendRawDemo).
 For **long codes** with more than 48 bits like from air conditioners, you can **change the length of the input buffer** in [IRremote.h](src/IRremote.h#L27).
- The [IRMP AllProtocol example](https://github.com/ukw100/IRMP#allprotocol-example) prints the protocol and data for one of the **40 supported protocols**.
 The same library can be used to send this codes.
- If you have a bigger Arduino board at hand (> 100 kByte program space) you can try the
 [IRremoteDecode example](https://github.com/bengtmartensson/Arduino-DecodeIR/blob/master/examples/IRremoteDecode/IRremoteDecode.ino) of the Arduino library [DecodeIR](https://github.com/bengtmartensson/Arduino-DecodeIR).
- Use [IrScrutinizer](http://www.harctoolbox.org/IrScrutinizer.html).
 It can automatically generate a send sketch for your protocol by exporting as "Arduino Raw". It supports IRremote,
 the old [IRLib](https://github.com/cyborg5/IRLib) and [Infrared4Arduino](https://github.com/bengtmartensson/Infrared4Arduino).
- To **increase strength of sent output signal** you can increase the current through the send diode, or use 2 diodes in series,
 since one IR diode requires only 1.5 volt. Changing `IR_SEND_DUTY_CYCLE` to 50 increases the signal current by 40%.

# Compile options / macros for this library
To customize the library to different requirements, there are some compile options / macros available.<br/>
Modify it by commenting them out or in, or change the values if applicable. Or define the macro with the -D compiler option for global compile (the latter is not possible with the Arduino IDE, so consider using [Sloeber](https://eclipse.baeyens.it).

| Name | File | Default value | Description |
|-|-|-|-|
| `IR_INPUT_IS_ACTIVE_HIGH` | IRremoteint.h | disabled | Enable it if you use a RF receiver, which has an active HIGH output signal. |
| `DEBUG` | IRremote.h | disabled | Enables lots of lovely debug output. |
| `USE_NEC_STANDARD` | IRremote.h | disabled | Use LSB first, address/code schema for encoding. |
| `USE_NO_SEND_PWM` | IRremote.h | disabled | Use no carrier PWM, just simulate an active low receiver signal. |
| `USE_SOFT_SEND_PWM` | IRremote.h | disabled | Use carrier PWM generation in software, instead of hardware PWM. |
| `PULSE_CORRECTION_MICROS` | IRremote.h | 3 | If USE_SOFT_SEND_PWM, this amount is subtracted from the on-time of the pulses. |
| `USE_SPIN_WAIT` | IRremote.h | disabled | If USE_SOFT_SEND_PWM, use spin wait instead of delayMicros(). |
| `SUPPORT_SEND_EXOTIC_PROTOCOLS` | IRremote.h | enabled | If activated, BOSEWAVE and LEGO_PF are supported in the write method. Costs around 500 bytes program space. |
| `RAW_BUFFER_LENGTH` | IRremoteint.h | 101 | Buffer size of raw input buffer. Must be odd! |
| `IR_SEND_DUTY_CYCLE` | IRremoteBoardDefs.h | 30 | Duty cycle of IR send signal. |
| `MICROS_PER_TICK` | IRremoteBoardDefs.h | 50 | Resolution of the raw input buffer data. |
| `USE_CUSTOM_DELAY` | irSend.cpp | disabled | Use old custom_delay_usec() function for mark and space delays. |
|-|-|-|-|
| `IR_INPUT_PIN` | TinyIRReceiver.h | 2 | The pin number for TinyIRReceiver IR input, which gets compiled in. |
| `IR_FEEDBACK_LED_PIN` | TinyIRReceiver.h | `LED_BUILTIN` | The pin number for TinyIRReceiver feedback LED, which gets compiled in. |
| `DO_NOT_USE_FEEDBACK_LED` | TinyIRReceiver.h | disabled | Enable it to disable the feedback LED function. |



### Modifying compile options with Arduino IDE
First use *Sketch > Show Sketch Folder (Ctrl+K)*.<br/>
If you did not yet stored the example as your own sketch, then you are instantly in the right library folder.<br/>
Otherwise you have to navigate to the parallel `libraries` folder and select the library you want to access.<br/>
In both cases the library files itself are located in the `src` directory.<br/>

### Modifying compile options with Sloeber IDE
If you are using Sloeber as your IDE, you can easily define global symbols with *Properties > Arduino > CompileOptions*.<br/>
![Sloeber settings](https://github.com/ArminJo/ServoEasing/blob/master/pictures/SloeberDefineSymbols.png)

## Other IR libraries
[Here](https://github.com/ukw100/IRMP#quick-comparison-of-4-arduino-ir-receiving-libraries) you find a **short comparison matrix** of 4 popular Arduino IR libraries.<br/>
[Here](https://github.com/crankyoldgit/IRremoteESP8266) you find an **ESP8266/ESP32** version of IRremote with an **[impressive list of supported protocols](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md)**.

# Supported Boards
- Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano etc.
- Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / Teensy-LC; Credits: @PaulStoffregen (Teensy Team)
- Sanguino
- ATmega8, 48, 88, 168, 328
- ATmega8535, 16, 32, 164, 324, 644, 1284,
- ATmega64, 128
- ATmega4809 (Nano every)
- ATtiny84, 85
- SAMD21 (receive only)
- ESP32
- [ESP8266 is supported in a fork](https://github.com/crankyoldgit/IRremoteESP8266) based on an old codebase. It works well given that perfectly timed sub millisecond interrupts are different on that chip.
- Sparkfun Pro Micro

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.

## Hardware specifications
The timer and the pin usage can be adjusted in [IRremoteBoardDefs.h](src/private/IRremoteBoardDefs.h)

| Board/CPU                                                                | IR-Send (PWM) Pin   | Timers            |
|--------------------------------------------------------------------------|---------------------|-------------------|
| [ATtiny84](https://github.com/SpenceKonde/ATTinyCore)                    | **6**               | **1**             |
| [ATtiny85 > 1 MHz](https://github.com/SpenceKonde/ATTinyCore)            | **1**, 4            | **0**, 1          |
| [ATmega8](https://github.com/MCUdude/MiniCore)                           | **9**               | **1**             |
| [ATmega48, ATmega88, ATmega168, **ATmega328**](https://github.com/MCUdude/MiniCore) | **3**, 9 | 1, **2**          |
| [ATmega1284](https://github.com/MCUdude/MightyCore)                      | 13, 14, 6           | 1, **2**, 3       |
| [ATmega164, ATmega324, ATmega644](https://github.com/MCUdude/MightyCore) | 13, **14**          | 1, **2**          |
| [ATmega8535 ATmega16, ATmega32](https://github.com/MCUdude/MightyCore)   | **13**              | **1**             |
| [ATmega64, ATmega128, ATmega1281, ATmega2561](https://github.com/MCUdude/MegaCore) | **13**    | **1**             |
| [ATmega8515, ATmega162](https://github.com/MCUdude/MajorCore)            | **13**              | **1**             |
| ATmega1280, ATmega2560                                                   | 5, 6, **9**, 11, 46 | 1, **2**, 3, 4, 5 |
| ATmega4809                                                               | 5, 6, **9**, 11, 46 | **TCB0**          |
| Leonardo (Atmega32u4)                                                    | 5, **9**, 13        | 1, 3, **4_HS**    |
| Zero (SAMD)                                                              | \*, **9**           | **TC3**           |
| [ESP32](http://esp32.net/)                                               | **4**, all pins     | **1**             |
| [Sparkfun Pro Micro](https://www.sparkfun.com/products/12640)            | **5**, 9            | 1, **3**          |
| [Teensy 1.0](https://www.pjrc.com/teensy/)                               | **17**              | **1**             |
| [Teensy 2.0](https://www.pjrc.com/teensy/)                               | **9**, 10, 14       | 1, **3**, 4_HS    |
| [Teensy++ 1.0 / 2.0](https://www.pjrc.com/teensy/)                       | **1**, 16, 25       | 1, **2**, 3       |
| [Teensy 3.0 / 3.1](https://www.pjrc.com/teensy/)                         | **5**               | **CMT**           |
| [Teensy-LC](https://www.pjrc.com/teensy/)                                | **16**              | **TPM1**          |

# Adding new protocols
To add a new protocol is quite straightforward. Best is too look at the existing protocols to find a similar one and modify it.<br/>
As a rule of thumb, it is easier to work with a description of the protocol rather than trying to entirely reverse-engineer the protocol.
Please include a link to the description in the header, if you found one.<br/>
The **durations** you receive are likely to be longer for marks and shorter for spaces than the protocol suggests,
but this depends on the receiver circuit in use. Most protocols use multiples of one time-unit for marks and spaces like e.g. [NEC](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.cpp#L50). It's easy to be off-by-one with the last bit, since the last space is not recorded by IRremote.

Try to make use of the template functions `decodePulseDistanceData()` and `sendPulseDistanceData()`.
If your protocol supports address and code fields, try to reflect this in your api like it is done in [`sendNEC(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aIsRepeat)`](https://github.com/z3t0/Arduino-IRremote/blob/master/src/ir_NEC.cpp#L86) and [`decodeNEC()`](https://github.com/z3t0/Arduino-IRremote/blob/master/src/ir_NEC.cpp#L145).<br/>

### Integration
To integrate your protocol, you need to extend the two functions `decode()` and `getProtocolString()` in *IRreceice.cpp*,
add macros and function declarations for sending and receiving and extend the `enum decode_type_t` in *IRremote.h*.<br/>
And at least it would be wonderful if you can provide an example how to use the new protocol.
A detailed description can be found in the [ir_Template.cpp](https://github.com/z3t0/Arduino-IRremote/blob/master/src/ir_Template.cpp#L18) file.

# Revision History
Please see [changelog.md](https://github.com/z3t0/Arduino-IRremote/blob/master/changelog.md).

# API documentation
See [API reference in wiki](https://github.com/z3t0/Arduino-IRremote/wiki/API-Reference).

To generate the API documentation,
Doxygen, as well as [Graphviz](http://www.graphviz.org/) should be installed.
(Note that on Windows, it may be necessary to add the Graphviz binary directory
(something like `C:\Program Files\Graphviz2.38\bin`)
to the `PATH` variable manually.)
With Doxygen and Graphviz installed, issue the command
`doxygen` from the command line in the main project directory, which will
generate the API documentation in HTML format.
The just generated `api-doc/index.html` can now be opened in a browser.

## Why do we use 33% duty cycle
We do it according to the statement in the [Vishay datasheet](https://www.vishay.com/docs/80069/circuit.pdf):
- Carrier duty cycle 50 %, peak current of emitter IF = 200 mA, the resulting transmission distance is 25 m.
- Carrier duty cycle 10 %, peak current of emitter IF = 800 mA, the resulting transmission distance is 29 m. - Factor 1.16
The reason is, that it is not the pure energy of the fundamental which is responsible for the receiver to detect a signal.
Due to automatic gain control and other bias effects high intensity and lower energy (duty cycle) of the 38 kHz pulse counts more than high low intensity and higher energy.

BTW, **the best way to increase the IR power** is to use 2 or 3 IR diodes in series. One diode requires 1.1 to 1.5 volt so you can supply 3 diodes with a 5 volt output.<br/>
To keep the current, you must reduce the resistor by (5 - 1.3) / (5 - 2.6) = 1.5 e.g. from 150 ohm to 100 ohm for 25 mA and 2 diodes with 1.3 volt and a 5 volt supply.<br/>
For 3 diodes it requires factor 2.5 e.g. from 150 ohm to 60 ohm.

# Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library
- Contribute new protocols

Check [here](https://github.com/z3t0/Arduino-IRremote/blob/master/Contributing.md) for some guidelines.

## Contributors
Check [here](https://github.com/z3t0/Arduino-IRremote/blob/master/Contributors.md)

# Contact
Email: zetoslab@gmail.com
Please only email me if it is more appropriate than creating an Issue / PR. I **will** not respond to requests for adding support for particular boards, unless of course you are the creator of the board and would like to cooperate on the project. I will also **ignore** any emails asking me to tell you how to implement your ideas. However, if you have a private inquiry that you would only apply to you and you would prefer it to be via email, by all means.

# License
Up to the version 2.7.0 the License is GPLv2.
From the version 2.8.0 the license is the MIT license.

## Copyright
Initially coded 2009 Ken Shirriff http://www.righto.com
Copyright (c) 2016 Rafi Khan
Copyright (c) 2020 Armin Joachimsmeyer
