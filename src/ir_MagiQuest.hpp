/*
 * ir_MagiQuest.hpp
 *
 *  Contains functions for receiving and sending LG IR Protocol in "raw" and standard format with 16 or 8 bit address and 8 bit command
 *  Based off the Magiquest fork of Arduino-IRremote by mpflaga https://github.com/mpflaga/Arduino-IRremote/
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2021 E. Stuart Hicks <ehicks@binarymagi.com>, Armin Joachimsmeyer
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
#ifndef IR_MAGIQUEST_HPP
#define IR_MAGIQUEST_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

//
//==============================================================================
//
//                            M A G I Q U E S T
//
//==============================================================================

#if !defined (DOXYGEN)
// MagiQuest packet is both Wand ID and magnitude of swish and flick
union magiquest_t {
    uint64_t llword;
    struct {
        uint16_t magnitude;
        uint32_t wand_id;
        uint8_t padding;
        uint8_t scrap;  // just to pad the struct out to 64 bits so we can union with llword
    } cmd;
};
#endif // !defined (DOXYGEN)

#define MAGIQUEST_MAGNITUDE_BITS   (sizeof(uint16_t) * 8)   // magiquest_t.cmd.magnitude
#define MAGIQUEST_WAND_ID_BITS     (sizeof(uint32_t) * 8)   // magiquest_t.cmd.wand_id
#define MAGIQUEST_PADDING_BITS     (sizeof(uint8_t) * 8)    // magiquest_t.cmd.padding

#define MAGIQUEST_PERIOD      1150   // Length of time a full MQ "bit" consumes (1100 - 1200 usec)
#define MAGIQUEST_BITS        (MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS)   // Size of the command itself

// The total size of a packet is the sum of all 3 expected fields * 2 to support start/stop bits
#define MAGIQUEST_PACKET_SIZE (2 * (MAGIQUEST_BITS + MAGIQUEST_PADDING_BITS))

/*
 * 0 = 25% mark & 75% space across 1 period
 *     1150 * 0.25 = 288 usec mark
 *     1150 - 288 = 862 usec space
 * 1 = 50% mark & 50% space across 1 period
 *     1150 * 0.5 = 575 usec mark
 *     1150 - 575 = 575 usec space
 */
#define MAGIQUEST_UNIT          (MAGIQUEST_PERIOD / 4)

#define MAGIQUEST_ONE_MARK      (2 * MAGIQUEST_UNIT) // 576
#define MAGIQUEST_ONE_SPACE     (2 * MAGIQUEST_UNIT) // 576
#define MAGIQUEST_ZERO_MARK     MAGIQUEST_UNIT
#define MAGIQUEST_ZERO_SPACE    (3 * MAGIQUEST_UNIT) // 864

//+=============================================================================
//
void IRsend::sendMagiQuest(uint32_t wand_id, uint16_t magnitude) {

    // Set IR carrier frequency
    enableIROut(38);

    // 2 start bits
    sendPulseDistanceWidthData(
    MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, 0, 2, PROTOCOL_IS_MSB_FIRST);

    // Data
    sendPulseDistanceWidthData(
    MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, wand_id, MAGIQUEST_WAND_ID_BITS,
    PROTOCOL_IS_MSB_FIRST);
    sendPulseDistanceWidthData(
    MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, magnitude, MAGIQUEST_MAGNITUDE_BITS,
    PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);
}

//+=============================================================================
//
/*
 * decodes a 56 bit result, which is not really compatible with standard decoder layout
 */
bool IRrecv::decodeMagiQuest() {
    magiquest_t data;  // Somewhere to build our code
    unsigned int offset = 1;  // Skip the gap reading

    unsigned int mark_;
    unsigned int space_;
    unsigned int ratio_;

#ifdef DEBUG
    char bitstring[(MAGIQUEST_PACKET_SIZE + 1)];
    bitstring[MAGIQUEST_PACKET_SIZE] = '\0';
#endif

    // Check we have the right amount of data
    if (decodedIRData.rawDataPtr->rawlen != MAGIQUEST_PACKET_SIZE) {
        IR_DEBUG_PRINT("MagiQuest: Bad packet length - got ");
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINT(", expected ");
        IR_DEBUG_PRINTLN(MAGIQUEST_PACKET_SIZE);
        return false;
    }

    // Read the bits in
    data.llword = 0;
    while (offset < (MAGIQUEST_PACKET_SIZE - 1)) {
        mark_ = decodedIRData.rawDataPtr->rawbuf[offset++];
        space_ = decodedIRData.rawDataPtr->rawbuf[offset++];
        ratio_ = space_ / mark_;

        IR_TRACE_PRINT("MagiQuest: mark=");
        IR_TRACE_PRINT(mark_ * MICROS_PER_TICK);
        IR_TRACE_PRINT(" space=");
        IR_TRACE_PRINT(space_ * MICROS_PER_TICK);
        IR_TRACE_PRINT(" ratio=");
        IR_TRACE_PRINTLN(ratio_);

        if (matchMark(space_ + mark_, MAGIQUEST_PERIOD)) {
            if (ratio_ > 1) {
                // It's a 0
                data.llword <<= 1;
#ifdef DEBUG
                bitstring[(offset / 2) - 1] = '0';
#endif
            } else {
                // It's a 1
                data.llword = (data.llword << 1) | 1;
#ifdef DEBUG
                bitstring[(offset / 2) - 1] = '1';
#endif
            }
        } else {
            IR_DEBUG_PRINTLN("MATCH_MARK failed");
            return false;
        }
    }
    IR_DEBUG_PRINTLN(bitstring);

    // Success
    decodedIRData.protocol = MAGIQUEST;
    decodedIRData.numberOfBits = offset / 2;
    decodedIRData.flags = IRDATA_FLAGS_EXTRA_INFO;
    decodedIRData.extra = data.cmd.magnitude;
    decodedIRData.decodedRawData = data.cmd.wand_id;

    return true;
}
#endif
#pragma once
