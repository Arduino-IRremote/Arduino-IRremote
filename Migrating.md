# Migrating legacy projects to the latest version

## Table of contents
  * [New features and changes in version 4.6](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#new-features-and-changes-in-version-46)
  * [New features of version 4.x](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#new-features-of-version-4x)
  * [New features of version 3.x](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#new-features-of-version-3x)
  * [Converting your 2.x program to the 4.x version](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#converting-your-2x-program-to-the-4x-version)
  * [How to convert old MSB first 32 bit IR data codes to new LSB first 32 bit IR data codes](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes)
  *  [Errors when using the 3.x versions for old tutorials](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#errors-when-using-the-3x-versions-for-old-tutorials)
  * [Staying on 2.x](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#staying-on-2x)

This library has been refactored, breaking backward compatibility with the old version, on which many examples on the Internet are based.<br/>
If your code uses `irrecv.decode(&results)`, it is written for Version 2.x. This library (Version 4.x) now uses a simpler `IrReceiver.decode()` syntax. See the Conversion guide below.

## New features and changes in version 4.6
** Version 4.5.0 does not work for ESP platform, because of missing ESP IRAM_ATTR for receiving interrupt.**
- Support for **multiple receiver instances**.<br/>
`irparams_struct irparams` is now member of `IRrecv`. Thus `rawDataPtr` (pointer to irparams) was removed from `IrReceiver.decodedIRData`.<br/>
Old `IrReceiver.decodedIRData.rawDataPtr->rawbuf` is now `IrReceiver.irparams.rawbuf`, the same holds for `IrReceiver.irparams.rawlen`.
- LED feedback is always enabled for sending. It can only be disabled by using `#define NO_LED_SEND_FEEDBACK_CODE` or `#define NO_LED_FEEDBACK_CODE`.
- The second parameter of the function `IrSender.begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback)` has changed to `uint_fast8_t aFeedbackLEDPin`. Therefore this function call no longer works as expected because it uses the old boolean value of e.g. `ENABLE_LED_FEEDBACK` which evaluates to true or 1 as pin number of the LED feedback pin number.
- New experimental Velux send function.
- Send function for Marantz version of the RC5x protocol. This protocol has an additional 6 bit command extension and adds a short pause after the address is sent to identify this variant of the protocol.

## New features of version 4.x
- **Since 4.3 `IrSender.begin(DISABLE_LED_FEEDBACK)` will no longer work**, use `#define NO_LED_SEND_FEEDBACK_CODE` instead.
- New universal **Pulse Distance / Pulse Width / Pulse Distance Width decoder** added, which covers many previous unknown protocols.
- Printout of code how to send received command by `IrReceiver.printIRSendUsage(&Serial)`.
- RawData type is now 64 bit for 32 bit platforms and therefore `decodedIRData.decodedRawData` can contain complete frame information for more protocols than with 32 bit as before.
- **Callback** after receiving a command - It calls your code as soon as a frame was received.
- Improved handling of `PULSE_DISTANCE` + `PULSE_WIDTH` protocols.
- New FAST protocol.
- Automatic printout of the **corresponding send function** with `printIRSendUsage()`.

### Converting your 3.x program to the 4.x version
- You must replace `#define DECODE_DISTANCE` by `#define DECODE_DISTANCE_WIDTH` (only if you explicitly enabled this decoder).
- The parameter `bool hasStopBit` is no longer required and removed e.g. for function `sendPulseDistanceWidth()`.

## New features of version 3.x
- **Any pin** can be used for receiving and if `SEND_PWM_BY_TIMER` is not defined also for sending.
- Feedback LED can be activated for sending / receiving.
- An 8/16 bit ****command** value as well as a 16-bit **address** and a protocol number is provided for decoding (instead of the old 32 bit value).
- Protocol values comply to **protocol standards**.<br/>
  NEC, Panasonic, Sony, Samsung and JVC decode & send LSB first.
- Supports **Universal Distance protocol**, which covers a lot of previous unknown protocols.
- Compatible with **tone()** library. See the [ReceiveDemo](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino#L284-L298) example.
- Simultaneous sending and receiving. See the [SendAndReceive](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SendAndReceive/SendAndReceive.ino#L167-L170) example.
- Supports **more platforms**.
- Allows for the generation of non-PWM signal to just **simulate an active low receiver signal** for direct connect to existing receiving devices without using IR.
- Easy protocol configuration, **directly in your [source code](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino#L33-L57)**.<br/>
  Reduces memory footprint and decreases decoding time.
- Contains a [very small NEC only decoder](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#minimal-nec-receiver), which **does not require any timer resource**.

[-> Feature comparison of 5 Arduino IR libraries](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#quick-comparison-of-5-arduino-ir-receiving-libraries).

<br/>

## Converting your 2.x program to the 4.x version
Starting with the 3.1 version, **the generation of PWM for sending is done by software**, thus saving the hardware timer and **enabling arbitrary output pins for sending**.<br/>
If you use an (old) Arduino core that does not use the `-flto` flag for compile, you can activate the line `#define SUPPRESS_ERROR_MESSAGE_FOR_BEGIN` in IRRemote.h,
if you get false error messages regarding begin() during compilation.

- **IRreceiver** and **IRsender** object have been added and can be used without defining them, like the well known Arduino **Serial** object.
- Just remove the line `IRrecv IrReceiver(IR_RECEIVE_PIN);` and/or `IRsend IrSender;` in your program and replace all occurrences of `IRrecv.` or `irrecv.` with `IrReceiver` and replace all `IRsend` or `irsend` with `IrSender`.
- Like for the Serial object, call [`IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK)`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino#L106)
 or `IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK)` instead of the `IrReceiver.enableIRIn()` or `irrecv.enableIRIn()` in setup().<br/>
If IR_SEND_PIN is not defined (before the line `#include <IRremote.hpp>`) you must use e.g. `IrSender.begin(3);`
- Old `decode(decode_results *aResults)` function is replaced by simple `decode()`. So if you have a statement `if(irrecv.decode(&results))` replace it with `if (IrReceiver.decode())`.
- The decoded result is now in in `IrReceiver.decodedIRData` and not in `results` any more, therefore replace any occurrences of `results.value` and `results.decode_type` (and similar) to
 `IrReceiver.decodedIRData.decodedRawData` and `IrReceiver.decodedIRData.protocol`.
- Overflow, Repeat and other flags are now in [`IrReceiver.receivedIRData.flags`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRProtocol.h#L90-L101).
- Seldom used: `results.rawbuf` and `results.rawlen` must be replaced by `IrReceiver.irparams.rawbuf` and `IrReceiver.decodedIRData.rawlen`.

- The 5 protocols **NEC, Panasonic, Sony, Samsung and JVC** have been converted to LSB first. Send functions for sending old MSB data were renamed to `sendNECMSB`, `sendSamsungMSB()`, `sendSonyMSB()` and `sendJVCMSB()`.
The old  `sendSAMSUNG()` and `sendSony()` MSB functions are still available.
The old MSB version of `sendPanasonic()` function was deleted, since it had bugs nobody recognized and therefore was assumed to be never used.<br/>
For converting MSB codes to LSB see [below](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes) or use the new functions `bitreverseOneByte()` or `bitreverse32Bit()` for reversing.

### Example
#### Old 2.x program:

```c++
#include <IRremote.h>
#define RECV_PIN 2

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
...
  Serial.begin(115200); // Establish serial communication
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
#define IR_RECEIVE_PIN 2

void setup()
{
...
  Serial.begin(115200); // // Establish serial communication
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
}

void loop() {
  if (IrReceiver.decode()) {
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
      IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
      IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
      ...
      IrReceiver.resume(); // Enable receiving of the next value
  }
  ...
}
```

#### Sample output
For more, see the [UnitTest log](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/UnitTest/UnitTest.log).

```
Protocol=NEC Address=0xF1 Command=0x76 Raw-Data=0x89760EF1 32 bits LSB first
Send with: IrSender.sendNEC(0xF1, 0x76, <numberOfRepeats>);

Protocol=Kaseikyo_Denon Address=0xFF1 Command=0x76 Raw-Data=0x9976FF10 48 bits LSB first
Send with: IrSender.sendKaseikyo_Denon(0xFF1, 0x76, <numberOfRepeats>);
```

## How to convert old MSB first 32 bit IR data codes to new LSB first 32 bit IR data codes
For the new decoders for **NEC, Panasonic, Sony, Samsung and JVC**, the result `IrReceiver.decodedIRData.decodedRawData` is now **LSB-first**, as the definition of these protocols suggests!<br/>
<br/>
To convert one into the other, you must reverse the nibble (4 bit / Hex numbers) positions and then reverse all bit positions of each nibble or write it as one binary string and reverse/mirror it.<br/>
This can be done by using the new functions `bitreverseOneByte()` or `bitreverse32Bit()`.

### Nibble reverse
`0xCB 34 01 02` MSB value<br/>
`0x20 10 43 BC` after nibble reverse<br/>
`0x40 80 2C D3` LSB value after bit reverse of each nibble

### Nibble reverse map
```
 0->0   1->8   2->4   3->C
 4->2   5->A   6->6   7->E
 8->1   9->9   A->5   B->D
 C->3   D->B   E->7   F->F
```

### Binary string reverse
If you **read the first binary sequence backwards** (right to left), you get the second sequence.<br/>
`0xCB340102` is binary `1100 1011 0011 0100 0000 0001 0000 0010`.<br/>
`0x40802CD3` is binary `0100 0000 1000 0000 0010 1100 1101 0011`.

### Online tool which reverses every byte, but not the order of the bytes
Use this [tool provided by analysir](https://www.analysir.com/hex2nec.php).

### Send MSB directly
Sending old MSB codes without conversion can be done by using `sendNECMSB()`, `sendSonyMSB()`, `sendSamsungMSB()`, `sendJVCMSB()`.

<br/>

## Errors when using the 4.x versions for old tutorials
If you suffer from errors with old tutorial code including `IRremote.h` instead of `IRremote.hpp`,
just try to rollback to [Version 2.4.0](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/v2.4.0).<br/>
Most likely your code will run and you will not miss the new features.

<br/>

## Staying on 2.x
Consider using the [original 2.4 release from 2017](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/v2.4.0)
or the last backwards compatible [2.8 version](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/2.8.0) for you project.<br/>
It may be sufficient and deals flawlessly with 32 bit IR codes.<br/>
If this doesn't work for you, you can be sure that 4.x is trying to be compatible with earlier versions, so your old examples should work just fine.

### Drawbacks of using 2.x
- Only the following decoders are available:<br/>
  ` NEC ` &nbsp; &nbsp; ` Denon ` &nbsp; &nbsp; ` Panasonic ` &nbsp; &nbsp; ` JVC ` &nbsp; &nbsp; ` LG `<br/>
  ` RC5 ` &nbsp; &nbsp; ` RC6 ` &nbsp; &nbsp; ` Samsung ` &nbsp; &nbsp; ` Sony `
- The call of `irrecv.decode(&results)` uses the old MSB first decoders like in 2.x and sets the 32 bit codes in `results.value`.
- No decoding to a more meaningful (constant) 8/16 bit address and 8 bit command.
