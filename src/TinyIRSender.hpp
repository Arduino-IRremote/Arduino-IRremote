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
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2026 Armin Joachimsmeyer
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

//#define NO_LED_SEND_FEEDBACK_CODE     // Disables the LED feedback code for send.
//#define IR_FEEDBACK_LED_PIN     12    // Use this, to disable use of LED_BUILTIN definition for IR_FEEDBACK_LED_PIN
#define USE_NO_SEND_PWM               // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition

#include "TinyIR.h" // Defines protocol timings

#include "digitalWriteFast.h"
/** \addtogroup TinySender Minimal sender for NEC and FAST protocol
 * @{
 */

#if !defined(NO_LED_SEND_FEEDBACK_CODE)
#define LED_SEND_FEEDBACK_CODE // Resolve the double negative
#endif

/**
 * Custom delay function that circumvents Arduino's delayMicroseconds 16 bit limit
 * and is (mostly) not extended by the duration of interrupt codes like the millis() interrupt.
 * I.e. I measured 2400 microseconds for delayMicroseconds(2100)
 */
void customDelayMicroseconds(unsigned long aMicroseconds) {
#if defined(_IR_SEND_HPP)
    IRsend::customDelayMicroseconds(aMicroseconds); // use the existing IRremote function
#else
#  if defined(ESP32) || defined(ESP8266)
    // from https://github.com/crankyoldgit/IRremoteESP8266/blob/00b27cc7ea2e7ac1e48e91740723c805a38728e0/src/IRsend.cpp#L123
    // Invoke a delay(), where possible, to avoid triggering the WDT.
    // see https://github.com/Arduino-IRremote/Arduino-IRremote/issues/1114 for the reason of checking for > 16383)
    // delayMicroseconds() is only accurate to 16383 us. Ref: https://www.arduino.cc/en/Reference/delayMicroseconds
    if (aMicroseconds > 16383) {
        delay(aMicroseconds / 1000UL);  // Delay for as many whole milliseconds as we can.
        // Delay the remaining sub-millisecond.
        delayMicroseconds(static_cast<uint16_t>(aMicroseconds % 1000UL));
    } else {
        delayMicroseconds(aMicroseconds);
    }
#  else

#    if defined(__AVR__)
    unsigned long start = micros() - (64 / clockCyclesPerMicrosecond()); // - (64 / clockCyclesPerMicrosecond()) for reduced resolution and additional overhead
#    else
    unsigned long start = micros();
#    endif
// overflow invariant comparison :-)
    while (micros() - start < aMicroseconds) {
    }
#  endif
#endif

}
/*
 * Generate 38 kHz IR signal by bit banging and using customDelayMicroseconds() and micros()
 */
void sendMark(uint_fast8_t aSendPin, unsigned int aMarkMicros) {
#if defined(F_CPU)
    unsigned long tMarkDurationMicros = (112 / (F_CPU / MICROS_IN_ONE_SECOND)) + aMarkMicros; // To compensate for call duration - 112 is an empirical value
#else
    unsigned long tMarkDurationMicros = aMarkMicros;
#endif
#if defined(USE_NO_SEND_PWM)
    /*
     * Here we generate no carrier PWM, just simulate an active low receiver signal.
     */
    digitalWriteFast(aSendPin, LOW); // Active state is low here.
    customDelayMicroseconds(tMarkDurationMicros);
    digitalWriteFast(aSendPin, HIGH); // Set output to inactive high.
#else
    unsigned long tStartMicros = micros();
    unsigned long tMicrosOfEndOfNextPWMPause = tStartMicros;

    do {
        /*
         * Generate pulse
         */
        noInterrupts(); // do not let interrupts extend the short on period
        digitalWriteFast(aSendPin, HIGH);
        customDelayMicroseconds(8); // 8 us for a 30 % duty cycle for 38 kHz
        digitalWriteFast(aSendPin, LOW);
        interrupts(); // Enable interrupts - to keep micros correct- for the longer off period 3.4 us until receive ISR is active (for 7 us + pop's)

        /*
         * PWM pause timing and end check
         * Minimal pause duration is 4.3 us
         */
        tMicrosOfEndOfNextPWMPause += 26; // 26.3 us period for 38 kHz; 26 us -> 38.48 kHz
        unsigned long tMicros;
        do {
            tMicros = micros(); // we have only 4 us resolution for AVR @16MHz
            /*
             * Exit the forever loop if aMarkMicros has reached
             * Overflow invariant: unsigned difference is the elapsed time - 32 bytes program space
             */
            if (tMicros - tStartMicros >= tMarkDurationMicros) {
                return;
            }
        } while ((long) (tMicros - tMicrosOfEndOfNextPWMPause) < 0);
    } while (true);
#endif
}

/*
 * Send NEC with 16 bit address and command, even if aCommand < 0x100 (I.E. ONKYO)
 * @param aAddress  - The 16 bit address to send.
 * @param aCommand  - The 16 bit command to send.
 * @param aNumberOfRepeats  - Number of repeats send at a period of 110 ms.
 * @param aSendNEC2Repeats - Instead of sending the NEC special repeat code, send the original frame for repeat.
 */
