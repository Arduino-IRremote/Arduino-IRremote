/*
 * ir_BangOlufsen.hpp
 *
 *  Contains functions for receiving and sending Bang & Olufsen IR and Datalink '86 protocols
 *  To receive B&O and ENABLE_BEO_WITHOUT_FRAME_GAP is NOT defined, you must set RECORD_GAP_MICROS to
 *  at least 16000 to accommodate the unusually long 3. start space.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2025 Daniel Wallner and Armin Joachimsmeyer
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
#ifndef _IR_BANG_OLUFSEN_HPP
#define _IR_BANG_OLUFSEN_HPP

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */

//==============================================================================
//
//
//                              Bang & Olufsen
//
//
//==============================================================================
// https://www.mikrocontroller.net/attachment/33137/datalink.pdf
// https://www.mikrocontroller.net/articles/IRMP_-_english#B&O
// BEO is a Pulse Distance Protocol with 200 us pulse
// This protocol is unusual in two ways:
// 1. The carrier frequency is 455 kHz
// You can build your own receiver as Bang & Olufsen did (check old schematics) or use a TSOP7000
// Vishay stopped producing TSOP7000 since 2009 so you will probably only find counterfeits:
// https://www.vishay.com/files/whatsnew/doc/ff_FastFacts_CounterfeitTSOP7000_Dec72018.pdf
// It is also likely that you will need an oscilloscope to debug a counterfeit TSOP7000
// The specimen used to test this code was very noisy and had a very low output current
// A somewhat working fix was to put a 4n7 capacitor across the output and ground followed by a pnp emitter follower
// Other examples may require a different treatment
// This particular receiver also did receive lower frequencies but rather poorly and with a lower delay than usual
// If you need to parallel a receiver with another one you may need to delay the signal to get in phase with the other receiver
// 2. A stream of messages can be sent back to back with a new message immediately following the previous stop space
// It might be that this only happens over IR and not on the datalink protocol
// You can choose to support this or not:
// Mode 1: Mode with gaps between frames
// Do NOT define ENABLE_BEO_WITHOUT_FRAME_GAP and set RECORD_GAP_MICROS to at least 16000 to accept the unusually long 3. start space
// Can only receive single messages. Back to back repeats will result in overflow
// Mode 2: Break at start mode
// Define ENABLE_BEO_WITHOUT_FRAME_GAP and set RECORD_GAP_MICROS to less than 15000
// This treats the 3. start space of 15.5 ms as a gap between 2 messages, which makes decoding easier :-).
// The receiving of a transmission will then result in a dummy decode of the first 2 start bits with 0 bits data
// followed by a 15.5 ms gap and a data frame with one start bit (originally sent as 4. start bit).
// If the receiver is not immediately resumed after the 2 start bit frame, partial second frame will be decoded!
// Thus debug printing in the wrong place is very likely to break reception!
// Make sure to check the number of bits to filter dummy and incomplete messages.
// !!! We assume that the real implementations never set the official first header bit to anything other than 0 !!!
// !!! We therefore use 4 start bits instead of the specified 3 and in turn ignore the first header bit of the specification !!!
// IR messages are 16 bits long. Datalink messages have different lengths.
// This implementation supports up to 40 bits total length split into 8 bit data/command and a header/address of variable length
// Header data with more than 16 bits is stored in decodedIRData.extra
// B&O is a pulse distance protocol, but it has 3 bit values 0, 1 and (equal/repeat) as well as a special start and trailing bit.
//
// MSB first, 4 start bits + 8 (to 16?) bit address + 8 bit command + 1 special trailing bit + 1 stop bit.
// Address can be longer than 8 bit.
/*
 * Options for this decoder
 *
 */
