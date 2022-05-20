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
#ifndef _IR_MAGIQUEST_HPP
#define _IR_MAGIQUEST_HPP

#include <Arduino.h>

#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

//
//==============================================================================
//
//                            M A G I Q U E S T
//
//==============================================================================
// MSB first, 8 Start bits (zero), 32 wand id bits, 16 magnitude bits, one stop bit
// Not all start bits must be received, since protocol is MSB first and so the LSB ends up always at the right position.

#if !defined (DOXYGEN)
// MagiQuest packet is both Wand ID and magnitude of swish and flick
union magiquest_t {
    uint64_t llword;
    struct {
        uint16_t magnitude;
        uint32_t wand_id;
        uint8_t StartBits;  // first 8 MSB start bits are zero.
        uint8_t HighByte;   // just to pad the struct out to 64 bits so we can union with llword
    } cmd;
};
#endif // !defined (DOXYGEN)

#define MAGIQUEST_MAGNITUDE_BITS   16   // magiquest_t.cmd.magnitude
#define MAGIQUEST_WAND_ID_BITS     32   // magiquest_t.cmd.wand_id
#define MAGIQUEST_START_BITS        8    // magiquest_t.cmd.StartBits

#define MAGIQUEST_PERIOD      1150   // Time for a full MagiQuest "bit" (1100 - 1200 usec)

#define MAGIQUEST_BITS        (MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS) // 48 Size of the command without the start bits
// The maximum size of a packet is the sum of all 3 expected fields * 2
#define MAGIQUEST_PACKET_SIZE (MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS + MAGIQUEST_START_BITS) // 56

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

    // 8 start bits
    sendPulseDistanceWidthData(
    MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, 0, 8, PROTOCOL_IS_MSB_FIRST);

    // Data
    sendPulseDistanceWidthData(
    MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, wand_id, MAGIQUEST_WAND_ID_BITS,
    PROTOCOL_IS_MSB_FIRST);
    sendPulseDistanceWidthData(
    MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, magnitude, MAGIQUEST_MAGNITUDE_BITS,
    PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);
    IrReceiver.restartAfterSend();
}

//+=============================================================================
//
/*
 * decodes a 56 bit result, which is not really compatible with standard decoder layout
 */
bool IRrecv::decodeMagiQuest() {
    magiquest_t data;  // Somewhere to build our code
    unsigned int tOffset = 1;  // Skip the gap between packets

    unsigned int tMark;
    unsigned int tSpace;

#if defined(DEBUG)
    char bitstring[(MAGIQUEST_PACKET_SIZE + 1)];
    bitstring[MAGIQUEST_PACKET_SIZE] = '\0';
#endif

    // Check we have the right amount of data, magnitude and ID bits and at least 2 start bits + 1 stop bit
    if (decodedIRData.rawDataPtr->rawlen < (2 * (MAGIQUEST_BITS + 3))
            || decodedIRData.rawDataPtr->rawlen > (2 * (MAGIQUEST_PACKET_SIZE + 1))) {
        IR_DEBUG_PRINT(F("MagiQuest: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not between 102 and 114"));
        return false;
    }

    // Read the bits in
    data.llword = 0;
    while (tOffset < (unsigned int) (decodedIRData.rawDataPtr->rawlen - 1)) {
        // get one mark and space pair
        tMark = decodedIRData.rawDataPtr->rawbuf[tOffset++];
        tSpace = decodedIRData.rawDataPtr->rawbuf[tOffset++];

        IR_TRACE_PRINT(F("MagiQuest: mark="));
        IR_TRACE_PRINT(tMark * MICROS_PER_TICK);
        IR_TRACE_PRINT(F(" space="));
        IR_TRACE_PRINTLN(tSpace * MICROS_PER_TICK);

        if (matchMark(tSpace + tMark, MAGIQUEST_PERIOD)) {
            if (tSpace > tMark) {
                // It's a 0
                data.llword <<= 1;
#if defined(DEBUG)
                bitstring[(tOffset / 2) - 1] = '0';
#endif
            } else {
                // It's a 1
                data.llword = (data.llword << 1) | 1;
#if defined(DEBUG)
                bitstring[(tOffset / 2) - 1] = '1';
#endif
            }
        } else {
            IR_DEBUG_PRINTLN(F("Mark and space does not match the constant MagiQuest period"));
            return false;
        }
    }

    IR_DEBUG_PRINTLN(bitstring);

    // Success
    decodedIRData.protocol = MAGIQUEST;
    decodedIRData.numberOfBits = tOffset / 2;
    decodedIRData.flags = IRDATA_FLAGS_EXTRA_INFO | IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.extra = data.cmd.magnitude;
    decodedIRData.decodedRawData = data.cmd.wand_id;

    return true;
}
#endif // _IR_MAGIQUEST_HPP
