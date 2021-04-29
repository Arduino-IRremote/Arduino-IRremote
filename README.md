# IRremote Arduino Library
Available as Arduino library "IRremote"

### [Version 3.3.0](https://github.com/Arduino-IRremote/Arduino-IRremote/archive/master.zip) - work in progress

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Commits since latest](https://img.shields.io/github/commits-since/Arduino-IRremote/Arduino-IRremote/latest)](https://github.com/Arduino-IRremote/Arduino-IRremote/commits/master)
[![Installation instructions](https://www.ardu-badge.com/badge/IRremote.svg?)](https://www.ardu-badge.com/IRremote)
[![Join the chat at https://gitter.im/Arduino-IRremote/Arduino-IRremote](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Arduino-IRremote/Arduino-IRremote?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![LibraryBuild](https://github.com/Arduino-IRremote/Arduino-IRremote/workflows/LibraryBuild/badge.svg)](https://github.com/Arduino-IRremote/Arduino-IRremote/actions)

This library enables you to send and receive using infra-red signals on an Arduino.

# API
A Doxygen documentation of the sources is available on the [project homepage](https://arduino-irremote.github.io/Arduino-IRremote/).

# Installation
Click on the LibraryManager badge above to see the [instructions](https://www.ardu-badge.com/IRremote/zip).

# Supported IR Protocols
Denon / Sharp, JVC, LG,  NEC / Onkyo / Apple, Panasonic / Kaseikyo, RC5, RC6, Samsung, Sony, (Pronto), BoseWave, Lego, Whynter, MagiQuest.<br/>
Protocols can be switched off and on by defining macros before the line `#include <IRremote.h>` like [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino#L14):

```
#define DECODE_NEC
//#define DECODE_DENON
#include <IRremote.h>
```

# [Wiki](https://github.com/Arduino-IRremote/Arduino-IRremote/wiki)
This is a quite old but maybe useful wiki for this library.

# Features of the 3.x version
- You can use any pin for sending now, like you are used with receiving.
- Simultaneous sending and receiving. See the [UnitTest](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/UnitTest/UnitTest.ino#L165-L166) example.
- No more need to use 32 bit hex values in your code. Instead a (8 bit) command value is provided for decoding (as well as an 16 bit address and a protocol number).
- Protocol values comply to protocol standards, i.e. NEC, Panasonic, Sony, Samsung and JVC decode and send LSB first.
- Supports more protocols, since adding a protocol is quite easy now.
- Better documentation and more examples :-).
- Compatible with tone() library, see [ReceiveDemo](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino#L150-L153).
- Supports more platforms, since the new structure allows to easily add a new platform.
- Feedback LED also for sending.
- Ability to generate a non PWM signal to just simulate an active low receiver signal for direct connect to existent receiving devices without using IR.
- Easy configuration of protocols required, directly in your [source code[(https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino#L18-L34). This reduces the memory footprint and increases decoding time.

# Converting your program to the 3.1 version
This must be done also for all versions > 3.0.1 if `USE_NO_SEND_PWM` is defined.<br/>
Starting with this version, **the generation of PWM is done by software**, thus saving the hardware timer and **enabling arbitrary output pins**.<br/>
Therefore you must change all `IrSender.begin(true);` by `IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);`.
If you use a core that does not use the `-flto` flag for compile, you can activate the line `#define SUPPRESS_ERROR_MESSAGE_FOR_BEGIN` in IRRemote.h, if you get false error messages regarding begin() during compilation.

# Converting your 2.x program to the 3.x version
- Now there is  an **IRreceiver** and **IRsender** object like the well known Arduino **Serial** object.
- Just remove the line `IRrecv IrReceiver(IR_RECEIVE_PIN);` and/or `IRsend IrSender;` in your program, and replace all occurrences of `IRrecv.` or `irrecv.` with `IrReceiver`.
- Since the decoded values are now in `IrReceiver.decodedIRData` and not in `results` any more, remove the line `decode_results results` or similar.
- Like for the Serial object, call [`IrReceiver.begin(IR_RECEIVE_PIN, ENABE_ED_FEEDBACK);`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino#L38) or `IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);` instead of the `IrReceiver.enableIRIn();` or `irrecv.enableIRIn();` in setup().
- Old `decode(decode_results *aResults)` function is replaced by simple `decode()`. So if you have a statement `if(irrecv.decode(&results))` replace it with `if (IrReceiver.decode())`.
- The decoded result is now in in `IrReceiver.decodedIRData` and not in `results` any more, therefore replace any occurrences of `results.value` and `results.decode_type` (and similar) to `IrReceiver.decodedIRData.decodedRawData` and `IrReceiver.decodedIRData.protocol`.
- Overflow, Repeat and other flags are now in [`IrReceiver.receivedIRData.flags`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremote.h#L126).
- Seldom used: `results.rawbuf` and `results.rawlen` must be replaced by `IrReceiver.decodedIRData.rawDataPtr->rawbuf` and `IrReceiver.decodedIRData.rawDataPtr->rawlen`.

# Running your 2.x program with the 3.x library version
If you program is like:
```
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
...
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);
      ...
      irrecv.resume(); // Receive the next value
  }
  ...
}
```
it should run on the 3.1.1 version as before. The following decoders are available: Denon, JVC, LG,  NEC, Panasonic, RC5, RC6, Samsung, Sony.
The `results.value` is set by the decoders for **NEC, Panasonic, Sony, Samsung and JVC** as MSB first like in 2.x.<br/>
- The old functions `sendNEC()` and `sendJVC()` are deprecated and renamed to `sendNECMSB()` and `sendJVCMSB()` to make it clearer that they send data with MSB first, which is not the standard for NEC and JVC. Use them to send your **old MSB-first 32 bit IR data codes**.
In the new version you will send NEC commands not by 32 bit codes but by a (constant) 8 bit address and an 8 bit command.

# Convert old MSB first 32 bit IR data codes to new LSB first 32 bit IR data codes
The new decoders for **NEC, Panasonic, Sony, Samsung and JVC** `IrReceiver.decodedIRData.decodedRawData` is now LSB-first, as the definition of these protocols suggests!
  To convert one into the other, you must reverse the byte positions and then reverse all bit positions of each byte or write it as one binary string and reverse/mirror it.<br/>
  Example:<br/>
  0xCB340102 byte reverse -> 02 01 34 CB bit reverse-> 40 80 2C D3.<br/>
  0xCB340102 is binary 11001011001101000000000100000010.<br/>
  0x40802CD3 is binary 01000000100000000010110011010011.<br/>
  If you read the first binary sequence backwards (right to left), you get the second sequence.

# FAQ
- IR does not work right when I use **Neopixels** (aka WS2811/WS2812/WS2812B) or other libraries blocking interrupts for a longer time (> 50 us).<br/>
 Whether you use the Adafruit Neopixel lib, or FastLED, interrupts get disabled on many lower end CPUs like the basic Arduinos for longer than 50 탎.
In turn, this stops the IR interrupt handler from running when it needs to. There are some solutions to this on some processors,
 [see this page from Marc MERLIN](http://marc.merlins.org/perso/arduino/post_2017-04-03_Arduino-328P-Uno-Teensy3_1-ESP8266-ESP32-IR-and-Neopixels.html)
- The default IR timer on AVR's is timer 2. Since the **Arduino Tone library** as well as **analogWrite() for pin 3 and pin 11** requires timer 2,
 this functionality cannot be used simultaneously. You can use tone() but after the tone has stopped, you must call `IrReceiver.start()` or better `IrReceiver.start(<microsecondsOfToneDuration>)` to restore the timer settings for receive. Or you change the timer to timer 1 in private/IRTimer.cpp.h.<br/>
If you can live with the NEC protocol, you can try the MinimalReceiver example, it requires no timer.
- You can use **multiple IR receiver** by just connecting the output pins of several IR receivers together.
 The IR receivers use an NPN transistor as output device with just a 30k resistor to VCC.
 This is almost "open collector" and allows connecting of several output pins to one Arduino input pin.
- The **minimal CPU frequency** for receiving is 4 MHz, since the 50 us timer ISR takes around 12 us on a 16 MHz ATmega.

# Minimal version
For applications only requiring NEC protocol, there is a receiver which has very **small codesize of 500 bytes and does NOT require any timer**. See the MinimalReceiver and IRDispatcherDemo example how to use it. Mapping of pins to interrupts can be found [here](https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/src/TinyIRReceiver.cpp.h#L307).

# Handling unknown Protocols
## Disclaimer
This library was never designed to handle long codes like the ones used by air conditioners.
See [Recording long Infrared Remote control signals with Arduino](https://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino).<br/>
The main reason is, that it was designed to fit inside MCUs with relatively low levels of resources and was intended to work as a library together with other applications which also require some resources of the MCU to operate.

## Protocol=UNKNOWN
If you see something like `Protocol=UNKNOWN Hash=0x13BD886C 35 bits received` as output of e.g. the ReceiveDemo example, you either have a problem with decoding a protocol, or an unsupported protocol.

- If you have an **odd number of bits** received, it is likely, that your receiver circuit has problems. Maybe because the IR signal is too weak.
- If you see timings like `+ 600,- 600     + 550,- 150     + 200,- 100     + 750,- 550` then one 450 탎 space was split into two 150 and 100 탎 spaces with a spike / error signal of 200 탎 between. Maybe because of a defective receiver or a weak signal in conjunction with another light emitting source nearby.
- If you see timings like `+ 500,- 550     + 450,- 550     + 500,- 500     + 500,-1550`, then marks are generally shorter than spaces and therefore `MARK_EXCESS_MICROS` (specified in your ino file) should be **negative** to compensate for this at decoding.
- If you see `Protocol=UNKNOWN Hash=0x0 1 bits received` it may be that the space after the initial mark is longer than [`RECORD_GAP_MICROS`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremote.h#L124). This was observed for some LG air conditioner protocols. Try again with a line e.g. `#define RECORD_GAP_MICROS 12000` before the line `#include <IRremote.h>` in your ino file.
- To see more info supporting you to find the reason for your UNKNOWN protocol, you must enable the line `//#define DEBUG` in IRremoteInt.h.

## How to deal with protocols not supported by IRremote
If you do not know which protocol your IR transmitter uses, you have several choices.
- Use the [IRreceiveDump example](examples/ReceiveDump) to dump out the IR timing.
 You can then reproduce/send this timing with the [SendRawDemo example](examples/SendRawDemo).
 For **long codes** with more than 48 bits like from air conditioners, you can **change the length of the input buffer** in [IRremote.h](src/IRremoteInt.h#L36).
- The [IRMP AllProtocol example](https://github.com/ukw100/IRMP#allprotocol-example) prints the protocol and data for one of the **40 supported protocols**.
 The same library can be used to send this codes.
- If you have a bigger Arduino board at hand (> 100 kByte program space) you can try the
 [IRremoteDecode example](https://github.com/bengtmartensson/Arduino-DecodeIR/blob/master/examples/IRremoteDecode/IRremoteDecode.ino) of the Arduino library [DecodeIR](https://github.com/bengtmartensson/Arduino-DecodeIR).
- Use [IrScrutinizer](http://www.harctoolbox.org/IrScrutinizer.html).
 It can automatically generate a send sketch for your protocol by exporting as "Arduino Raw". It supports IRremote,
 the old [IRLib](https://github.com/cyborg5/IRLib) and [Infrared4Arduino](https://github.com/bengtmartensson/Infrared4Arduino).
 
# Hints
- To **increase strength of sent output signal** you can increase the current through the send diode, and/or use 2 diodes in series,
 since one IR diode requires only 1.5 volt.
 - The line \#include "ATtinySerialOut.h" in PinDefinitionsAndMore.h (requires the library to be installed) saves 370 bytes program space and 38 bytes RAM for **Digispark boards** as well as enables serial output at 8MHz.
 - The default software generated PWM has **problems on AVR running with 8 MHz**. The PWM frequency is around 30 instead of 38 kHz and RC6 is not reliable. You can switch to timer PWM generation by `#define SEND_PWM_BY_TIMER`.

# Examples
In order to fit the examples to the 8K flash of ATtiny85 and ATtiny88, the [Arduino library ATtinySerialOut](https://github.com/ArminJo/ATtinySerialOut) is required for this CPU's.

### SimpleReceiver + SimpleSender
This examples are a good starting point.

### ReceiveDemo + SendDemo
More complete examples for the advanced user.

### ReceiveAndSend + UnitTest
ReceiveDemo + SendDemo in one program. **Receiving while sending**.

### ReceiveAndSend
Record and play back last received IR signal at button press.

### MinimalReceiver + SmallReceiver
If code size matters, look at these examples.

### IRDispatcherDemo
Framework for calling different functions for different IR codes.

### IRrelay
Control a relay (connected to an output pin) with your remote.

### IRremoteExtensionTest
Example for a user defined class, which itself uses the IRrecv class from IRremote.

# Compile options / macros for this library
To customize the library to different requirements, there are some compile options / macros available.<br/>
Modify it by commenting them out or in, or change the values if applicable. Or define the macro with the -D compiler option for global compile (the latter is not possible with the Arduino IDE, so consider using [Sloeber](https://eclipse.baeyens.it).

| Name | File | Default value | Description |
|-|-|-|-|
| `SEND_PWM_BY_TIMER` | Before `#include <IRremote.h>` | disabled | Disable carrier PWM generation in software and use (restricted) hardware PWM except for ESP32 where both modes are using the flexible `hw_timer_t`. |
| `USE_NO_SEND_PWM` | Before `#include <IRremote.h>` | disabled | Use no carrier PWM, just simulate an active low receiver signal. Overrides `SEND_PWM_BY_TIMER` definition. |
| `NO_LEGACY_COMPATIBILITY` | IRremoteInt.h | disabled | Disables the old decoder for version 2.x compatibility, where all protocols -especially NEC, Panasonic, Sony, Samsung and JVC- were MSB first. Saves around 60 bytes program space and 14 bytes RAM. |
| `EXCLUDE_EXOTIC_PROTOCOLS` | Before `#include <IRremote.h>` | disabled | If activated, BOSEWAVE, MAGIQUEST,WHYNTER and LEGO_PF are excluded in `decode()` and in sending with `IrSender.write()`. Saves up to 900 bytes program space. |
| `EXCLUDE_UNIVERSAL_PROTOCOLS` | Before `#include <IRremote.h>` | disabled | If activated, the universal decoder for pulse width or pulse distance protocols and decodeHash (special decoder for all protocols) are excluded in `decode()`. Saves up to 1000 bytes program space. |
| `MARK_EXCESS_MICROS` | Before `#include <IRremote.h>` | 20 | MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding, to compensate for the signal forming of different IR receiver modules. |
| `RECORD_GAP_MICROS` | Before `#include <IRremote.h>` | 5000 | Minimum gap between IR transmissions, to detect the end of a protocol.<br/>Must be greater than any space of a protocol e.g. the NEC header space of 4500 us.<br/>Must be smaller than any gap between a command and a repeat; e.g. the retransmission gap for Sony is around 24 ms.<br/>Keep in mind, that this is the delay between the end of the received command and the start of decoding. |
| `FEEDBACK_LED_IS_ACTIVE_LOW` | Before `#include <IRremote.h>` | disabled | Required on some boards (like my BluePill and my ESP8266 board), where the feedback LED is active low. |
| `DISABLE_LED_FEEDBACK_FOR_RECEIVE` | Before `#include <IRremote.h>` | disabled | This completely disables the LED feedback code for receive, thus saving around 108 bytes program space and halving the receiver ISR processing time. |
| `IR_INPUT_IS_ACTIVE_HIGH` | Before `#include <IRremote.h>` | disabled | Enable it if you use a RF receiver, which has an active HIGH output signal. |
| `RAW_BUFFER_LENGTH` | IRremoteInt.h | 101 | Buffer size of raw input buffer. Must be odd! |
| `DEBUG` | IRremoteInt.h | disabled | Enables lots of lovely debug output. |
| `IR_SEND_DUTY_CYCLE` | IRremoteInt.h | 30 | Duty cycle of IR send signal. |
| `MICROS_PER_TICK` | IRremoteInt.h | 50 | Resolution of the raw input buffer data. |
|-|-|-|-|
| `IR_INPUT_PIN` | TinyIRReceiver.h | 2 | The pin number for TinyIRReceiver IR input, which gets compiled in. |
| `IR_FEEDBACK_LED_PIN` | TinyIRReceiver.h | `LED_BUILTIN` | The pin number for TinyIRReceiver feedback LED, which gets compiled in. |
| `DO_NOT_USE_FEEDBACK_LED` | TinyIRReceiver.h | disabled | Enable it to disable the feedback LED function. |

### Modifying compile options with Arduino IDE
First, use *Sketch > Show Sketch Folder (Ctrl+K)*.<br/>
If you did not yet stored the example as your own sketch, then you are instantly in the right library folder.<br/>
Otherwise you have to navigate to the parallel `libraries` folder and select the library you want to access.<br/>
In both cases the library files itself are located in the `src` directory.<br/>

### Modifying compile options with Sloeber IDE
If you are using Sloeber as your IDE, you can easily define global symbols with *Properties > Arduino > CompileOptions*.<br/>
![Sloeber settings](https://github.com/ArminJo/ServoEasing/blob/master/pictures/SloeberDefineSymbols.png)

# Supported Boards
- Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano etc.
- Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / Teensy-LC; Credits: PaulStoffregen (Teensy Team)
- Sanguino
- ATmega8, 48, 88, 168, 328
- ATmega8535, 16, 32, 164, 324, 644, 1284,
- ATmega64, 128
- ATmega4809 (Nano every)
- ATtiny84, 85
- SAMD21 (DUE, Zero)
- ESP32
- ESP8266. [This fork](https://github.com/crankyoldgit/IRremoteESP8266) supports an [impressive set of protocols](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md).
- Sparkfun Pro Micro
- Nano Every, Uno WiFi Rev2, nRF5 BBC MicroBit, Nano33_BLE

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.

# Timer and pin usage
The receiver sample interval is generated by a timer. On many boards this must be a hardware timer, on some a software timer is available and used. The code for the timer and the timer selection is located in [private/IRTimer.cpp.h](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/private/IRTimer.cpp.h).<br/>
Every pin can be used for receiving.<br/>
The send PWM signal is by default generated by software. **Therefore every pin can be used for sending**. The PWM pulse length is guaranteed to be constant by using `delayMicroseconds()`. Take care not to generate interrupts during sending with software generated PWM, otherwise you will get jitter in the generated PWM. E.g. wait for a former `Serial.print()` statement to be finished by `Serial.flush()`. Since the Arduino `micros()` function has a resolution of 4 us at 16 MHz, we always see a small jitter in the signal, which seems to be OK for the receivers.

| Software generated PWM showing small jitter because of the limited resolution of 4 us of the Arduino core `micros()` function for an ATmega328 | Detail (ATmega328 generated) showing 33% Duty cycle |
|-|-|
| ![Software PWM](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/IR_PWM_by_software_jitter.png) | ![Software PWM detail](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/IR_PWM_by_software_detail.png) |

## Hardware-PWM signal generation for sending
If you define `SEND_PWM_BY_TIMER`, the send PWM signal is generated by a hardware timer. The same timer as for the receiver is used.
Since each hardware timer has its dedicated output pins, you must change timer to change PWN output.<br/>
The timer and the pin usage can be adjusted in [private/IRTimer.cpp.h](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/private/IRTimer.cpp.h)

| Board/CPU                                                                | Hardware-PWM Pin    | Timers            |
|--------------------------------------------------------------------------|---------------------|-------------------|
| [ATtiny84](https://github.com/SpenceKonde/ATTinyCore)                    | **6**               | **1**             |
| [ATtiny85 > 1 MHz](https://github.com/SpenceKonde/ATTinyCore)            | **1**, 4            | **0**, 1          |
| [ATtiny1604](https://github.com/SpenceKonde/megaTinyCore)                | **PA05**            | **TCB0**          |
| [ATmega8](https://github.com/MCUdude/MiniCore)                           | **9**               | **1**             |
| [ATmega48, ATmega88, ATmega168, **ATmega328**](https://github.com/MCUdude/MiniCore) | **3**, 9 | 1, **2**          |
| [ATmega1284](https://github.com/MCUdude/MightyCore)                      | 13, 14, 6           | 1, **2**, 3       |
| [ATmega164, ATmega324, ATmega644](https://github.com/MCUdude/MightyCore) | 13, **14**          | 1, **2**          |
| [ATmega8535 ATmega16, ATmega32](https://github.com/MCUdude/MightyCore)   | **13**              | **1**             |
| [ATmega64, ATmega128, ATmega1281, ATmega2561](https://github.com/MCUdude/MegaCore) | **13**    | **1**             |
| [ATmega8515, ATmega162](https://github.com/MCUdude/MajorCore)            | **13**              | **1**             |
| ATmega1280, ATmega2560                                                   | 5, 6, **9**, 11, 46 | 1, **2**, 3, 4, 5 |
| ATmega4809                                                               | **A4**              | **TCB0**          |
| Leonardo (Atmega32u4)                                                    | 5, **9**, 13        | 1, 3, **4_HS**    |
| Zero (SAMD)                                                              | \*, **9**           | **TC3**           |
| [ESP32](http://esp32.net/)                                               | **4**, all pins     | **1**             |
| [Sparkfun Pro Micro](https://www.sparkfun.com/products/12640)            | **5**, 9            | 1, **3**          |
| [Teensy 1.0](https://www.pjrc.com/teensy/pinout.html)                    | **17**              | **1**             |
| [Teensy 2.0](https://www.pjrc.com/teensy/pinout.html)                    | **9**, 10, 14       | 1, **3**, 4_HS    |
| [Teensy++ 1.0 / 2.0](https://www.pjrc.com/teensy/pinout.html)            | **1**, 16, 25       | 1, **2**, 3       |
| [Teensy 3.0 / 3.1](https://www.pjrc.com/teensy/pinout.html)              | **5**               | **CMT**           |
| [Teensy-LC](https://www.pjrc.com/teensy/pinout.html)                     | **16**              | **TPM1**          |

# Adding new protocols
To add a new protocol is quite straightforward. Best is too look at the existing protocols to find a similar one and modify it.<br/>
As a rule of thumb, it is easier to work with a description of the protocol rather than trying to entirely reverse-engineer the protocol.
Please include a link to the description in the header, if you found one.<br/>
The **durations** you receive are likely to be longer for marks and shorter for spaces than the protocol suggests,
but this depends on the receiver circuit in use. Most protocols use multiples of one time-unit for marks and spaces like e.g. [NEC](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.cpp#L50). It's easy to be off-by-one with the last bit, since the last space is not recorded by IRremote.

Try to make use of the template functions `decodePulseDistanceData()` and `sendPulseDistanceData()`.
If your protocol supports address and code fields, try to reflect this in your api like it is done in [`sendNEC(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat)`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.cpp#L86) and [`decodeNEC()`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.cpp#L145).<br/>

### Integration
To integrate your protocol, you need to extend the two functions `decode()` and `getProtocolString()` in *IRreceice.cpp*,
add macros and function declarations for sending and receiving and extend the `enum decode_type_t` in *IRremote.h*.<br/>
And at least it would be wonderful if you can provide an example how to use the new protocol.
A detailed description can be found in the [ir_Template.cpp](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_Template.cpp#L18) file.

# NEC encoding
8 bit address NEC code
![8 bit address NEC code](https://user-images.githubusercontent.com/6750655/108884951-78e42b80-7607-11eb-9513-b07173a169c0.png)
16 bit address NEC code
![16 bit address NEC code](https://user-images.githubusercontent.com/6750655/108885081-a6c97000-7607-11eb-8d35-274a7065b6c4.png)

# Revision History
Please see [changelog.md](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/changelog.md).

# API documentation
See [API reference in wiki](https://github.com/Arduino-IRremote/Arduino-IRremote/wiki/API-Reference).

To generate the API documentation,
Doxygen, as well as [Graphviz](http://www.graphviz.org/) should be installed.
(Note that on Windows, it is useful to specify the installer to add Graphviz to PATH or to do it manually.
With Doxygen and Graphviz installed, issue the command
`doxygen` from the command line in the main project directory, which will
generate the API documentation in HTML format.
The just generated `docs/index.html` can now be opened in a browser.

## Why do we use 33% duty cycle
We do it according to the statement in the [Vishay datasheet](https://www.vishay.com/docs/80069/circuit.pdf):
- Carrier duty cycle 50 %, peak current of emitter IF = 200 mA, the resulting transmission distance is 25 m.
- Carrier duty cycle 10 %, peak current of emitter IF = 800 mA, the resulting transmission distance is 29 m. - Factor 1.16
The reason is, that it is not the pure energy of the fundamental which is responsible for the receiver to detect a signal.
Due to automatic gain control and other bias effects high intensity and lower energy (duty cycle) of the 38 kHz pulse counts more than high low intensity and higher energy.

BTW, **the best way to increase the IR power** is to use 2 or 3 IR diodes in series. One diode requires 1.1 to 1.5 volt so you can supply 3 diodes with a 5 volt output.<br/>
To keep the current, you must reduce the resistor by (5 - 1.3) / (5 - 2.6) = 1.5 e.g. from 150 ohm to 100 ohm for 25 mA and 2 diodes with 1.3 volt and a 5 volt supply.<br/>
For 3 diodes it requires factor 2.5 e.g. from 150 ohm to 60 ohm.

# Quick comparison of 4 Arduino IR receiving libraries
[Here](https://github.com/crankyoldgit/IRremoteESP8266) you find an **ESP8266/ESP32** version of IRremote with an **[impressive list of supported protocols](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md)**.

## This is a short comparison and may not be complete or correct
I created this comparison matrix for [myself](https://github.com/ArminJo) in order to choose a small IR lib for my project and to have a quick overview, when to choose which library.<br/>
It is dated from **03.02.2021**. If you have complains about the data or request for extensions, please send a PM or open a discussion.

| Subject | [IRMP](https://github.com/ukw100/IRMP) | [IRLremote](https://github.com/NicoHood/IRLremote) | [IRLib2](https://github.com/cyborg5/IRLib2)<br/>**mostly unmaintained** | [IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) | [Minimal NEC](https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/examples/MinimalReceiver) |
|---------|------|-----------|--------|----------|----------|
| Number of protocols | **50** | Nec + Panasonic + Hash \* | 12 + Hash \* | 17 + Hash \* | NEC |
| 3.Party libs needed| % | PinChangeInterrupt if not pin 2 or 3 | % | % | % |
| Timing method receive | Timer2 or interrupt for pin 2 or 3 | **Interrupt** | Timer2 or interrupt for pin 2 or 3 | Timer2 or interrupt for NEC | **Interrupt** |
| Timing method send | PWM and timing with Timer2 interrupts | Timer2 interrupts | Timer2 and blocking wait | PWM with Timer2 and blocking wait with delayMicroseconds() | % |
| Send pins| All | All | All ? | Timer dependent | % |
| Decode method | OnTheFly | OnTheFly | RAM | RAM | OnTheFly |
| Encode method | OnTheFly | OnTheFly | OnTheFly | OnTheFly or RAM | % |
| Callback suppport | x | % | % | % | x |
| Repeat handling | Receive + Send (partially) | % | ? | Receive + Send | x |
| LED feedback | x | % | x | x | x |
| FLASH usage (simple NEC example with 5 prints) | 1820<br/>(4300 for 15 main / 8000 for all 40 protocols)<br/>(+200 for callback)<br/>(+80 for interrupt at pin 2+3)| 1270<br/>(1400 for pin 2+3) | 4830 | 1770 | **900** |
| RAM usage | 52<br/>(73 / 100 for 15 (main) / 40 protocols) | 62 | 334 | 227 | **19** |
| Supported platforms | **avr, megaAVR, attiny, Digispark (Pro), esp8266, ESP32, STM32, SAMD 21, Apollo3<br/>(plus arm and pic for non Arduino IDE)** | avr, esp8266 | avr, SAMD 21, SAMD 51 | avr, attiny, [esp8266](https://github.com/crankyoldgit/IRremoteESP8266), esp32, SAM, SAMD | **All platforms with attachInterrupt()** |
| Last library update | 2/2021 | 4/2018 | 9/2019 | 2/2021 | 2/2021 |
| Remarks | Decodes 40 protocols concurrently.<br/>39 Protocols to send.<br/>Work in progress. | Only one protocol at a time. | Consists of 5 libraries. **Project containing bugs - 45 issues, no reaction for at least one year.** | Decoding and sending are easy to extend.<br/>Supports **Pronto** codes. | Requires no timer. |

\* The Hash protocol gives you a hash as code, which may be sufficient to distinguish your keys on the remote, but may not work with some protocols like Mitsubishi


# Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library
- Contribute new protocols

Check [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/Contributing.md) for some guidelines.

## Contributors
Check [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/Contributors.md)

# Contact
Email: zetoslab@gmail.com
Please only email me if it is more appropriate than creating an Issue / PR. I **will** not respond to requests for adding support for particular boards, unless of course you are the creator of the board and would like to cooperate on the project. I will also **ignore** any emails asking me to tell you how to implement your ideas. However, if you have a private inquiry that you would only apply to you and you would prefer it to be via email, by all means.

# License
Up to the version 2.7.0 the License is GPLv2.
From the version 2.8.0 the license is the MIT license.

## Copyright
Initially coded 2009 Ken Shirriff http://www.righto.com
Copyright (c) 2016 Rafi Khan
Copyright (c) 2020-2021 Armin Joachimsmeyer