#define ENABLE_BEO_WITHOUT_FRAME_GAP // Requires additional 30 bytes program memory. Enabled by default, see https://github.com/Arduino-IRremote/Arduino-IRremote/discussions/1181
//#define SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE // This also supports headers up to 32 bit. Requires additional 150 bytes program memory.
#if defined(DECODE_BEO)
#  if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
#    if RECORD_GAP_MICROS > 15000 && !defined(SUPPRESS_BEO_RECORD_GAP_MICROS_WARNING)
#warning If defined ENABLE_BEO_WITHOUT_FRAME_GAP, RECORD_GAP_MICROS must be set to <= 15000 by "#define RECORD_GAP_MICROS 12750"
#    endif
#  else
#    if RECORD_GAP_MICROS < 16000 && !defined(SUPPRESS_BEO_RECORD_GAP_MICROS_WARNING)
#error If not defined ENABLE_BEO_WITHOUT_FRAME_GAP, RECORD_GAP_MICROS must be set to a value >= 16000 by "#define RECORD_GAP_MICROS 16000"
#    endif
#  endif
#endif

#define BEO_DATA_BITS         8                // Command or character

#define BEO_UNIT              3125             // All timings are in microseconds

#define BEO_BIT_MARK          200              // The length of a mark in the IR protocol

// With decode we see length from 200 to 300 and the 300 leads to errors, if we use 200 as mark length for decode.
// And the space value is at least 3125, so we can do a reluctant test for the mark anyway.
#define BEO_BIT_MARK_FOR_DECODE 250
#define BEO_DATALINK_BIT_MARK       (BEO_UNIT / 2)   // The length of a mark in the Datalink protocol

/*
 * For all spaces really sent, the time of one bit mark (200 or 1562) is subtracted before the durations defined below
 */
#define BEO_ZERO_SPACE          BEO_UNIT      // 3125
#define BEO_REPETITION_OF_PREVIOUS_BIT_SPACE (2 * BEO_UNIT)   // 6250 The length of an repetition bit
#define BEO_ONE_SPACE           (3 * BEO_UNIT)   // 9375
#define BEO_TRAILING_BIT_SPACE  (4 * BEO_UNIT)   // 12500 The length of the space of stop bit
#define BEO_START_BIT_SPACE     (5 * BEO_UNIT)   // 15625
#define BEO_REPEAT_PERIOD       100000 // 100 ms - Not used yet

// It is not allowed to send two ones or zeros, you must send a one or zero and a equal instead.

/************************************
 * Start of send and decode functions
 ************************************/

/*
 * TODO aNumberOfRepeats are handled not correctly if ENABLE_BEO_WITHOUT_FRAME_GAP is defined
 * By default 16 bits are sent.
 * @param aNumberOfHeaderBits   default is 8, can be 24 at maximum
 */
void IRsend::sendBangOlufsen(uint16_t aHeader, uint8_t aData, int_fast8_t aNumberOfRepeats, int8_t aNumberOfHeaderBits) {
    for (int_fast8_t i = 0; i < aNumberOfRepeats + 1; ++i) {
        // send 16 bits by default
        sendBangOlufsenRaw((uint32_t(aHeader) << aNumberOfHeaderBits) | aData, aNumberOfHeaderBits + BEO_DATA_BITS, i != 0);
    }
}

void IRsend::sendBangOlufsenDataLink(uint32_t aHeader, uint8_t aData, int_fast8_t aNumberOfRepeats, int8_t aNumberOfHeaderBits) {
    for (int_fast8_t i = 0; i < aNumberOfRepeats + 1; ++i) {
        sendBangOlufsenRawDataLink((uint64_t(aHeader) << aNumberOfHeaderBits) | aData, aNumberOfHeaderBits + BEO_DATA_BITS, i != 0,
                true);
    }
}

/*
 * @param aBackToBack   If true send data back to back, which cannot be decoded if ENABLE_BEO_WITHOUT_FRAME_GAP is NOT defined
 */
