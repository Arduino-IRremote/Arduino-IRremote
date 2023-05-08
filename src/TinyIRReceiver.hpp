/*
 *  TinyIRReceiver.hpp
 *
 *  Receives IR protocol data of NEC protocol using pin change interrupts.
 *  NEC is the protocol of most cheap remote controls for Arduino.
 *
 *  Parity check is done for address and data.
 *  On a completely received IR command, the user function handleReceivedIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags)
 *  is called in interrupt context but with interrupts being enabled to enable use of delay() etc.
 *  !!!!!!!!!!!!!!!!!!!!!!
 *  Functions called in interrupt context should be running as short as possible,
 *  so if you require longer action, save the data (address + command) and handle them in the main loop.
 *  !!!!!!!!!!!!!!!!!!!!!
 *  aFlags can contain one of IRDATA_FLAGS_EMPTY, IRDATA_FLAGS_IS_REPEAT and IRDATA_FLAGS_PARITY_FAILED bits
 *
 * The FAST protocol is a proprietary modified JVC protocol without address, with parity and with a shorter header.
 *  FAST Protocol characteristics:
 * - Bit timing is like NEC or JVC
 * - The header is shorter, 3156 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *     leading to a fixed protocol length of (6 + (16 * 3) + 1) * 526 = 55 * 526 = 28930 microseconds or 29 ms.
 * - Repeats are sent as complete frames but in a 50 ms period / with a 21 ms distance.
 *
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Armin Joachimsmeyer
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
 */

/*
 * This library can be configured at compile time by the following options / macros:
 * For more details see: https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library (scroll down)
 *
 * - IR_RECEIVE_PIN         The pin number for TinyIRReceiver IR input.
 * - IR_FEEDBACK_LED_PIN    The pin number for TinyIRReceiver feedback LED.
 * - NO_LED_FEEDBACK_CODE   Disables the feedback LED function. Saves 14 bytes program memory.
 * - DISABLE_PARITY_CHECKS  Disable parity checks. Saves 48 bytes of program memory.
 * - USE_ONKYO_PROTOCOL     Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value.
 * - USE_FAST_PROTOCOL      Use FAST protocol (no address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command) instead of NEC.
 * - ENABLE_NEC2_REPEATS    Instead of sending / receiving the NEC special repeat code, send / receive the original frame for repeat.
 */

#ifndef _TINY_IR_RECEIVER_HPP
#define _TINY_IR_RECEIVER_HPP

#include <Arduino.h>

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

//#define DISABLE_PARITY_CHECKS // Disable parity checks. Saves 48 bytes of program memory.
//#define USE_ONKYO_PROTOCOL    // Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value.
//#define USE_FAST_PROTOCOL     // Use FAST protocol instead of NEC / ONKYO.
//#define ENABLE_NEC2_REPEATS // Instead of sending / receiving the NEC special repeat code, send / receive the original frame for repeat.

#include "TinyIR.h" // If not defined, it defines IR_RECEIVE_PIN, IR_FEEDBACK_LED_PIN and TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT

#include "digitalWriteFast.h"
/** \addtogroup TinyReceiver Minimal receiver for NEC and FAST protocol
 * @{
 */

#if defined(DEBUG)
#define LOCAL_DEBUG_ATTACH_INTERRUPT
#else
//#define LOCAL_DEBUG_ATTACH_INTERRUPT  // to see if attachInterrupt() or static interrupt (by register tweaking) is used
#endif
#if defined(TRACE)
#define LOCAL_TRACE_STATE_MACHINE
#else
//#define LOCAL_TRACE_STATE_MACHINE  // to see the state of the ISR (Interrupt Service Routine) state machine
#endif

//#define _IR_MEASURE_TIMING        // Activate this if you want to enable internal hardware timing measurement.
//#define _IR_TIMING_TEST_PIN 7
TinyIRReceiverStruct TinyIRReceiverControl;

/*
 * Set input pin and output pin definitions etc.
 */
