/*
 * ir_Denon.cpp
 *
 *  Contains functions for receiving and sending Denon/Sharp IR Protocol
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2021 Armin Joachimsmeyer
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
//                    DDDD   EEEEE  N   N   OOO   N   N
//                     D  D  E      NN  N  O   O  NN  N
//                     D  D  EEE    N N N  O   O  N N N
//                     D  D  E      N  NN  O   O  N  NN
//                    DDDD   EEEEE  N   N   OOO   N   N
//==============================================================================
//                       SSSS  H   H   AAA   RRRR   PPPP
//                      S      H   H  A   A  R   R  P   P
//                       SSS   HHHHH  AAAAA  RRRR   PPPP
//                          S  H   H  A   A  R  R   P
//                      SSSS   H   H  A   A  R   R  P
//==============================================================================
// Denon publish all their IR codes:
//  https://www.google.co.uk/search?q=DENON+MASTER+IR+Hex+Command+Sheet
//  -> http://assets.denon.com/documentmaster/us/denon%20master%20ir%20hex.xls

// Having looked at the official Denon Pronto sheet and reverse engineered
// the timing values from it, it is obvious that Denon have a range of
// different timings and protocols ...the values here work for my AVR-3801 Amp!

// MSB first, no start bit, 5 address + 8 command + 2 frame + 1 stop bit - each frame 2 times
//
#define DENON_ADDRESS_BITS      5
#define DENON_COMMAND_BITS      8
#define DENON_FRAME_BITS        2 // 00/10 for 1. frame Denon/Sharp, inverted for autorepeat frame

#define DENON_BITS              (DENON_ADDRESS_BITS + DENON_COMMAND_BITS + DENON_FRAME_BITS) // The number of bits in the command
#define DENON_UNIT              260

#define DENON_BIT_MARK          DENON_UNIT  // The length of a Bit:Mark
#define DENON_ONE_SPACE         (7 * DENON_UNIT) // 1820 // The length of a Bit:Space for 1's
#define DENON_ZERO_SPACE        (3 * DENON_UNIT) // 780 // The length of a Bit:Space for 0's

#define DENON_AUTO_REPEAT_SPACE 45000 // Every frame is auto repeated with a space period of 45 ms and the command inverted.
#define DENON_REPEAT_PERIOD     110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.

// for old decoder
#define DENON_HEADER_MARK       DENON_UNIT // The length of the Header:Mark
#define DENON_HEADER_SPACE      (3 * DENON_UNIT) // 780 // The lenght of the Header:Space

//+=============================================================================
void IRsend::sendSharp(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    sendDenon(aAddress, aCommand, aNumberOfRepeats, true);
}

//+=============================================================================
void IRsend::sendDenon(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aSendSharp) {
    // Set IR carrier frequency
    enableIROut(38);

    // Shift command and add frame marker
    uint16_t tCommand = aCommand << DENON_FRAME_BITS; // the lowest bits are 00 for Denon and 10 for Sharp
    if (aSendSharp) {
        tCommand |= 0x02;
    }
    uint16_t tData = tCommand | ((uint16_t) aAddress << (DENON_COMMAND_BITS + DENON_FRAME_BITS));
    uint16_t tInvertedData = ((~tCommand) & 0x3FF) | (uint16_t) aAddress << (DENON_COMMAND_BITS + DENON_FRAME_BITS);

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        noInterrupts();

        // Data
        sendPulseDistanceWidthData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE, tData, DENON_BITS, true,
                true);

        // Inverted autorepeat frame
        interrupts();
        delay(DENON_AUTO_REPEAT_SPACE / 1000);
        noInterrupts();
        sendPulseDistanceWidthData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE, tInvertedData, DENON_BITS,
                true, true);

        interrupts();

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command with a fixed space gap
            delay( DENON_AUTO_REPEAT_SPACE / 1000);
        }
    }
}

//+=============================================================================
bool IRrecv::decodeSharp() {
    return decodeDenon();
}

//+=============================================================================
#if defined(USE_STANDARD_DECODE)
bool IRrecv::decodeDenon() {

    // we have no start bit, so check for the exact amount of data bits
    // Check we have the right amount of data (32). The + 2 is for initial gap + stop bit mark
    if (irparams.rawlen != (2 * DENON_BITS) + 2) {
        return false;
    }

    // Read the bits in
    if (!decodePulseDistanceData(DENON_BITS, 1, DENON_BIT_MARK, DENON_ONE_SPACE, DENON_ZERO_SPACE)) {
        DBG_PRINT("Denon: ");
        DBG_PRINTLN("Decode failed");
        return false;
    }

    // Check for stop mark
    if (!MATCH_MARK(results.rawbuf[(2 * DENON_BITS) + 1], DENON_HEADER_MARK)) {
        DBG_PRINT("Denon: ");
        DBG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    uint8_t tFrameBits = results.value & 0x03;
    decodedIRData.command = results.value >> DENON_FRAME_BITS;
    decodedIRData.address = decodedIRData.command >> DENON_COMMAND_BITS;
    uint8_t tCommand = decodedIRData.command & 0xFF;
    decodedIRData.command = tCommand;

    // check for autorepeated inverted command
    if (results.rawbuf[0] < ((DENON_AUTO_REPEAT_SPACE + (DENON_AUTO_REPEAT_SPACE / 4)) / MICROS_PER_TICK)) {
        repeatCount++;
        if (tFrameBits == 0x3 || tFrameBits == 0x1) {
            // We are in the auto repeated frame with the inverted command
            decodedIRData.flags = IRDATA_FLAGS_IS_AUTO_REPEAT;
            // check parity
            uint8_t tLastCommand = lastDecodedCommand;
            if (tLastCommand != (uint8_t) (~tCommand)) {
                decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
            }
            // always take non inverted command
            decodedIRData.command = tLastCommand;
        }
        if (repeatCount > 1) {
            decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
        }
    } else {
        repeatCount = 0;
    }

    decodedIRData.numberOfBits = DENON_BITS;
    if (tFrameBits == 1 || tFrameBits == 2) {
        decodedIRData.protocol = SHARP;
    } else {
        decodedIRData.protocol = DENON;
    }
    return true;
}
#else

#warning "Old decoder functions decodeDenon() and decodeDenon(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeDenon() instead."

bool IRrecv::decodeDenon() {
    unsigned int offset = 1;  // Skip the gap reading

    // Check we have the right amount of data
    if (irparams.rawlen != 1 + 2 + (2 * DENON_BITS) + 1) {
        return false;
    }

    // Check initial Mark+Space match
    if (!MATCH_MARK(results.rawbuf[offset], DENON_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (!MATCH_SPACE(results.rawbuf[offset], DENON_HEADER_SPACE)) {
        return false;
    }
    offset++;

    // Read the bits in
    if (!decodePulseDistanceData(DENON_BITS, offset, DENON_BIT_MARK, DENON_ONE_SPACE, DENON_ZERO_SPACE)) {
        return false;
    }

    // Success
    results.bits = DENON_BITS;
    decodedIRData.protocol = DENON;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;
    return true;
}

bool IRrecv::decodeDenon(decode_results *aResults) {
    bool aReturnValue = decodeDenon();
    *aResults = results;
    return aReturnValue;
}
#endif

void IRsend::sendDenon(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(DENON_HEADER_MARK);
    space(DENON_HEADER_SPACE);

    // Data
    sendPulseDistanceWidthData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE, data, nbits, true, true);

}

void IRsend::sendSharp(unsigned int aAddress, unsigned int aCommand) {
    sendDenon(aAddress, aCommand, true, 0);
}
