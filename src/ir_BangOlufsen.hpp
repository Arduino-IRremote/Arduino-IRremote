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
 * Copyright (c) 2022 Daniel Wallner
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

//==============================================================================
//
//
//                              Bang & Olufsen
//
//
//==============================================================================
// https://www.mikrocontroller.net/attachment/33137/datalink.pdf
// https://www.mikrocontroller.net/articles/IRMP_-_english#B&O

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

// Alt 1: Mode with gaps between frames
// Set RECORD_GAP_MICROS to at least 16000 to accommodate the unusually long 3. start space
// Can only receive single messages and back to back repeats will result in overflow

// Alt 2: Break at start mode
// Define ENABLE_BEO_WITHOUT_FRAME_GAP and set RECORD_GAP_MICROS to 13000 to treat the 3. start space as a gap between messages
// The start of a transmission will result in a dummy decode with 0 bits data followed by the actual messages
// If the receiver is not resumed within a ms or so, partial messages will be decoded
// Debug printing in the wrong place is very likely to break reception
// Make sure to check the number of bits to filter dummy and incomplete messages

// !!! We assume that the real implementations never set the official first header bit to anything other than 0 !!!
// !!! We therefore use 4 start bits instead of the specified 3 and in turn ignore the first header bit of the specification !!!

// IR messages are 16 bits long and datalink messages have different lengths
// This implementation supports up to 40 bits total length split into 8 bit data/command and a header/address of variable length
// Header data with more than 16 bits is stored in decodedIRData.extra

// B&O is a pulse distance protocol, but it has 3 bit values 0, 1 and (equal/repeat) as well as a special start and trailing bit.
// MSB first, 4 start bits + 8 to 16? bit address + 8 bit command + 1 special trailing bit + 1 stop bit.
// Address can be longer than 8 bit.

/*
 * Options for this decoder
 */
//#define ENABLE_BEO_WITHOUT_FRAME_GAP // Requires additional 30 bytes program memory.
//#define SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE // This also supports headers up to 32 bit. Requires additional 150 bytes program memory.
#if defined(DECODE_BEO)
#  if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
#    if RECORD_GAP_MICROS > 13000
#warning If defined ENABLE_BEO_WITHOUT_FRAME_GAP, RECORD_GAP_MICROS must be set to 1300 by "#define RECORD_GAP_MICROS 13000"
#    endif
#  else
#    if RECORD_GAP_MICROS < 16000
#error If not defined ENABLE_BEO_WITHOUT_FRAME_GAP, RECORD_GAP_MICROS must be set to a value >= 1600 by "#define RECORD_GAP_MICROS 16000"
#    endif
#  endif
#endif

#define BEO_DATA_BITS         8                // Command or character

#define BEO_UNIT              3125             // All timings are in microseconds

#define BEO_IR_MARK           200              // The length of a mark in the IR protocol
#define BEO_DATALINK_MARK     (BEO_UNIT / 2)   // The length of a mark in the Datalink protocol

#define BEO_PULSE_LENGTH_ZERO           BEO_UNIT      // The length of a one to zero transition
#define BEO_PULSE_LENGTH_EQUAL          (2 * BEO_UNIT)   // 6250 The length of an equal bit
#define BEO_PULSE_LENGTH_ONE            (3 * BEO_UNIT)   // 9375 The length of a zero to one transition
#define BEO_PULSE_LENGTH_TRAILING_BIT   (4 * BEO_UNIT)   // 12500 The length of the stop bit
#define BEO_PULSE_LENGTH_START_BIT      (5 * BEO_UNIT)   // 15625 The length of the start bit
// It is not allowed to send two ones or zeros, you must send a one or zero and a equal instead.

//#define BEO_LOCAL_DEBUG
//#define BEO_LOCAL_TRACE

#ifdef BEO_LOCAL_DEBUG
#  define BEO_DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
#  define BEO_DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#  define BEO_DEBUG_PRINT(...) void()
#  define BEO_DEBUG_PRINTLN(...) void()
#endif

