<div align = center>

#  Arduino IRremote
A library enabling the sending & receiving of infra-red signals.

[![Badge License: MIT](https://img.shields.io/badge/License-MIT-ac8b11.svg?style=for-the-badge&labelColor=yellow)](https://opensource.org/licenses/MIT)
 &nbsp; &nbsp;
[![Badge Version](https://img.shields.io/github/v/release/Arduino-IRremote/Arduino-IRremote?include_prereleases&style=for-the-badge&color=33660e&labelColor=428813&logoColor=white&logo=DocuSign)](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/latest)
 &nbsp; &nbsp;
[![Badge Commits since latest](https://img.shields.io/github/commits-since/Arduino-IRremote/Arduino-IRremote/latest?style=for-the-badge&color=004463&labelColor=00557f)](https://github.com/Arduino-IRremote/Arduino-IRremote/commits/master)
 &nbsp; &nbsp;
[![Badge LibraryBuild](https://img.shields.io/github/actions/workflow/status/Arduino-IRremote/Arduino-IRremote/LibraryBuild.yml?branch=master&style=for-the-badge&color=551f47&labelColor=752a61)](https://github.com/Arduino-IRremote/Arduino-IRremote/actions)
<br/>
<br/>
[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

Available as [Arduino library "IRremote"](https://www.arduinolibraries.info/libraries/i-rremote).

[![Button Install](https://img.shields.io/badge/Install-yellow?style=for-the-badge&logoColor=white&logo=GitBook)](https://www.ardu-badge.com/IRremote)
 &nbsp; &nbsp;
[![Button API](https://img.shields.io/badge/API-1c8840?style=for-the-badge&logoColor=white&logo=OpenStreetMap)](https://arduino-irremote.github.io/Arduino-IRremote/classIRrecv.html)
 &nbsp; &nbsp;
[![Button Changelog](https://img.shields.io/badge/Changelog-00557f?style=for-the-badge&logoColor=white&logo=AzureArtifacts)](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/changelog.md)
 &nbsp; &nbsp;
[![Button Contribute](https://img.shields.io/badge/Contribute-752a61?style=for-the-badge&logoColor=white&logo=GitHub)](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/Contributing.md)

</div>

# Overview
- [Supported IR Protocols](https://github.com/Arduino-IRremote/Arduino-IRremote#supported-ir-protocols)
- [Features](https://github.com/Arduino-IRremote/Arduino-IRremote#features)
  * [New features with version 4.x](https://github.com/Arduino-IRremote/Arduino-IRremote#new-features-with-version-4x)
  * [New features with version 3.x](https://github.com/Arduino-IRremote/Arduino-IRremote#new-features-with-version-3x)
- [Converting your 2.x program to the 4.x version](https://github.com/Arduino-IRremote/Arduino-IRremote#converting-your-2x-program-to-the-4x-version)
  * [How to convert old MSB first 32 bit IR data codes to new LSB first 32 bit IR data codes](https://github.com/Arduino-IRremote/Arduino-IRremote#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes)
-  [Errors with using the 3.x versions for old tutorials](https://github.com/Arduino-IRremote/Arduino-IRremote#errors-with-using-the-3x-versions-for-old-tutorials)
  * [Staying on 2.x](https://github.com/Arduino-IRremote/Arduino-IRremote#staying-on-2x)
- [Why *.hpp instead of *.cpp](https://github.com/Arduino-IRremote/Arduino-IRremote#why-hpp-instead-of-cpp)
- [Using the new *.hpp files](https://github.com/Arduino-IRremote/Arduino-IRremote#using-the-new-hpp-files)
- [Receiving IR codes](https://github.com/Arduino-IRremote/Arduino-IRremote#receiving-ir-codes)
  * [Data format](https://github.com/Arduino-IRremote/Arduino-IRremote#data-format)
  * [Ambiguous protocols](https://github.com/Arduino-IRremote/Arduino-IRremote#ambiguous-protocols)
- [Sending IR codes](https://github.com/Arduino-IRremote/Arduino-IRremote#sending-ir-codes)
  * [Send pin](https://github.com/Arduino-IRremote/Arduino-IRremote#send-pin)
    + [List of public IR code databases](https://github.com/Arduino-IRremote/Arduino-IRremote#list-of-public-ir-code-databases)
- [Tiny NEC receiver and sender](https://github.com/Arduino-IRremote/Arduino-IRremote#tiny-nec-receiver-and-sender)
- [The FAST protocol](https://github.com/Arduino-IRremote/Arduino-IRremote#the-fast-protocol)
- [FAQ and hints](https://github.com/Arduino-IRremote/Arduino-IRremote#faq-and-hints)
  * [Problems with Neopixels, FastLed etc.](https://github.com/Arduino-IRremote/Arduino-IRremote#problems-with-neopixels-fastled-etc)
  * [Does not work/compile with another library](https://github.com/Arduino-IRremote/Arduino-IRremote#does-not-workcompile-with-another-library)
  * [Multiple IR receiver](https://github.com/Arduino-IRremote/Arduino-IRremote#multiple-ir-receiver)
  * [Increase strength of sent output signal](https://github.com/Arduino-IRremote/Arduino-IRremote#increase-strength-of-sent-output-signal)
  * [Minimal CPU clock frequency](https://github.com/Arduino-IRremote/Arduino-IRremote#minimal-cpu-clock-frequency)
  * [Bang & Olufsen protocol](https://github.com/Arduino-IRremote/Arduino-IRremote#bang--olufsen-protocol)
- [Handling unknown Protocols](https://github.com/Arduino-IRremote/Arduino-IRremote#handling-unknown-protocols)
  * [Disclaimer](https://github.com/Arduino-IRremote/Arduino-IRremote#disclaimer)
  * [Protocol=PULSE_DISTANCE](https://github.com/Arduino-IRremote/Arduino-IRremote#protocolpulse_distance)
  * [Protocol=UNKNOWN](https://github.com/Arduino-IRremote/Arduino-IRremote#protocolunknown)
  * [How to deal with protocols not supported by IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote#how-to-deal-with-protocols-not-supported-by-irremote)
- [Examples for this library](https://github.com/Arduino-IRremote/Arduino-IRremote#examples-for-this-library)
- [WOKWI online examples](https://github.com/Arduino-IRremote/Arduino-IRremote#wokwi-online-examples)
- [Issues and discussions](https://github.com/Arduino-IRremote/Arduino-IRremote#issues-and-discussions)
- [Compile options / macros for this library](https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library)
    + [Changing include (*.h) files with Arduino IDE](https://github.com/Arduino-IRremote/Arduino-IRremote#changing-include-h-files-with-arduino-ide)
    + [Modifying compile options with Sloeber IDE](https://github.com/Arduino-IRremote/Arduino-IRremote#modifying-compile-options--macros-with-sloeber-ide)
- [Supported Boards](https://github.com/Arduino-IRremote/Arduino-IRremote#supported-boards)
- [Timer and pin usage](https://github.com/Arduino-IRremote/Arduino-IRremote#timer-and-pin-usage)
  * [Incompatibilities to other libraries and Arduino commands like tone() and analogWrite()](https://github.com/Arduino-IRremote/Arduino-IRremote#incompatibilities-to-other-libraries-and-arduino-commands-like-tone-and-analogwrite)
  * [Hardware-PWM signal generation for sending](https://github.com/Arduino-IRremote/Arduino-IRremote#hardware-pwm-signal-generation-for-sending)
  * [Why do we use 30% duty cycle for sending](https://github.com/Arduino-IRremote/Arduino-IRremote#why-do-we-use-30-duty-cycle-for-sending)
- [How we decode signals](https://github.com/Arduino-IRremote/Arduino-IRremote#how-we-decode-signals)
- [NEC encoding diagrams](https://github.com/Arduino-IRremote/Arduino-IRremote#nec-encoding-diagrams)
- [Quick comparison of 5 Arduino IR receiving libraries](https://github.com/Arduino-IRremote/Arduino-IRremote#quick-comparison-of-5-arduino-ir-receiving-libraries)
- [Useful links](https://github.com/Arduino-IRremote/Arduino-IRremote#useful-links)
- [Contributors](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/Contributors.md)
- [License](https://github.com/Arduino-IRremote/Arduino-IRremote#license)
- [Copyright](https://github.com/Arduino-IRremote/Arduino-IRremote#copyright)

<br/>

# Supported IR Protocols
` NEC / Onkyo / Apple ` &nbsp; &nbsp; ` Denon / Sharp ` &nbsp; &nbsp; ` Panasonic / Kaseikyo `

` JVC ` &nbsp; &nbsp; ` LG ` &nbsp; &nbsp; ` RC5 ` &nbsp; &nbsp; ` RC6 ` &nbsp; &nbsp; ` Samsung ` &nbsp; &nbsp; ` Sony `

` Universal Pulse Distance ` &nbsp; &nbsp; ` Universal Pulse Width ` &nbsp; &nbsp; ` Hash ` &nbsp; &nbsp; ` Pronto `

` BoseWave ` &nbsp; &nbsp; ` Bang & Olufsen ` &nbsp; &nbsp; ` Lego ` &nbsp; &nbsp; ` FAST ` &nbsp; &nbsp; ` Whynter ` &nbsp; &nbsp; ` MagiQuest `

Protocols can be switched off and on by defining macros before the line `#include <IRremote.hpp>` like [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino#L33):

```c++
#define DECODE_NEC
//#define DECODE_DENON
#include <IRremote.hpp>
```
<br/>

# Features
- Lots of tutorials and examples.
- Actively maintained.
- Allows receiving and sending of **raw timing data**.

## New features with version 4.x
- New universal **Pulse Distance / Pulse Width decoder** added, which covers many previous unknown protocols.
- Printout of code how to send received command by `IrReceiver.printIRSendUsage(&Serial)`.
- RawData type is now 64 bit for 32 bit platforms and therefore `decodedIRData.decodedRawData` can contain complete frame information for more protocols than with 32 bit as before.
- Callback after receiving a command - It calls your code as soon as a message was received.
- Improved handling of `PULSE_DISTANCE` + `PULSE_WIDTH` protocols.
- New FAST protocol.

#### Converting your 3.x program to the 4.x version
- You must replace `#define DECODE_DISTANCE` by `#define DECODE_DISTANCE_WIDTH` (only if you explicitly enabled this decoder).
- The parameter `bool hasStopBit` is not longer required and removed e.g. for function `sendPulseDistanceWidth()`.

## New features with version 3.x
- **Any pin** can be used for sending -if `SEND_PWM_BY_TIMER` is not defined- and receiving.
- Feedback LED can be activated for sending / receiving.
- An 8/16 bit ****command** value as well as an 16 bit **address** and a protocol number is provided for decoding (instead of the old 32 bit value).
- Protocol values comply to **protocol standards**.<br/>
  NEC, Panasonic, Sony, Samsung and JVC decode & send LSB first.
- Supports **Universal Distance protocol**, which covers a lot of previous unknown protocols.
- Compatible with **tone()** library. See the [ReceiveDemo](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/21b5747a58e9d47c9e3f1beb056d58c875a92b47/examples/ReceiveDemo/ReceiveDemo.ino#L159-L169) example.
- Simultaneous sending and receiving. See the [SendAndReceive](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SendAndReceive/SendAndReceive.ino#L167-L170) example.
- Supports **more platforms**.
- Allows for the generation of non PWM signal to just **simulate an active low receiver signal** for direct connect to existent receiving devices without using IR.
- Easy protocol configuration, **directly in your [source code](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino#L33-L57)**.<br/>
  Reduces memory footprint and decreases decoding time.
- Contains a [very small NEC only decoder](https://github.com/Arduino-IRremote/Arduino-IRremote#minimal-nec-receiver), which **does not require any timer resource**.

[-> Feature comparison of 5 Arduino IR libraries](https://github.com/Arduino-IRremote/Arduino-IRremote#quick-comparison-of-5-arduino-ir-receiving-libraries).

<br/>

# Converting your 2.x program to the 4.x version
Starting with the 3.1 version, **the generation of PWM for sending is done by software**, thus saving the hardware timer and **enabling arbitrary output pins for sending**.<br/>
If you use an (old) Arduino core that does not use the `-flto` flag for compile, you can activate the line `#define SUPPRESS_ERROR_MESSAGE_FOR_BEGIN` in IRRemote.h, if you get false error messages regarding begin() during compilation.

- **IRreceiver** and **IRsender** object have been added and can be used without defining them, like the well known Arduino **Serial** object.
- Just remove the line `IRrecv IrReceiver(IR_RECEIVE_PIN);` and/or `IRsend IrSender;` in your program, and replace all occurrences of `IRrecv.` or `irrecv.` with `IrReceiver` and replace all `IRsend` or `irsend` with `IrSender`.
- Since the decoded values are now in `IrReceiver.decodedIRData` and not in `results` any more, remove the line `decode_results results` or similar.
- Like for the Serial object, call [`IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK)`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino#L106)
 or `IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK)` instead of the `IrReceiver.enableIRIn()` or `irrecv.enableIRIn()` in setup().<br/>
For sending, call `IrSender.begin(ENABLE_LED_FEEDBACK);` or `IrSender.begin(DISABLE_LED_FEEDBACK);` in setup().<br/>
If IR_SEND_PIN is not defined you must use e.g. `IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);`
- Old `decode(decode_results *aResults)` function is replaced by simple `decode()`. So if you have a statement `if(irrecv.decode(&results))` replace it with `if (IrReceiver.decode())`.
- The decoded result is now in in `IrReceiver.decodedIRData` and not in `results` any more, therefore replace any occurrences of `results.value` and `results.decode_type` (and similar) to
 `IrReceiver.decodedIRData.decodedRawData` and `IrReceiver.decodedIRData.protocol`.
- Overflow, Repeat and other flags are now in [`IrReceiver.receivedIRData.flags`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRProtocol.h#L90-L101).
- Seldom used: `results.rawbuf` and `results.rawlen` must be replaced by `IrReceiver.decodedIRData.rawDataPtr->rawbuf` and `IrReceiver.decodedIRData.rawDataPtr->rawlen`.

- The 5 protocols **NEC, Panasonic, Sony, Samsung and JVC** have been converted to LSB first. Send functions for sending old MSB data for **NEC** and **JVC** were renamed to `sendNECMSB`, and `sendJVCMSB()`. The old  `sendSAMSUNG()` and `sendSony()` MSB functions are still available. The old MSB version of `sendPanasonic()` function was deleted, since it had bugs nobody recognized.<br/>
For converting MSB codes to LSB see [below](https://github.com/Arduino-IRremote/Arduino-IRremote#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes).

### Example
#### Old 2.x program:

```c++
#include <IRremote.h>

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

#### New 4.x program:

```c++
#include <IRremote.hpp>

void setup()
{
...
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
}

void loop() {
  if (IrReceiver.decode()) {
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
      // USE NEW 3.x FUNCTIONS
      IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
      IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
      ...
      IrReceiver.resume(); // Enable receiving of the next value
  }
  ...
}
```

## How to convert old MSB first 32 bit IR data codes to new LSB first 32 bit IR data codes
For the new decoders for **NEC, Panasonic, Sony, Samsung and JVC**, the result `IrReceiver.decodedIRData.decodedRawData` is now **LSB-first**, as the definition of these protocols suggests!<br/>
<br/>
To convert one into the other, you must reverse the byte/nibble positions and then reverse all bit positions of each byte/nibble or write it as one binary string and reverse/mirror it.<br/><br/>
Example:
`0xCB 34 01 02`<br/>
`0x20 10 43 BC` after nibble reverse<br/>
`0x40 80 2C D3` after bit reverse of each nibble<br/><br/>
### Nibble reverse map:
```
 0->0   1->8   2->4   3->C
 4->2   5->A   6->6   7->E
 8->1   9->9   A->5   B->D
 C->3   D->B   E->7   F->F
```
`0xCB340102` is binary `1100 1011 0011 0100 0000 0001 0000 0010`.<br/>
`0x40802CD3` is binary `0100 0000 1000 0000 0010 1100 1101 0011`.<br/>
If you **read the first binary sequence backwards** (right to left), you get the second sequence.

<br/>

# Errors with using the 4.x versions for old tutorials
If you suffer from errors with old tutorial code including `IRremote.h` instead of `IRremote.hpp`, just try to rollback to [Version 2.4.0](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/v2.4.0).<br/>
Most likely your code will run and you will not miss the new features...

<br/>

## Staying on 2.x
Consider using the [original 2.4 release form 2017](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/v2.4.0)
or the last backwards compatible [2.8 version](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/2.8.0) for you project.<br/>
It may be sufficient and deals flawlessly with 32 bit IR codes.<br/>
If this doesn't fit your case, be assured that 3.x is at least trying to be backwards compatible, so your old examples should still work fine.

### Drawbacks
- Only the following decoders are available:<br/>
  ` NEC ` &nbsp; &nbsp; ` Denon ` &nbsp; &nbsp; ` Panasonic ` &nbsp; &nbsp; ` JVC ` &nbsp; &nbsp; ` LG `<br/>
  ` RC5 ` &nbsp; &nbsp; ` RC6 ` &nbsp; &nbsp; ` Samsung ` &nbsp; &nbsp; ` Sony `
- The call of `irrecv.decode(&results)` uses the old MSB first decoders like in 2.x and sets the 32 bit codes in `results.value`.
- The old functions `sendNEC()` and `sendJVC()` are renamed to `sendNECMSB()` and `sendJVCMSB()`.<br/>
  Use them to send your **old MSB-first 32 bit IR data codes**.
- No decoding by a (constant) 8/16 bit address and an 8 bit command.

<br/>

# Why *.hpp instead of *.cpp?
**Every \*.cpp file is compiled separately** by a call of the compiler exclusively for this cpp file. These calls are managed by the IDE / make system.
In the Arduino IDE the calls are executed when you click on *Verify* or *Upload*.

And now our problem with Arduino is:<br/>
**How to set [compile options](#compile-options--macros-for-this-library) for all *.cpp files, especially for libraries used?**<br/>
IDE's like [Sloeber](https://github.com/ArminJo/ServoEasing#modifying-compile-options--macros-with-sloeber-ide) or [PlatformIO](https://github.com/ArminJo/ServoEasing#modifying-compile-options--macros-with-platformio) support this by allowing to specify a set of options per project.
They add these options at each compiler call e.g. `-DTRACE`.

But Arduino lacks this feature.
So the **workaround** is not to compile all sources separately, but to concatenate them to one huge source file by including them in your source.<br/>
This is done by e.g. `#include "IRremote.hpp"`.

But why not `#include "IRremote.cpp"`?<br/>
Try it and you will see tons of errors, because each function of the *.cpp file is now compiled twice,
first by compiling the huge file and second by compiling the *.cpp file separately, like described above.<br/>
So using the extension *cpp* is not longer possible, and one solution is to use *hpp* as extension, to show that it is an included *.cpp file.<br/>
Every other extension e.g. *cinclude* would do, but *hpp* seems to be common sense.

# Using the new *.hpp files
In order to support [compile options](#compile-options--macros-for-this-library) more easily,
you must use the statement `#include <IRremote.hpp>` instead of `#include <IRremote.h>` in your main program (aka *.ino file with setup() and loop()).

In **all other files** you must use the following, to **prevent `multiple definitions` linker errors**:

```c++
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
```

**Ensure that all macros in your main program are defined before any** `#include <IRremote.hpp>`.<br/>
The following macros will definitely be overridden with default values otherwise:
- `RAW_BUFFER_LENGTH`
- `IR_SEND_PIN`
- `SEND_PWM_BY_TIMER`

<br/>

# Receiving IR codes
Check for **received data** with:<br/>
`if (IrReceiver.decode()) {}`<br/>
This also decodes the received data.

## Data format
After successful decoding, the IR data is contained in the IRData structure, available as `IrReceiver.decodedIRData`.

```c++
struct IRData {
    decode_type_t protocol;     // UNKNOWN, NEC, SONY, RC5, PULSE_DISTANCE, ...
    uint16_t address;           // Decoded address
    uint16_t command;           // Decoded command
    uint16_t extra;             // Used for Kaseikyo unknown vendor ID. Ticks used for decoding Distance protocol.
    uint16_t numberOfBits;      // Number of bits received for data (address + command + parity) - to determine protocol length if different length are possible.
    uint8_t flags;              // IRDATA_FLAGS_IS_REPEAT, IRDATA_FLAGS_WAS_OVERFLOW etc. See IRDATA_FLAGS_* definitions
    IRRawDataType decodedRawData;    // Up to 32 (64 bit for 32 bit CPU architectures) bit decoded raw data, used for sendRaw functions.
    uint32_t decodedRawDataArray[RAW_DATA_ARRAY_SIZE]; // 32 bit decoded raw data, to be used for send function.
    irparams_struct *rawDataPtr; // Pointer of the raw timing data to be decoded. Mainly the data buffer filled by receiving ISR.
};
```
#### Flags
This is the [list of flags](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRProtocol.h#L88) contained in the flags field.<br/>
Check it with e.g. `if(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)`.

| Flag name | Description |
|:---|----|
| IRDATA_FLAGS_IS_REPEAT | The gap between the preceding frame is as smaller than the maximum gap expected for a repeat. !!!We do not check for changed command or address, because it is almost not possible to press 2 different buttons on the remote within around 100 ms!!!
| IRDATA_FLAGS_IS_AUTO_REPEAT | The current repeat frame is a repeat, that is always sent after a regular frame and cannot be avoided. Only specified for protocols DENON, and LEGO. |
| IRDATA_FLAGS_PARITY_FAILED | The current (autorepeat) frame violated parity check. |
| IRDATA_FLAGS_TOGGLE_BIT | Is set if RC5 or RC6 toggle bit is set. |
| IRDATA_FLAGS_EXTRA_INFO | There is extra info not contained in address and data (e.g. Kaseikyo unknown vendor ID, or in decodedRawDataArray). |
| IRDATA_FLAGS_WAS_OVERFLOW | irparams.rawlen is set to 0 in this case to avoid endless OverflowFlag. |
| IRDATA_FLAGS_IS_MSB_FIRST | This value is mainly determined by the (known) protocol. |

#### To access the **RAW data**, use:
```c++
auto myRawdata= IrReceiver.decodedIRData.decodedRawData;
```

The definitions for the `IrReceiver.decodedIRData.flags` are described [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremoteInt.h#L128-L140).

#### Print all fields:
```c++
IrReceiver.printIRResultShort(&Serial);
```

#### Print the raw timing data received:
```c++
IrReceiver.printIRResultRawFormatted(&Serial, true);`
```
The raw data depends on the internal state of the Arduino timer in relation to the received signal and might therefore be slightly different each time. (resolution problem). The decoded values are the interpreted ones which are tolerant to such slight differences!

#### Print how to send the received data:
```c++
IrReceiver.printIRSendUsage(&Serial);
```

## Ambiguous protocols
### NEC, Extended NEC, ONKYO
The **NEC protocol** is defined as 8 bit address and 8 bit command. But the physical address and data fields are each 16 bit wide.
The additional 8 bits are used to send the inverted address or command for parity checking.<br/>
The **extended NEC protocol** uses the additional 8 parity bit of address for a 16 bit address, thus disabling the parity check for address.<br/>
The **ONKYO protocol** in turn uses the additional 8 parity bit of address and command for a 16 bit address and command.

The decoder reduces the 16 bit values to 8 bit ones if the parity is correct.
If the parity is not correct, it assumes no parity error, but takes the values as 16 bit values without parity assuming extended NEC or extended NEC protocol protocol.

But now we have a problem when we want to receive e.g. the **16 bit** address 0x00FF or 0x32CD!
The decoder interprets this as a NEC 8 bit address 0x00 / 0x32 with correct parity of 0xFF / 0xCD and reduces it to 0x00 / 0x32.

One way to handle this, is to force the library to **always** use the ONKYO protocol interpretation by using `#define DECODE_ONKYO`.
Another way is to check if `IrReceiver.decodedIRData.protocol` is NEC and not ONKYO and to revert the parity reducing manually.

### NEC, NEC2
On a long press, the **NEC protocol** does not repeat its frame, it sends a special short repeat frame.
This enables an easy distinction between long presses and repeated presses and saves a bit of battery energy.
This behavior is quite unique for NEC and its derived protocols like LG.

So there are of course also remote control systems, which uses the NEC protocol but on a long press just repeat the first frame instead of sending the special short repeat frame. We named this the  **NEC2** protocol and it is sent with `sendNEC2()`.<br/>
But be careful, the NEC2 protocol can only be detected by the NEC library decoder **after** the first frame and if you do a long press!

<br/>

# Sending IR codes
If you have a device at hand which can generate the IR codes you want to work with (aka IR remote), **it is recommended** to receive the codes with the [ReceiveDemo example](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino), which will tell you on the serial output how to send them.

```
Protocol=LG Address=0x2 Command=0x3434 Raw-Data=0x23434E 28 bits MSB first
Send with: IrSender.sendLG(0x2, 0x3434, <numberOfRepeats>);
```
You will discover that **the address is a constant** and the commands sometimes are sensibly grouped.<br/>
If you are uncertain about the numbers of repeats to use for sending, **3** is a good starting point. If this works, you can check lower values afterwards.

The codes found in the [irdb database](https://github.com/probonopd/irdb/tree/master/codes) specify  a **device**, a **subdevice** and a **function**. Most of the times, *device* and *subdevice* can be taken as upper and lower byte of the **address parameter** and *function* is the **command parameter** for the **new structured functions** with address, command and repeat-count parameters like e.g. `IrSender.sendNEC((device << 8) | subdevice, 0x19, 2)`.<br/>
An **exact mapping** can be found in the [IRP definition files for IR protocols](https://github.com/probonopd/MakeHex/tree/master/protocols). "D" and "S" denotes device and subdevice and "F" denotes the function.

**All sending functions support the sending of repeats** if sensible.
Repeat frames are sent at a fixed period determined by the protocol. e.g. 110 ms from start to start for NEC.<br/>
Keep in mind, that **there is no delay after the last sent mark**.
If you handle the sending of repeat frames by your own, you must insert sensible delays before the repeat frames to enable correct decoding.

The old send*Raw() functions for sending like e.g. `IrSender.sendNECRaw(0xE61957A8,2)` are kept for backward compatibility to **(old)** tutorials and unsupported as well as error prone.

## Send pin
Any pin can be choosen as send pin, because the PWM signal is generated by default with software bit banging, since `SEND_PWM_BY_TIMER` is not active.
If `IR_SEND_PIN` is specified (as constant), it reduces program size and improves send timing for AVR. If you want to use a variable to specify send pin e.g. with `setSendPin(uint8_t aSendPinNumber)`, you must disable this macro. Then you can change send pin at any time before sending an IR frame. See also [Compile options / macros for this library](https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library).

### List of public IR code databases
http://www.harctoolbox.org/IR-resources.html

<br/>


# Tiny NEC receiver and sender
For applications only requiring NEC or FAST -see below- protocol, there is a special receiver / sender included,<br/>
which has very **small code size of 500 bytes and does NOT require any timer**.

Check out the [TinyReceiver](https://github.com/Arduino-IRremote/Arduino-IRremote#tinyreceiver--tinysender) and [IRDispatcherDemo](https://github.com/Arduino-IRremote/Arduino-IRremote#irdispatcherdemo) examples.<br/>
Take care to include `TinyIRReceiver.hpp` or `TinyIRSender.hpp` instead of `IRremote.hpp`.

### TinyIRReceiver usage
```c++
//#define USE_ONKYO_PROTOCOL    // Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value.
//#define USE_FAST_PROTOCOL     // Use FAST protocol instead of NEC / ONKYO
#include "TinyIRReceiver.hpp"

void setup() {
  initPCIInterruptForTinyReceiver(); // Enables the interrupt generation on change of IR input signal
}

void loop() {}

// This is the function, which is called if a complete command was received
// It runs in an ISR context with interrupts enabled, so functions like delay() etc. should work here
void handleReceivedTinyIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags) {
  printTinyReceiverResultMinimal(&Serial, aAddress, aCommand, aFlags);
}
```

### TinyIRSender usage
```c++
#include "TinyIRSender.hpp"

void setup() {
  sendNECMinimal(3, 0, 11, 2); // Send address 0 and command 11 on pin 3 with 2 repeats.
}

void loop() {}
```

Another tiny receiver and sender **supporting more protocols** can be found [here](https://github.com/LuisMiCa/IRsmallDecoder).

# The FAST protocol
The FAST protocol is a proprietary modified JVC protocol **without address, with parity and with a shorter header**.
It is meant to have a quick response to the event which sent the protocol frame on another board.
FAST takes **21 ms for sending** and is sent at a **50 ms period**.
It has full 8 bit parity for error detection.

### FAST protocol characteristics:
- Bit timing is like JVC
- The header is shorter, 3156 &micro;s vs. 12500 &micro;s
- No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command, leading to a fixed protocol length of (6 + (16 * 3) + 1) * 526 = 55 * 526 = 28930 microseconds or 29 ms.
- Repeats are sent as complete frames but in a 50 ms period / with a 21 ms distance.

### Sending FAST protocol with IRremote
```c++
#define IR_SEND_PIN 3
#include <IRremote.hpp>

void setup() {
  sendFAST(11, 2); // Send command 11 on pin 3 with 2 repeats.
}

void loop() {}
```

### Sending FAST protocol with TinyIRSender
```c++
#define USE_FAST_PROTOCOL // Use FAST protocol. No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command
#include "TinyIRSender.hpp"

void setup() {
  sendFAST(3, 11, 2); // Send command 11 on pin 3 with 2 repeats.
}

void loop() {}
```
<br/>

The FAST protocol can be received by IRremote and TinyIRReceiver.

# FAQ and hints

## Problems with Neopixels, FastLed etc.
IRremote will not work right when you use **Neopixels** (aka WS2811/WS2812/WS2812B) or other libraries blocking interrupts for a longer time (> 50 &micro;s).<br/>
Whether you use the Adafruit Neopixel lib, or FastLED, interrupts get disabled on many lower end CPUs like the basic Arduinos for longer than 50 &micro;s.
In turn, this stops the IR interrupt handler from running when it needs to. See also this [video](https://www.youtube.com/watch?v=62-nEJtm070).

One **workaround** is to wait for the IR receiver to be idle before you send the Neopixel data with `if (IrReceiver.isIdle()) { strip.show();}`.<br/>
This **prevents at least breaking a running IR transmission** and -depending of the update rate of the Neopixel- may work quite well.<br/>
There are some other solutions to this on more powerful processors,
[see this page from Marc MERLIN](http://marc.merlins.org/perso/arduino/post_2017-04-03_Arduino-328P-Uno-Teensy3_1-ESP8266-ESP32-IR-and-Neopixels.html)

## Does not work/compile with another library
**Another library is only working/compiling** if you deactivate the line `IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);`.<br/>
This is often due to **timer resource conflicts** with the other library. Please see [below](https://github.com/Arduino-IRremote/Arduino-IRremote#timer-and-pin-usage).

## Multiple IR receivers
IRreceiver consists of one timer triggered function reading the digital IR signal value from one pin every 50 &micro;s.<br/>
So **multiple IR receivers** can only be used by connecting the output pins of several IR receivers together.
The IR receivers use an NPN transistor as output device with just a 30k resistor to VCC.
This is almost "open collector" and allows connecting of several output pins to one Arduino input pin.<br/>
But keep in mind, that any weak / disturbed signal from one of the receivers will in turn also disturb a good signal from another one.

## Increase strength of sent output signal
**The best way to increase the IR power for free** is to use 2 or 3 IR diodes in series. One diode requires 1.2 volt at 20 mA or 1.5 volt at 100 mA so you can supply up to 3 diodes with a 5 volt output.<br/>
To power **2 diodes** with 1.2 V and 20 mA and a 5 V supply, set the resistor to: (5 V - 2.4 V) -> 2.6 V / 20 mA = **130 &ohm;**.<br/>
For **3 diodes** it requires 1.4 V / 20 mA = **70 &ohm;**.<br/>
The actual current might be lower since of **loss at the AVR pin**. E.g. 0.3 V at 20 mA.<br/>
If you do not require more current than 20 mA, there is no need to use an external transistor (at least for AVR chips).

On my Arduino Nanos, I always use a 100 &ohm; series resistor and one IR LED :grinning:.

## Minimal CPU clock frequency
For receiving, the **minimal CPU clock frequency is 4 MHz**, since the 50 &micro;s timer ISR (Interrupt Service Routine) takes around 12 &micro;s on a 16 MHz ATmega.<br/>
The TinyReceiver, which reqires no polling, runs with 1 MHz.<br/>
For sending, the **default software generated PWM has problems on AVR running with 8 MHz**. The PWM frequency is around 30 instead of 38 kHz and RC6 is not reliable. You can switch to timer PWM generation by `#define SEND_PWM_BY_TIMER`.

## Bang & Olufsen protocol
The Bang & Olufsen protocol decoder is not enabled by default, i.e if no protocol is enabled explicitly by #define `DECODE_<XYZ>`. It must always be enabled explicitly by `#define DECODE_BEO`.
This is because it has an **IR transmit frequency of 455 kHz** and therefore requires a different receiver hardware (TSOP7000).<br/>
And because **generating a 455 kHz PWM signal is currently only implemented for `SEND_PWM_BY_TIMER`**, sending only works if `SEND_PWM_BY_TIMER` or `USE_NO_SEND_PWM` is defined.<br/>
For more info, see [ir_BangOlufsen.hpp](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_BangOlufsen.hpp#L44).

# Handling unknown Protocols
## Disclaimer
**This library was designed to fit inside MCUs with relatively low levels of resources and was intended to work as a library together with other applications which also require some resources of the MCU to operate.**

For **air conditioners** [see this fork](https://github.com/crankyoldgit/IRremoteESP8266), which supports an impressive set of protocols and a lot of air conditioners.

For **long signals** see the blog entry: ["Recording long Infrared Remote control signals with Arduino"](https://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino).


## Protocol=PULSE_DISTANCE
If you get something like this:
```
PULSE_DISTANCE: HeaderMarkMicros=8900 HeaderSpaceMicros=4450 MarkMicros=550 OneSpaceMicros=1700 ZeroSpaceMicros=600  NumberOfBits=56 0x43D8613C 0x3BC3BC
```
then you have a code consisting of **56 bits**, which is probably from an air conditioner remote.<br/>
You can send it with sendPulseDistance().
```c++
uint32_t tRawData[] = { 0xB02002, 0xA010 };
IrSender.sendPulseDistance(38, 3450, 1700, 450, 1250, 450, 400, &tRawData[0], 48, false, 0, 0);
```
You can send it with calling sendPulseDistanceWidthData() twice, once for the first 32 bit and next for the remaining 24 bits.<br/>
**The PulseDistance or PulseWidth decoders just decode a timing steam to a bit stream**.
They can not put any semantics like address, command or checksum on this bitstream, since it is no known protocol.
But the bitstream is way more readable, than a timing stream. This bitstream is read **LSB first by default**.
If this does not suit you for further research, you can change it [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_DistanceProtocol.hpp#L48).

## Protocol=UNKNOWN
If you see something like `Protocol=UNKNOWN Hash=0x13BD886C 35 bits received` as output of e.g. the ReceiveDemo example, you either have a problem with decoding a protocol, or an unsupported protocol.

- If you have an **odd number of bits** received, your receiver circuit probably has problems. Maybe because the IR signal is too weak.
- If you see timings like `+ 600,- 600     + 550,- 150     + 200,- 100     + 750,- 550` then one 450 &micro;s space was split into two 150 and 100 &micro;s spaces with a spike / error signal of 200 &micro;s between. Maybe because of a defective receiver or a weak signal in conjunction with another light emitting source nearby.
- If you see timings like `+ 500,- 550     + 450,- 550     + 500,- 500     + 500,-1550`, then marks are generally shorter than spaces and therefore `MARK_EXCESS_MICROS` (specified in your ino file) should be **negative** to compensate for this at decoding.
- If you see `Protocol=UNKNOWN Hash=0x0 1 bits received` it may be that the space after the initial mark is longer than [`RECORD_GAP_MICROS`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremote.h#L124).
  This was observed for some LG air conditioner protocols. Try again with a line e.g. `#define RECORD_GAP_MICROS 12000` before the line `#include <IRremote.hpp>` in your .ino file.
- To see more info supporting you to find the reason for your UNKNOWN protocol, you must enable the line `//#define DEBUG` in IRremoteInt.h.

## How to deal with protocols not supported by IRremote
If you do not know which protocol your IR transmitter uses, you have several choices.
- Use the [IRreceiveDump example](examples/ReceiveDump) to dump out the IR timing.
 You can then reproduce/send this timing with the [SendRawDemo example](examples/SendRawDemo).
 For **long codes** with more than 48 bits like from air conditioners, you can **change the length of the input buffer** in [IRremote.h](src/IRremoteInt.h#L36).
- The [IRMP AllProtocol example](https://github.com/IRMP-org/IRMP#allprotocol-example) prints the protocol and data for one of the **40 supported protocols**.
 The same library can be used to send this codes.
- If you have a bigger Arduino board at hand (> 100 kByte program memory) you can try the
 [IRremoteDecode example](https://github.com/bengtmartensson/Arduino-DecodeIR/blob/master/examples/IRremoteDecode/IRremoteDecode.ino) of the Arduino library [DecodeIR](https://github.com/bengtmartensson/Arduino-DecodeIR).
- Use [IrScrutinizer](http://www.harctoolbox.org/IrScrutinizer.html).
 It can automatically generate a send sketch for your protocol by exporting as "Arduino Raw". It supports IRremote,
 the old [IRLib](https://github.com/cyborg5/IRLib) and [Infrared4Arduino](https://github.com/bengtmartensson/Infrared4Arduino).

<br/>

# Examples for this library
The examples are available at File > Examples > Examples from Custom Libraries / IRremote.<br/>
 In order to fit the examples to the 8K flash of ATtiny85 and ATtiny88, the [Arduino library ATtinySerialOut](https://github.com/ArminJo/ATtinySerialOut) is required for this CPU's.

#### SimpleReceiver + SimpleSender
The **[SimpleReceiver](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino)**  and **[SimpleSender](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleSender/SimpleSender.ino)** examples are a good starting point.
A simple example can be tested online with [WOKWI](https://wokwi.com/projects/338611596994544210).

#### TinyReceiver + TinySender
If **code size** or **timer usage** matters, look at these examples.<br/>
The **[TinyReceiver](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/TinyReceiver/TinyReceiver.ino)** example uses the **TinyIRReceiver** library  which can **only receive NEC, ONKYO and FAST protocols, but does not require any timer**. They use pin change interrupt for on the fly decoding, which is the reason for the restricted protocol choice.<br/>
TinyReceiver can be tested online with [WOKWI](https://wokwi.com/arduino/projects/339264565653013075).

The **[TinySender](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/TinySender/TinySender.ino)** example uses the **TinyIRSender** library  which can **only send NEC, ONKYO and FAST protocols**.<br/>
It sends NEC protocol codes in standard format with 8 bit address and 8 bit command as in SimpleSender example.
Saves  780 bytes program memory and 26 bytes RAM compared to SimpleSender, which does the same, but uses the IRRemote library (and is therefore much more flexible).

#### SmallReceiver
If the protocol is not NEC and code size matters, look at this [example](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SmallReceiver/SmallReceiver.ino).<br/>

#### ReceiveDemo + AllProtocolsOnLCD
[ReceiveDemo](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino) receives all protocols and **generates a beep with the Arduino tone() function** on each packet received.<br/>
Long press of one IR button (receiving of multiple repeats for one command) is detected.<br/>
[AllProtocolsOnLCD](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/AllProtocolsOnLCD/AllProtocolsOnLCD.ino) additionally **displays the short result on a 1602 LCD**. The LCD can be connected parallel or serial (I2C).<br/>
By connecting debug pin to ground, you can force printing of the raw values for each frame. The pin number of the debug pin is printed during setup, because it depends on board and LCD connection type.<br/>
This example also serves as an **example how to use IRremote and tone() together**.

#### ReceiveDump
Receives all protocols and dumps the received signal in different flavors including Pronto format. Since the printing takes much time, repeat signals may be skipped or interpreted as UNKNOWN.

#### SendDemo
Sends all available protocols at least once.

#### SendAndReceive
Demonstrates **receiving while sending**.

#### ReceiveAndSend
Record and **play back last received IR signal** at button press. IR frames of known protocols are sent by the approriate protocol encoder. `UNKNOWN` protocol frames are stored as raw data and sent with `sendRaw()`.

#### ReceiveAndSendDistanceWidth
Try to decode each IR frame with the *universal* **DistanceWidth decoder**, store the data and send it on button press with `sendPulseDistanceWidthFromArray()`.<br/>
Storing data for distance width protocol requires 17 bytes.
The ReceiveAndSend example requires 16 bytes for known protocol data and 37 bytes for raw data of e.g.NEC protocol.

#### ReceiveOneAndSendMultiple
Serves as a IR **remote macro expander**. Receives Samsung32 protocol and on receiving a specified input frame, it sends multiple Samsung32 frames with appropriate delays in between.
This serves as a **Netflix-key emulation** for my old Samsung H5273 TV.

#### IRDispatcherDemo
Framework for **calling different functions of your program** for different IR codes.

#### IRrelay
**Control a relay** (connected to an output pin) with your remote.

#### IRremoteExtensionTest
[Example](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/IRremoteExtensionTest/IRremoteExtensionTest.ino) for a user defined class, which itself uses the IRrecv class from IRremote.

#### SendLGAirConditionerDemo
[Example](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SendLGAirConditionerDemo/SendLGAirConditionerDemo.ino) for sending LG air conditioner IR codes controlled by Serial input.<br/>
By just using the function `bool Aircondition_LG::sendCommandAndParameter(char aCommand, int aParameter)` you can control the air conditioner by any other command source.<br/>
The file *acLG.h* contains the command documentation of the LG air conditioner IR protocol. Based on reverse engineering of the LG AKB73315611 remote.
![LG AKB73315611 remote](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/LG_AKB73315611.jpg)<br/>
IReceiverTimingAnalysis can be tested online with [WOKWI](https://wokwi.com/projects/299033930562011656)
Click on the receiver while simulation is running to specify individual IR codes.

#### ReceiverTimingAnalysis
This [example](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiverTimingAnalysis/ReceiverTimingAnalysis.ino) analyzes the signal delivered by your IR receiver module.
Values can be used to determine the stability of the received signal as well as a hint for determining the protocol.<br/>
It also computes the `MARK_EXCESS_MICROS` value, which is the extension of the mark (pulse) duration introduced by the IR receiver module.<br/>
It can be tested online with [WOKWI](https://wokwi.com/arduino/projects/299033930562011656).
Click on the receiver while simulation is running to specify individual NEC IR codes.

#### UnitTest
ReceiveDemo + SendDemo in one program. Demonstrates **receiving while sending**.

# WOKWI online examples
- [Simple receiver](https://wokwi.com/projects/338611596994544210)
- [Simple toggle by IR key 5](https://wokwi.com/projects/338611596994544210)
- [TinyReceiver](https://wokwi.com/arduino/projects/339264565653013075)
- [ReceiverTimingAnalysis](https://wokwi.com/projects/299033930562011656)
- [Receiver with LCD output and switch statement](https://wokwi.com/projects/298934082074575369)

<br/>

# Issues and discussions
- Do not open an issue without first testing some of the examples!
- If you have a problem, please post the MCVE (Minimal Complete Verifiable Example) showing this problem. My experience is, that most of the times you will find the problem while creating this MCVE :smile:.
- [Use code blocks](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet#code); **it helps us help you when we can read your code!**

<br/>

# Compile options / macros for this library
To customize the library to different requirements, there are some compile options / macros available.<br/>
These macros must be defined in your program **before** the line `#include <IRremote.hpp>` to take effect.<br/>
Modify them by enabling / disabling them, or change the values if applicable.

| Name | Default value | Description |
|:---|---:|----|
| `RAW_BUFFER_LENGTH` |  100 | Buffer size of raw input buffer. Must be even! 100 is sufficient for *regular* protocols of up to 48 bits, but for most air conditioner protocols a value of up to 750 is required. Use the ReceiveDump example to find smallest value for your requirements. |
| `EXCLUDE_UNIVERSAL_PROTOCOLS` |  disabled | Excludes the universal decoder for pulse distance protocols and decodeHash (special decoder for all protocols) from `decode()`. Saves up to 1000 bytes program memory. |
| `DECODE_<Protocol name>` |  all | Selection of individual protocol(s) to be decoded. You can specify multiple protocols. See [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremote.hpp#L98-L121)  |
| `DECODE_STRICT_CHECKS` |  disabled | Check for additional characteristics of protocol timing like length of mark for a constant mark protocol, where space length determines the bit value. Requires up to 194 additional bytes of program memory. |
| `IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK` |  disabled | Saves up to 60 bytes of program memory and 2 bytes RAM. |
| `MARK_EXCESS_MICROS` |  20 | MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding, to compensate for the signal forming of different IR receiver modules. |
| `RECORD_GAP_MICROS` |  5000 | Minimum gap between IR transmissions, to detect the end of a protocol.<br/>Must be greater than any space of a protocol e.g. the NEC header space of 4500 &micro;s.<br/>Must be smaller than any gap between a command and a repeat; e.g. the retransmission gap for Sony is around 24 ms.<br/>Keep in mind, that this is the delay between the end of the received command and the start of decoding. |
| `IR_INPUT_IS_ACTIVE_HIGH` |  disabled | Enable it if you use a RF receiver, which has an active HIGH output signal. |
| `IR_SEND_PIN` |  disabled | If specified (as constant), it reduces program size and improves send timing for AVR. If you want to use a variable to specify send pin e.g. with `setSendPin(uint8_t aSendPinNumber)`, you must not use / disable this macro in your source. |
| `SEND_PWM_BY_TIMER` |  disabled | Disables carrier PWM generation in software and use hardware PWM (by timer). Has the advantage of more exact PWM generation, especially the duty cycle (which is not very relevant for most IR receiver circuits), and the disadvantage of using a hardware timer, which in turn is not available for other libraries and to fix the send pin (but not the receive pin) at the [dedicated timer output pin(s)](https://github.com/Arduino-IRremote/Arduino-IRremote#timer-and-pin-usage). Is enabled for ESP32 and RP2040 in all examples, since they support PWM gereration for each pin without using a shared resource (timer). |
| `USE_NO_SEND_PWM` |  disabled | Uses no carrier PWM, just simulate an **active low** receiver signal. Used for transferring signal by cable instead of IR. Overrides `SEND_PWM_BY_TIMER` definition. |
| `IR_SEND_DUTY_CYCLE_PERCENT` |  30 | Duty cycle of IR send signal. |
| `USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN` |  disabled | Uses or simulates open drain output mode at send pin. **Attention, active state of open drain is LOW**, so connect the send LED between positive supply and send pin! |
| `DISABLE_CODE_FOR_RECEIVER` |  disabled | Saves up to 450 bytes program memory and 269 bytes RAM if receiving functionality is not required. |
| `EXCLUDE_EXOTIC_PROTOCOLS` |  disabled | Excludes BANG_OLUFSEN, BOSEWAVE, WHYNTER, FAST and LEGO_PF from `decode()` and from sending with `IrSender.write()`. Saves up to 650 bytes program memory. |
| `FEEDBACK_LED_IS_ACTIVE_LOW` |  disabled | Required on some boards (like my BluePill and my ESP8266 board), where the feedback LED is active low. |
| `NO_LED_FEEDBACK_CODE` |  disabled | Disables the LED feedback code for send and receive. Saves around 100 bytes program memory for receiving, around 500 bytes for sending and halving the receiver ISR (Interrupt Service Routine) processing time. |
| `MICROS_PER_TICK` |  50 | Resolution of the raw input buffer data. Corresponds to 2 pulses of each 26.3 &micro;s at 38 kHz. |
| `TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING` | 25 | Relative tolerance (in percent) for matchTicks(), matchMark() and matchSpace() functions used for protocol decoding. |
| `DEBUG` | disabled | Enables lots of lovely debug output. |
| `IR_USE_AVR_TIMER*` |  | Selection of timer to be used for generating IR receiving sample interval. |

These next macros for **TinyIRReceiver** must be defined in your program before the line `#include <TinyIRReceiver.hpp>` to take effect.
| Name | Default value | Description |
|:---|---:|----|
| `IR_RECEIVE_PIN` | 2 | The pin number for TinyIRReceiver IR input, which gets compiled in. |
| `IR_FEEDBACK_LED_PIN` | `LED_BUILTIN` | The pin number for TinyIRReceiver feedback LED, which gets compiled in. |
| `NO_LED_FEEDBACK_CODE` | disabled | Disables the feedback LED function. Saves 14 bytes program memory. |
| `DISABLE_PARITY_CHECKS` | disabled | Disables the addres and command parity checks. Saves 48 bytes program memory. |
| `USE_ONKYO_PROTOCOL` | disabled | Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value. |
| `USE_FAST_PROTOCOL` | disabled | Use FAST protocol (no address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command) instead of NEC. |
| `ENABLE_NEC2_REPEATS` | disabled | Instead of sending / receiving the NEC special repeat code, send / receive the original frame for repeat. |

The next macro for **IRCommandDispatcher** must be defined in your program before the line `#include <IRCommandDispatcher.hpp>` to take effect.
| `IR_COMMAND_HAS_MORE_THAN_8_BIT` | disabled | Enables mapping and dispatching of IR commands consisting of more than 8 bits. Saves up to 160 bytes program memory and 4 bytes RAM + 1 byte RAM per mapping entry. |

### Changing include (*.h) files with Arduino IDE
First, use *Sketch > Show Sketch Folder (Ctrl+K)*.<br/>
If you have not yet saved the example as your own sketch, then you are instantly in the right library folder.<br/>
Otherwise you have to navigate to the parallel `libraries` folder and select the library you want to access.<br/>
In both cases the library source and include files are located in the libraries `src` directory.<br/>
The modification must be renewed for each new library version!

### Modifying compile options / macros with PlatformIO
If you are using PlatformIO, you can define the macros in the *[platformio.ini](https://docs.platformio.org/en/latest/projectconf/section_env_build.html)* file with `build_flags = -D MACRO_NAME` or `build_flags = -D MACRO_NAME=macroValue`.

### Modifying compile options / macros with Sloeber IDE
If you are using [Sloeber](https://eclipse.baeyens.it) as your IDE, you can easily define global symbols with *Properties > Arduino > CompileOptions*.<br/>
![Sloeber settings](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/SloeberDefineSymbols.png)

<br/>

# Supported Boards
**Issues and discussions with the content "Is it possible to use this library with the ATTinyXYZ? / board XYZ" without any reasonable explanations will be immediately closed without further notice.**<br/>
<br/>
ATtiny and Digispark boards are only tested with the recommended [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore) using `New Style` pin mapping for the pro board.
- Arduino Uno / Mega / Leonardo / Duemilanove / Diecimila / LilyPad / Mini / Fio / Nano etc.
- Teensy 1.0 / 1.0++ / 2.0 / 2++ / 3.0 / 3.1 / 3.2 / Teensy-LC - but [limited support](https://forum.pjrc.com/threads/65912-Enable-Continuous-Integration-with-arduino-cli-for-3-party-libraries); Credits: PaulStoffregen (Teensy Team)
- Sanguino
- ATmega8, 48, 88, 168, 328
- ATmega8535, 16, 32, 164, 324, 644, 1284,
- ATmega64, 128
- ATmega4809 (Nano every)
- ATtiny3217 (Tiny Core 32 Dev Board)
- ATtiny84, 85, 167 (Digispark + Digispark Pro)
- SAMD21 (Zero, MKR*, **but not SAMD51 and not DUE, the latter is SAM architecture**)
- ESP8266
- ESP32 (ESP32 C3 since board package 2.0.2 from Espressif)
- Sparkfun Pro Micro
- Nano Every, Uno WiFi Rev2, nRF5 BBC MicroBit, Nano33_BLE
- BluePill with STM32
- RP2040 based boards (Raspberry Pi Pico, Nano RP2040 Connect etc.)

For ESP8266/ESP32, [this library](https://github.com/crankyoldgit/IRremoteESP8266) supports an [impressive set of protocols and a lot of air conditioners](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md)

We are open to suggestions for adding support to new boards, however we highly recommend you contact your supplier first and ask them to provide support from their side.<br/>
If you can provide **examples of using a periodic timer for interrupts** for the new board, and the board name for selection in the Arduino IDE, then you have way better chances to get your board supported by IRremote.

<br/>

# Timer and pin usage
The **receiver sample interval of 50 &micro;s is generated by a timer**. On many boards this must be a hardware timer. On some boards where a software timer is available, the software timer is used.<br/>
Every pin can be used for receiving.

The TinyReceiver example uses the **TinyReceiver** library,  which can **only receive NEC codes, but does not require any timer** and runs even on a 1 MHz ATtiny85.

The code for the timer and the **timer selection** is located in [private/IRTimer.hpp](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/private/IRTimer.hpp). It can be adjusted here.<br/>
**Be aware that the hardware timer used for receiving should not be used for analogWrite()!**.<br/>

| Board/CPU                                                                | Receive<br/>& PWM Timers| Hardware-PWM Pin | analogWrite()<br/>pins occupied by timer |
|--------------------------------------------------------------------------|-------------------|---------------------|-----------------------|
| [ATtiny84](https://github.com/SpenceKonde/ATTinyCore)                    | **1**             | **6**               |
| [ATtiny85 > 4 MHz](https://github.com/SpenceKonde/ATTinyCore)            | **0**, 1          | **0**, 4            | **0**, 1 & 4          |
| [ATtiny88 > 4 MHz](https://github.com/SpenceKonde/ATTinyCore)            | **1**             | **PB1 / 8**         | **PB1 / 8 & PB2 / 9** |
| [ATtiny167 > 4 MHz](https://github.com/SpenceKonde/ATTinyCore)           | **1**             | **9**, 8 - 15       | **8 - 15**            |
| [ATtiny1604](https://github.com/SpenceKonde/megaTinyCore)                | **TCB0**          | **PA05**            |
| [ATtiny1614, ATtiny816](https://github.com/SpenceKonde/megaTinyCore)     | **TCA0**          | **PA3**             |
| [ATtiny3217](https://github.com/SpenceKonde/megaTinyCore)                | **TCA0**, TCD     | %                   |
| [ATmega8](https://github.com/MCUdude/MiniCore)                           | **1**             | **9**               |
| ATmega168, **ATmega328**                                                 | 1, **2**          | 9, **3**            | 9 & 10, **3 & 11**    |
| [ATmega1284](https://github.com/MCUdude/MightyCore)                      | 1, **2**, 3       | 13, 14, 6           |
| [ATmega164, ATmega324, ATmega644](https://github.com/MCUdude/MightyCore) | 1, **2**          | 13, **14**          |
| [ATmega8535 ATmega16, ATmega32](https://github.com/MCUdude/MightyCore)   | **1**             | **13**              |
| [ATmega64, ATmega128, ATmega1281, ATmega2561](https://github.com/MCUdude/MegaCore) | **1**   | **13**              |
| [ATmega8515, ATmega162](https://github.com/MCUdude/MajorCore)            | **1**             | **13**              |
| ATmega1280, ATmega2560                                                   | 1, **2**, 3, 4, 5 | 5, 6, **9**, 11, 46 |
| ATmega4809                                                               | **TCB0**          | **A4**              |
| Leonardo (Atmega32u4)                                                    | 1, 3, **4_HS**    | 5, **9**, 13        |
| Zero (SAMD)                                                              | **TC3**           | \*, **9**           |
| [ESP32](http://esp32.net/)                                               | **Ledc chan. 0**  | All pins            |
| [Sparkfun Pro Micro](https://www.sparkfun.com/products/12640)            | 1, **3**          | **5**, 9            |
| [Teensy 1.0](https://www.pjrc.com/teensy/pinout.html)                    | **1**             | **17**              | 15, 18 |
| [Teensy 2.0](https://www.pjrc.com/teensy/pinout.html)                    | 1, 3, **4_HS**    | 9, **10**, 14       | 12 |
| [Teensy++ 1.0 / 2.0](https://www.pjrc.com/teensy/pinout.html)            | 1, **2**, 3       | **1**, 16, 25       | 0 |
| [Teensy-LC](https://www.pjrc.com/teensy/pinout.html)                     | **TPM1**          | **16**              | 17 |
| [Teensy 3.0 - 3.6](https://www.pjrc.com/teensy/pinout.html)              | **CMT**           | **5**               |
| [Teensy 4.0 - 4.1](https://www.pjrc.com/teensy/pinout.html)              | **FlexPWM1.3**    | **8**               | 7, 25 |
| [BluePill / STM32F103C8T6](https://github.com/stm32duino/Arduino_Core_STM32)  | **3**    | %                   | **PA6 & PA7 & PB0 & PB1** |
| [BluePill / STM32F103C8T6](https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill) | **TIM4** | %                   | **PB6 & PB7 & PB8 & PB9** |
| [RP2040 / Pi Pico](https://github.com/earlephilhower/arduino-pico)       | [default alarm pool](https://raspberrypi.github.io/pico-sdk-doxygen/group__repeating__timer.html) | All pins             | No pin |
| [RP2040 / Mbed based](https://github.com/arduino/ArduinoCore-mbed)       | Mbed Ticker       | All pins            | No pin |

### No timer required for sending
The **send PWM signal** is by default generated by software. **Therefore every pin can be used for sending**.
The PWM pulse length is guaranteed to be constant by using `delayMicroseconds()`.
Take care not to generate interrupts during sending with software generated PWM, otherwise you will get jitter in the generated PWM.
E.g. wait for a former `Serial.print()` statement to be finished by `Serial.flush()`.
Since the Arduino `micros()` function has a resolution of 4 &micro;s at 16 MHz, we always see a small jitter in the signal, which seems to be OK for the receivers.

| Software generated PWM showing small jitter because of the limited resolution of 4 &micro;s of the Arduino core `micros()` function for an ATmega328 | Detail (ATmega328 generated) showing 30% duty cycle |
|-|-|
| ![Software PWM](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/IR_PWM_by_software_jitter.png) | ![Software PWM detail](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/IR_PWM_by_software_detail.png) |

## Incompatibilities to other libraries and Arduino commands like tone() and analogWrite()
If you use a library which requires the same timer as IRremote, you have a problem, since **the timer resource cannot be shared simultaneously** by both libraries.

### Change timer
The best approach is to change the timer used for IRremote, which can be accomplished by specifying the timer before `#include <IRremote.hpp>`.<br/>
The timer specifications available for your board can be found in [private/IRTimer.hpp](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/private/IRTimer.hpp).<br/>

```c++
// Arduino Mega
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER2) && !defined(IR_USE_AVR_TIMER3) && !defined(IR_USE_AVR_TIMER4) && !defined(IR_USE_AVR_TIMER5)
//#define IR_USE_AVR_TIMER1   // send pin = pin 11
#define IR_USE_AVR_TIMER2     // send pin = pin 9
//#define IR_USE_AVR_TIMER3   // send pin = pin 5
//#define IR_USE_AVR_TIMER4   // send pin = pin 6
//#define IR_USE_AVR_TIMER5   // send pin = pin 46
#  endif
```
Here you see the Arduino Mega board and the available specifications are `IR_USE_AVR_TIMER[1,2,3,4,5]`.<br/>
You **just have to include a line** e.g. `#define IR_USE_AVR_TIMER3` before `#include <IRremote.hpp>` to enable timer 3.

But be aware that the new timer in turn might be incompatible with other libraries or commands.<br/>
For other boards/platforms you must look for the appropriate section guarded by e.g. `#elif defined(ESP32)`.

### Stop and start timer
Another approach can be to share the timer **sequentially** if their functionality is used only for a short period of time like for the **Arduino tone() command**.
An example can be seen [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/21b5747a58e9d47c9e3f1beb056d58c875a92b47/examples/ReceiveDemo/ReceiveDemo.ino#L159-L169), where the timer settings for IR receive are restored after the tone has stopped.
For this we must call `IrReceiver.start()` or better `IrReceiver.start(microsecondsOfToneDuration)`.<br/>
This only works since each call to` tone()` completely initializes the timer 2 used by the `tone()` command.

## Hardware-PWM signal generation for sending
If you define `SEND_PWM_BY_TIMER`, the send PWM signal is forced to be generated by a hardware timer on most platforms.<br/>
By default, the same timer as for the receiver is used.<br/>
Since each hardware timer has its dedicated output pin(s), you must change timer or timer sub-specifications to change PWM output pin. See [private/IRTimer.hpp](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/private/IRTimer.hpp)<br/>
**Exeptions** are currently [ESP32, ARDUINO_ARCH_RP2040, PARTICLE and ARDUINO_ARCH_MBED](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/39bdf8d7bf5b90dc221f8ae9fb3efed9f0a8a1db/examples/SimpleSender/PinDefinitionsAndMore.h#L273), where **PWM generation does not require a timer**.

## Why do we use 30% duty cycle for sending
We do it according to the statement in the [Vishay datasheet](https://www.vishay.com/docs/80069/circuit.pdf):
- Carrier duty cycle 50 %, peak current of emitter IF = 200 mA, the resulting transmission distance is 25 m.
- Carrier duty cycle 10 %, peak current of emitter IF = 800 mA, the resulting transmission distance is 29 m. - Factor 1.16
The reason is, that it is not the pure energy of the fundamental which is responsible for the receiver to detect a signal.
Due to automatic gain control and other bias effects, high intensity of the 38 kHz pulse counts more than medium intensity (e.g. 50% duty cycle) at the same total energy.

<br/>

# How we decode signals
The IR signal is sampled at a **50 &micro;s interval**. For a constant 525 &micro;s pulse or pause we therefore get 10 or 11 samples, each with 50% probability.<br/>
And believe me, if you send a 525 &micro;s signal, your receiver will output something between around 400 and 700 &micro;s!<br/>
Therefore **we decode by default with a +/- 25% margin** using the formulas [here](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRremoteInt.h#L376-L399).<br/>
E.g. for the NEC protocol with its 560 &micro;s unit length, we have TICKS_LOW = 8.358 and TICKS_HIGH = 15.0. This means, we accept any value between 8 ticks / 400 &micro;s and 15 ticks / 750 &micro;s (inclusive) as a mark or as a zero space. For a one space we have TICKS_LOW = 25.07 and TICKS_HIGH = 45.0.<br/>
And since the receivers generated marks are longer or shorter than the spaces, we have introduced the [`MARK_EXCESS_MICROS` value]/https://github.com/Arduino-IRremote/Arduino-IRremote#protocolunknown)
to compensate for this receiver (and signal strength as well as ambient light dependent :disappointed: ) specific deviation.<br/>
Welcome to the world of **real world signal processing**.

<br/>

# NEC encoding diagrams
Created with sigrok PulseView with IR_NEC decoder by DjordjeMandic.<br/>
8 bit address NEC code
![8 bit address NEC code](https://user-images.githubusercontent.com/6750655/108884951-78e42b80-7607-11eb-9513-b07173a169c0.png)
16 bit address NEC code
![16 bit address NEC code](https://user-images.githubusercontent.com/6750655/108885081-a6c97000-7607-11eb-8d35-274a7065b6c4.png)

<br/>

# Quick comparison of 5 Arduino IR receiving libraries
[Here](https://github.com/crankyoldgit/IRremoteESP8266) you find an **ESP8266/ESP32** version of IRremote with an **[impressive list of supported protocols](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md)**.

**This is a short comparison and may not be complete or correct.**

I created this comparison matrix for [myself](https://github.com/ArminJo) in order to choose a small IR lib for my project and to have a quick overview, when to choose which library.<br/>
It is dated from **24.06.2022**. If you have complains about the data or request for extensions, please send a PM or open a discussion.

| Subject | [IRMP](https://github.com/IRMP-org/IRMP) | [IRLremote](https://github.com/NicoHood/IRLremote) | [IRLib2](https://github.com/cyborg5/IRLib2)<br/>**mostly unmaintained** | [IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) | [Minimal NEC](https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/examples/TinyReceiver) | [IRsmallDecoder](https://github.com/LuisMiCa/IRsmallDecoder)
|---------|------|-----------|--------|----------|----------|----------|
| Number of protocols | **50** | Nec + Panasonic + Hash \* | 12 + Hash \* | 17 + PulseDistance + Hash \* | NEC | NEC + RC5 + Sony + Samsung |
| Timing method receive | Timer2 or interrupt for pin 2 or 3 | **Interrupt** | Timer2 or interrupt for pin 2 or 3 | Timer2 | **Interrupt** | **Interrupt** |
| Timing method send | PWM and timing with Timer2 interrupts | Timer2 interrupts | Timer2 and blocking wait | PWM with Timer2 and/or blocking wait with delay<br/>Microseconds() | blocking wait with delay<br/>Microseconds() | % |
| Send pins| All | All | All ? | Timer dependent | All | % |
| Decode method | OnTheFly | OnTheFly | RAM | RAM | OnTheFly | OnTheFly |
| Encode method | OnTheFly | OnTheFly | OnTheFly | OnTheFly or RAM | OnTheFly | % |
| Callback support | x | % | % | x | x | % |
| Repeat handling | Receive + Send (partially) | % | ? | Receive + Send | Receive + Send | Receive |
| LED feedback | x | % | x | x | Receive | % |
| FLASH usage (simple NEC example with 5 prints) | 1820<br/>(4300 for 15 main / 8000 for all 40 protocols)<br/>(+200 for callback)<br/>(+80 for interrupt at pin 2+3)| 1270<br/>(1400 for pin 2+3) | 4830 | 1770 | **900** | ?1100? |
| RAM usage | 52<br/>(73 / 100 for 15 (main) / 40 protocols) | 62 | 334 | 227 | **19** | 29 |
| Supported platforms | **avr, megaavr, attiny, Digispark (Pro), esp8266, ESP32, STM32, SAMD 21, Apollo3<br/>(plus arm and pic for non Arduino IDE)** | avr, esp8266 | avr, SAMD 21, SAMD 51 | avr, attiny, [esp8266](https://github.com/crankyoldgit/IRremoteESP8266), esp32, SAM, SAMD | **All platforms with attach<br/>Interrupt()** | **All platforms with attach<br/>Interrupt()** |
| Last library update | 6/2022 | 4/2018 | 3/2022 | 6/2022 | 6/2022 | 2/2022 |
| Remarks | Decodes 40 protocols concurrently.<br/>39 Protocols to send.<br/>Work in progress. | Only one protocol at a time. | Consists of 5 libraries. **Project containing bugs - 45 issues, no reaction for at least one year.** | Universal decoder and encoder.<br/>Supports **Pronto** codes and sending of raw timing values. | Requires no timer. | Requires no timer. |

\* The Hash protocol gives you a hash as code, which may be sufficient to distinguish your keys on the remote, but may not work with some protocols like Mitsubishi

<br/>

# Useful links
- [List of public IR code databases](http://www.harctoolbox.org/IR-resources.html)
- [LIRC database](http://lirc-remotes.sourceforge.net/remotes-table.html)
- [IRMP list of IR protocols](https://www.mikrocontroller.net/articles/IRMP_-_english#IR_Protocols)
- [IRDB database for IR codes](https://github.com/probonopd/irdb/tree/master/codes)
- [IRP definition files for IR protocols](https://github.com/probonopd/MakeHex/tree/master/protocols)
- [IR Remote Control Theory and some protocols (upper right hamburger icon)](https://www.sbprojects.net/knowledge/ir/)
- [Interpreting Decoded IR Signals (v2.45)](http://www.hifi-remote.com/johnsfine/DecodeIR.html)
- ["Recording long Infrared Remote control signals with Arduino"](https://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino)
- The original blog post of Ken Shirriff [A Multi-Protocol Infrared Remote Library for the Arduino](http://www.arcfn.com/2009/08/multi-protocol-infrared-remote-library.html)
- [Vishay datasheet](https://www.vishay.com/docs/80069/circuit.pdf)

# License
Up to the version 2.7.0, the License is GPLv2.
From the version 2.8.0, the license is the MIT license.

# Copyright
Initially coded 2009 Ken Shirriff http://www.righto.com<br/>
Copyright (c) 2016-2017 Rafi Khan<br/>
Copyright (c) 2020-2023 [Armin Joachimsmeyer](https://github.com/ArminJo)