void IRsend::sendBangOlufsenRaw(uint32_t aRawData, int_fast8_t aBits, bool aBackToBack) {
#if defined(USE_NO_SEND_PWM) || defined(SEND_PWM_BY_TIMER) || BEO_KHZ == 38 // BEO_KHZ == 38 is for unit test which runs the B&O protocol with 38 kHz

    /*
     * 455 kHz PWM is currently only supported with SEND_PWM_BY_TIMER defined, otherwise maximum is 180 kHz
     */
#  if !defined(USE_NO_SEND_PWM)
#    if defined(SEND_PWM_BY_TIMER)
    enableHighFrequencyIROut (BEO_KHZ);
#    elif (BEO_KHZ == 38)
    enableIROut (BEO_KHZ); // currently only for unit test
#    endif
#  endif

// AGC / Start - 3 bits + first constant 0 header bit described in the official documentation
    if (!aBackToBack) {
        mark(BEO_BIT_MARK);
    }
    space(BEO_ZERO_SPACE - BEO_BIT_MARK);
    mark(BEO_BIT_MARK);
    space(BEO_ZERO_SPACE - BEO_BIT_MARK);
    mark(BEO_BIT_MARK);
    space(BEO_START_BIT_SPACE - BEO_BIT_MARK);

// First bit of header is assumed to be a constant 0 to have a fixed state to begin with the equal decisions.
// So this first 0 is treated as the last bit of AGC
    mark(BEO_BIT_MARK);
    space(BEO_ZERO_SPACE - BEO_BIT_MARK);
    bool tLastBitValueWasOne = false;

    // Send 8 (default) to 24 bit header and 8 bit data.
    uint32_t mask = 1UL << (aBits - 1);
    for (; mask; mask >>= 1) {
        if (tLastBitValueWasOne && !(aRawData & mask)) {
            mark(BEO_BIT_MARK);
            space(BEO_ZERO_SPACE - BEO_BIT_MARK);
            tLastBitValueWasOne = false;
        } else if (!tLastBitValueWasOne && (aRawData & mask)) {
            mark(BEO_BIT_MARK);
            space(BEO_ONE_SPACE - BEO_BIT_MARK);
            tLastBitValueWasOne = true;
        } else {
            mark(BEO_BIT_MARK);
            space(BEO_REPETITION_OF_PREVIOUS_BIT_SPACE - BEO_BIT_MARK);
        }
    }

// Stop
    mark(BEO_BIT_MARK);
    space(BEO_TRAILING_BIT_SPACE - BEO_BIT_MARK);
    mark(BEO_BIT_MARK);

#else
    (void) aRawData;
    (void) aBits;
    (void) aBackToBack;
#endif
}

/*
 * Version with 64 bit aRawData, which can send both timings, but costs more program memory
 * @param aBackToBack   If true send data back to back, which cannot be decoded if ENABLE_BEO_WITHOUT_FRAME_GAP is NOT defined
 * @param aUseDatalinkTiming    if false it does the same as sendBangOlufsenRaw()
 */
