/*
 *  TinyIRReceiver.hpp
 *
 *  Receives IR protocol data of NEC protocol using pin change interrupts.
 *  NEC is the protocol of most cheap remote controls for Arduino.
 *
 *  No parity check is done!
 *  On a completely received IR command, the user function handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition)
 *  is called in Interrupt context but with interrupts being enabled to enable use of delay() etc.
 *  !!!!!!!!!!!!!!!!!!!!!!
 *  Functions called in interrupt context should be running as short as possible,
 *  so if you require longer action, save the data (address + command) and handle them in the main loop.
 *  !!!!!!!!!!!!!!!!!!!!!
 *
 *
 *  Copyright (C) 2021-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/ukw100/IRMP.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */
/*
 * This library can be configured at compile time by the following options / macros:
 * For more details see: https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library (scroll down)
 *
 * - IR_INPUT_PIN           The pin number for TinyIRReceiver IR input.
 * - IR_FEEDBACK_LED_PIN    The pin number for TinyIRReceiver feedback LED.
 * - NO_LED_FEEDBACK_CODE   Disables the feedback LED function. Saves 14 bytes program space.
 *
 */

#ifndef TINY_IR_RECEIVER_HPP
#define TINY_IR_RECEIVER_HPP

#include <Arduino.h>

#include "TinyIRReceiver.h" // If not defined, it defines IR_INPUT_PIN, IR_FEEDBACK_LED_PIN and TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT
//#define NO_LED_FEEDBACK_CODE   // Activate this if you want to suppress LED feedback or if you do not have a LED. This saves 2 bytes code and 2 clock cycles per interrupt.

#include "digitalWriteFast.h"
/** \addtogroup TinyReceiver Minimal receiver for NEC protocol
 * @{
 */

//#define DEBUG // to see if attachInterrupt used
//#define TRACE // to see the state of the ISR state machine

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
#if defined(ESP8266)
void ICACHE_RAM_ATTR handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition);
#elif defined(ESP32)
void IRAM_ATTR handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition);
#else
void handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition);
#endif

/**
 * The ISR of TinyIRRreceiver.
 * It handles the NEC protocol decoding and calls the user callback function on complete.
 * 5 us + 3 us for push + pop for a 16MHz ATmega
 */
#if defined(ESP8266) || defined(ESP32)
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
    uint16_t tMicrosOfMarkOrSpace = tCurrentMicros - TinyIRReceiverControl.LastChangeMicros;
    TinyIRReceiverControl.LastChangeMicros = tCurrentMicros;

    uint8_t tState = TinyIRReceiverControl.IRReceiverState;

#ifdef TRACE
    Serial.print(tState);
    Serial.print(' ');
