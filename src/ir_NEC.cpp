/*
 * ir_NEC.cpp
 *
 *  Contains functions for receiving and sending NEC IR Protocol in "raw" and standard format with 16 or 8 bit address and 8 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
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

//#define DEBUG // Activate this for lots of lovely debug output.
#include "IRremote.h"
#include "LongUnion.h"

//==============================================================================
//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC
//==============================================================================
// see: https://www.sbprojects.net/knowledge/ir/nec.php

// LSB first, 1 start bit + 16 bit address + 8 bit command + 8 bit inverted command + 1 stop bit.
//
#define NEC_ADDRESS_BITS        16 // 16 bit address or 8 bit address and 8 bit inverted address
#define NEC_COMMAND_BITS        16 // Command and inverted command

#define NEC_BITS                (NEC_ADDRESS_BITS + NEC_COMMAND_BITS)
#define NEC_UNIT                560

#define NEC_HEADER_MARK         (16 * NEC_UNIT) // 9000
#define NEC_HEADER_SPACE        (8 * NEC_UNIT)  // 4500

#define NEC_BIT_MARK            NEC_UNIT
#define NEC_ONE_SPACE           (3 * NEC_UNIT)  // 1690
#define NEC_ZERO_SPACE          NEC_UNIT

#define NEC_REPEAT_HEADER_SPACE (4 * NEC_UNIT)  // 2250

#define NEC_AVERAGE_DURATION    62000 // NEC_HEADER_MARK + NEC_HEADER_SPACE + 32 * 2,5 * NEC_UNIT + NEC_UNIT // 2.5 because we assume more zeros than ones
#define NEC_REPEAT_DURATION     (NEC_HEADER_MARK  + NEC_HEADER_SPACE + NEC_BIT_MARK)
#define NEC_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.

//+=============================================================================
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendNECRepeat() {
    enableIROut(38);
    noInterrupts();
    mark(NEC_HEADER_MARK);
    space(NEC_REPEAT_HEADER_SPACE);
    mark(NEC_BIT_MARK);
    space(0); // Always end with the LED off
    interrupts();
}

/*
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 * https://www.sbprojects.net/knowledge/ir/nec.php
 * @param aIsRepeat if true, send only one repeat frame without leading and trailing space
 */
void IRsend::sendNEC(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aIsRepeat) {
    if (aIsRepeat) {
        sendNECRepeat();
        return;
    }
    // Set IR carrier frequency
    enableIROut(38);

    noInterrupts();
    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);
    // Address 16 bit LSB first
    if ((aAddress & 0xFF00) == 0) {
        // assume 8 bit address -> send 8 address bits and then 8 inverted address bits LSB first
        aAddress = aAddress & 0xFF;
        aAddress = ((~aAddress) << 8) | aAddress;
    }
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, aAddress, NEC_ADDRESS_BITS, false);

    // send 8 command bits and then 8 inverted command bits LSB first
    uint16_t tCommand = ((~aCommand) << 8) | aCommand;
    // Command 16 bit LSB first
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, tCommand, NEC_COMMAND_BITS, false, true);

    interrupts();

    for (uint8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        if (i == 0) {
            delay((NEC_REPEAT_PERIOD - NEC_AVERAGE_DURATION) / 1000);
        } else {
            delay((NEC_REPEAT_PERIOD - NEC_REPEAT_DURATION) / 1000);
        }
        // send repeat
        sendNECRepeat();
    }
}

//+=============================================================================
// NECs have a repeat only 4 items long
//
#if defined(USE_STANDARD_DECODE)
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 */
bool IRrecv::decodeNEC() {

    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (results.rawlen != ((2 * NEC_BITS) + 4) && (results.rawlen != 4)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header "mark" and "space", this must be done for repeat and data
    if (!MATCH_MARK(results.rawbuf[1], NEC_HEADER_MARK) || !MATCH_SPACE(results.rawbuf[2], NEC_HEADER_SPACE)) {
        // TRACE_PRINT since I saw this too often
        TRACE_PRINT(F("NEC: "));
        TRACE_PRINTLN(F("Header mark or space length is wrong"));
        return false;
    }

    // Check for repeat
    if (results.rawlen == 4) {
        if (MATCH_MARK(results.rawbuf[3], NEC_BIT_MARK)) {
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            return true;
        }
        return false;
    }

    if (!decodePulseDistanceData(NEC_BITS, 3, NEC_BIT_MARK, NEC_ONE_SPACE, NEC_ZERO_SPACE, false)) {
        DBG_PRINT(F("NEC: "));
        DBG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[3 + (2 * NEC_BITS)], NEC_BIT_MARK)) {
        DBG_PRINT(F("NEC: "));
        DBG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    LongUnion tValue;
    tValue.ULong = results.value;
    decodedIRData.command = tValue.UByte.MidHighByte;
    // plausi check for command
    if (tValue.UByte.MidHighByte != (uint8_t) (~tValue.UByte.HighByte)) {
        DBG_PRINT(F("NEC: "));
        DBG_PRINTLN(F("Command and inverted command check failed"));
        return false;
    }
    decodedIRData.protocol = NEC;
    decodedIRData.numberOfBits = NEC_BITS;

    if (tValue.UByte.LowByte == (uint8_t) (~tValue.UByte.MidLowByte)) {
        // standard 8 bit address NEC protocol
        decodedIRData.address = tValue.UByte.LowByte; // first 8 bit
    } else {
        // extended NEC protocol
        decodedIRData.address = tValue.UWord.LowWord; // first 16 bit
    }

    return true;
}
#else

#warning "Old decoder functions decodeNEC() and decodeNEC(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeNEC() instead."
bool IRrecv::decodeNEC() {
    unsigned int offset = 1;  // Index in to results; Skip first space.

// Check header "mark"
    if (!MATCH_MARK(results.rawbuf[offset], NEC_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat
    if ((results.rawlen == 4) && MATCH_SPACE(results.rawbuf[offset], NEC_REPEAT_HEADER_SPACE)
            && MATCH_MARK(results.rawbuf[offset + 1], NEC_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER | IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = NEC;
        return true;
    }

    // Check we have the right amount of data (32). +4 for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != (2 * NEC_BITS) + 4) {
        DBG_PRINT("NEC: ");
        DBG_PRINT("Data length=");
        DBG_PRINT(results.rawlen);
        DBG_PRINTLN(" is not 68");
        return false;
    }

// Check header "space"
    if (!MATCH_SPACE(results.rawbuf[offset], NEC_HEADER_SPACE)) {
        DBG_PRINT("NEC: ");
        DBG_PRINTLN("Header space length is wrong");
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(NEC_BITS, offset, NEC_BIT_MARK, NEC_ONE_SPACE, NEC_ZERO_SPACE)) {
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * NEC_BITS)], NEC_BIT_MARK)) {
        DBG_PRINT("NEC: ");
        DBG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

// Success
    results.bits = NEC_BITS;
    decodedIRData.protocol = NEC;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;

    return true;
}

bool IRrecv::decodeNEC(decode_results *aResults) {
    bool aReturnValue = decodeNEC();
    *aResults = results;
    return aReturnValue;
}
#endif

/*
 * Repeat commands should be sent in a 110 ms raster.
 * https://www.sbprojects.net/knowledge/ir/nec.php
 */
void IRsend::sendNEC(uint32_t data, uint8_t nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut(38);

    if (data == REPEAT || repeat) {
        sendNECRepeat();
        return;
    }

    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);

    // Data + stop bit
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, data, nbits, true, true);

}
