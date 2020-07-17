## 2.5.0 2020/07
- Added support for MagiQuest IR wands

## 2.5.0 2020/06
- corrected keywords.txt.
- BoseWave protocol added PR #690.
- Formatting comply to the new stylesheet.
- Renamed "boarddefs.h" [ISSUE #375](https://github.com/z3t0/Arduino-IRremote/issues/375).
- Renamed SEND_PIN to IR_SEND_PIN.
- Renamed state macros.
- Enabled DUTY_CYCLE for send signal.
- Added sending for ESP32.
- Changed rawlen from uint8_t to unsigned int allowing bigger receive buffer and renamed RAWBUF to RAW_BUFFER_LENGTH.
- Introduced USE_NO_CARRIER for simulating an IR receiver.
Changes from #283 by bengtmartensson
- Added function sendRaw_P() for sending data from flash.
Changes from #268 by adamlhumphreys
- Optimized by reducing floating point operations as suggested by @madmalkav (#193)
- Optimized with macros when using default MICROS_PER_TICK and TOLERANCE
- Made decodeHash as a settable protocol defined by DECODE_HASH

## 2.5.0 ???
- Added Philips Extended RC-5 protocol support [PR #522] (https://github.com/z3t0/Arduino-IRremote/pull/522)

## 2.4.0 - 2017/08/10
 - Cleanup of hardware dependencies. Merge in SAM support [PR #437](https://github.com/z3t0/Arduino-IRremote/pull/437)

## 2.3.3 - 2017/03/31
- Added ESP32 IR receive support [PR #427](https://github.com/z3t0/Arduino-IRremote/pull/425)

## 2.2.3 - 2017/03/27
- Fix calculation of pause length in LEGO PF protocol [PR #427](https://github.com/z3t0/Arduino-IRremote/pull/427)

## 2.2.2 - 2017/01/20
- Fixed naming bug [PR #398](https://github.com/z3t0/Arduino-IRremote/pull/398)

## 2.2.1 - 2016/07/27
- Added tests for Lego Power Functions Protocol [PR #336](https://github.com/z3t0/Arduino-IRremote/pull/336)

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

[PR](https://github.com/z3t0/Arduino-IRremote/pull/324)

## 2.1.1 - 2016/05/04
- Added Lego Power Functions Protocol [PR #309](https://github.com/z3t0/Arduino-IRremote/pull/309)

## 2.1.0 - 2016/02/20
- Improved Debugging [PR #258](https://github.com/z3t0/Arduino-IRremote/pull/258)
- Display TIME instead of TICKS [PR #258](https://github.com/z3t0/Arduino-IRremote/pull/258)

## 2.0.4 - 2016/02/20
- Add Panasonic and JVC to IRrecord example [PR](https://github.com/z3t0/Arduino-IRremote/pull/54)

## 2.0.3 - 2016/02/20
- Change IRSend Raw parameter to const [PR](https://github.com/z3t0/Arduino-IRremote/pull/227)

## 2.0.2 - 2015/12/02
- Added IRremoteInfo Sketch - [PR](https://github.com/z3t0/Arduino-IRremote/pull/241)
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
- Switched to Stable Release of @PlatformIO

### Additions
- Added Aiwa RC-T501 Protocol
- Added Denon Protocol
- Added Pronto Support
- Added Library Properties
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