//    Serial.print(F(" I="));
//    Serial.print(tIRLevel);
//    Serial.print(F(" D="));
//    Serial.print(tDeltaMicros);
//    Serial.println();
#endif

    if (tIRLevel == LOW)
    {
        /*
         * We have a mark here
         */
        if (tMicrosOfMarkOrSpace > 2 * NEC_HEADER_MARK)
        {
            // timeout -> must reset state machine
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK)
        {
            // We are at the beginning of the header mark, check timing at the next transition
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_SPACE;
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK)
        {
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_SPACE))
            {
                /*
                 * We have a valid data header space here -> initialize data
                 */
                TinyIRReceiverControl.IRRawDataBitCounter = 0;
                TinyIRReceiverControl.IRRawData.ULong = 0;
                TinyIRReceiverControl.IRRawDataMask = 1;
                TinyIRReceiverControl.IRRepeatDetected = false;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
            }
            else if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && TinyIRReceiverControl.IRRawDataBitCounter >= NEC_BITS)
            {
                /*
                 * We have a repeat header here and no broken receive before -> set repeat flag
                 */
                TinyIRReceiverControl.IRRepeatDetected = true;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
            }
            else
            {
                // This parts are optimized by the compiler into jumps to one code :-)
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK)
        {
            // Check data space length
            if (tMicrosOfMarkOrSpace >= lowerValue(NEC_ZERO_SPACE) && tMicrosOfMarkOrSpace <= upperValue(NEC_ONE_SPACE))
            {
                // We have a valid bit here
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
                if (tMicrosOfMarkOrSpace >= 2 * NEC_UNIT)
                {
                    // we received a 1
                    TinyIRReceiverControl.IRRawData.ULong |= TinyIRReceiverControl.IRRawDataMask;
                }
                else
                {
                    // we received a 0 - empty code for documentation
                }
                // prepare for next bit
                TinyIRReceiverControl.IRRawDataMask = TinyIRReceiverControl.IRRawDataMask << 1;
                TinyIRReceiverControl.IRRawDataBitCounter++;
            }
            else
            {
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }
        else
        {
            // error wrong state for the received level, e.g. if we missed one change interrupt -> reset state
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    }

    else
    {
        /*
         * We have a space here
         */
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_SPACE)
        {
            /*
             * Check length of header mark here
             */
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_MARK)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_MARK))
            {
                tState = IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK;
            }
            else
            {
                // Wrong length of header mark -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }

        else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE)
        {
            // Check data mark length
            if (tMicrosOfMarkOrSpace >= lowerValue(NEC_BIT_MARK) && tMicrosOfMarkOrSpace <= upperValue(NEC_BIT_MARK))
            {
                /*
                 * We have a valid mark here, check for transmission complete
                 */
                if (TinyIRReceiverControl.IRRawDataBitCounter >= NEC_BITS || TinyIRReceiverControl.IRRepeatDetected)
                {
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
                            == (uint8_t) (~TinyIRReceiverControl.IRRawData.UByte.MidLowByte))
                    {
                        // standard 8 bit address NEC protocol
                        TinyIRReceiverControl.IRRawData.UByte.MidLowByte = 0; // Address is the first 8 bit
                    }

                    /*
                     * Call user provided callback here
                     */
                    handleReceivedTinyIRData(TinyIRReceiverControl.IRRawData.UWord.LowWord,
                            TinyIRReceiverControl.IRRawData.UByte.MidHighByte, TinyIRReceiverControl.IRRepeatDetected);

                }
                else
                {
                    // not finished yet
                    tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK;
                }
            }
            else
            {
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        }
        else
        {
            // error wrong state for the received level, e.g. if we missed one change interrupt -> reset state
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    }

    TinyIRReceiverControl.IRReceiverState = tState;
#ifdef _IR_MEASURE_TIMING
    digitalWriteFast(_IR_TIMING_TEST_PIN, LOW); // 2 clock cycles
#endif
}

bool isTinyReceiverIdle()
{
    return (TinyIRReceiverControl.IRReceiverState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK);
}

/**
 * Sets IR_INPUT_PIN mode to INPUT_PULLUP, if required, sets feedback LED output mode and call enablePCIInterruptForTinyReceiver()
 */
void initPCIInterruptForTinyReceiver()
{
    pinModeFast(IR_INPUT_PIN, INPUT_PULLUP);

#if !defined(NO_LED_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#endif
    enablePCIInterruptForTinyReceiver();
}

#if defined (DEBUG) && !defined(STR)
// Helper macro for getting a macro definition as string
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
/**
 * Initializes hardware interrupt generation according to IR_INPUT_PIN or use attachInterrupt() function.
 */
