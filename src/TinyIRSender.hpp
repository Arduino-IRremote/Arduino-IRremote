/*
 *  TinyIRSender.hpp
 *
 *  Sends IR protocol data of NEC and FAST protocol using bit banging.
 *  NEC is the protocol of most cheap remote controls for Arduino.
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

#ifndef _TINY_IR_SENDER_HPP
#define _TINY_IR_SENDER_HPP

#include <Arduino.h>

//#define ENABLE_NEC2_REPEATS // Instead of sending / receiving the NEC special repeat code, send / receive the original frame for repeat.

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif
#include "TinyIR.h" // Defines protocol timings

#include "digitalWriteFast.h"
/** \addtogroup TinySender Minimal sender for NEC and FAST protocol
 * @{
 */

/*
 * Generate 38 kHz IR signal by bit banging
 */
void sendMark(uint8_t aSendPin, unsigned int aMarkMicros) {
    unsigned long tStartMicros = micros();
    unsigned long tNextPeriodEnding = tStartMicros;
    unsigned long tMicros;
    do {
        /*
         * Generate pulse
         */
        noInterrupts(); // do not let interrupts extend the short on period
        digitalWriteFast(aSendPin, HIGH);
        delayMicroseconds(8); // 8 us for a 30 % duty cycle for 38 kHz
        digitalWriteFast(aSendPin, LOW);
        interrupts(); // Enable interrupts - to keep micros correct- for the longer off period 3.4 us until receive ISR is active (for 7 us + pop's)

        /*
         * PWM pause timing and end check
         * Minimal pause duration is 4.3 us
         */
        tNextPeriodEnding += 26; // for 38 kHz
        do {
            tMicros = micros(); // we have only 4 us resolution for AVR @16MHz
            /*
             * Exit the forever loop if aMarkMicros has reached
             */
            unsigned int tDeltaMicros = tMicros - tStartMicros;
#if defined(__AVR__)
            // Just getting variables and check for end condition takes minimal 3.8 us
            if (tDeltaMicros >= aMarkMicros - (112 / (F_CPU / MICROS_IN_ONE_SECOND))) { // To compensate for call duration - 112 is an empirical value
#else
                if (tDeltaMicros >= aMarkMicros) {
    #endif
                return;
            }
        } while (tMicros < tNextPeriodEnding);
    } while (true);
}

/*
 * Send NEC with 16 bit command, even if aCommand < 0x100
 * @param aAddress  - The 16 bit address to send.
 * @param aCommand  - The 16 bit command to send.
 * @param aNumberOfRepeats  - Number of repeats send at a period of 110 ms.
 */
void sendONKYO(uint8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats) {
    pinModeFast(aSendPin, OUTPUT);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        sendMark(aSendPin, NEC_HEADER_MARK);
#if !defined(ENABLE_NEC2_REPEATS)
        if (tNumberOfCommands < aNumberOfRepeats + 1) {
            // send the NEC special repeat
            delayMicroseconds(NEC_REPEAT_HEADER_SPACE); // - 2250
        } else
#endif
        {
            // send header
            delayMicroseconds(NEC_HEADER_SPACE);
            LongUnion tData;
            tData.UWord.LowWord = aAddress;
            tData.UWord.HighWord = aCommand;
            // Send data
            for (uint_fast8_t i = 0; i < NEC_BITS; ++i) {
                sendMark(aSendPin, NEC_BIT_MARK); // constant mark length
                if (tData.ULong & 1) {
                    delayMicroseconds(NEC_ONE_SPACE);
                } else {
                    delayMicroseconds(NEC_ZERO_SPACE);
                }
                tData.ULong >>= 1; // shift command for next bit
            }
        }        // send stop bit
        sendMark(aSendPin, NEC_BIT_MARK);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (NEC_REPEAT_PERIOD / 1000 > tFrameDurationMillis) {
                delay(NEC_REPEAT_PERIOD / 1000 - tFrameDurationMillis);
            }
        }
    }
}

