# Migrating legacy projects to the latest version

## Table of contents
  * [New features and changes in version 5](#new-features-and-changes-in-version-5)
  * [New features and changes in version 4.7.1](#new-features-and-changes-in-version-471)
  * [New features of version 4.x](#new-features-of-version-4x)
  * [Converting your 3.x program to the 4.x version](#converting-your-3x-program-to-the-4x-version)
  * [New features of version 3.x](#new-features-of-version-3x)
  * [Converting your 2.x program to the 4.x version](#converting-your-2x-program-to-the-4x-version)
  * [How to convert old MSB first 32 bit IR data codes to new LSB first 32 bit IR data codes](#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes)
  * [Errors when using the 4.x versions for old tutorials](#errors-when-using-the-4x-versions-for-old-tutorials)
  * [Staying on 2.x](#staying-on-2x)

This library has been refactored, breaking backward compatibility with the old version, on which many examples on the Internet are based.<br/>
If your code uses `irrecv.decode(&results)`, it is written for Version 2.x. This library (Version 5.x) now uses a simpler `IrReceiver.decode()` syntax. See the Conversion guide below.<br/>
A detailed list of changes for each version can be found in the [changelog](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/changelog.md).

## New features and changes in version 5
**Version 5 is a fresh start. It removes all the code for backward compatibility as well as all deprecated functions.
To migrate to 5.x, temporarily use library version 4.7.1 and fix all deprecation warnings before switching to the 5.x library.**
- The old 2.x API `decode(decode_results *aResults)`, the `decode_results` structure and all old MSB decoders were removed.
- The old MSB send functions like `sendNECMSB()`, `sendSonyMSB()`, `sendSamsungMSB()` and `sendJVCMSB()` were removed.
  Convert your MSB codes to LSB as described [below](#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes).
- OpenLASIR is no longer decoded by default. To receive OpenLASIR, add `#define DECODE_OPENLASIR`, which disables the NEC and ONKYO decoders.

### Receiver API changes
- **The parameters of `IrReceiver.begin()` have changed.** The second parameter is now the feedback LED pin and the third (mandatory) one enables the LED feedback.<br/>
  Old `IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK)` must be changed to `IrReceiver.begin(IR_RECEIVE_PIN, USE_DEFAULT_FEEDBACK_LED_PIN, ENABLE_LED_FEEDBACK)`.<br/>
  Old `IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK)` must be changed to `IrReceiver.begin(IR_RECEIVE_PIN)`.<br/>
  The old 2 parameter calls no longer compile.
- `enableIRIn()` and `disableIRIn()` were removed, use `start()` and `stop()` or `end()`.
- `compensateAndStoreIRResultInArray()` has a new second parameter for the length of the destination array.

### Sender API changes
- **The default value `NO_REPEATS` for the parameter `aNumberOfRepeats` was removed from all send functions.**
  Calls like `IrSender.write(&tIRData)`, `IrSender.sendNECRaw(0x12345678)`, `IrSender.sendBoseWave(0x10)` or `IrSender.sendPronto(tProntoString)`
  now require the number of repeats, e.g. `IrSender.sendNECRaw(0x12345678, 0)`.
- `IrSender.begin(aSendPin, aEnableLEDFeedback, aFeedbackLEDPin)` was removed, use `IrSender.begin(aSendPin, aFeedbackLEDPin)`.
- `IrSender.begin(aFeedbackLEDPin)`, which was available if `IR_SEND_PIN` is defined, was removed. Use `IrSender.begin()` and `setLEDFeedbackPin(aFeedbackLEDPin)`.
- `sendLegoPowerFunctions(uint16_t aRawData, ...)` was renamed to `sendLegoPowerFunctionsRaw()` and the last 2 parameters of `sendLegoPowerFunctions()` were swapped.
- `sendSamsung48()` now has a `uint16_t` command parameter.
- `sendFAST()` now has a `uint16_t` command parameter. Commands above 0xFF are sent as 16 bit without parity.
- The parameter order of `sendVelux()` has changed to `sendVelux(aMotorNumber, aMotorSet, aCommand, aSecurityCode, aNumberOfRepeats)`.
- The internal function `setFeedbackLED()` was renamed to `writeFeedbackLED()`.

### TinyIRReceiver and TinyIRSender API changes
| Old | New |
|-|-|
| `TinyReceiverDecode()`, `TinyIRReceiverDecode()` | `decodeTinyIRReceiver()` |
| `isIRReceiverAttachedForTinyReceiver()`, `isIRReceiverAttachedForTinyIRReceiver()` | `isTinyIRReceiverAttached()` |
| All other `...TinyReceiver...()` functions | `...TinyIRReceiver...()` |
| `TinyIRReceiverCallbackDataStruct` | `TinyIRReceiverDataStruct` |
| `TinyIRReceiverData.Address`, `.Command`, `.Flags` | `TinyIRReceiverData.address`, `.command`, `.flags`. The old names still work. |
| `sendONKYO()` | `sendOnkyo()` |
| `sendFast8BitAndParity()` | `sendFAST8BitAndParity()` |
| `sendExtendedNEC()` | `sendNEC()`, which sends a 16 bit address if the address is above 0xFF. |

- The default value 0 for the parameter `aNumberOfRepeats` was removed from all TinyIRSender functions, e.g. `sendNEC(3, 0, 11)` must be changed to `sendNEC(3, 0, 11, 0)`.

## New features and changes in version 4.7.1
**Version 4.5.0 does not work for ESP platform, because of missing ESP IRAM_ATTR for receiving interrupt.**
- `irparams_struct irparams` is now member of `IRrecv`. Thus `rawDataPtr` (pointer to irparams) was removed from `IrReceiver.decodedIRData`.<br/>
Old `IrReceiver.decodedIRData.rawDataPtr->rawbuf` is now `IrReceiver.irparams.rawbuf`, the same holds for `IrReceiver.irparams.rawlen`.
- LED feedback is always enabled for sending. It can only be disabled by using `#define NO_LED_SEND_FEEDBACK_CODE` or `#define NO_LED_FEEDBACK_CODE`.
- The second parameter of the function `IrSender.begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback)` has changed to `uint_fast8_t aFeedbackLEDPin`. Therefore this function call no longer works as expected because it uses the old boolean value of e.g. `ENABLE_LED_FEEDBACK` which evaluates to true or 1 as pin number of the LED feedback pin number.

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
- An 8/16 bit **command** value as well as a 16-bit **address** and a protocol number is provided for decoding (instead of the old 32 bit value).
- Protocol values comply to **protocol standards**.<br/>
  NEC, Panasonic, Sony, Samsung and JVC decode & send LSB first.
- Supports **Universal Distance protocol**, which covers a lot of previous unknown protocols.
- Compatible with **tone()** library. See the [ReceiveDemo](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino#L284-L298) example.
- Simultaneous sending and receiving. See the [SendAndReceive](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SendAndReceive/SendAndReceive.ino#L167-L170) example.
- Supports **more platforms**.
- Allows for the generation of non-PWM signal to just **simulate an active low receiver signal** for direct connect to existing receiving devices without using IR.
- Easy protocol configuration, **directly in your [source code](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/SimpleReceiver/SimpleReceiver.ino#L33-L57)**.<br/>
  Reduces memory footprint and decreases decoding time.
- Contains a [very small NEC only decoder](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#tiny-nec-receiver-and-sender), which **does not require any timer resource**.

[-> Feature comparison of 5 Arduino IR libraries](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#quick-comparison-of-5-arduino-ir-receiving-libraries).

<br/>

## Converting your 2.x program to the 4.x version
Starting with the 3.1 version, **the generation of PWM for sending is done by software**, thus saving the hardware timer and **enabling arbitrary output pins for sending**.<br/>
If you use an (old) Arduino core that does not use the `-flto` flag for compile, you can activate the line `#define SUPPRESS_ERROR_MESSAGE_FOR_BEGIN` in IRremote.hpp,
if you get false error messages regarding begin() during compilation.

- **IrReceiver** and **IrSender** objects have been added and can be used without defining them, like the well known Arduino **Serial** object.
- Just remove the line `IRrecv IrReceiver(IR_RECEIVE_PIN);` and/or `IRsend IrSender;` in your program and replace all occurrences of `IRrecv.` or `irrecv.` with `IrReceiver` and replace all `IRsend` or `irsend` with `IrSender`.
- Like for the Serial object, call [`IrReceiver.begin(IR_RECEIVE_PIN, USE_DEFAULT_FEEDBACK_LED_PIN, ENABLE_LED_FEEDBACK)`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/examples/ReceiveDemo/ReceiveDemo.ino)
 or `IrReceiver.begin(IR_RECEIVE_PIN)` (without LED feedback) instead of the `IrReceiver.enableIRIn()` or `irrecv.enableIRIn()` in setup().<br/>
If IR_SEND_PIN is not defined (before the line `#include <IRremote.hpp>`) you must use e.g. `IrSender.begin(3);`
- Old `decode(decode_results *aResults)` function is replaced by simple `decode()`. So if you have a statement `if(irrecv.decode(&results))` replace it with `if (IrReceiver.decode())`.
- The decoded result is now in `IrReceiver.decodedIRData` and not in `results` any more, therefore replace any occurrences of `results.value` and `results.decode_type` (and similar) to
 `IrReceiver.decodedIRData.decodedRawData` and `IrReceiver.decodedIRData.protocol`.
- Overflow, Repeat and other flags are now in [`IrReceiver.decodedIRData.flags`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/IRProtocol.h).
- Seldom used: `results.rawbuf` and `results.rawlen` must be replaced by `IrReceiver.irparams.rawbuf` and `IrReceiver.decodedIRData.rawlen`.

- The 5 protocols **NEC, Panasonic, Sony, Samsung and JVC** have been converted to LSB first. Since version 5, there are no send functions for old MSB data any more.<br/>
For converting MSB codes to LSB see [below](#how-to-convert-old-msb-first-32-bit-ir-data-codes-to-new-lsb-first-32-bit-ir-data-codes) or use the functions `bitreverseOneByte()` or `bitreverse32Bit()` for reversing.

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

#### New 5.x program:

```c++
#include <IRremote.hpp>
#define IR_RECEIVE_PIN 2

void setup()
{
...
  Serial.begin(115200); // Establish serial communication
  IrReceiver.begin(IR_RECEIVE_PIN, USE_DEFAULT_FEEDBACK_LED_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
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

### Send MSB codes
The MSB send functions like `sendNECMSB()` were removed in version 5.
Convert your codes with `bitreverse32Bit()` and send them with e.g. `IrSender.sendNECRaw(bitreverse32Bit(0xCB340102), <numberOfRepeats>)`.

<br/>

## Errors when using the 4.x versions for old tutorials
If you suffer from errors with old tutorial code including `IRremote.h` instead of `IRremote.hpp`,
just try to rollback to [Version 2.4.0](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/v2.4.0).<br/>
Most likely your code will run and you will not miss the new features.

<br/>

## Staying on 2.x
Consider using the [original 2.4 release from 2017](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/v2.4.0)
or the last backwards compatible [2.8 version](https://github.com/Arduino-IRremote/Arduino-IRremote/releases/tag/2.8.0) for your project.<br/>
It may be sufficient and deals flawlessly with 32 bit IR codes.<br/>
Versions 4.x still contain a compatibility layer for old 2.x code. **Version 5 does not**, so old 2.x examples must be converted as described [above](#converting-your-2x-program-to-the-4x-version).

### Drawbacks of using 2.x
- Only the following decoders are available:<br/>
  ` NEC ` &nbsp; &nbsp; ` Denon ` &nbsp; &nbsp; ` Panasonic ` &nbsp; &nbsp; ` JVC ` &nbsp; &nbsp; ` LG `<br/>
  ` RC5 ` &nbsp; &nbsp; ` RC6 ` &nbsp; &nbsp; ` Samsung ` &nbsp; &nbsp; ` Sony `
- The call of `irrecv.decode(&results)` uses the old MSB first decoders and sets the 32 bit codes in `results.value`.
- No decoding to a more meaningful (constant) 8/16 bit address and 8 bit command.
