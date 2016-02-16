
## 2.0.3 - 2016/02/16
### Changes
- Updated IRrecvDumpV2.ino to include gap reading to determine send repeat timing
- Fixed false positive REPEAT in Sony and Sanyo protocols due to code errors
- Restored macros for MATCH, MATCH_MARK, and MATCH_SPACE for efficiency
- Updated protocols with separate “offset++;” in place of “[offset++]” to function with macros
- Optimized by reducing floating point operations as suggested by @madmalkav (#193)
- Optimized with macros when using default USECPERTICK and TOLERANCE
- Made decodeHash as a settable protocol defined by DECODE_HASH
- Fixed some minor typos

### Additions
- Added Jensen Protocol [@adamlhumphreys]
- Added Heater Protocol [@adamlhumphreys]
- Added “SHUZU_” prefix to macro variables in ir_Template.cpp to maintain distinction and standardization
- Added notice to use “offset++;” in place of “[offset++]” in ir_Template.cpp for macros
- Added missing semicolon found by @mattman00000 (#271)


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
