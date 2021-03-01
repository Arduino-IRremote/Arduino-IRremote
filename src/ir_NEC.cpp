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
#include "IRremoteInt.h"
#include "LongUnion.h"

//==============================================================================
//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC
//==============================================================================
// see: https://www.sbprojects.net/knowledge/ir/nec.php

// for Apple see https://en.wikipedia.org/wiki/Apple_Remote

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
#define NEC_REPEAT_DURATION     (NEC_HEADER_MARK  + NEC_REPEAT_HEADER_SPACE + NEC_BIT_MARK) // 12 ms
#define NEC_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
#define NEC_REPEAT_SPACE        (NEC_REPEAT_PERIOD - NEC_AVERAGE_DURATION) // 48 ms

#define APPLE_ADDRESS           0x87EE
//+=============================================================================
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendNECRepeat() {
    enableIROut(38);
    mark(NEC_HEADER_MARK);
    space(NEC_REPEAT_HEADER_SPACE);
    mark(NEC_BIT_MARK);
    ledOff(); // Always end with the LED off
}

/*
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 * https://www.sbprojects.net/knowledge/ir/nec.php
 * @param aIsRepeat if true, send only one repeat frame without leading and trailing space
 */
void IRsend::sendNEC(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat) {

    LongUnion tRawData;

    // Address 16 bit LSB first
    if ((aAddress & 0xFF00) == 0) {
        // assume 8 bit address -> send 8 address bits and then 8 inverted address bits LSB first
        tRawData.UByte.LowByte = aAddress;
        tRawData.UByte.MidLowByte = ~tRawData.UByte.LowByte;
    } else {
        tRawData.UWord.LowWord = aAddress;
    }

    // send 8 command bits and then 8 inverted command bits LSB first
    tRawData.UByte.MidHighByte = aCommand;
    tRawData.UByte.HighByte = ~aCommand;

    sendNECRaw(tRawData.ULong, aNumberOfRepeats, aIsRepeat);
}

/*
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 * https://en.wikipedia.org/wiki/Apple_Remote
 * https://gist.github.com/darconeous/4437f79a34e3b6441628
 * @param aAddress is the DeviceId*
 * @param aIsRepeat if true, send only one repeat frame without leading and trailing space
 */
void IRsend::sendApple(uint8_t aDeviceId, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat) {

    LongUnion tRawData;

    // Address 16 bit LSB first
    tRawData.UWord.LowWord = APPLE_ADDRESS;

    // send Apple code and then 8 command bits LSB first
    tRawData.UByte.MidHighByte = aCommand;
    tRawData.UByte.HighByte = aDeviceId; // e.g. 0xD7

    sendNECRaw(tRawData.ULong, aNumberOfRepeats, aIsRepeat);
}

void IRsend::sendNECRaw(uint32_t aRawData, uint_fast8_t aNumberOfRepeats, bool aIsRepeat) {
    if (aIsRepeat) {
        sendNECRepeat();
        return;
    }
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);

    // LSB first + stop bit
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, aRawData, NEC_BITS, PROTOCOL_IS_LSB_FIRST,
    SEND_STOP_BIT);

    for (uint_fast8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        if (i == 0) {
            delay(NEC_REPEAT_SPACE / 1000);
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
#if !defined(USE_OLD_DECODE)
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 *
 * Decodes also Apple
 */
bool IRrecv::decodeNEC() {

    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawDataPtr->rawlen != ((2 * NEC_BITS) + 4) && (decodedIRData.rawDataPtr->rawlen != 4)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header "mark" this must be done for repeat and data
    if (!MATCH_MARK(decodedIRData.rawDataPtr->rawbuf[1], NEC_HEADER_MARK)) {
        return false;
    }

    // Check for repeat - here we have another header space length
    if (decodedIRData.rawDataPtr->rawlen == 4) {
        if (MATCH_SPACE(decodedIRData.rawDataPtr->rawbuf[2], NEC_REPEAT_HEADER_SPACE)
                && MATCH_MARK(decodedIRData.rawDataPtr->rawbuf[3], NEC_BIT_MARK)) {
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
//            decodedIRData.protocol = NEC; do not set it, because it can also be an LG repeat
            return true;
        }
        return false;
    }

    // Check command header space
    if (!MATCH_SPACE(decodedIRData.rawDataPtr->rawbuf[2], NEC_HEADER_SPACE)) {
        DBG_PRINT(F("NEC: "));
        DBG_PRINTLN(F("Header space length is wrong"));
        return false;
    }

    if (!decodePulseDistanceData(NEC_BITS, 3, NEC_BIT_MARK, NEC_ONE_SPACE, NEC_ZERO_SPACE, false)) {
        DBG_PRINT(F("NEC: "));
        DBG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(decodedIRData.rawDataPtr->rawbuf[3 + (2 * NEC_BITS)], NEC_BIT_MARK)) {
        DBG_PRINT(F("NEC: "));
        DBG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;
    decodedIRData.command = tValue.UByte.MidHighByte;
    // Address
    if (tValue.UWord.LowWord == APPLE_ADDRESS) {
        /*
         * Apple
         */
        decodedIRData.protocol = APPLE;
        decodedIRData.command = tValue.UByte.MidHighByte;
        decodedIRData.address = tValue.UByte.HighByte;

    } else {
        /*
         * NEC
         */
        decodedIRData.protocol = NEC;
        if (tValue.UByte.LowByte == (uint8_t) (~tValue.UByte.MidLowByte)) {
            // standard 8 bit address NEC protocol
            decodedIRData.address = tValue.UByte.LowByte; // first 8 bit
        } else {
            // extended NEC protocol
            decodedIRData.address = tValue.UWord.LowWord; // first 16 bit
        }
        // plausi check for command
        if (tValue.UByte.MidHighByte != (uint8_t) (~tValue.UByte.HighByte)) {
            DBG_PRINT(F("NEC: "));
            DBG_PRINT(F("Command=0x"));
            DBG_PRINT(tValue.UByte.MidHighByte, HEX);
            DBG_PRINT(F(" is not inverted value of 0x"));
            DBG_PRINTLN(tValue.UByte.HighByte, HEX);
            decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_LSB_FIRST;
        }
    }
    decodedIRData.numberOfBits = NEC_BITS;

    return true;
}
#else

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
        results.value = 0xFFFFFFFF;
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
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

    if (!decodePulseDistanceData(NEC_BITS, offset, NEC_BIT_MARK, NEC_ONE_SPACE, NEC_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
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

    return true;
}
#endif

/*
 * With Send sendNECMSB() you can send your old 32 bit codes.
 * To convert one into the other, you must reverse the byte positions and then reverse all positions of each byte.
 * Example:
 * 0xCB340102 byte reverse -> 0x020134CB bit reverse-> 40802CD3
 */
void IRsend::sendNECMSB(uint32_t data, uint8_t nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut(38);

    if (data == 0xFFFFFFFF || repeat) {
        sendNECRepeat();
        return;
    }

    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);

    // Old version with MSB first Data + stop bit
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);

}
