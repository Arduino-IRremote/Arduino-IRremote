/*
 *  TinyIRReceiver.hpp
 *
 *  Receives IR protocol data of NEC protocol using pin change interrupts.
 *  NEC is the protocol of most cheap remote controls for Arduino.
 *
 *  No parity check is done!
 *  On a completely received IR command, the user function handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition)
 *  is called in interrupt context but with interrupts being enabled to enable use of delay() etc.
 *  !!!!!!!!!!!!!!!!!!!!!!
 *  Functions called in interrupt context should be running as short as possible,
 *  so if you require longer action, save the data (address + command) and handle them in the main loop.
 *  !!!!!!!!!!!!!!!!!!!!!
 *
 *
 *  Copyright (C) 2021-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  TinyIRReceiver is free software: you can redistribute it and/or modify
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/*
 * This library can be configured at compile time by the following options / macros:
 * For more details see: https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library (scroll down)
 *
 * - IR_INPUT_PIN           The pin number for TinyIRReceiver IR input.
 * - IR_FEEDBACK_LED_PIN    The pin number for TinyIRReceiver feedback LED.
 * - NO_LED_FEEDBACK_CODE   Disables the feedback LED function. Saves 14 bytes program memory.
 *
 */

#ifndef _TINY_IR_RECEIVER_HPP
#define _TINY_IR_RECEIVER_HPP

#include <Arduino.h>

// - DISABLE_NEC_SPECIAL_REPEAT_SUPPORT    // Activating this disables detection of full NEC frame repeats. Saves 40 bytes program memory.

#include "TinyIRReceiver.h" // If not defined, it defines IR_INPUT_PIN, IR_FEEDBACK_LED_PIN and TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT

#include "digitalWriteFast.h"
/** \addtogroup TinyReceiver Minimal receiver for NEC protocol
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
//#define LOCAL_TRACE_STATE_MACHINE  // to see the state of the ISR state machine
#endif

//#define _IR_MEASURE_TIMING        // Activate this if you want to enable internal hardware timing measurement.
//#define _IR_TIMING_TEST_PIN 7
TinyIRReceiverStruct TinyIRReceiverControl;

/*
 * Set input pin and output pin definitions etc.
 */
#if !defined(IR_INPUT_PIN)
#if defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
#warning "IR_INPUT_PIN is not defined, so it is set to 10"
#define IR_INPUT_PIN    10
#else
#warning "IR_INPUT_PIN is not defined, so it is set to 2"
#define IR_INPUT_PIN    2
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
|| ( (defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && ( (defined(ARDUINO_AVR_DIGISPARKPRO) && ((IR_INPUT_PIN == 3) || (IR_INPUT_PIN == 9))) /*ATtinyX7(digisparkpro) and pin 3 or 9 */\
        || (! defined(ARDUINO_AVR_DIGISPARKPRO) && ((IR_INPUT_PIN == 3) || (IR_INPUT_PIN == 14)))) ) /*ATtinyX7(ATTinyCore) and pin 3 or 14 */ \
)
#define TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT // Cannot use any static ISR vector here. In other cases we have code provided for generating interrupt on pin change.
#endif

/**
 * Declaration of the callback function provided by the user application.
 * It is called every time a complete IR command or repeat was received.
 */
#if defined(ESP32) || defined(ESP8266)
extern void IRAM_ATTR handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition);
#else
extern void handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition);
#endif

/**
 * The ISR of TinyIRRreceiver.
 * It handles the NEC protocol decoding and calls the user callback function on complete.
 * 5 us + 3 us for push + pop for a 16MHz ATmega
 */