void enablePCIInterruptForTinyReceiver()
{
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif

#if defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
    attachInterrupt(IR_INPUT_PIN, IRPinChangeInterruptHandler, CHANGE); // 2.2 us more than version configured with macros and not compatible

#elif !defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
    // costs 112 bytes FLASH + 4bytes RAM
    attachInterrupt(digitalPinToInterrupt(IR_INPUT_PIN), IRPinChangeInterruptHandler, CHANGE);
#  ifdef DEBUG
    Serial.println(F("Use attachInterrupt for pin=" STR(IR_INPUT_PIN)));
#  endif
#else
#  ifdef DEBUG
    Serial.println(F("Use static interrupt for pin=" STR(IR_INPUT_PIN)));
#  endif
#  if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    // use PinChangeInterrupt no INT0 for pin PB2
    PCMSK = _BV(IR_INPUT_PIN);
    // clear interrupt bit
    GIFR |= 1 << PCIF;
    // enable interrupt on next change
    GIMSK |= 1 << PCIE;

#  elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#    if defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_INPUT_PIN == 3)
    // interrupt on any logical change
    EICRA |= _BV(ISC00);
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // enable interrupt on next change
    EIMSK |= 1 << INT0;
#      elif (IR_INPUT_PIN == 9)
    EICRA |= _BV(ISC10);
    // clear interrupt bit
    EIFR |= 1 << INTF1;
    // enable interrupt on next change
    EIMSK |= 1 << INT1;
#      else
#        error "IR_INPUT_PIN must be 9 or 3."
#      endif // if (IR_INPUT_PIN == 9)

#    else // defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_INPUT_PIN == 14)
    // interrupt on any logical change
    EICRA |= _BV(ISC00);
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // enable interrupt on next change
    EIMSK |= 1 << INT0;
#      elif (IR_INPUT_PIN == 3)
    EICRA |= _BV(ISC10);
    // clear interrupt bit
    EIFR |= 1 << INTF1;
    // enable interrupt on next change
    EIMSK |= 1 << INT1;
#      else
#        error "IR_INPUT_PIN must be 14 or 3."
#      endif // if (IR_INPUT_PIN == 14)
#    endif

#  else // defined(__AVR_ATtiny25__)
    /*
     * ATmegas + ATtiny88 here
     */
#    if (IR_INPUT_PIN == 2)
    // interrupt on any logical change
    EICRA |= _BV(ISC00);
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // enable interrupt on next change
    EIMSK |= 1 << INT0;
#    elif (IR_INPUT_PIN == 3)
    EICRA |= _BV(ISC10);
// clear interrupt bit
    EIFR |= 1 << INTF1;
// enable interrupt on next change
    EIMSK |= 1 << INT1;
#    elif IR_INPUT_PIN == 4 || IR_INPUT_PIN == 5 || IR_INPUT_PIN == 6 || IR_INPUT_PIN == 7
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 20 to 23 for port PD4 to PD7 (Arduino pin 4 to 7)
        PCICR |= _BV(PCIE2);
        PCMSK2 = digitalPinToBitMask(IR_INPUT_PIN);
#    elif IR_INPUT_PIN == 8 || IR_INPUT_PIN == 9 || IR_INPUT_PIN == 10 || IR_INPUT_PIN == 11 || IR_INPUT_PIN == 12 || IR_INPUT_PIN == 13
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 0 to 5 for port PB0 to PB5 (Arduino pin 8 to 13)
        PCICR |= _BV(PCIE0);
        PCMSK0 = digitalPinToBitMask(IR_INPUT_PIN);
#    elif IR_INPUT_PIN == A0 || IR_INPUT_PIN == A1 || IR_INPUT_PIN == A2 || IR_INPUT_PIN == A3 || IR_INPUT_PIN == A4 || IR_INPUT_PIN == A5
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt 8 to 13 for port PC0 to PC5 (Arduino pin A0 to A5)
        PCICR |= _BV(PCIE1);
        PCMSK1 = digitalPinToBitMask(IR_INPUT_PIN);
#    else
#      error "IR_INPUT_PIN not allowed."
#    endif // if (IR_INPUT_PIN == 2)
#  endif // defined(__AVR_ATtiny25__)
#endif // ! defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
}

void disablePCIInterruptForTinyReceiver()
{
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif

#if defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
    detachInterrupt(IR_INPUT_PIN);

#elif !defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
    // costs 112 bytes FLASH + 4bytes RAM
    detachInterrupt(digitalPinToInterrupt(IR_INPUT_PIN));
#else
#  if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    // clear interrupt bit
    GIFR |= 1 << PCIF;
    // disable interrupt on next change
    GIMSK &= ~(1 << PCIE);

#  elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#    if defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_INPUT_PIN == 3)
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // disable interrupt on next change
    EIMSK &= ~( 1 << INT0);
#      elif (IR_INPUT_PIN == 9)
    // clear interrupt bit
    EIFR |= 1 << INTF1;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT1);
#      else
#        error "IR_INPUT_PIN must be 9 or 3."
#      endif // if (IR_INPUT_PIN == 9)

#    else // defined(ARDUINO_AVR_DIGISPARKPRO)
#      if (IR_INPUT_PIN == 14)
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT0);
#      elif (IR_INPUT_PIN == 3)
    // clear interrupt bit
    EIFR |= 1 << INTF1;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT1);
#      else
#        error "IR_INPUT_PIN must be 14 or 3."
#      endif // if (IR_INPUT_PIN == 14)
#    endif

#  else // defined(__AVR_ATtiny25__)
    /*
     * ATmegas + ATtiny88 here
     */
