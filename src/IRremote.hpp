/**
 * @file IRremote.hpp
 *
 * @brief Public API to the library.
 *
 * @code
 * !!! All the macro values defined here can be overwritten with values,    !!!
 * !!! the user defines in its source code BEFORE the #include <IRremote.hpp> !!!
 * @endcode
 *
 * This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2015-2023 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 *
 * For Ken Shiriffs original blog entry, see http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html
 * Initially influenced by:
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * and http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

/*
 * This library can be configured at compile time by the following options / macros:
 * For more details see: https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library
 *
 * - RAW_BUFFER_LENGTH                  Buffer size of raw input buffer. Must be even! 100 is sufficient for *regular* protocols of up to 48 bits.
 * - IR_SEND_PIN                        If specified (as constant), reduces program size and improves send timing for AVR.
 * - SEND_PWM_BY_TIMER                  Disable carrier PWM generation in software and use (restricted) hardware PWM.
 * - USE_NO_SEND_PWM                    Use no carrier PWM, just simulate an **active low** receiver signal. Overrides SEND_PWM_BY_TIMER definition.
 * - USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN Use or simulate open drain output mode at send pin. Attention, active state of open drain is LOW, so connect the send LED between positive supply and send pin!
 * - EXCLUDE_EXOTIC_PROTOCOLS           If activated, BANG_OLUFSEN, BOSEWAVE, WHYNTER, FAST and LEGO_PF are excluded in decode() and in sending with IrSender.write().
 * - EXCLUDE_UNIVERSAL_PROTOCOLS        If activated, the universal decoder for pulse distance protocols and decodeHash (special decoder for all protocols) are excluded in decode().
 * - DECODE_*                           Selection of individual protocols to be decoded. See below.
 * - MARK_EXCESS_MICROS                 Value is subtracted from all marks and added to all spaces before decoding, to compensate for the signal forming of different IR receiver modules.
 * - RECORD_GAP_MICROS                  Minimum gap between IR transmissions, to detect the end of a protocol.
 * - FEEDBACK_LED_IS_ACTIVE_LOW         Required on some boards (like my BluePill and my ESP8266 board), where the feedback LED is active low.
 * - NO_LED_FEEDBACK_CODE               This completely disables the LED feedback code for send and receive.
 * - IR_INPUT_IS_ACTIVE_HIGH            Enable it if you use a RF receiver, which has an active HIGH output signal.
 * - IR_SEND_DUTY_CYCLE_PERCENT         Duty cycle of IR send signal.
 * - MICROS_PER_TICK                    Resolution of the raw input buffer data. Corresponds to 2 pulses of each 26.3 us at 38 kHz.
 * - IR_USE_AVR_TIMER*                  Selection of timer to be used for generating IR receiving sample interval.
 */

#ifndef _IR_REMOTE_HPP
#define _IR_REMOTE_HPP

#include "IRVersion.h"

// activate it for all cores that does not use the -flto flag, if you get false error messages regarding begin() during compilation.
//#define SUPPRESS_ERROR_MESSAGE_FOR_BEGIN

/*
 * If activated, BANG_OLUFSEN, BOSEWAVE, MAGIQUEST, WHYNTER, FAST and LEGO_PF are excluded in decoding and in sending with IrSender.write
 */
//#define EXCLUDE_EXOTIC_PROTOCOLS
/****************************************************
 *                     PROTOCOLS
 ****************************************************/
/*
 * Supported IR protocols
 * Each protocol you include costs memory and, during decode, costs time
 * Copy the lines with the protocols you need in your program before the  #include <IRremote.hpp> line
 * See also SimpleReceiver example
 */

#if !defined(NO_DECODER) // for sending raw only
#  if (!(defined(DECODE_DENON) || defined(DECODE_JVC) || defined(DECODE_KASEIKYO) \
|| defined(DECODE_PANASONIC) || defined(DECODE_LG) || defined(DECODE_NEC) || defined(DECODE_ONKYO) || defined(DECODE_SAMSUNG) \
|| defined(DECODE_SONY) || defined(DECODE_RC5) || defined(DECODE_RC6) \
|| defined(DECODE_DISTANCE_WIDTH) || defined(DECODE_HASH) || defined(DECODE_BOSEWAVE) \
|| defined(DECODE_LEGO_PF) || defined(DECODE_MAGIQUEST) || defined(DECODE_FAST) || defined(DECODE_WHYNTER)))
/*
 * If no protocol is explicitly enabled, we enable all protocols
 */
#define DECODE_DENON        // Includes Sharp
#define DECODE_JVC
#define DECODE_KASEIKYO
#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
#define DECODE_LG
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_SAMSUNG
#define DECODE_SONY
#define DECODE_RC5
#define DECODE_RC6

#    if !defined(EXCLUDE_EXOTIC_PROTOCOLS) // saves around 2000 bytes program memory
#define DECODE_BOSEWAVE
#define DECODE_LEGO_PF
#define DECODE_MAGIQUEST
#define DECODE_WHYNTER
#define DECODE_FAST
#    endif

