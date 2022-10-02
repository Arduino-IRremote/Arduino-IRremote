/*
 * ir_BangOlufsen.hpp
 *
 *  Contains functions for receiving and sending Bang & Olufsen IR and Datalink '86 protocols
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

// This protocol is unusual in two ways:

// 1. The carrier frequency is 455 kHz

// You can build your own receiver as Bang & Olufsen did (check old schematics) or use a TSOP7000
// Vishay stopped producing TSOP7000 a long time ago so you will likely only find counterfeits:
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

// Alt 1: Strict mode
// Define BEO_STRICT and set RECORD_GAP_MICROS to at least 16000 to accomodate the unusually long start space
// Can only receive single messages and repeats will result in overflow

// Alt 2: Break at start mode
// Set RECORD_GAP_MICROS to 13000 to treat the start space as a gap between messages
// The start of a transmision will result in a dummy decode with 0 bits data followed by the actual messages
// If the receiver is not resumed within a ms or so partial messages will be decoded
// Debug printing in the wrong place is very likely to break reception
// Make sure to check the number of bits to filter dummy and incomplete messages


// It's possible that official implementations never set the top bit to anything other than 0 and hence the actual bits
// sent may be one bit less than this implementation reports
// The specification never specifically names the first bit a start bit and this code supports the use of it

// IR messages are 17 bits long and datalink messages have different lengths
// This implementation supports up to 40 bits total length split into 8 bit data/command and a header/address of variable length
// Header data with more than 16 bits is stored in decodedIRData.extra

#define BEO_DATA_BITS         8                // Command or character

#define BEO_UNIT              3125             // All timings are in microseconds

#define BEO_IR_MARK           200              // The length of a mark in the IR protocol
#define BEO_DATALINK_MARK     (BEO_UNIT / 2)   // The length of a mark in the Datalink protocol

#define BEO_PULSE_LENGTH_1    BEO_UNIT         // The length of a one to zero transistion
#define BEO_PULSE_LENGTH_2    (2 * BEO_UNIT)   // The length of an equal bit
#define BEO_PULSE_LENGTH_3    (3 * BEO_UNIT)   // The length of a zero to one transistion
#define BEO_PULSE_LENGTH_4    (4 * BEO_UNIT)   // The length of the stop bit
#define BEO_PULSE_LENGTH_5    (5 * BEO_UNIT)   // The length of the start bit


//#define BEO_LOCAL_DEBUG
//#define BEO_LOCAL_TRACE

#ifdef BEO_LOCAL_DEBUG
#  undef IR_DEBUG_PRINT
#  undef IR_DEBUG_PRINTLN
#  define IR_DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
#  define IR_DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#endif

#ifdef BEO_LOCAL_TRACE
#  undef IR_TRACE_PRINT
#  undef IR_TRACE_PRINTLN
#  define IR_TRACE_PRINT(...)    Serial.print(__VA_ARGS__)
#  define IR_TRACE_PRINTLN(...)  Serial.println(__VA_ARGS__)
#endif

/************************************
 * Start of send and decode functions
 ************************************/

void IRsend::sendBangOlufsen(uint32_t aHeader, uint8_t aData, int8_t aHeaderBits, bool aDatalink, int_fast8_t aNumberOfRepeats)
{
    for (uint_fast8_t i = 0; i < aNumberOfRepeats + 1; ++i) {
        sendBangOlufsenRaw((uint64_t(aHeader) << 8) | aData, aHeaderBits + 8, aDatalink, i != 0);
    }
}

void IRsend::sendBangOlufsenRaw(uint64_t aRawData, int_fast8_t aBits, bool aDatalink, bool aBackToBack)
{
    uint16_t markLength = aDatalink ? BEO_DATALINK_MARK : BEO_IR_MARK;

    enableIROut(BEO_KHZ);

    // AGC / Start
    if (!aBackToBack) {
        mark(markLength);
    }
    space(BEO_PULSE_LENGTH_1 - markLength);
    mark(markLength);
    space(BEO_PULSE_LENGTH_1 - markLength);
    mark(markLength);
    space(BEO_PULSE_LENGTH_5 - markLength);

    bool lastBit = true;

    // Header / Data
    uint64_t mask = 1UL << (aBits - 1);
    for (; mask; mask >>= 1) {
        if (lastBit && !(aRawData & mask)) {
            mark(markLength);
            space(BEO_PULSE_LENGTH_1 - markLength);
            lastBit = false;
        } else if (!lastBit && (aRawData & mask)) {
            mark(markLength);
            space(BEO_PULSE_LENGTH_3 - markLength);
            lastBit = true;
        } else {
            mark(markLength);
            space(BEO_PULSE_LENGTH_2 - markLength);
        }
    }

    // Stop
    mark(markLength);
    space(BEO_PULSE_LENGTH_4 - markLength);
    mark(markLength);

    IrReceiver.restartAfterSend();
}