void IRsend::sendBangOlufsenRawDataLink(uint64_t aRawData, int_fast8_t aBits, bool aBackToBack, bool aUseDatalinkTiming) {
#if defined(USE_NO_SEND_PWM) || BEO_KHZ == 38 // BEO_KHZ == 38 is for unit test which runs the B&O protocol with 38 kHz instead 0f 455 kHz
    uint16_t tSendBEOMarkLength = aUseDatalinkTiming ? BEO_DATALINK_BIT_MARK : BEO_BIT_MARK;

    /*
     * 455 kHz PWM is currently not supported, maximum is 180 kHz
     */
#if !defined(USE_NO_SEND_PWM)
    enableIROut (BEO_KHZ);
#endif

// AGC / Start - 3 bits + first constant 0 header bit described in the official documentation
    if (!aBackToBack) {
        mark(tSendBEOMarkLength);
    }
    space(BEO_ZERO_SPACE - tSendBEOMarkLength);
    mark(tSendBEOMarkLength);
    space(BEO_ZERO_SPACE - tSendBEOMarkLength);
    mark(tSendBEOMarkLength);
    space(BEO_START_BIT_SPACE - tSendBEOMarkLength);

// First bit of header is assumed to be a constant 0 to have a fixed state to begin with the equal decisions.
// So this first 0 is treated as the last bit of AGC
    mark(tSendBEOMarkLength);
    space(BEO_ZERO_SPACE - tSendBEOMarkLength);
    bool tLastBitValueWasOne = false;

// Header / Data
    uint32_t mask = 1UL << (aBits - 1);
    for (; mask; mask >>= 1) {
        if (tLastBitValueWasOne && !(aRawData & mask)) {
            mark(tSendBEOMarkLength);
            space(BEO_ZERO_SPACE - tSendBEOMarkLength);
            tLastBitValueWasOne = false;
        } else if (!tLastBitValueWasOne && (aRawData & mask)) {
            mark(tSendBEOMarkLength);
            space(BEO_ONE_SPACE - tSendBEOMarkLength);
            tLastBitValueWasOne = true;
        } else {
            mark(tSendBEOMarkLength);
            space(BEO_REPETITION_OF_PREVIOUS_BIT_SPACE - tSendBEOMarkLength);
        }
    }

// Stop
    mark(tSendBEOMarkLength);
    space(BEO_TRAILING_BIT_SPACE - tSendBEOMarkLength);
    mark(tSendBEOMarkLength);

#else
    (void) aRawData;
    (void) aBits;
    (void) aUseDatalinkTiming;
    (void) aBackToBack;
#endif
}

#define BEO_MATCH_DELTA (BEO_UNIT / 2 - MICROS_PER_TICK) // use a bigger margin for match than regular matching function
static bool matchBeoLength(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
    const uint16_t tMeasuredMicros = aMeasuredTicks * MICROS_PER_TICK;
    return aMatchValueMicros - BEO_MATCH_DELTA < tMeasuredMicros && tMeasuredMicros < aMatchValueMicros + BEO_MATCH_DELTA;
}