#if defined(ESP32) || defined(ESP8266)
void IRAM_ATTR IRPinChangeInterruptHandler(void)
#else
void IRPinChangeInterruptHandler(void)
#endif
        {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
    /*
     * Save IR input level
     * Negative logic, true / HIGH means inactive / IR space, LOW / false means IR mark.
     */
    uint_fast8_t tIRLevel = digitalReadFast(IR_INPUT_PIN);

#if !defined(NO_LED_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, !tIRLevel);
#endif

    /*
     * 1. compute microseconds after last change
     */
    uint32_t tCurrentMicros = micros();
#if defined(DISABLE_NEC_SPECIAL_REPEAT_SUPPORT)
    uint16_t tMicrosOfMarkOrSpace = tCurrentMicros - TinyIRReceiverControl.LastChangeMicros;
#else
    uint32_t tMicrosOfMarkOrSpace32 = tCurrentMicros - TinyIRReceiverControl.LastChangeMicros;
    uint16_t tMicrosOfMarkOrSpace = tMicrosOfMarkOrSpace32;
#endif
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
        if (tMicrosOfMarkOrSpace > 2 * NEC_HEADER_MARK) {
            // timeout -> must reset state machine
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK) {
            // We are at the beginning of the header mark, check timing at the next transition
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_SPACE;
            TinyIRReceiverControl.IRRepeatFrameDetected = false; // If we do it here, it saves 4 bytes
#if !defined(DISABLE_NEC_SPECIAL_REPEAT_SUPPORT)
            // Check for special repeat, where full frame is sent again after 110 ms
            // Must use 32 bit arithmetic here!
            TinyIRReceiverControl.IRRepeatDistanceDetected = (tMicrosOfMarkOrSpace32 < NEC_MAXIMUM_REPEAT_SPACE);
#endif
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK) {
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_SPACE)) {
                /*
                 * We have a valid data header space here -> initialize data
                 */
                TinyIRReceiverControl.IRRawDataBitCounter = 0;
                TinyIRReceiverControl.IRRawData.ULong = 0;
                TinyIRReceiverControl.IRRawDataMask = 1;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
            } else if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && TinyIRReceiverControl.IRRawDataBitCounter >= NEC_BITS) {
                /*
                 * We have a repeat header here and no broken receive before -> set repeat flag
                 */
                TinyIRReceiverControl.IRRepeatFrameDetected = true;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
            } else {
                // This parts are optimized by the compiler into jumps to one code :-)
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK) {
            // Check data space length
            if (tMicrosOfMarkOrSpace >= lowerValue50Percent(NEC_ZERO_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue50Percent(NEC_ONE_SPACE)) {
                // We have a valid bit here
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
                if (tMicrosOfMarkOrSpace >= 2 * NEC_UNIT) {
                    // we received a 1
                    TinyIRReceiverControl.IRRawData.ULong |= TinyIRReceiverControl.IRRawDataMask;
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
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_MARK)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_MARK)) {
                tState = IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK;
            } else {
                // Wrong length of header mark -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE) {
            // Check data mark length
            if (tMicrosOfMarkOrSpace >= lowerValue50Percent(NEC_BIT_MARK)
                    && tMicrosOfMarkOrSpace <= upperValue50Percent(NEC_BIT_MARK)) {
                /*
                 * We have a valid mark here, check for transmission complete, i.e. the mark of the stop bit
                 */
                if (TinyIRReceiverControl.IRRawDataBitCounter >= NEC_BITS || TinyIRReceiverControl.IRRepeatFrameDetected) {
                    /*
                     * Code complete -> call callback, no parity check!
                     */
                    // Reset state for new start
                    tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
#if !defined(ARDUINO_ARCH_MBED) && !defined(ESP32) // no Serial etc. in callback for ESP -> no interrupt required, WDT is running!
                    interrupts(); // enable interrupts, so delay() etc. works in callback
#endif
                    /*
                     * Address reduction to 8 bit
                     */
                    if (TinyIRReceiverControl.IRRawData.UByte.LowByte
                            == (uint8_t) (~TinyIRReceiverControl.IRRawData.UByte.MidLowByte)) {
                        // standard 8 bit address NEC protocol
                        TinyIRReceiverControl.IRRawData.UByte.MidLowByte = 0; // Address is the first 8 bit
                    }

                    /*
                     * Call user provided callback here
                     */
                    handleReceivedTinyIRData(TinyIRReceiverControl.IRRawData.UWord.LowWord,
                            TinyIRReceiverControl.IRRawData.UByte.MidHighByte, (TinyIRReceiverControl.IRRepeatFrameDetected
#if !defined(DISABLE_NEC_SPECIAL_REPEAT_SUPPORT)
                                    || TinyIRReceiverControl.IRRepeatDistanceDetected
#endif
                            ));

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
 * Sets IR_INPUT_PIN mode to INPUT_PULLUP, if required, sets feedback LED output mode and call enablePCIInterruptForTinyReceiver()
 */
bool initPCIInterruptForTinyReceiver() {
    pinModeFast(IR_INPUT_PIN, INPUT_PULLUP);

#if !defined(NO_LED_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#endif
    return enablePCIInterruptForTinyReceiver();
}

#if defined (LOCAL_DEBUG_ATTACH_INTERRUPT) && !defined(STR)
// Helper macro for getting a macro definition as string
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

/**************************************************
 * Pin to interrupt mapping for different platforms
 **************************************************/
#if defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
#define USE_ATTACH_INTERRUPT_DIRECT

#elif !defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
// Default for all NON AVR platforms
#define USE_ATTACH_INTERRUPT

#else
#  if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define USE_PCIE

#  elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#    if defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_INPUT_PIN == 3)
#define USE_INT0
#      elif (IR_INPUT_PIN == 9)
#define USE_INT1
#      else
#        error "IR_INPUT_PIN must be 9 or 3."
#      endif // if (IR_INPUT_PIN == 9)
#    else // defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_INPUT_PIN == 14)
#define USE_INT0
#      elif (IR_INPUT_PIN == 3)
#define USE_INT1
#      else
#        error "IR_INPUT_PIN must be 14 or 3."
#      endif // if (IR_INPUT_PIN == 14)
#    endif

#  elif (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
#    if (IR_INPUT_PIN == 21)
#define USE_INT0
#    elif (IR_INPUT_PIN == 20)
#define USE_INT1
#    else
#warning "No pin mapping for IR_INPUT_PIN to interrupt found -> attachInterrupt() is used now."
#define USE_ATTACH_INTERRUPT
#    endif

#  else // defined(__AVR_ATtiny25__)
/*
 * ATmegas + ATtiny88 here
 */
#    if (IR_INPUT_PIN == 2)
#define USE_INT0
#    elif (IR_INPUT_PIN == 3)
#define USE_INT1

#    elif IR_INPUT_PIN == 4 || IR_INPUT_PIN == 5 || IR_INPUT_PIN == 6 || IR_INPUT_PIN == 7
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 20 to 23 for port PD4 to PD7 (Arduino pin 4 to 7)
#define USE_PCINT2
#    elif IR_INPUT_PIN == 8 || IR_INPUT_PIN == 9 || IR_INPUT_PIN == 10 || IR_INPUT_PIN == 11 || IR_INPUT_PIN == 12 || IR_INPUT_PIN == 13
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 0 to 5 for port PB0 to PB5 (Arduino pin 8 to 13)
#define USE_PCINT0
#    elif IR_INPUT_PIN == A0 || IR_INPUT_PIN == A1 || IR_INPUT_PIN == A2 || IR_INPUT_PIN == A3 || IR_INPUT_PIN == A4 || IR_INPUT_PIN == A5
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 8 to 13 for port PC0 to PC5 (Arduino pin A0 to A5)
#define USE_PCINT1

#    else
#warning "No pin mapping for IR_INPUT_PIN to interrupt found -> attachInterrupt() is used now."
#define USE_ATTACH_INTERRUPT
#    endif // if (IR_INPUT_PIN == 2)
#  endif // defined(__AVR_ATtiny25__)
#endif // ! defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)

/**
 * Initializes hardware interrupt generation according to IR_INPUT_PIN or use attachInterrupt() function.
 */
bool enablePCIInterruptForTinyReceiver() {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif

#if defined(USE_ATTACH_INTERRUPT) || defined(USE_ATTACH_INTERRUPT_DIRECT)
#  if defined(USE_ATTACH_INTERRUPT)
#if defined(NOT_AN_INTERRUPT)
    if(digitalPinToInterrupt(IR_INPUT_PIN) == NOT_AN_INTERRUPT){
        return false;
    }
#endif
    // costs 112 bytes program space + 4 bytes RAM
    attachInterrupt(digitalPinToInterrupt(IR_INPUT_PIN), IRPinChangeInterruptHandler, CHANGE);
#  else
    // 2.2 us more than version configured with macros and not compatible
    attachInterrupt(IR_INPUT_PIN, IRPinChangeInterruptHandler, CHANGE); // no extra pin mapping here
#  endif

#  if defined(LOCAL_DEBUG_ATTACH_INTERRUPT)
    Serial.println(F("Use attachInterrupt for pin=" STR(IR_INPUT_PIN)));
#  endif

#else
#  if defined(LOCAL_DEBUG_ATTACH_INTERRUPT)
    Serial.println(F("Use static interrupt for pin=" STR(IR_INPUT_PIN)));
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
    PCMSK = _BV(IR_INPUT_PIN);
    // clear interrupt bit
    GIFR |= 1 << PCIF;
    // enable interrupt on next change
    GIMSK |= 1 << PCIE;

#  elif defined(USE_PCINT0)
    PCICR |= _BV(PCIE0);
    PCMSK0 = digitalPinToBitMask(IR_INPUT_PIN);
#  elif defined(USE_PCINT1)
    PCICR |= _BV(PCIE1);
    PCMSK1 = digitalPinToBitMask(IR_INPUT_PIN);
#  elif defined(USE_PCINT2)
    PCICR |= _BV(PCIE2);
    PCMSK2 = digitalPinToBitMask(IR_INPUT_PIN);
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
    detachInterrupt(digitalPinToInterrupt(IR_INPUT_PIN));
#  else
    detachInterrupt(IR_INPUT_PIN);
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
#endif // _TINY_IR_RECEIVER_HPP
