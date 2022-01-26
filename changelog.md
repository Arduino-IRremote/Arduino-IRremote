# Changelog
The latest version may not be released!
See also the commit log at github: https://github.com/Arduino-IRremote/Arduino-IRremote/commits/master

## 3.5.3
- RP2040 support added.
- Refactored IRTimer.hpp.
- Refactored IR_SEND_PIN and IrSender.sendPin handling.
- Renamed IR_SEND_DUTY_CYCLE to IR_SEND_DUTY_CYCLE_PERCENT.
- Fixed bugs for SEND_PWM_BY_TIMER active.

## 3.5.2
- Improved support for Teensy boards by Paul Stoffregen.

## 3.5.1
- Renamed INFO_PRINT to IR_INFO_PRINT as well as for DEBUG and TRACE.
- Fixed error with DEBUG in TinyIRReceiver.hpp.
- Support for ATmega88 see issue #923. Thanks to Dolmant.
- NO_LED_FEEDBACK_CODE replaces and extends DISABLE_LED_FEEDBACK_FOR_RECEIVE.
- Removed NO_LEGACY_COMPATIBILITY macro, it was useless now.
- Fix ESP32 send bug see issue #927.

## 3.5.0
- Improved ir_DistanceProtocol.
- Tone for ESP32.
- last phase renamed *.cpp.h to .hpp.
- No deprecation print for ATtinies.
- Renamed ac_LG.cpp to ac_LG.hpp.
- Maintained MagiQuest by E. Stuart Hicks.
- Improved print Pronto by Asuki Kono.
- Added printActiveIRProtocols() function.
- Used IR_SEND_PIN to reduce code size and improved send timing for AVR.