/*
 * Send NEC with 8 or 16 bit address or data depending on the values of aAddress and aCommand.
 * @param aAddress  - If aAddress < 0x100 send 8 bit address and 8 bit inverted address, else send 16 bit address.
 * @param aCommand  - If aCommand < 0x100 send 8 bit command and 8 bit inverted command, else send 16 bit command.
 * @param aNumberOfRepeats  - Number of repeats send at a period of 110 ms.
 */
void sendNECMinimal(uint8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendNEC(aSendPin, aAddress, aCommand, aNumberOfRepeats); // sendNECMinimal() is deprecated
}
void sendNEC(uint8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats) {
    pinModeFast(aSendPin, OUTPUT);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        sendMark(aSendPin, NEC_HEADER_MARK);
#if !defined(ENABLE_NEC2_REPEATS)
        if (tNumberOfCommands < aNumberOfRepeats + 1) {
            // send the NEC special repeat
            delayMicroseconds(NEC_REPEAT_HEADER_SPACE); // - 2250
        } else
#endif
        {
            // send header
            delayMicroseconds(NEC_HEADER_SPACE);
            LongUnion tData;
            /*
             * The compiler is intelligent and removes the code for "(aAddress > 0xFF)" if we are called with an uint8_t address :-).
             * Using an uint16_t address requires additional 28 bytes program memory.
             */
            if (aAddress > 0xFF) {
                tData.UWord.LowWord = aAddress;
            } else {
                tData.UByte.LowByte = aAddress; // LSB first
                tData.UByte.MidLowByte = ~aAddress;
            }
            if (aCommand > 0xFF) {
                tData.UWord.HighWord = aCommand;
            } else {
                tData.UByte.MidHighByte = aCommand;
                tData.UByte.HighByte = ~aCommand; // LSB first
            }
            // Send data
            for (uint_fast8_t i = 0; i < NEC_BITS; ++i) {
                sendMark(aSendPin, NEC_BIT_MARK); // constant mark length

                if (tData.ULong & 1) {
                    delayMicroseconds(NEC_ONE_SPACE);
                } else {
                    delayMicroseconds(NEC_ZERO_SPACE);
                }
                tData.ULong >>= 1; // shift command for next bit
            }
        }        // send stop bit
        sendMark(aSendPin, NEC_BIT_MARK);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (NEC_REPEAT_PERIOD / 1000 > tFrameDurationMillis) {
                delay(NEC_REPEAT_PERIOD / 1000 - tFrameDurationMillis);
            }
        }
    }
}

/*
 * LSB first, send header, command, inverted command and stop bit
 */
void sendFast8BitAndParity(uint8_t aSendPin, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendFAST(aSendPin, aCommand, aNumberOfRepeats);
}

/*
 * LSB first, send header, 16 bit command or 8 bit command, inverted command and stop bit
 */
void sendFAST(uint8_t aSendPin, uint16_t aCommand, uint_fast8_t aNumberOfRepeats) {
    pinModeFast(aSendPin, OUTPUT);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        // send header
        sendMark(aSendPin, FAST_HEADER_MARK);
        delayMicroseconds(FAST_HEADER_SPACE);
        uint16_t tData;
        /*
         * The compiler is intelligent and removes the code for "(aCommand > 0xFF)" if we are called with an uint8_t command :-).
         * Using an uint16_t command requires additional 56 bytes program memory.
         */
        if (aCommand > 0xFF) {
            tData = aCommand;
        } else {
            tData = aCommand | (((uint8_t) (~aCommand)) << 8); // LSB first
        }
        // Send data
        for (uint_fast8_t i = 0; i < FAST_BITS; ++i) {
            sendMark(aSendPin, FAST_BIT_MARK); // constant mark length

            if (tData & 1) {
                delayMicroseconds(FAST_ONE_SPACE);
            } else {
                delayMicroseconds(FAST_ZERO_SPACE);
            }
            tData >>= 1; // shift command for next bit
        }
        // send stop bit
        sendMark(aSendPin, FAST_BIT_MARK);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (FAST_REPEAT_PERIOD / 1000 > tFrameDurationMillis) {
                delay(FAST_REPEAT_PERIOD / 1000 - tFrameDurationMillis);
            }
        }
    }
}

/** @}*/

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _TINY_IR_SENDER_HPP
