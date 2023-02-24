/*
 * ir_FAST.hpp
 *
 *  Contains functions for receiving and sending FAST IR protocol with 8 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2023 Armin Joachimsmeyer
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
#ifndef _IR_FAST_HPP
#define _IR_FAST_HPP

#include "TinyIR.h"

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
// generated with https://patorjk.com/software/taag/#p=display&f=Alphabet&t=FAST
//==============================================================================
//                             FFFF   AA    SSS  TTTTTT
//                             F     A  A  S       TT
//                             FFF   AAAA   SSS    TT
//                             F     A  A      S   TT
//                             F     A  A  SSSS    TT
//==============================================================================
#include "TinyIR.h"
/*
Protocol=FAST Address=0x0 Command=0x76 Raw-Data=0x8976 16 bits LSB first
 +2100,-1050
 + 550,- 500 + 550,-1550 + 550,-1550 + 550,- 500
 + 550,-1550 + 550,-1550 + 550,-1550 + 550,- 500
 + 550,-1550 + 550,- 500 + 550,- 500 + 550,-1550
 + 550,- 500 + 550,- 500 + 550,- 500 + 550,-1550
 + 550
Sum: 28900
*/
struct PulseDistanceWidthProtocolConstants FASTProtocolConstants = { FAST, FAST_KHZ, FAST_HEADER_MARK, FAST_HEADER_SPACE,
FAST_BIT_MARK, FAST_ONE_SPACE, FAST_BIT_MARK, FAST_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST, (FAST_REPEAT_PERIOD / MICROS_IN_ONE_MILLI),
NULL };

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * The FAST protocol repeats by skipping the header mark and space -> this leads to a poor repeat detection for JVC protocol.
 */
void IRsend::sendFAST(uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(FAST_KHZ); // 38 kHz

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        mark(FAST_HEADER_MARK);
        space(FAST_HEADER_SPACE);

        sendPulseDistanceWidthData(&FASTProtocolConstants, aCommand | (((uint8_t)(~aCommand)) << 8), FAST_BITS);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(FAST_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        }
    }
}

bool IRrecv::decodeFAST() {

//    uint_fast8_t tRawlen = decodedIRData.rawDataPtr->rawlen; // Using a local variable does not improve code size

    // Check we have the right amount of data (36). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawDataPtr->rawlen != ((2 * FAST_BITS) + 4)) {
        IR_DEBUG_PRINT(F("FAST: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 36"));
        return false;
    }

    if (!checkHeader(&FASTProtocolConstants)) {
        return false;
    }

    if (!decodePulseDistanceWidthData(&FASTProtocolConstants, FAST_BITS)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("FAST: "));
        Serial.println(F("Decode failed"));
#endif
        return false;
    }

    WordUnion tValue;
    tValue.UWord = decodedIRData.decodedRawData;

    if (tValue.UByte.LowByte != (uint8_t)~(tValue.UByte.HighByte)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("FAST: "));
        Serial.print(F("8 bit parity is not correct. Expected=0x"));
        Serial.print((uint8_t)~(tValue.UByte.LowByte), HEX);
        Serial.print(F(" received=0x"));
        Serial.print(tValue.UByte.HighByte, HEX);
        Serial.print(F(" data=0x"));
        Serial.println(tValue.UWord, HEX);
#endif
        decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED;
    }

    checkForRepeatSpaceTicksAndSetFlag(FAST_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    decodedIRData.command = tValue.UByte.LowByte;
    decodedIRData.address = 0; // No address for this protocol
    decodedIRData.numberOfBits = FAST_BITS;
    decodedIRData.protocol = FAST;

    return true;
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_FAST_HPP