static bool matchBeoLength(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros)
{
    static const uint16_t limit = BEO_UNIT / 2 - MICROS_PER_TICK;
    const uint16_t measuredMicros = aMeasuredTicks * MICROS_PER_TICK;
    return measuredMicros + limit > aMatchValueMicros && measuredMicros < aMatchValueMicros + limit;
}

bool IRrecv::decodeBangOlufsen() {
#ifdef BEO_STRICT
    if (decodedIRData.rawDataPtr->rawlen < 42) { // 16 bits minimun
#else
    if (decodedIRData.rawDataPtr->rawlen != 6 && decodedIRData.rawDataPtr->rawlen < 36) { // 16 bits minimun
#endif
        return false;
    }

#if !defined(BEO_STRICT) && (defined(DEBUG) || defined(TRACE) || defined(BEO_LOCAL_DEBUG) || defined(BEO_LOCAL_TRACE))
    if (decodedIRData.rawDataPtr->rawlen == 6) {
        // Short circuit to avoid spending too much time printing and then miss the actual message
        decodedIRData.protocol = BANG_OLUFSEN;
        decodedIRData.address = 0;
        decodedIRData.command = 0;
        decodedIRData.extra = 0;
        decodedIRData.numberOfBits = 0;
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
        decodedIRData.decodedRawData = 0;
        return true;
    }
#endif

    uint16_t protocolMarkLength = 0;
    uint8_t lastBit = 1;
    uint8_t pulseNum = 0;
    uint8_t bits = 0;
    uint64_t receivedData = 0;
    bool complete = false;

    for (uint8_t rawPos = 1; rawPos < decodedIRData.rawDataPtr->rawlen; rawPos += 2) {
        uint16_t markLength = decodedIRData.rawDataPtr->rawbuf[rawPos];
        uint16_t spaceLength = rawPos + 1 < decodedIRData.rawDataPtr->rawlen ? decodedIRData.rawDataPtr->rawbuf[rawPos + 1] : 0;

        if (pulseNum == 0) {
            IR_TRACE_PRINT(F("Pre space: "));
            IR_TRACE_PRINT(decodedIRData.rawDataPtr->rawbuf[0] * 50);
            IR_TRACE_PRINT(F(" raw len: "));
            IR_TRACE_PRINTLN(decodedIRData.rawDataPtr->rawlen);
        }

        IR_TRACE_PRINT(pulseNum);
        IR_TRACE_PRINT(F(" "));
        IR_TRACE_PRINT(markLength * MICROS_PER_TICK);
        IR_TRACE_PRINT(F(" "));
        IR_TRACE_PRINT(spaceLength * MICROS_PER_TICK);
        IR_TRACE_PRINT(F(" ("));
        IR_TRACE_PRINT((markLength + spaceLength) * MICROS_PER_TICK);
        IR_TRACE_PRINTLN(F(") "));

#ifndef BEO_STRICT
        if (bits == 0 && rawPos + 1 == decodedIRData.rawDataPtr->rawlen) {
            IR_TRACE_PRINTLN(F(": Jump to end"));
            pulseNum = 3;
            complete = true;
            continue;
        }
#endif

        // Check start
        if (pulseNum < 3) {
            if (protocolMarkLength == 0) {
                if (matchMark(markLength, BEO_IR_MARK)) {
                    protocolMarkLength = BEO_IR_MARK;
                }
                if (matchMark(markLength, BEO_DATALINK_MARK)) {
                    protocolMarkLength = BEO_DATALINK_MARK;
                }
                if (!protocolMarkLength) {
                    IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    IR_DEBUG_PRINTLN(F(": Start mark length 1 is wrong"));
                    return false;
                }
            } else {
                if (!matchMark(markLength, protocolMarkLength)) {
                    IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    IR_DEBUG_PRINTLN(F(": Start mark length is wrong"));
                    return false;
                }
            }
#ifdef BEO_STRICT
            if (!matchBeoLength(markLength + spaceLength, (pulseNum == 2) ? BEO_PULSE_LENGTH_5 : BEO_PULSE_LENGTH_1)) {
                IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                IR_DEBUG_PRINTLN(F(": Start length is wrong"));
                return false;
            }
#else
            if (matchSpace(decodedIRData.rawDataPtr->rawbuf[0], BEO_PULSE_LENGTH_5 - BEO_IR_MARK)) {
                // Jump to bits
                IR_TRACE_PRINTLN(F(": Jump to bits 1"));
                pulseNum = 2;
                rawPos = -1;
            } else if (matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_1)) {
                if (pulseNum == 2) {
                    IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    IR_DEBUG_PRINTLN(F(": Start sequence is wrong"));
                    return false;
                }
            } else if (matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_5)) {
                // Jump to bits
                pulseNum = 2;
                IR_TRACE_PRINTLN(F(": Jump to bits 2"));
            } else {
                IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                IR_DEBUG_PRINTLN(F(": Start length is wrong"));
                return false;
            }
#endif
        }
        // Decode header / data
        else {
            if (!matchMark(markLength, protocolMarkLength)) {
                IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                IR_DEBUG_PRINTLN(F(": Mark length is wrong"));
                return false;
            }
            if (complete) {
#ifdef BEO_STRICT
                if (rawPos + 1 != decodedIRData.rawDataPtr->rawlen) {
                    IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    IR_DEBUG_PRINTLN(F(": Extra data"));
                    return false;
                }
#endif
                break;
            }
            if (bits > BEO_DATA_BITS) {
                // Check for stop
                if (matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_4)) {
                    if (rawPos + 2 < decodedIRData.rawDataPtr->rawlen) {
                        markLength = 0;
                        spaceLength = 0;
                        complete = true;
                        continue;
                    }
                    IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                    IR_DEBUG_PRINTLN(F(": Incomplete"));
                    return false;
                }
            }
            if (lastBit == 0 && matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_3)) {
                lastBit = 1;
            } else if (lastBit == 1 && matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_1)) {
                lastBit = 0;
            } else if (!matchBeoLength(markLength + spaceLength, BEO_PULSE_LENGTH_2)) {
                IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
                IR_DEBUG_PRINT(F(": Length "));
                IR_DEBUG_PRINT((markLength + spaceLength) * MICROS_PER_TICK);
                IR_DEBUG_PRINTLN(F(" is wrong"));
                return false;
            }
            receivedData <<= 1;
            receivedData |= lastBit;
            ++bits;
            IR_TRACE_PRINT(F("Bits "));
            IR_TRACE_PRINT(bits);
            IR_TRACE_PRINT(F(" "));
            IR_TRACE_PRINT(uint32_t(receivedData >> BEO_DATA_BITS), HEX);
            IR_TRACE_PRINT(F(" "));
            IR_TRACE_PRINTLN(uint8_t(receivedData & ((1 << BEO_DATA_BITS) - 1)), HEX);
        }

        ++pulseNum;
    }

    if (!complete) {
        IR_DEBUG_PRINT(::getProtocolString(BANG_OLUFSEN));
        IR_DEBUG_PRINTLN(F(": Not enough bits"));
        return false;
    }

    decodedIRData.protocol = BANG_OLUFSEN;
    decodedIRData.address = receivedData >> BEO_DATA_BITS;  // lower header bits
    decodedIRData.command = receivedData & ((1 << BEO_DATA_BITS) - 1);    // lower 8 bits
    decodedIRData.extra = receivedData >> (BEO_DATA_BITS + 16);  // upper header bits
    decodedIRData.numberOfBits = bits;
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.decodedRawData = receivedData;

    return true;
}

#ifdef BEO_LOCAL_DEBUG
#  undef IR_DEBUG_PRINT
#  undef IR_DEBUG_PRINTLN
#  define IR_DEBUG_PRINT(...) void()
#  define IR_DEBUG_PRINTLN(...) void()
#endif

#ifdef BEO_LOCAL_TRACE
#  undef IR_TRACE_PRINT
#  undef IR_TRACE_PRINTLN
#  define IR_TRACE_PRINT(...) void()
#  define IR_TRACE_PRINTLN(...) void()
#endif

#endif // _IR_BANG_OLUFSEN_HPP