#ifdef BEO_LOCAL_TRACE
#  undef BEO_TRACE_PRINT
#  undef BEO_TRACE_PRINTLN
#  define BEO_TRACE_PRINT(...)    Serial.print(__VA_ARGS__)
#  define BEO_TRACE_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#  define BEO_TRACE_PRINT(...) void()
#  define BEO_TRACE_PRINTLN(...) void()
#endif

/************************************
 * Start of send and decode functions
 ************************************/

/*
 * TODO aNumberOfRepeats are handled not correctly if ENABLE_BEO_WITHOUT_FRAME_GAP is defined
 */
void IRsend::sendBangOlufsen(uint16_t aHeader, uint8_t aData, int_fast8_t aNumberOfRepeats, int8_t aNumberOfHeaderBits) {
    for (int_fast8_t i = 0; i < aNumberOfRepeats + 1; ++i) {
        sendBangOlufsenRaw((uint32_t(aHeader) << 8) | aData, aNumberOfHeaderBits + 8, i != 0);
    }
}

void IRsend::sendBangOlufsenDataLink(uint32_t aHeader, uint8_t aData, int_fast8_t aNumberOfRepeats, int8_t aNumberOfHeaderBits) {
    for (int_fast8_t i = 0; i < aNumberOfRepeats + 1; ++i) {
        sendBangOlufsenRawDataLink((uint64_t(aHeader) << 8) | aData, aNumberOfHeaderBits + 8, i != 0, true);
    }
}

/*
 * @param aBackToBack   If true send data back to back, which cannot be decoded if ENABLE_BEO_WITHOUT_FRAME_GAP is NOT defined
 */
void IRsend::sendBangOlufsenRaw(uint32_t aRawData, int_fast8_t aBits, bool aBackToBack) {
#if defined(USE_NO_SEND_PWM) || BEO_KHZ == 38 // BEO_KHZ == 38 is for unit test which runs the B&O protocol with 38 kHz

    /*
     * 455 kHz PWM is currently not supported, maximum is 180 kHz
     */
#if !defined(USE_NO_SEND_PWM)
    enableIROut (BEO_KHZ);
#endif

// AGC / Start - 3 bits + first constant 0 header bit described in the official documentation
    if (!aBackToBack) {
        mark(BEO_IR_MARK);
    }
    space(BEO_PULSE_LENGTH_ZERO - BEO_IR_MARK);
    mark(BEO_IR_MARK);
    space(BEO_PULSE_LENGTH_ZERO - BEO_IR_MARK);
    mark(BEO_IR_MARK);
    space(BEO_PULSE_LENGTH_START_BIT - BEO_IR_MARK);

// First bit of header is assumed to be a constant 0 to have a fixed state to begin with the equal decisions.
// So this first 0 is treated as the last bit of AGC
    mark(BEO_IR_MARK);
    space(BEO_PULSE_LENGTH_ZERO - BEO_IR_MARK);
    bool tLastBitValueWasOne = false;

// Header / Data
    uint32_t mask = 1UL << (aBits - 1);
    for (; mask; mask >>= 1) {
        if (tLastBitValueWasOne && !(aRawData & mask)) {
            mark(BEO_IR_MARK);
            space(BEO_PULSE_LENGTH_ZERO - BEO_IR_MARK);
            tLastBitValueWasOne = false;
        } else if (!tLastBitValueWasOne && (aRawData & mask)) {
            mark(BEO_IR_MARK);
            space(BEO_PULSE_LENGTH_ONE - BEO_IR_MARK);
            tLastBitValueWasOne = true;
        } else {
            mark(BEO_IR_MARK);
            space(BEO_PULSE_LENGTH_EQUAL - BEO_IR_MARK);
        }
    }

// Stop
    mark(BEO_IR_MARK);
    space(BEO_PULSE_LENGTH_TRAILING_BIT - BEO_IR_MARK);
    mark(BEO_IR_MARK);

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
    uint16_t tSendBEOMarkLength = aUseDatalinkTiming ? BEO_DATALINK_MARK : BEO_IR_MARK;

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
    space(BEO_PULSE_LENGTH_ZERO - tSendBEOMarkLength);
    mark(tSendBEOMarkLength);
    space(BEO_PULSE_LENGTH_ZERO - tSendBEOMarkLength);
    mark(tSendBEOMarkLength);
    space(BEO_PULSE_LENGTH_START_BIT - tSendBEOMarkLength);

// First bit of header is assumed to be a constant 0 to have a fixed state to begin with the equal decisions.
// So this first 0 is treated as the last bit of AGC
    mark(tSendBEOMarkLength);
    space(BEO_PULSE_LENGTH_ZERO - tSendBEOMarkLength);
    bool tLastBitValueWasOne = false;

// Header / Data
    uint32_t mask = 1UL << (aBits - 1);
    for (; mask; mask >>= 1) {
        if (tLastBitValueWasOne && !(aRawData & mask)) {
            mark(tSendBEOMarkLength);
            space(BEO_PULSE_LENGTH_ZERO - tSendBEOMarkLength);
            tLastBitValueWasOne = false;
        } else if (!tLastBitValueWasOne && (aRawData & mask)) {
            mark(tSendBEOMarkLength);
            space(BEO_PULSE_LENGTH_ONE - tSendBEOMarkLength);
            tLastBitValueWasOne = true;
        } else {
            mark(tSendBEOMarkLength);
            space(BEO_PULSE_LENGTH_EQUAL - tSendBEOMarkLength);
        }
    }

// Stop
    mark(tSendBEOMarkLength);
    space(BEO_PULSE_LENGTH_TRAILING_BIT - tSendBEOMarkLength);
    mark(tSendBEOMarkLength);

#else
    (void) aRawData;
    (void) aBits;
    (void) aUseDatalinkTiming;
    (void) aBackToBack;
#endif
}