bool IRrecv::decodeBangOlufsen() {
#if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    if (decodedIRData.rawlen != 6 && decodedIRData.rawlen < 36) { // 16 bits minimum
        DEBUG_PRINT(F("B&O: Data length="));
        DEBUG_PRINT(decodedIRData.rawlen);
        DEBUG_PRINTLN(F(" is not < 36 or 6"));
#else
    if (decodedIRData.rawlen < 44) { // 16 bits minimum
        DEBUG_PRINT(F("B&O: Data length="));
        DEBUG_PRINT(decodedIRData.rawlen);
        DEBUG_PRINTLN(F(" is not < 44"));
#endif
        return false;
    }

#if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
    uint16_t protocolMarkLength = 0; // contains BEO_BIT_MARK or BEO_DATALINK_BIT_MARK depending of 4. mark received
    uint64_t tDecodedRawData = 0;
#else
    uint32_t tDecodedRawData = 0;
#endif
    uint8_t tLastDecodedBitValue = 0; // the last start bit is assumed to be zero
    uint8_t tPulseNumber = 0;
    uint8_t tBitNumber = 0;

    TRACE_PRINT(F("Pre gap: "));
    TRACE_PRINT((uint32_t)decodedIRData.initialGapTicks * 50);
    TRACE_PRINT(F(" raw len: "));
    TRACE_PRINTLN(decodedIRData.rawlen);

#if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    /*
     * Check if we have the AGC part of the first frame, i.e. start bit 1 and 2.
     */
    if (decodedIRData.rawlen == 6) {
        if ((matchMark(irparams.rawbuf[3], BEO_BIT_MARK_FOR_DECODE) || matchMark(irparams.rawbuf[3], BEO_DATALINK_BIT_MARK))
                && (matchSpace(irparams.rawbuf[4], BEO_ZERO_SPACE - BEO_BIT_MARK_FOR_DECODE)
                        || matchSpace(irparams.rawbuf[4], BEO_ZERO_SPACE - BEO_DATALINK_BIT_MARK))) {
            TRACE_PRINTLN(F("B&O: AGC only part (start bits 1 + 2 of 4) detected"));
        } else {
            return false; // no B&O protocol
        }
    } else {
        /*
         * Check if leading gap is trailing bit of first AGC frame
         */
        if (!matchSpace(decodedIRData.initialGapTicks, BEO_START_BIT_SPACE)) {
            TRACE_PRINT(F("B&O: Leading gap of ")); // Leading gap is trailing bit of first frame
            TRACE_PRINT((uint32_t)decodedIRData.initialGapTicks * 50); // Leading gap is trailing bit of first frame
            TRACE_PRINTLN(F(" us is wrong")); // Leading gap is trailing bit of first frame
            return false; // no B&O protocol
        }

        if (matchMark(irparams.rawbuf[1], BEO_BIT_MARK_FOR_DECODE)) {
#  if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
            protocolMarkLength = BEO_BIT_MARK_FOR_DECODE;
        } else if (matchMark(irparams.rawbuf[1], BEO_DATALINK_BIT_MARK)) {
            protocolMarkLength = BEO_DATALINK_BIT_MARK;
#  endif
        } else {
            TRACE_PRINTLN(F("B&O: mark length is wrong"));
            return false;
        }

        // skip first zero header bit
        for (uint8_t tRawBufferMarkIndex = 3; tRawBufferMarkIndex < decodedIRData.rawlen; tRawBufferMarkIndex += 2) {
#else
    for (uint8_t tRawBufferMarkIndex = 1; tRawBufferMarkIndex < decodedIRData.rawlen; tRawBufferMarkIndex += 2) {
#endif // defined(ENABLE_BEO_WITHOUT_FRAME_GAP)

            uint16_t markLength = irparams.rawbuf[tRawBufferMarkIndex];
            uint16_t spaceLength = irparams.rawbuf[tRawBufferMarkIndex + 1];

            TRACE_PRINT(tPulseNumber);
            TRACE_PRINT(' ');
            TRACE_PRINT(markLength * MICROS_PER_TICK);
            TRACE_PRINT(' ');
            TRACE_PRINT(spaceLength * MICROS_PER_TICK);
            TRACE_PRINT(F(" ("));
            TRACE_PRINT((markLength + spaceLength) * MICROS_PER_TICK);
            TRACE_PRINTLN(F(") "));

#if !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
        /*
         * Handle the first 4 start bits
         * Check if the 3. bit is the long start bit. If we see the long start bit earlier, synchronize bit counter.
         */
        if (tPulseNumber < 4) {
            if (tPulseNumber < 2) {
                // bit 0 and 1
                if (matchSpace(spaceLength, BEO_START_BIT_SPACE - BEO_BIT_MARK_FOR_DECODE)) {
                    TRACE_PRINTLN(F(": detected long start bit -> synchronize state now"));
                    tPulseNumber = 2;
                }
            } else {
                if (tPulseNumber == 3) {
                    if (matchMark(markLength, BEO_BIT_MARK_FOR_DECODE)) {
#  if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
                        protocolMarkLength = BEO_BIT_MARK_FOR_DECODE;
                        } else if (matchMark(markLength, BEO_DATALINK_BIT_MARK)) {
                            protocolMarkLength = BEO_DATALINK_BIT_MARK;
#  endif
                    } else {
                        DEBUG_PRINTLN(F("B&O: 4. (start) mark length is wrong"));
                        return false;
                    }
                }
                // bit 2 and 3
                if (!matchBeoLength(markLength + spaceLength,
                        (tPulseNumber == 2) ? BEO_START_BIT_SPACE : BEO_ZERO_SPACE)) {
                    DEBUG_PRINTLN(F("B&O: Start length is wrong"));
                    return false;
                }
            }
        } else {
#endif // !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)

            /*
             * Decode header / data
             * First check for length of mark
             */
#if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
            if (!matchMark(markLength, protocolMarkLength)) {
#else
            if (!matchMark(markLength, BEO_BIT_MARK_FOR_DECODE)) {
#endif
                DEBUG_PRINTLN(F("B&O: Mark length is wrong"));
                return false;
            }

            /*
             * Check for stop after receiving at least 8 bits for data and 4 bits for header
             */
            if (tBitNumber > BEO_DATA_BITS + 4) {
                if (matchBeoLength(markLength + spaceLength, BEO_TRAILING_BIT_SPACE)) {
                    DEBUG_PRINTLN(F("B&O: Trailing bit detected"));
                    break;
                }
#if !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
                if (tRawBufferMarkIndex >= decodedIRData.rawlen - 3) { // (rawlen - 3) is index of trailing bit mark
                    DEBUG_PRINTLN(F("B&O: End of buffer, but no trailing bit detected"));
                    return false;
                }
#endif
            }

            /*
             * Decode bit
             */
            if (tLastDecodedBitValue == 0 && matchBeoLength(markLength + spaceLength, BEO_ONE_SPACE)) {
                tLastDecodedBitValue = 1;
            } else if (tLastDecodedBitValue == 1 && matchBeoLength(markLength + spaceLength, BEO_ZERO_SPACE)) {
                tLastDecodedBitValue = 0;
            } else if (!matchBeoLength(markLength + spaceLength, BEO_REPETITION_OF_PREVIOUS_BIT_SPACE)) {
                DEBUG_PRINT(F("B&O: Index="));
                DEBUG_PRINT(tRawBufferMarkIndex);
                DEBUG_PRINT(F(" Length "));
                DEBUG_PRINT((markLength + spaceLength) * MICROS_PER_TICK);
                DEBUG_PRINTLN(F(" is wrong"));
                return false;
            }
            tDecodedRawData <<= 1;
            tDecodedRawData |= tLastDecodedBitValue;
            ++tBitNumber;
            TRACE_PRINT(F("Bits "));
            TRACE_PRINT(tBitNumber);
            TRACE_PRINT(F(" "));
            TRACE_PRINT(uint32_t(tDecodedRawData >> BEO_DATA_BITS), HEX);
            TRACE_PRINT(F(" "));
            TRACE_PRINTLN(uint8_t(tDecodedRawData & ((1 << BEO_DATA_BITS) - 1)), HEX);
            // End of bit decode
#if !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
        }

#else
            /*
             * Check for last bit after decoding it
             */
            if (tRawBufferMarkIndex >= decodedIRData.rawlen - 3) { // (rawlen - 3) is index of last bit mark
                TRACE_PRINTLN(F("B&O: Last bit reached"));
                break;
            }
#endif

            ++tPulseNumber;
        }
#if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    }
#endif

    LongUnion tValue;
    tValue.ULong = tDecodedRawData;
    decodedIRData.decodedRawData = tDecodedRawData;
    decodedIRData.numberOfBits = tBitNumber;

    decodedIRData.protocol = BANG_OLUFSEN;
    decodedIRData.command = tValue.UByte.LowByte;
    tValue.ULong = tValue.ULong >> BEO_DATA_BITS;
    decodedIRData.address = tValue.UWord.LowWord;
    if (tBitNumber > 24) { // 24 = 8 bit command and 16 bit address
        decodedIRData.extra = tValue.UByte.MidHighByte;
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST | IRDATA_FLAGS_EXTRA_INFO;
    } else {
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    }

    return true;
}

/** @}*/
#include "LocalDebugLevelEnd.h"

#endif // _IR_BANG_OLUFSEN_HPP