void sendOnkyo(uint_fast8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendNEC2Repeats) {
    pinModeFast(aSendPin, OUTPUT);

#if defined(LED_SEND_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, LOW);
#  else
    digitalWriteFast(IR_FEEDBACK_LED_PIN, HIGH);
#  endif
#endif

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        sendMark(aSendPin, NEC_HEADER_MARK);
        if ((!aSendNEC2Repeats) && (tNumberOfCommands < aNumberOfRepeats + 1)) {
            // send the NEC special repeat
            customDelayMicroseconds (NEC_REPEAT_HEADER_SPACE); // - 2250
        } else {
            // send header
            delayMicroseconds (NEC_HEADER_SPACE);
            LongUnion tData;
            tData.UWord.LowWord = aAddress;
            tData.UWord.HighWord = aCommand;
            // Send data
            for (uint_fast8_t i = 0; i < NEC_BITS; ++i) {
                sendMark(aSendPin, NEC_BIT_MARK); // constant mark length
                if (tData.ULong & 1) {
                    customDelayMicroseconds (NEC_ONE_SPACE);
                } else {
                    customDelayMicroseconds (NEC_ZERO_SPACE);
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
#if defined(LED_SEND_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, HIGH);
#  else
    digitalWriteFast(IR_FEEDBACK_LED_PIN, LOW);
#  endif
#endif
}

/*
 * Send NEC with 8 or 16 bit address or command depending on the values of aAddress and aCommand.
 * To force sending of a 16 bit address, use: sendOnkyo().
 * @param aAddress  - If aAddress < 0x100 send 8 bit address and 8 bit inverted address, else send 16 bit address.
 * @param aCommand  - If aCommand < 0x100 send 8 bit command and 8 bit inverted command, else send 16 bit command.
 * @param aNumberOfRepeats  - Number of repeats send at a period of 110 ms.
 * @param aSendNEC2Repeats - Instead of sending the NEC special repeat code, send the original frame for repeat.
 */
void sendNEC(uint_fast8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendNEC2Repeats) {
    pinModeFast(aSendPin, OUTPUT);

#if defined(LED_SEND_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, LOW);
#  else
    digitalWriteFast(IR_FEEDBACK_LED_PIN, HIGH);
#  endif
#endif

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        sendMark(aSendPin, NEC_HEADER_MARK);
        if ((!aSendNEC2Repeats) && (tNumberOfCommands < aNumberOfRepeats + 1)) {
            // send the NEC special repeat
            customDelayMicroseconds (NEC_REPEAT_HEADER_SPACE); // - 2250
        } else {
            // send header
            customDelayMicroseconds (NEC_HEADER_SPACE);
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
                    customDelayMicroseconds (NEC_ONE_SPACE);
                } else {
                    customDelayMicroseconds (NEC_ZERO_SPACE);
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
#if defined(LED_SEND_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, HIGH);
#  else
    digitalWriteFast(IR_FEEDBACK_LED_PIN, LOW);
#  endif
#endif
}

/*
 * LSB first, send header, command, inverted command and stop bit
 */
void sendFAST(uint_fast8_t aSendPin, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    uint16_t tData = aCommand | (((uint16_t) ((uint8_t) (~aCommand)) << 8));
    sendFAST16Bit(aSendPin, tData, aNumberOfRepeats);
}

/*
 * LSB first, send header and command and stop bit
 * @param aCommand 16 bit RAW command
 */
void sendFAST16Bit(uint_fast8_t aSendPin, uint16_t aCommand, uint_fast8_t aNumberOfRepeats) {
    pinModeFast(aSendPin, OUTPUT);

#if defined(LED_SEND_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, LOW);
#  else
    digitalWriteFast(IR_FEEDBACK_LED_PIN, HIGH);
#  endif
#endif

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        auto tStartOfFrameMillis = millis();

        // send header
        sendMark(aSendPin, FAST_HEADER_MARK);
        customDelayMicroseconds(FAST_HEADER_SPACE);
        uint16_t tData = aCommand;
        // Send data
        for (uint_fast8_t i = 0; i < FAST_BITS; ++i) {
            sendMark(aSendPin, FAST_BIT_MARK); // constant mark length

            if (tData & 1) {
                customDelayMicroseconds(FAST_ONE_SPACE);
            } else {
                customDelayMicroseconds(FAST_ZERO_SPACE);
            }
            tData >>= 1; // shift command for next bit
        }
        // send stop bit
        sendMark(aSendPin, FAST_BIT_MARK);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Send repeated command in a fixed raster
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            uint16_t tFrameDurationMillis = millis() - tStartOfFrameMillis; // we know that values are smaller than 13000
            delay((FAST_REPEAT_PERIOD / 1000) - tFrameDurationMillis);
        }
    }
#if defined(LED_SEND_FEEDBACK_CODE) && defined(IR_FEEDBACK_LED_PIN)
    pinModeFast(IR_FEEDBACK_LED_PIN, OUTPUT);
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    digitalWriteFast(IR_FEEDBACK_LED_PIN, HIGH);
#  else
    digitalWriteFast(IR_FEEDBACK_LED_PIN, LOW);
#  endif
#endif
}

/** @}*/

#endif // _TINY_IR_SENDER_HPP