#if defined(IR_INPUT_PIN)
#warning "IR_INPUT_PIN is deprecated, use IR_RECEIVE_PIN"
#define IR_RECEIVE_PIN  IR_INPUT_PIN
#endif
#if !defined(IR_RECEIVE_PIN)
#if defined(__AVR_ATtiny1616__) || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
#warning "IR_RECEIVE_PIN is not defined, so it is set to 10"
#define IR_RECEIVE_PIN    10
#elif defined(__AVR_ATtiny816__)
#warning "IR_RECEIVE_PIN is not defined, so it is set to 14"
#define IR_RECEIVE_PIN    14
#else
#warning "IR_RECEIVE_PIN is not defined, so it is set to 2"
#define IR_RECEIVE_PIN    2
#endif
#endif

#if !defined(IR_FEEDBACK_LED_PIN) && defined(LED_BUILTIN)
#define IR_FEEDBACK_LED_PIN    LED_BUILTIN
#endif

#if !( \
   (defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)) /* ATtinyX5 */ \
|| defined(__AVR_ATtiny88__) /* MH-ET LIVE Tiny88 */ \
|| defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) \
|| defined(__AVR_ATmega16U4__) || defined(__AVR_ATmega32U4__) \
|| defined(__AVR_ATmega8__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega48P__) || defined(__AVR_ATmega48PB__) || defined(__AVR_ATmega88P__) || defined(__AVR_ATmega88PB__) \
|| defined(__AVR_ATmega168__) || defined(__AVR_ATmega168PA__) || defined(__AVR_ATmega168PB__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) \
  /* ATmegas with ports 0,1,2 above and ATtiny167 only 2 pins below */ \
|| ( (defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && ( (defined(ARDUINO_AVR_DIGISPARKPRO) && ((IR_RECEIVE_PIN == 3) || (IR_RECEIVE_PIN == 9))) /*ATtinyX7(digisparkpro) and pin 3 or 9 */\
        || (! defined(ARDUINO_AVR_DIGISPARKPRO) && ((IR_RECEIVE_PIN == 3) || (IR_RECEIVE_PIN == 14)))) ) /*ATtinyX7(ATTinyCore) and pin 3 or 14 */ \
)
#define TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT // Cannot use any static ISR vector here. In other cases we have code provided for generating interrupt on pin change.
#endif

/**
 * Declaration of the callback function provided by the user application.
 * It is called every time a complete IR command or repeat was received.
 */
extern void handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition);

#if defined(LOCAL_DEBUG)
uint32_t sMicrosOfGap; // The length of the gap before the start bit
#endif
/**
 * The ISR (Interrupt Service Routine) of TinyIRRreceiver.
 * It handles the NEC protocol decoding and calls the user callback function on complete.
 * 5 us + 3 us for push + pop for a 16MHz ATmega
 */