#    if (IR_INPUT_PIN == 2)
    // clear interrupt bit
    EIFR |= 1 << INTF0;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT0);
#    elif (IR_INPUT_PIN == 3)
    // clear interrupt bit
    EIFR |= 1 << INTF1;
    // disable interrupt on next change
    EIMSK &= ~(1 << INT1);
#    elif IR_INPUT_PIN == 4 || IR_INPUT_PIN == 5 || IR_INPUT_PIN == 6 || IR_INPUT_PIN == 7
    //ATmega328 (Uno, Nano ) etc. disable pin change interrupt 20 to 23 for port PD4 to PD7 (Arduino pin 4 to 7)
        PCICR &= ~(_BV(PCIE2));
#    elif IR_INPUT_PIN == 8 || IR_INPUT_PIN == 9 || IR_INPUT_PIN == 10 || IR_INPUT_PIN == 11 || IR_INPUT_PIN == 12 || IR_INPUT_PIN == 13
    //ATmega328 (Uno, Nano ) etc. disable pin change interrupt 0 to 5 for port PB0 to PB5 (Arduino pin 8 to 13)
        PCICR &= ~(_BV(PCIE0));
#    elif IR_INPUT_PIN == A0 || IR_INPUT_PIN == A1 || IR_INPUT_PIN == A2 || IR_INPUT_PIN == A3 || IR_INPUT_PIN == A4 || IR_INPUT_PIN == A5
    //ATmega328 (Uno, Nano ) etc. disable pin change interrupt 8 to 13 for port PC0 to PC5 (Arduino pin A0 to A5)
        PCICR &= ~(_BV(PCIE1));
#    else
#      error "IR_INPUT_PIN not allowed."
#    endif // if (IR_INPUT_PIN == 2)
#  endif // defined(__AVR_ATtiny25__)
#endif // ! defined(__AVR__) || defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
}

/*
 * Specify the right INT0, INT1 or PCINT0 interrupt vector according to different pins and cores.
 * The default value of TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT is set in TinyIRReceiver.h
 */
#if defined(__AVR__) && !defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)
#  if (IR_INPUT_PIN == 2)
ISR(INT0_vect) // Pin 2 global assignment

#  elif (IR_INPUT_PIN == 3)
#    if  defined(ARDUINO_AVR_DIGISPARKPRO)
ISR(INT0_vect) //  Pin 3 / PB6 / INT0 is connected to USB+ on DigisparkPro boards
#    else
ISR(INT1_vect) // Pin 3 global assignment
#    endif

#  elif (IR_INPUT_PIN == 9) && defined(ARDUINO_AVR_DIGISPARKPRO) // Digispark pro
ISR(INT1_vect)

#  elif (IR_INPUT_PIN == 14) && (defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__))// For AVR_ATtiny167 INT0 is on pin 14 / PB6
ISR(INT0_vect)

#  elif (! defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
// on ATtinyX5 we do not have a INT1_vect but we can use the PCINT0_vect
ISR(PCINT0_vect)

#  elif IR_INPUT_PIN == 4 || IR_INPUT_PIN == 5 || IR_INPUT_PIN == 6 || IR_INPUT_PIN == 7
// PCINT for ATmega328 Arduino pins 4 (PD4) to 7 (PD7) - (PCINT 20 to 23)
ISR(PCINT2_vect)

#  elif IR_INPUT_PIN == 8 || IR_INPUT_PIN == 9 || IR_INPUT_PIN == 10 || IR_INPUT_PIN == 11 || IR_INPUT_PIN == 12 || IR_INPUT_PIN == 13
// PCINT for ATmega328 Arduino pins 8 (PB0) to 13 (PB5) - (PCINT 0 to 5)
ISR(PCINT0_vect)

# elif IR_INPUT_PIN == A0 || IR_INPUT_PIN == A1 || IR_INPUT_PIN == A2 || IR_INPUT_PIN == A3 || IR_INPUT_PIN == A4 || IR_INPUT_PIN == A5
// PCINT for ATmega328 Arduino pins A1 (PC0) to A5 (PC5) - (PCINT 8 to 13)
ISR(PCINT1_vect)

#  endif
{
    IRPinChangeInterruptHandler();
}
#endif // defined(__AVR__) && ! defined(TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT)

/** @}*/

#endif // TINY_IR_RECEIVER_HPP
#pragma once

