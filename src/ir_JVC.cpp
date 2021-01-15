/*
 * ir_JVC.cpp
 *
 *  Contains functions for receiving and sending JVC IR Protocol in "raw" and standard format with 8 bit address and 8 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2021 Kristian Lauszus, Armin Joachimsmeyer
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

//==============================================================================
//                             JJJJJ  V   V   CCCC
//                               J    V   V  C
//                               J     V V   C
//                             J J     V V   C
//                              J       V     CCCC
//==============================================================================

// https://www.sbprojects.net/knowledge/ir/jvc.php
// http://www.hifi-remote.com/johnsfine/DecodeIR.html#JVC
// IRP: {38k,525}<1,-1|1,-3>(16,-8,(D:8,F:8,1,-45)+)
// LSB first, 1 start bit + 8 bit address + 8 bit command + 1 stop bit.
// The JVC protocol repeats by skipping the header.
#define JVC_ADDRESS_BITS      8 // 8 bit address
#define JVC_COMMAND_BITS      8 // Command

#define JVC_BITS              (JVC_ADDRESS_BITS + JVC_COMMAND_BITS) // The number of bits in the protocol
#define JVC_UNIT              526

#define JVC_HEADER_MARK       (16 * JVC_UNIT) // The length of the Header:Mark
#define JVC_HEADER_SPACE      (8 * JVC_UNIT)  // The lenght of the Header:Space

#define JVC_BIT_MARK          JVC_UNIT        // The length of a Bit:Mark
#define JVC_ONE_SPACE         (3 * JVC_UNIT)  // The length of a Bit:Space for 1's
#define JVC_ZERO_SPACE        JVC_UNIT        // The length of a Bit:Space for 0's

#define JVC_REPEAT_SPACE      (uint16_t)(45 * JVC_UNIT)  // 23625 - Commands are repeated with a distance of 23 ms for as long as the key on the remote control is held down.

//+=============================================================================
// JVC does NOT repeat by sending a separate code (like NEC does).
// The JVC protocol repeats by skipping the header.
//
void IRsend::sendJVC(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(38);

    noInterrupts();
    // Header
    mark(JVC_HEADER_MARK);
    space(JVC_HEADER_SPACE);

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // Address + command
        sendPulseDistanceWidthData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE,
                aAddress | aCommand << JVC_ADDRESS_BITS, JVC_BITS, false, true); // false , true -> LSB first + stop bit

        interrupts();

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(JVC_REPEAT_SPACE / 1000);
        }
    }
}

#if defined(USE_STANDARD_DECODE)
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 */
bool IRrecv::decodeJVC() {

    // Check we have the right amount of data (36 or 34). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (results.rawlen != ((2 * JVC_BITS) + 4) && results.rawlen != ((2 * JVC_BITS) + 2)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }


    if (results.rawlen == ((2 * JVC_BITS) + 2)) {
        /*
         * Check for repeat
         */
        if (results.rawbuf[0] < ((JVC_REPEAT_SPACE + (JVC_REPEAT_SPACE / 2) / MICROS_PER_TICK))
                && MATCH_MARK(results.rawbuf[1], JVC_BIT_MARK) && MATCH_MARK(results.rawbuf[results.rawlen - 1], JVC_BIT_MARK)) {
            /*
             * We have a repeat here, so do not check for start bit
             */
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        }
    } else {

        // Check header "mark" and "space"
        if (!MATCH_MARK(results.rawbuf[1], JVC_HEADER_MARK) || !MATCH_SPACE(results.rawbuf[2], JVC_HEADER_SPACE)) {
//            DBG_PRINT("JVC: ");
//            DBG_PRINTLN("Header mark or space length is wrong");
            return false;
        }
    }

    // false -> LSB first
    if (!decodePulseDistanceData(JVC_BITS, 3, JVC_BIT_MARK, JVC_ONE_SPACE, JVC_ZERO_SPACE, false)) {
        DBG_PRINT(F("JVC: "));
        DBG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Success
    uint8_t tCommand = results.value >> JVC_ADDRESS_BITS;  // upper 8 bits of LSB first value
    uint8_t tAddress = results.value & 0xFF;    // lowest 8 bit of LSB first value

    decodedIRData.command = tCommand;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = JVC_BITS;
    decodedIRData.protocol = JVC;

    return true;
}
#else

#warning "Old decoder functions decodeJVC() and decodeJVC(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeJVC() instead."

//+=============================================================================
bool IRrecv::decodeJVC() {
    unsigned int offset = 1; // Skip first space

    // Check for repeat
    if ((results.rawlen - 1 == 33) && MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)
            && MATCH_MARK(results.rawbuf[results.rawlen - 1], JVC_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER | IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = JVC;
        return true;
    }

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], JVC_HEADER_MARK)) {
        return false;
    }
    offset++;

    // Check we have enough data - +3 for start bit mark and space + stop bit mark
    if (results.rawlen <= (2 * JVC_BITS) + 3) {
        DBG_PRINT("Data length=");
        DBG_PRINT(results.rawlen);
        DBG_PRINTLN(" is too small. >= 36 is required.");

        return false;
    }

    // Initial space
    if (!MATCH_SPACE(results.rawbuf[offset], JVC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(JVC_BITS, offset, JVC_BIT_MARK, JVC_ONE_SPACE, JVC_ZERO_SPACE)) {
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * JVC_BITS)], JVC_BIT_MARK)) {
        DBG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    results.bits = JVC_BITS;
    decodedIRData.protocol = JVC;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;

    return true;
}

bool IRrecv::decodeJVC(decode_results *aResults) {
    bool aReturnValue = decodeJVC();
    *aResults = results;
    return aReturnValue;
}
#endif

//+=============================================================================
// JVC does NOT repeat by sending a separate code (like NEC does).
// The JVC protocol repeats by skipping the header.
//
void IRsend::sendJVC(unsigned long data, int nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut(38);

    // Only send the Header if this is NOT a repeat command
    if (!repeat) {
        mark(JVC_HEADER_MARK);
        space(JVC_HEADER_SPACE);
    }

    // Data + stop bit
    sendPulseDistanceWidthData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE, data, nbits, true,true);

}