#    if !defined(EXCLUDE_UNIVERSAL_PROTOCOLS)
#define DECODE_DISTANCE_WIDTH     // universal decoder for pulse distance width protocols - requires up to 750 bytes additional program memory
#define DECODE_HASH         // special decoder for all protocols - requires up to 250 bytes additional program memory
#    endif
#  endif
#endif // !defined(NO_DECODER)

//#define DECODE_BEO // Bang & Olufsen protocol always must be enabled explicitly. It prevents decoding of SONY!

#if defined(DECODE_NEC) && !(~(~DECODE_NEC + 0) == 0 && ~(~DECODE_NEC + 1) == 1)
#warning "The macros DECODE_XXX no longer require a value. Decoding is now switched by defining / non defining the macro."
#endif

//#define DEBUG // Activate this for lots of lovely debug output from the IRremote core.

/****************************************************
 *                    RECEIVING
 ****************************************************/
/**
 * MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
 * to compensate for the signal forming of different IR receiver modules
 * For Vishay TSOP*, marks tend to be too long and spaces tend to be too short.
 * If you set MARK_EXCESS_MICROS to approx. 50us then the TSOP4838 works best.
 * At 100us it also worked, but not as well.
 * Set MARK_EXCESS to 100us and the VS1838 doesn't work at all.
 *
 * The right value is critical for IR codes using short pulses like Denon / Sharp / Lego
 *
 *  Observed values:
 *  Delta of each signal type is around 50 up to 100 and at low signals up to 200. TSOP is better, especially at low IR signal level.
 *  VS1838      Mark Excess -50 at low intensity to +50 us at high intensity
 *  TSOP31238   Mark Excess 0 to +50
 */
#if !defined(MARK_EXCESS_MICROS)
// To change this value, you simply can add a line #define "MARK_EXCESS_MICROS <My_new_value>" in your ino file before the line "#include <IRremote.hpp>"
#define MARK_EXCESS_MICROS    20
#endif

/**
 * Minimum gap between IR transmissions, to detect the end of a protocol.
 * Must be greater than any space of a protocol e.g. the NEC header space of 4500 us.
 * Must be smaller than any gap between a command and a repeat; e.g. the retransmission gap for Sony is around 15 ms for Sony20 protocol.
 * Keep in mind, that this is the delay between the end of the received command and the start of decoding.
 */
#if !defined(RECORD_GAP_MICROS)
// To change this value, you simply can add a line #define "RECORD_GAP_MICROS <My_new_value>" in your *.ino file before the line "#include <IRremote.hpp>"
// Maximum value for RECORD_GAP_MICROS, which fit into 8 bit buffer, using 50 us as tick, is 12750
#define RECORD_GAP_MICROS   8000 // RECS80 (https://www.mikrocontroller.net/articles/IRMP#RECS80) 1 bit space is 7500µs , NEC header space is 4500
#endif
/**
 * Threshold for warnings at printIRResult*() to report about changing the RECORD_GAP_MICROS value to a higher value.
 */
#if !defined(RECORD_GAP_MICROS_WARNING_THRESHOLD)
// To change this value, you simply can add a line #define "RECORD_GAP_MICROS_WARNING_THRESHOLD <My_new_value>" in your *.ino file before the line "#include <IRremote.hpp>"
#define RECORD_GAP_MICROS_WARNING_THRESHOLD   15000
#endif

/** Minimum gap between IR transmissions, in MICROS_PER_TICK */
#define RECORD_GAP_TICKS    (RECORD_GAP_MICROS / MICROS_PER_TICK)

/*
 * Activate this line if your receiver has an external output driver transistor / "inverted" output
 */
//#define IR_INPUT_IS_ACTIVE_HIGH
#if defined(IR_INPUT_IS_ACTIVE_HIGH)
// IR detector output is active high
#define INPUT_MARK   1 ///< Sensor output for a mark ("flash")
#else
// IR detector output is active low
#define INPUT_MARK   0 ///< Sensor output for a mark ("flash")
#endif
/****************************************************
 *                     SENDING
 ****************************************************/
/**
 * Define to disable carrier PWM generation in software and use (restricted) hardware PWM.
 */
//#define SEND_PWM_BY_TIMER // restricts send pin on many platforms to fixed pin numbers
#if (defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(PARTICLE)) || defined(ARDUINO_ARCH_MBED)
#  if !defined(SEND_PWM_BY_TIMER)
#define SEND_PWM_BY_TIMER       // the best and default method for ESP32 etc.
#warning INFO: For ESP32, RP2040, mbed and particle boards SEND_PWM_BY_TIMER is enabled by default, since we have the resources and timing is more exact than the software generated one. If this is not intended, deactivate the line in IRremote.hpp over this warning message in file IRremote.hpp.
#  endif
#else
#  if defined(SEND_PWM_BY_TIMER)
#    if defined(IR_SEND_PIN)
#undef IR_SEND_PIN // to avoid warning 3 lines later
#warning Since SEND_PWM_BY_TIMER is defined, the existing value of IR_SEND_PIN is discarded and replaced by the value determined by timer used for PWM generation
#    endif
#define IR_SEND_PIN     DeterminedByTimer // must be set here, since it is evaluated at IRremoteInt.h, before the include of private/IRTimer.hpp
#  endif
#endif

