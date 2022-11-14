/*
 *  TinyIRSender.hpp
 *
 *  Sends IR protocol data of NEC and FAST protocol using bit banging.
 *  NEC is the protocol of most cheap remote controls for Arduino.
 *  FAST protocol is proprietary and a JVC protocol without address and with a shorter header.
 *  FAST takes 21 ms for sending and can be sent at a 50 ms period. It still supports parity.
 *
 *
 *  Copyright (C) 2022  Armin Joachimsmeyer
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

#ifndef _TINY_IR_SENDER_HPP
#define _TINY_IR_SENDER_HPP

#include <Arduino.h>

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/*
 * FAST_8_BIT_CS Protocol characteristics:
 * - Bit timing is like JVC
 * - The header is shorter, 4000 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *     leading to a fixed protocol length of (7 + (16 * 2) + 1) * 526 = 40 * 560 = 21040 microseconds or 21 ms.
 * - Repeats are sent as complete frames but in a 50 ms period.
 */
#include "TinyIR.h" // Defines protocol timings

#include "digitalWriteFast.h"
/** \addtogroup TinySender Minimal sender for NEC and FAST protocol
 * @{
 */

/*
 * Generate IR signal by bit banging
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

void sendNECMinimal(uint8_t aSendPin, uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    pinModeFast(aSendPin, OUTPUT);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        sendMark(aSendPin, NEC_HEADER_MARK);
        if (tNumberOfCommands < aNumberOfRepeats + 1) {
            // send the NEC special repeat
            delayMicroseconds(NEC_REPEAT_HEADER_SPACE); // - 2250
        } else {
            // send header
            delayMicroseconds(NEC_HEADER_SPACE);
            LongUnion tData;
            tData.UByte.LowByte = aAddress; // LSB first
            tData.UByte.MidLowByte = ~aAddress;
            tData.UByte.MidHighByte = aCommand;
            tData.UByte.HighByte = ~aCommand; // LSB first
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
    pinModeFast(aSendPin, OUTPUT);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        // send header
        sendMark(aSendPin, FAST_8_BIT_PARITY_HEADER_MARK);
        delayMicroseconds(FAST_8_BIT_PARITY_HEADER_SPACE);
        uint16_t tData = aCommand | (((uint8_t) (~aCommand)) << 8); // LSB first
        // Send data
        for (uint_fast8_t i = 0; i < FAST_8_BIT_PARITY_BITS; ++i) {
            sendMark(aSendPin, FAST_8_BIT_PARITY_BIT_MARK); // constant mark length

            if (tData & 1) {
                delayMicroseconds(FAST_8_BIT_PARITY_ONE_SPACE);
            } else {
                delayMicroseconds(FAST_8_BIT_PARITY_ZERO_SPACE);
            }
            tData >>= 1; // shift command for next bit
        }
        // send stop bit
        sendMark(aSendPin, FAST_8_BIT_PARITY_BIT_MARK);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (FAST_8_BIT_PARITY_REPEAT_PERIOD / 1000 > tFrameDurationMillis) {
                delay(FAST_8_BIT_PARITY_REPEAT_PERIOD / 1000 - tFrameDurationMillis);
            }
        }
    }
}

/** @}*/

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _TINY_IR_SENDER_HPP
