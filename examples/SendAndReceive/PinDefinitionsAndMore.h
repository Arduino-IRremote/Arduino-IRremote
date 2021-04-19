/*
 *  PinDefinitionsAndMore.h
 *
 *  Contains pin definitions for IRremote examples for various platforms
 *  as well as definitions for feedback LED and tone() and includes
 *
 *  Copyright (C) 2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  Arduino-IRremote is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/*
 * Pin mapping table for different platforms
 *
 * Platform     IR input    IR output   Tone
 * -----------------------------------------
 * DEFAULT/AVR  2           3           4
 * ATtinyX5     0           4           3
 * ATtiny167    9           8           5 // Digispark pro number schema
 * ATtiny167    3           2           7
 * ATtiny3217   10          11          3 // TinyCore schema
 * ATtiny1604   2           PA5/3       %
 * SAMD21       3           4           5
 * ESP8266      14 // D5    12 // D6    %
 * ESP32        15          4           %
 * BluePill     PA6         PA7         PA3
 * APOLLO3      11          12          5
 */
//#define IRMP_MEASURE_TIMING // For debugging purposes.
//
#if defined(ESP8266)
#define FEEDBACK_LED_IS_ACTIVE_LOW // The LED on my board is active LOW
#define IR_RECEIVE_PIN          14 // D5
#define IR_RECEIVE_PIN_STRING   "D5"
#define IR_SEND_PIN             12 // D6 - D4/pin 2 is internal LED
#define IR_SEND_PIN_STRING      "D6"
#define tone(a,b,c) void()      // tone() inhibits receive timer
#define noTone(a) void()
#define TONE_PIN                42 // Dummy for examples using it
#define IR_TIMING_TEST_PIN      13 // D7
#define APPLICATION_PIN         0 // D3

#elif defined(ESP32)
#define IR_RECEIVE_PIN          15  // D15
#define IR_SEND_PIN              4  // D4
#define tone(a,b,c) void()      // no tone() available on ESP32
#define noTone(a) void()
#define TONE_PIN                42 // Dummy for examples using it
#define APPLICATION_PIN         16 // RX2 pin

#elif defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_STM32F1)
// BluePill in 2 flavors
// Timer 3 of IRMP blocks PA6, PA7, PB0, PB1 for use by Servo or tone()
#define IR_RECEIVE_PIN          PA6
#define IR_RECEIVE_PIN_STRING   "PA6"
#define IR_SEND_PIN             PA7
#define IR_SEND_PIN_STRING      "PA7"
#define TONE_PIN                PA3
#define IR_TIMING_TEST_PIN      PA5
#define APPLICATION_PIN         PA2

#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#include "ATtinySerialOut.h" // Available as Arduino library. saves 370 bytes program space and 38 bytes RAM for digistump core
#define IR_RECEIVE_PIN  0
#define IR_SEND_PIN     4 // Pin 2 is serial output with ATtinySerialOut. Pin 1 is internal LED and Pin3 is USB+ with pullup on Digispark board.
#define TONE_PIN        3
#define IR_TIMING_TEST_PIN 3

#elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#include "ATtinySerialOut.h"
// For ATtiny167 Pins PB6 and PA3 are usable as interrupt source.
#  if defined(ARDUINO_AVR_DIGISPARKPRO)
#define IR_RECEIVE_PIN   9 // PA3 - on Digispark board labeled as pin 9
//#define IR_RECEIVE_PIN  14 // PB6 / INT0 is connected to USB+ on DigisparkPro boards
#define IR_SEND_PIN      8 // PA2 - on Digispark board labeled as pin 8
#define TONE_PIN         5 // PA7
#define IR_TIMING_TEST_PIN 10 // PA4
#  else
#define IR_RECEIVE_PIN  3
#define IR_SEND_PIN     2
#define TONE_PIN        7
#  endif

#elif defined(__AVR_ATtiny88__) // MH-ET Tiny88 board
#include "ATtinySerialOut.h" // Available as Arduino library. Saves 128 bytes program space
// Pin 6 is TX pin 7 is RX
#define IR_RECEIVE_PIN   3 // INT1
#define IR_SEND_PIN      4
#define TONE_PIN         9
#define IR_TIMING_TEST_PIN 8

#elif defined(__AVR_ATtiny3217__)
#define IR_RECEIVE_PIN  10
#define IR_SEND_PIN     11
#define TONE_PIN         3
#define APPLICATION_PIN  5

#elif defined(__AVR_ATtiny1604__)
#define IR_RECEIVE_PIN   2 // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN      3
#define APPLICATION_PIN  5
#define TONE_PIN        42 // Dummy for examples using it
#define tone(a,b,c) void() // tone() uses the same vector as receive timer
#define noTone(a) void()

#  elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__) || defined(__AVR_ATmega32__) \
|| defined(__AVR_ATmega16__) || defined(__AVR_ATmega8535__) \
|| defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) \
|| defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) \
|| defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__)
#define IR_RECEIVE_PIN      2
#define IR_SEND_PIN        13
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7

#elif defined(ARDUINO_ARCH_APOLLO3)
#define IR_RECEIVE_PIN  11
#define IR_SEND_PIN     12
#define TONE_PIN         5

#elif defined(ARDUINO_ARCH_MBED) // Arduino Nano 33 BLE
#define IR_RECEIVE_PIN      2
#define IR_SEND_PIN         3
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7

#elif defined(TEENSYDUINO)
#define IR_RECEIVE_PIN      2
#define IR_SEND_PIN         3
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7

#elif defined(__AVR__)
#define IR_RECEIVE_PIN      2 // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN         3
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
#define IR_RECEIVE_PIN      2
#define IR_SEND_PIN         3
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7

// On the Zero and others we switch explicitly to SerialUSB
#define Serial SerialUSB

// Definitions for the Chinese SAMD21 M0-Mini clone, which has no led connected to D13/PA17.
// Attention!!! D2 and D4 are switched on these boards!!!
// If you connect the LED, it is on pin 24/PB11. In this case activate the next two lines.
//#undef LED_BUILTIN
//#define LED_BUILTIN 24 // PB11
// As an alternative you can choose pin 25, it is the RX-LED pin (PB03), but active low.In this case activate the next 3 lines.
//#undef LED_BUILTIN
//#define LED_BUILTIN 25 // PB03
//#define FEEDBACK_LED_IS_ACTIVE_LOW // The RX LED on the M0-Mini is active LOW

#else
#warning Board / CPU is not detected using pre-processor symbols -> using default values, which may not fit. Please extend PinDefinitionsAndMore.h.
// Default valued for unidentified boards
#define IR_RECEIVE_PIN      2
#define IR_SEND_PIN         3
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7
#endif // defined(ESP8266)

#if !defined (FLASHEND)
#define FLASHEND 0xFFFF // Dummy value for platforms where FLASHEND is not defined
#endif
/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