#define BEO_MATCH_DELTA (BEO_UNIT / 2 - MICROS_PER_TICK)
static bool matchBeoLength(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
    const uint16_t tMeasuredMicros = aMeasuredTicks * MICROS_PER_TICK;
    return aMatchValueMicros - BEO_MATCH_DELTA < tMeasuredMicros && tMeasuredMicros < aMatchValueMicros + BEO_MATCH_DELTA;
}

bool IRrecv::decodeBangOlufsen() {
#if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    if (decodedIRData.rawDataPtr->rawlen != 6 && decodedIRData.rawDataPtr->rawlen < 36) { // 16 bits minimum
#else
    if (decodedIRData.rawDataPtr->rawlen < 44) { // 16 bits minimum
#endif
        return false;
    }

#if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
    uint16_t protocolMarkLength = 0; // contains BEO_IR_MARK or BEO_DATALINK_MARK depending of 4. mark received
    uint64_t tDecodedRawData = 0;
#else
    uint32_t tDecodedRawData = 0;
#endif
    uint8_t tLastDecodedBitValue = 0; // the last start bit is assumed to be zero
    uint8_t tPulseNumber = 0;
    uint8_t tBitNumber = 0;

    BEO_TRACE_PRINT(F("Pre gap: "));
    BEO_TRACE_PRINT(decodedIRData.rawDataPtr->rawbuf[0] * 50);
    BEO_TRACE_PRINT(F(" raw len: "));
    BEO_TRACE_PRINTLN(decodedIRData.rawDataPtr->rawlen);

#if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    /*
     * Check if we have the AGC part of the first frame in a row
     */
    if (decodedIRData.rawDataPtr->rawlen == 6) {
        if ((matchMark(decodedIRData.rawDataPtr->rawbuf[3], BEO_IR_MARK)
                || matchMark(decodedIRData.rawDataPtr->rawbuf[3], BEO_DATALINK_MARK))
                && (matchSpace(decodedIRData.rawDataPtr->rawbuf[4], BEO_PULSE_LENGTH_ZERO - BEO_IR_MARK)
                        || matchSpace(decodedIRData.rawDataPtr->rawbuf[4], BEO_PULSE_LENGTH_ZERO - BEO_DATALINK_MARK))) {
            BEO_TRACE_PRINT(::getProtocolString(BANG_OLUFSEN));
            BEO_TRACE_PRINTLN(F("B&O: AGC only part detected"));
        } else {
            return false; // no B&O protocol
        }
    } else {
        /*
         * Check if leading gap is trailing bit of first frame
         */
        if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[0], BEO_PULSE_LENGTH_START_BIT)) {
            BEO_TRACE_PRINT(::getProtocolString(BANG_OLUFSEN));
            BEO_TRACE_PRINTLN(F(": Leading gap is wrong")); // Leading gap is trailing bit of first frame
            return false; // no B&O protocol
        }

        if (matchMark(decodedIRData.rawDataPtr->rawbuf[1], BEO_IR_MARK)) {
#if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
            protocolMarkLength = BEO_IR_MARK;
        } else if (matchMark(decodedIRData.rawDataPtr->rawbuf[1], BEO_DATALINK_MARK)) {
            protocolMarkLength = BEO_DATALINK_MARK;
#endif
        } else {
            BEO_TRACE_PRINT(::getProtocolString(BANG_OLUFSEN));
            BEO_TRACE_PRINTLN(F(": mark length is wrong"));
            return false;
        }

        // skip first zero header bit
        for (uint8_t tRawBufferMarkIndex = 3; tRawBufferMarkIndex < decodedIRData.rawDataPtr->rawlen; tRawBufferMarkIndex += 2) {
#else
    for (uint8_t tRawBufferMarkIndex = 1; tRawBufferMarkIndex < decodedIRData.rawDataPtr->rawlen; tRawBufferMarkIndex += 2) {
#endif

            uint16_t markLength = decodedIRData.rawDataPtr->rawbuf[tRawBufferMarkIndex];
            uint16_t spaceLength = decodedIRData.rawDataPtr->rawbuf[tRawBufferMarkIndex + 1];

            BEO_TRACE_PRINT(tPulseNumber);
            BEO_TRACE_PRINT(' ');
            BEO_TRACE_PRINT(markLength * MICROS_PER_TICK);
            BEO_TRACE_PRINT(' ');
            BEO_TRACE_PRINT(spaceLength * MICROS_PER_TICK);
            BEO_TRACE_PRINT(F(" ("));
            BEO_TRACE_PRINT((markLength + spaceLength) * MICROS_PER_TICK);
            BEO_TRACE_PRINTLN(F(") "));

#if !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
        /*
         * Handle the first 4 start bits
         * Check if the 3. bit is the long start bit. If we see the long start bit earlier, synchronize bit counter.
         */
        if (tPulseNumber < 4) {
            if (tPulseNumber < 2) {
                // bit 0 and 1
                if (matchSpace(spaceLength, BEO_PULSE_LENGTH_START_BIT - BEO_IR_MARK)) {
                    BEO_TRACE_PRINTLN(F(": detected long start bit -> synchronize state now"));
                    tPulseNumber = 2;
                }
            } else {
                if (tPulseNumber == 3) {
                    if (matchMark(markLength, BEO_IR_MARK)) {
#if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
                        protocolMarkLength = BEO_IR_MARK;
                        } else if (matchMark(markLength, BEO_DATALINK_MARK)) {
                            protocolMarkLength = BEO_DATALINK_MARK;
#endif
                    } else {
                        BEO_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                        BEO_DEBUG_PRINTLN(F(": 4. (start) mark length is wrong"));
                        return false;
                    }
                }
                // bit 2 and 3
                if (!matchBeoLength(markLength + spaceLength,
                        (tPulseNumber == 2) ? BEO_PULSE_LENGTH_START_BIT : BEO_PULSE_LENGTH_ZERO)) {
                    BEO_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    BEO_DEBUG_PRINTLN(F(": Start length is wrong"));
                    return false;
                }
            }
        } else {
#endif

            /*
             * Decode header / data
             * First check for length of mark
             */
#if defined(SUPPORT_BEO_DATALINK_TIMING_FOR_DECODE)
            if (!matchMark(markLength, protocolMarkLength)) {
#else
            if (!matchMark(markLength, BEO_IR_MARK)) {
#endif
                BEO_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                BEO_DEBUG_PRINTLN(F(": Mark length is wrong"));
                return false;
            }

            /*
             * Check for stop after receiving at least 8 bits for data and 4 bits for header
             */
            if (tBitNumber > BEO_DATA_BITS + 4) {
                if (matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_TRAILING_BIT)) {
                    BEO_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    BEO_DEBUG_PRINTLN(F(": Trailing bit detected"));
                    break;
                }
#if !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
                if (tRawBufferMarkIndex >= decodedIRData.rawDataPtr->rawlen - 3) { // (rawlen - 3) is index of trailing bit mark
                    BEO_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    BEO_DEBUG_PRINTLN(F(": End of buffer, but no trailing bit detected"));
                    return false;
                }
#endif
            }

            /*
             * Decode bit
             */
            if (tLastDecodedBitValue == 0 && matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_ONE)) {
                tLastDecodedBitValue = 1;
            } else if (tLastDecodedBitValue == 1 && matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_ZERO)) {
                tLastDecodedBitValue = 0;
            } else if (!matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_EQUAL)) {
                BEO_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                BEO_DEBUG_PRINT(F(": Index="));
                BEO_DEBUG_PRINT(tRawBufferMarkIndex);
                BEO_DEBUG_PRINT(F(" Length "));
                BEO_DEBUG_PRINT((markLength + spaceLength) * MICROS_PER_TICK);
                BEO_DEBUG_PRINTLN(F(" is wrong"));
                return false;
            }
            tDecodedRawData <<= 1;
            tDecodedRawData |= tLastDecodedBitValue;
            ++tBitNumber;
            BEO_TRACE_PRINT(F("Bits "));
            BEO_TRACE_PRINT(tBitNumber);
            BEO_TRACE_PRINT(F(" "));
            BEO_TRACE_PRINT(uint32_t(tDecodedRawData >> BEO_DATA_BITS), HEX);
            BEO_TRACE_PRINT(F(" "));
            BEO_TRACE_PRINTLN(uint8_t(tDecodedRawData & ((1 << BEO_DATA_BITS) - 1)), HEX);
            // End of bit decode
#if !defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
        }

#else
            /*
             * Check for last bit after decoding it
             */
            if (tRawBufferMarkIndex >= decodedIRData.rawDataPtr->rawlen - 3) { // (rawlen - 3) is index of last bit mark
                BEO_TRACE_PRINT(::getProtocolString(BANG_OLUFSEN));
                BEO_TRACE_PRINTLN(F(": Last bit reached"));
                break;
            }
#endif

            ++tPulseNumber;
        }
#if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    }
#endif

    decodedIRData.protocol = BANG_OLUFSEN;
    decodedIRData.address = tDecodedRawData >> BEO_DATA_BITS;              // lower header tBitNumber
    decodedIRData.command = tDecodedRawData & ((1 << BEO_DATA_BITS) - 1);  // lower 8 tBitNumber
    decodedIRData.extra = tDecodedRawData >> (BEO_DATA_BITS + 16);         // upper header tBitNumber
    decodedIRData.numberOfBits = tBitNumber;
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.decodedRawData = tDecodedRawData;

    return true;
}
#endif // _IR_BANG_OLUFSEN_HPP