#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif
void IRPinChangeInterruptHandler(void) {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
    /*
     * Save IR input level
     * Negative logic, true / HIGH means inactive / IR space, LOW / false means IR mark.
     */
    uint_fast8_t tIRLevel = digitalReadFast(IR_RECEIVE_PIN);

#if !defined(NO_LED_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, !tIRLevel);
#endif

    /*
     * 1. compute microseconds after last change
     */
    // Repeats can be sent after a pause, which is longer than 64000 microseconds, so we need a 32 bit value for check of repeats
    uint32_t tCurrentMicros = micros();
    uint32_t tMicrosOfMarkOrSpace32 = tCurrentMicros - TinyIRReceiverControl.LastChangeMicros;
    uint16_t tMicrosOfMarkOrSpace = tMicrosOfMarkOrSpace32;

    TinyIRReceiverControl.LastChangeMicros = tCurrentMicros;

    uint8_t tState = TinyIRReceiverControl.IRReceiverState;

#if defined(LOCAL_TRACE_STATE_MACHINE)
    Serial.print(tState);
    Serial.print(F(" D="));
    Serial.print(tMicrosOfMarkOrSpace);
//    Serial.print(F(" I="));
//    Serial.print(tIRLevel);
    Serial.print('|');
#endif

    if (tIRLevel == LOW) {
        /*
         * We have a mark here
         */
        if (tMicrosOfMarkOrSpace > 2 * TINY_RECEIVER_HEADER_MARK) {
            // timeout -> must reset state machine
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK) {
            // We are at the beginning of the header mark, check timing at the next transition
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_SPACE;
            TinyIRReceiverControl.Flags = IRDATA_FLAGS_EMPTY; // If we do it here, it saves 4 bytes
#if defined(LOCAL_TRACE)
            sMicrosOfGap = tMicrosOfMarkOrSpace32;
#endif
#if defined(ENABLE_NEC2_REPEATS)
            // Check for repeat, where full frame is sent again after TINY_RECEIVER_REPEAT_PERIOD ms
            // Not required for NEC, where repeats are detected by a special header space duration
            // Must use 32 bit arithmetic here!
            if (tMicrosOfMarkOrSpace32 < TINY_RECEIVER_MAXIMUM_REPEAT_DISTANCE) {
                TinyIRReceiverControl.Flags = IRDATA_FLAGS_IS_REPEAT;
            }
#endif
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK) {
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(TINY_RECEIVER_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(TINY_RECEIVER_HEADER_SPACE)) {
                /*
                 * We have a valid data header space here -> initialize data
                 */
                TinyIRReceiverControl.IRRawDataBitCounter = 0;
#if (TINY_RECEIVER_BITS > 16)
                TinyIRReceiverControl.IRRawData.ULong = 0;
#else
                TinyIRReceiverControl.IRRawData.UWord = 0;
#endif
                TinyIRReceiverControl.IRRawDataMask = 1;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
#if !defined(ENABLE_NEC2_REPEATS)
                // Check for NEC repeat header
            } else if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && TinyIRReceiverControl.IRRawDataBitCounter >= TINY_RECEIVER_BITS) {
                /*
                 * We have a repeat header here and no broken receive before -> set repeat flag
                 */
                TinyIRReceiverControl.Flags = IRDATA_FLAGS_IS_REPEAT;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
#endif
            } else {
                // This parts are optimized by the compiler into jumps to one code :-)
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK) {
            // Check data space length
            if (tMicrosOfMarkOrSpace >= lowerValue50Percent(TINY_RECEIVER_ZERO_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue50Percent(TINY_RECEIVER_ONE_SPACE)) {
                // We have a valid bit here
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
                if (tMicrosOfMarkOrSpace >= 2 * TINY_RECEIVER_UNIT) {
                    // we received a 1
#if (TINY_RECEIVER_BITS > 16)
                    TinyIRReceiverControl.IRRawData.ULong |= TinyIRReceiverControl.IRRawDataMask;
#else
                    TinyIRReceiverControl.IRRawData.UWord |= TinyIRReceiverControl.IRRawDataMask;
#endif
                } else {
                    // we received a 0 - empty code for documentation
                }
                // prepare for next bit
                TinyIRReceiverControl.IRRawDataMask = TinyIRReceiverControl.IRRawDataMask << 1;
                TinyIRReceiverControl.IRRawDataBitCounter++;
            } else {
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else {
            // error wrong state for the received level, e.g. if we missed one change interrupt -> reset state
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    }

    else {
        /*
         * We have a space here
         */
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_SPACE) {
            /*
             * Check length of header mark here
             */
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(TINY_RECEIVER_HEADER_MARK)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(TINY_RECEIVER_HEADER_MARK)) {
                tState = IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK;
            } else {
                // Wrong length of header mark -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE) {
            // Check data mark length
            if (tMicrosOfMarkOrSpace >= lowerValue50Percent(TINY_RECEIVER_BIT_MARK)
                    && tMicrosOfMarkOrSpace <= upperValue50Percent(TINY_RECEIVER_BIT_MARK)) {
                /*
                 * We have a valid mark here, check for transmission complete, i.e. the mark of the stop bit
                 */
                if (TinyIRReceiverControl.IRRawDataBitCounter >= TINY_RECEIVER_BITS
#if !defined(ENABLE_NEC2_REPEATS)
                        || (TinyIRReceiverControl.Flags & IRDATA_FLAGS_IS_REPEAT) // Do not check for full length received, if we have a short repeat frame
#endif
                        ) {
                    /*
                     * Code complete -> optionally check parity
                     */
                    // Reset state for new start
                    tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;

#if !defined(DISABLE_PARITY_CHECKS) && (TINY_RECEIVER_ADDRESS_BITS == 16) && TINY_RECEIVER_ADDRESS_HAS_8_BIT_PARITY
                    /*
                     * Check address parity
                     * Address is sent first and contained in the lower word
                     */
                    if (TinyIRReceiverControl.IRRawData.UBytes[0] != (uint8_t) (~TinyIRReceiverControl.IRRawData.UBytes[1])) {
#if defined(ENABLE_NEC2_REPEATS)
                        TinyIRReceiverControl.Flags |= IRDATA_FLAGS_PARITY_FAILED; // here we can have the repeat flag already set
#else
                        TinyIRReceiverControl.Flags = IRDATA_FLAGS_PARITY_FAILED; // here we do not check anything, if we have a repeat
#endif
                    }
#endif
#if !defined(DISABLE_PARITY_CHECKS) && (TINY_RECEIVER_COMMAND_BITS == 16) && TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY
                    /*
                     * Check command parity
                     */
#if (TINY_RECEIVER_ADDRESS_BITS > 0)
                    if (TinyIRReceiverControl.IRRawData.UBytes[2] != (uint8_t) (~TinyIRReceiverControl.IRRawData.UBytes[3])) {
#if defined(ENABLE_NEC2_REPEATS)
                        TinyIRReceiverControl.Flags |= IRDATA_FLAGS_PARITY_FAILED;
#else
                        TinyIRReceiverControl.Flags = IRDATA_FLAGS_PARITY_FAILED;
#endif
#  if defined(LOCAL_DEBUG)
                        Serial.print(F("Parity check for command failed. Command="));
                        Serial.print(TinyIRReceiverControl.IRRawData.UBytes[2], HEX);
                        Serial.print(F(" parity="));
                        Serial.println(TinyIRReceiverControl.IRRawData.UBytes[3], HEX);
#  endif
#else
                    // No address, so command and parity are in the lowest bytes
                    if (TinyIRReceiverControl.IRRawData.UBytes[0] != (uint8_t) (~TinyIRReceiverControl.IRRawData.UBytes[1])) {
                        TinyIRReceiverControl.Flags |= IRDATA_FLAGS_PARITY_FAILED;
#  if defined(LOCAL_DEBUG)
                        Serial.print(F("Parity check for command failed. Command="));
                        Serial.print(TinyIRReceiverControl.IRRawData.UBytes[0], HEX);
                        Serial.print(F(" parity="));
                        Serial.println(TinyIRReceiverControl.IRRawData.UBytes[1], HEX);
#  endif
#endif
                    }
#endif
                    /*
                     * Call user provided callback here
                     * The parameter size is dependent of the code variant used in order to save program memory.
                     * We have 6 cases: 0, 8 bit or 16 bit address, each with 8 or 16 bit command
                     */
#if !defined(ARDUINO_ARCH_MBED) && !defined(ESP32) // no Serial etc. in callback for ESP -> no interrupt required, WDT is running!
                    interrupts(); // enable interrupts, so delay() etc. works in callback
#endif
                    handleReceivedTinyIRData(
#if (TINY_RECEIVER_ADDRESS_BITS > 0)
#  if TINY_RECEIVER_ADDRESS_HAS_8_BIT_PARITY
                            // Here we have 8 bit address
                            TinyIRReceiverControl.IRRawData.UBytes[0],
#  else
                            // Here we have 16 bit address
                            TinyIRReceiverControl.IRRawData.UWord.LowWord,
#  endif
#  if TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY
                            // Here we have 8 bit command
                            TinyIRReceiverControl.IRRawData.UBytes[2],
#  else
                            // Here we have 16 bit command
                            TinyIRReceiverControl.IRRawData.UWord.HighWord,
#  endif
#else

                            // Here we have NO address
#  if TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY
                            // Here we have 8 bit command
                            TinyIRReceiverControl.IRRawData.UBytes[0],
#  else
                            // Here we have 16 bit command
                            TinyIRReceiverControl.IRRawData.UWord,
#  endif
#endif
                            TinyIRReceiverControl.Flags);

                } else {
                    // not finished yet
                    tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK;
                }
            } else {
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else {
            // error wrong state for the received level, e.g. if we missed one change interrupt -> reset state
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    }

    TinyIRReceiverControl.IRReceiverState = tState;
#ifdef _IR_MEASURE_TIMING
    digitalWriteFast(_IR_TIMING_TEST_PIN, LOW); // 2 clock cycles
#endif
}

bool isTinyReceiverIdle() {
    return (TinyIRReceiverControl.IRReceiverState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK);
}

/**
 * Sets IR_RECEIVE_PIN mode to INPUT, and if IR_FEEDBACK_LED_PIN is defined, sets feedback LED output mode.
 * Then call enablePCIInterruptForTinyReceiver()
 */
bool initPCIInterruptForTinyReceiver() {
    pinModeFast(IR_RECEIVE_PIN, INPUT);

#if !defined(NO_LED_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#endif
    return enablePCIInterruptForTinyReceiver();
}

#if defined(USE_FAST_PROTOCOL)
void printTinyReceiverResultMinimal(Print *aSerial, uint16_t aCommand, uint8_t aFlags)
#else
void printTinyReceiverResultMinimal(Print *aSerial, uint8_t aAddress, uint8_t aCommand, uint8_t aFlags)
#endif
        {
// Print only very short output, since we are in an interrupt context and do not want to miss the next interrupts of the repeats coming soon
    // Print only very short output, since we are in an interrupt context and do not want to miss the next interrupts of the repeats coming soon
#if defined(USE_FAST_PROTOCOL)
    aSerial->print(F("C=0x"));
#else
    aSerial->print(F("A=0x"));
    aSerial->print(aAddress, HEX);
    aSerial->print(F(" C=0x"));
#endif
    aSerial->print(aCommand, HEX);
    if (aFlags == IRDATA_FLAGS_IS_REPEAT) {
        aSerial->print(F(" R"));
    }
#if !defined(DISABLE_PARITY_CHECKS)
    if (aFlags == IRDATA_FLAGS_PARITY_FAILED) {
        aSerial->print(F(" P"));
    }
#endif
    aSerial->println();
}

#if defined (LOCAL_DEBUG_ATTACH_INTERRUPT) && !defined(STR)
// Helper macro for getting a macro definition as string
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

/**************************************************
 * Pin to interrupt mapping for different platforms
 **************************************************/
#if defined(__AVR_ATtiny816__) || defined(__AVR_ATtiny1616__) || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
#define USE_ATTACH_INTERRUPT_DIRECT

#elif !defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
// Default for all NON AVR platforms
#define USE_ATTACH_INTERRUPT

#else
#  if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define USE_PCIE

#  elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#    if defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_RECEIVE_PIN == 3)
#define USE_INT0
#      elif (IR_RECEIVE_PIN == 9)
#define USE_INT1
#      else
#        error "IR_RECEIVE_PIN must be 9 or 3."
#      endif // if (IR_RECEIVE_PIN == 9)
#    else // defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_RECEIVE_PIN == 14)
#define USE_INT0
#      elif (IR_RECEIVE_PIN == 3)
#define USE_INT1
#      else
#        error "IR_RECEIVE_PIN must be 14 or 3."
#      endif // if (IR_RECEIVE_PIN == 14)
#    endif

#  elif (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
#    if (IR_RECEIVE_PIN == 21)
#define USE_INT0
#    elif (IR_RECEIVE_PIN == 20)
#define USE_INT1
#    else
#warning "No pin mapping for IR_RECEIVE_PIN to interrupt found -> attachInterrupt() is used now."
#define USE_ATTACH_INTERRUPT
#    endif

#  else // defined(__AVR_ATtiny25__)
/*
 * ATmegas + ATtiny88 here
 */
#    if (IR_RECEIVE_PIN == 2)
#define USE_INT0
#    elif (IR_RECEIVE_PIN == 3)
#define USE_INT1

#    elif IR_RECEIVE_PIN == 4 || IR_RECEIVE_PIN == 5 || IR_RECEIVE_PIN == 6 || IR_RECEIVE_PIN == 7
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 20 to 23 for port PD4 to PD7 (Arduino pin 4 to 7)
#define USE_PCINT2
#    elif IR_RECEIVE_PIN == 8 || IR_RECEIVE_PIN == 9 || IR_RECEIVE_PIN == 10 || IR_RECEIVE_PIN == 11 || IR_RECEIVE_PIN == 12 || IR_RECEIVE_PIN == 13
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 0 to 5 for port PB0 to PB5 (Arduino pin 8 to 13)
#define USE_PCINT0
#    elif IR_RECEIVE_PIN == A0 || IR_RECEIVE_PIN == A1 || IR_RECEIVE_PIN == A2 || IR_RECEIVE_PIN == A3 || IR_RECEIVE_PIN == A4 || IR_RECEIVE_PIN == A5
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 8 to 13 for port PC0 to PC5 (Arduino pin A0 to A5)
#define USE_PCINT1

#    else
#warning "No pin mapping for IR_RECEIVE_PIN to interrupt found -> attachInterrupt() is used now."
#define USE_ATTACH_INTERRUPT
#    endif // if (IR_RECEIVE_PIN == 2)
#  endif // defined(__AVR_ATtiny25__)
#endif // ! defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)

/**
 * Initializes hardware interrupt generation according to IR_RECEIVE_PIN or use attachInterrupt() function.
 * @return true if interrupt was successfully enabled
 */
bool enablePCIInterruptForTinyReceiver() {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif

#if defined(USE_ATTACH_INTERRUPT) || defined(USE_ATTACH_INTERRUPT_DIRECT)
#  if defined(USE_ATTACH_INTERRUPT)
#if defined(NOT_AN_INTERRUPT)
    if(digitalPinToInterrupt(IR_RECEIVE_PIN) == NOT_AN_INTERRUPT){
        return false;
    }
#endif
    // costs 112 bytes program memory + 4 bytes RAM
    attachInterrupt(digitalPinToInterrupt(IR_RECEIVE_PIN), IRPinChangeInterruptHandler, CHANGE);
#  else
    // 2.2 us more than version configured with macros and not compatible
    attachInterrupt(IR_RECEIVE_PIN, IRPinChangeInterruptHandler, CHANGE); // no extra pin mapping here
#  endif

#  if defined(LOCAL_DEBUG_ATTACH_INTERRUPT)
    Serial.println(F("Use attachInterrupt for pin=" STR(IR_RECEIVE_PIN)));
#  endif

#else
#  if defined(LOCAL_DEBUG_ATTACH_INTERRUPT)
    Serial.println(F("Use static interrupt for pin=" STR(IR_RECEIVE_PIN)));
#  endif
#  if defined(USE_INT0)
    // interrupt on any logical change
    EICRA |= _BV(ISC00);
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // enable interrupt on next change
    EIMSK |= 1 << INT0;

#  elif defined(USE_INT1)
    EICRA |= _BV(ISC10);
// clear interrupt bit
    EIFR |= 1 << INTF1;
// enable interrupt on next change
    EIMSK |= 1 << INT1;

#  elif defined(USE_PCIE) // For ATtiny85 etc.
    // use PinChangeInterrupt no INT0 for pin PB2
    PCMSK = _BV(IR_RECEIVE_PIN);
    // clear interrupt bit
    GIFR |= 1 << PCIF;
    // enable interrupt on next change
    GIMSK |= 1 << PCIE;

#  elif defined(USE_PCINT0)
    PCICR |= _BV(PCIE0);
    PCMSK0 = digitalPinToBitMask(IR_RECEIVE_PIN);
#  elif defined(USE_PCINT1)
    PCICR |= _BV(PCIE1);
    PCMSK1 = digitalPinToBitMask(IR_RECEIVE_PIN);
#  elif defined(USE_PCINT2)
    PCICR |= _BV(PCIE2);
    PCMSK2 = digitalPinToBitMask(IR_RECEIVE_PIN);
#  else
    return false;
#  endif
#endif // defined(USE_ATTACH_INTERRUPT)
    return true;
}

void disablePCIInterruptForTinyReceiver() {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif

#if defined(USE_ATTACH_INTERRUPT) || defined(USE_ATTACH_INTERRUPT_DIRECT)
#  if defined(USE_ATTACH_INTERRUPT)
    detachInterrupt(digitalPinToInterrupt(IR_RECEIVE_PIN));
#  else
    detachInterrupt(IR_RECEIVE_PIN);
#  endif

#else
#  if defined(USE_INT0)
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT0);

#  elif defined(USE_INT1)
    // clear interrupt bit
    EIFR |= 1 << INTF1;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT1);

#  elif defined(USE_PCIE) // For ATtiny85 etc.
    // clear interrupt bit
    GIFR |= 1 << PCIF;
    // disable interrupt on next change
    GIMSK &= ~(1 << PCIE);

#  elif defined(USE_PCINT0)
    PCICR &= ~(_BV(PCIE0));
#  elif defined(USE_PCINT1)
    PCICR &= ~(_BV(PCIE1));
#  elif defined(USE_PCINT2)
    PCICR &= ~(_BV(PCIE2));

#  endif
#endif // defined(USE_ATTACH_INTERRUPT)
}

/*
 * Specify the right INT0, INT1 or PCINT0 interrupt vector according to different pins and cores.
 * The default value of TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT is set in TinyIRReceiver.h
 */
#if !(defined(USE_ATTACH_INTERRUPT) || defined(USE_ATTACH_INTERRUPT_DIRECT))
#  if defined(USE_INT0)
ISR(INT0_vect)

#  elif defined(USE_INT1)
ISR(INT1_vect)

#  elif defined(USE_PCIE) // For ATtiny85 etc.
// on ATtinyX5 we do not have a INT1_vect but we can use the PCINT0_vect
ISR(PCINT0_vect)

#  elif defined(USE_PCINT0)
ISR(PCINT0_vect)
#  elif defined(USE_PCINT1)
ISR(PCINT1_vect)
#  elif defined(USE_PCINT2)
ISR(PCINT2_vect)
#  else
void dummyFunctionToAvoidCompilerErrors()
#  endif
{
    IRPinChangeInterruptHandler();
}
#endif // !(defined(USE_ATTACH_INTERRUPT) || defined(USE_ATTACH_INTERRUPT_DIRECT))

/** @}*/

#if defined(LOCAL_DEBUG_ATTACH_INTERRUPT)
#undef LOCAL_DEBUG_ATTACH_INTERRUPT
#endif
#if defined(LOCAL_TRACE_STATE_MACHINE)
#undef LOCAL_TRACE_STATE_MACHINE
#endif

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _TINY_IR_RECEIVER_HPP