/**
 * Define to use no carrier PWM, just simulate an active low receiver signal.
 */
//#define USE_NO_SEND_PWM
#if defined(SEND_PWM_BY_TIMER) && defined(USE_NO_SEND_PWM)
#warning "SEND_PWM_BY_TIMER and USE_NO_SEND_PWM are both defined -> undefine SEND_PWM_BY_TIMER now!"
#undef SEND_PWM_BY_TIMER // USE_NO_SEND_PWM overrides SEND_PWM_BY_TIMER
#endif

/**
 * Define to use or simulate open drain output mode at send pin.
 * Attention, active state of open drain is LOW, so connect the send LED between positive supply and send pin!
 */
//#define USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN
#if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
#warning Pin mode OUTPUT_OPEN_DRAIN is not supported on this platform -> mimick open drain mode by switching between INPUT and OUTPUT mode.
#endif
/**
 * This amount is subtracted from the on-time of the pulses generated for software PWM generation.
 * It should be the time used for digitalWrite(sendPin, LOW) and the call to delayMicros()
 * Measured value for Nano @16MHz is around 3000, for Bluepill @72MHz is around 700, for Zero 3600
 */
#if !defined(PULSE_CORRECTION_NANOS)
#  if defined(F_CPU)
// To change this value, you simply can add a line #define "PULSE_CORRECTION_NANOS <My_new_value>" in your ino file before the line "#include <IRremote.hpp>"
#define PULSE_CORRECTION_NANOS (48000L / (F_CPU/MICROS_IN_ONE_SECOND)) // 3000 @16MHz, 666 @72MHz
#  else
#define PULSE_CORRECTION_NANOS 600
#  endif
#endif

/**
 * Duty cycle in percent for sent signals.
 */
#if ! defined(IR_SEND_DUTY_CYCLE_PERCENT)
#define IR_SEND_DUTY_CYCLE_PERCENT 30 // 30 saves power and is compatible to the old existing code
#endif

/**
 * microseconds per clock interrupt tick
 */
#if ! defined(MICROS_PER_TICK)
#define MICROS_PER_TICK    50L // must be with L to get 32 bit results if multiplied with rawbuf[] content.
#endif

#define MILLIS_IN_ONE_SECOND 1000L
#define MICROS_IN_ONE_SECOND 1000000L
#define MICROS_IN_ONE_MILLI 1000L

#include "IRremoteInt.h"
/*
 * We always use digitalWriteFast() and digitalReadFast() functions to have a consistent mapping for pins.
 * For most non AVR cpu's, it is just a mapping to digitalWrite() and digitalRead() functions.
 */
#include "digitalWriteFast.h"

#if !defined(USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE)
#include "private/IRTimer.hpp"  // defines IR_SEND_PIN for AVR and SEND_PWM_BY_TIMER

#  if !defined(NO_LED_FEEDBACK_CODE)
#    if !defined(LED_BUILTIN)
/*
 * print a warning
 */
#warning INFO: No definition for LED_BUILTIN found -> default LED feedback is disabled.
#    endif
#include "IRFeedbackLED.hpp"
#  else
void disableLEDFeedback() {}; // dummy function for examples
#  endif

#include "LongUnion.h" // used in most decoders

/*
 * Include the sources here to enable compilation with macro values set by user program.
 */
#include "IRProtocol.hpp" // must be first, it includes definition for PrintULL (unsigned long long)
#if !defined(DISABLE_CODE_FOR_RECEIVER)
#include "IRReceive.hpp"
#endif
#include "IRSend.hpp"

/*
 * Include the sources of all decoders here to enable compilation with macro values set by user program.
 */
#include "ir_BangOlufsen.hpp"
#include "ir_BoseWave.hpp"
#include "ir_Denon.hpp"
#include "ir_JVC.hpp"
#include "ir_Kaseikyo.hpp"
#include "ir_Lego.hpp"
#include "ir_LG.hpp"
#include "ir_MagiQuest.hpp"
#include "ir_NEC.hpp"
#include "ir_RC5_RC6.hpp"
#include "ir_Samsung.hpp"
#include "ir_Sony.hpp"
#include "ir_FAST.hpp"
#include "ir_Others.hpp"
#include "ir_Pronto.hpp" // pronto is an universal decoder and encoder
#  if defined(DECODE_DISTANCE_WIDTH)     // universal decoder for pulse distance width protocols - requires up to 750 bytes additional program memory
#include <ir_DistanceWidthProtocol.hpp>
#  endif
#endif // #if !defined(USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE)

/**
 * Macros for legacy compatibility
 */
#define RAWBUF  101  // Maximum length of raw duration buffer
#define REPEAT 0xFFFFFFFF
#define USECPERTICK MICROS_PER_TICK
#define MARK_EXCESS MARK_EXCESS_MICROS

#endif // _IR_REMOTE_HPP