## 3.4.0
- Added LG2 protocol.
- Added ATtiny167 (Digispark Pro) support.
- Renamed *.cpp.h to .hpp.
- organized carrier frequencies.
- Compiler switch USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN added.
- Moved blink13() back to IRrecv class.
- Added Kaseikyo convenience functions like sendKaseikyo_Denon().
- Improved / adjusted LG protocol and added class Aircondition_LG based on real hardware supplied by makerspace 201 (https://wiki.hackerspaces.org/ZwoNullEins) from Cologne.
- Improved universal decoder for pulse width or pulse distance protocols to support more than 32 bits.
- Added mbed support.

## 3.3.0
- Fix errors if LED_BUILTIN is not defined.
- Fixed error for AVR timer1. Thanks to alexbarcelo.
- New example IRremoteExtensionTest.
- Enabled megaAVR 0-series devices.
- Added universal decoder for pulse width or pulse distance protocols.

## 3.2.0
- Fix for ESP32 send Error, removed `USE_SOFT_SEND_PWM` macro.
- Added Onkyo protocol.
- Support for old 2.x code by backwards compatible `decode(decode_results *aResults)` function.
- Removed USE_OLD_DECODE macro and added NO_LEGACY_COMPATIBILITY macro.
- Added ATtiny1604 support.
- New SendAndReceive example.
- Added ESP8266 support.
- Extended DEBUG output.

## 3.1.0
- Generation of PWM by software is active by default.
- Removed decode_results results.
- Renamed most irparams_struct values.
- Fixed LG send bug and added unit test.
- Replaced `#define DECODE_NEC 1/0` by defining/not defining.
- Use LED_BUILTIN instead of FEEDBACK_LED if FeedbackLEDPin is 0.
- Use F_CPU instead of SYSCLOCK.
- Removed SENDPIN_ON and SENDPIN_OFF macros.

- Refactored board specific code for timer and feedback LED.
- Extracted common LED feedback functions and implemented feedback for send.
- MATCH_MARK() etc. now available as matchMark().
- Added STM32F1 by (by Roger Clark) support.
- Added stm32 (by ST) support. Thanks to Paolo Malaspina.
- Added ATtiny88 support.

## 3.0.2
- Bug fix for USE_OLD_DECODE.
- Increase RECORD_GAP_MICROS to 11000.
- Fix overflow message. (#793).
- Improved handling for HASH decoder.
- Tested for ATtiny85.
- Added `printIRResultMinimal()`.
- Added missing IRAM_ATTR for ESP32.
- Adapted to TinyCore 0.0.7.
- Fixed decodeSony 20 bit bug #811.
- Replaced delayMicroseconds with customDelayMicroseconds and removed NoInterrupt() for send functions, removed SPIN_WAIT macro, sleepMicros() and sleepUntilMicros().
- Fixed LG checksum error.
- Fixed JVC repeat error.

## 3.0.0 + 3.0.1 2021/02
- New LSB first decoders are default.
- Added SendRaw with byte data.
- Fixed resume bug if irparams.rawlen >= RAW_BUFFER_LENGTH. Thanks to Iosif Peterfi
- Added `dumpPronto(String *aString, unsigned int frequency)` with String object as argument. Thanks to Iosif Peterfi
- Removed Test2 example.
- Fixed swapped cases in `getProtocolString()`. Thanks to Jim-2249
- Added compile option `IR_INPUT_IS_ACTIVE_HIGH`. Thanks to Jim-2249
- Corrected template. Thanks to Jim-2249
- Introduced standard decode and send functions.
- Added compatibility with tone for AVR's.
- New TinyIRreceiver does not require a timer.
- New MinimalReceiver and IRDispatcherDemo examples.
- Added TinyCore 32 / ATtiny3217 support.
- Added Apple protocol.

## 2.8.1 2020/10
- Fixed bug in Sony decode introduced in 2.8.0.

## 2.8.0 2020/10
- Changed License to MIT see https://github.com/Arduino-IRremote/Arduino-IRremote/issues/397.
- Added ATtiny timer 1 support.
- Changed wrong return code signature of decodePulseDistanceData() and its handling.
- Removed Mitsubishi protocol, since the implementation is in contradiction with all documentation I found and therefore supposed to be wrong.
- Removed AIWA implementation, since it is only for 1 device and at least the sending was implemented wrong.
- Added Lego_PF decode.
- Changed internal usage of custom_delay_usec.
- Moved dump/print functions from example to irReceiver.
- irPronto.cpp: Using Print instead of Stream saves 1020 bytes program memory. Changed from & to * parameter type to be more transparent and consistent with other code of IRremote.

## 2.7.0 2020/09
- Added ATmega328PB support.
- Renamed hardware specific macro and function names.
- Renamed `USE_SOFT_CARRIER`, `USE_NO_CARRIER`, `DUTY_CYCLE` macros to `USE_SOFT_SEND_PWM`, `USE_NO_SEND_PWM`, `IR_SEND_DUTY_CYCLE`.
- Removed blocking wait for ATmega32U4 Serial in examples.
- Deactivated default debug output.
- Optimized types in sendRC5ext and sendSharpAlt.
- Added `DECODE_NEC_STANDARD` and `SEND_NEC_STANDARD`.
- Renamed all IRrecv* examples to IRreceive*.
- Added functions `printIRResultShort(&Serial)` and `getProtocolString(decode_type_t aDecodeType)`.
- Added flag `decodedIRData.isRepeat`.
- Updated examples.

## 2.6.1 2020/08
- Adjusted JVC and LG timing.
- Fixed 4809 bug.

## 2.6.0 2020/08
- Added support for MagiQuest IR wands.
- Corrected Samsung timing.
- NEC repeat implementation.
- Formatting and changing `TIMER_CONFIG_KHZ` and `TIMER_CONFIG_NORMAL` macros to static functions.
- Added `IRAM_ATTR` for ESP32 ISR.
- Removed `#define HAS_AVR_INTERRUPT_H`.
- Changed Receiver States. Now starting with 0.
- Changed switch to if / else if in IRRemote.cpp because of ESP32 compiler bug.
- Changed `DEBUG` handling since compiler warns about empty "IF" or "ELSE" statements in IRRemote.cpp.

## 2.5.0 2020/06
- Corrected keywords.txt.
- BoseWave protocol added PR #690.
- Formatting comply to the new stylesheet.
- Renamed "boarddefs.h" [ISSUE #375](https://github.com/Arduino-IRremote/Arduino-IRremote/issues/375).
- Renamed `SEND_PIN` to `IR_SEND_PIN`.
- Renamed state macros.
- Enabled `DUTY_CYCLE` for send signal.
- Added sending for ESP32.
- Changed rawlen from uint8_t to unsigned int allowing bigger receive buffer and renamed `RAWBUF` to `RAW_BUFFER_LENGTH`.
- Introduced `USE_NO_CARRIER` for simulating an IR receiver.
Changes from #283 by bengtmartensson
- Added function sendRaw_P() for sending data from flash.
Changes from #268 by adamlhumphreys
- Optimized by reducing floating point operations as suggested by madmalkav (#193).
- Optimized with macros when using default `MICROS_PER_TICK` and `TOLERANCE`.
- Made decodeHash as a settable protocol defined by `DECODE_HASH`.
- Added Philips Extended RC-5 protocol support [PR #522] (https://github.com/Arduino-IRremote/Arduino-IRremote/pull/522)

## 2.4.0 - 2017/08/10
 - Cleanup of hardware dependencies. Merge in SAM support [PR #437](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/437)

## 2.3.3 - 2017/03/31
- Added ESP32 IR receive support [PR #427](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/425)

## 2.2.3 - 2017/03/27
- Fix calculation of pause length in LEGO PF protocol [PR #427](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/427)

## 2.2.2 - 2017/01/20
- Fixed naming bug [PR #398](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/398)

## 2.2.1 - 2016/07/27
- Added tests for Lego Power Functions Protocol [PR #336](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/336)

## 2.2.0 - 2016/06/28
- Added support for ATmega8535
- Added support for ATmega16
- Added support for ATmega32
- Added support for ATmega164
- Added support for ATmega324
- Added support for ATmega644
- Added support for ATmega1284
- Added support for ATmega64
- Added support for ATmega128

[PR](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/324)

## 2.1.1 - 2016/05/04
- Added Lego Power Functions Protocol [PR #309](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/309)

## 2.1.0 - 2016/02/20
- Improved Debugging [PR #258](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/258)
- Display TIME instead of TICKS [PR #258](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/258)

## 2.0.4 - 2016/02/20
- Add Panasonic and JVC to IRrecord example [PR](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/54)

## 2.0.3 - 2016/02/20
- Change IRSend Raw parameter to const [PR](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/227)

## 2.0.2 - 2015/12/02
- Added IRremoteInfo Sketch - [PR](https://github.com/Arduino-IRremote/Arduino-IRremote/pull/241)
- Enforcing changelog.md

## 2.0.1 - 2015/07/26 - [Release](https://github.com/shirriff/Arduino-IRremote/releases/tag/BETA)
### Changes
- Updated README
- Updated Contributors
- Fixed #110 Mess
- Created Gitter Room
- Added Gitter Badge
- Standardised Code Base
- Clean Debug Output
- Optimized Send Loops
- Modularized Design
- Optimized and Updated Examples
- Improved Documentation
- Fixed and Improved many coding errors
- Fixed Aiwa RC-T501 Decoding
- Fixed Interrupt on ATmega8
- Switched to Stable Release of PlatformIO

### Additions
- Added Aiwa RC-T501 Protocol
- Added Denon Protocol
- Added Pronto Support
- Added compile options
- Added Template For New Protocols
- Added this changelog
- Added Teensy LC Support
- Added ATtiny84 Support
- Added ATtiny85 Support
- Added isIdle method

### Deletions
- Removed (Fixed) #110
- Broke Teensy 3 / 3.1 Support

### Not Working
- Teensy 3 / 3.1 Support is in Development
