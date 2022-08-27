/*
 * ir_JVC.hpp
 *
 *  Contains functions for receiving and sending JVC IR Protocol in "raw" and standard format with 8 bit address and 8 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
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
#ifndef _IR_JVC_HPP
#define _IR_JVC_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
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
// The JVC protocol repeats by skipping the header mark and space -> this leads to a poor repeat detection for JVC protocol.
#define JVC_ADDRESS_BITS      8 // 8 bit address
#define JVC_COMMAND_BITS      8 // Command

#define JVC_BITS              (JVC_ADDRESS_BITS + JVC_COMMAND_BITS) // 16 - The number of bits in the protocol
#define JVC_UNIT              526 // 20 periods of 38 kHz (526.315789)

#define JVC_HEADER_MARK       (16 * JVC_UNIT) // 8400
#define JVC_HEADER_SPACE      (8 * JVC_UNIT)  // 4200

#define JVC_BIT_MARK          JVC_UNIT        // The length of a Bit:Mark
#define JVC_ONE_SPACE         (3 * JVC_UNIT)  // 1578 - The length of a Bit:Space for 1's
#define JVC_ZERO_SPACE        JVC_UNIT        // The length of a Bit:Space for 0's

#define JVC_REPEAT_SPACE      (uint16_t)(45 * JVC_UNIT)  // 23625 - Commands are repeated with a distance of 23 ms for as long as the key on the remote control is held down.
#define JVC_REPEAT_PERIOD     65000 // assume around 40 ms for a JVC frame

//+=============================================================================
// JVC does NOT repeat by sending a separate code (like NEC does).
// The JVC protocol repeats by skipping the header.
//

void IRsend::sendJVC(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(JVC_KHZ); // 38 kHz

    // The JVC protocol repeats by skipping the header.
    mark(JVC_HEADER_MARK);
    space(JVC_HEADER_SPACE);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // Address + command
        sendPulseDistanceWidthData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE,
                aAddress | (aCommand << JVC_ADDRESS_BITS), JVC_BITS, PROTOCOL_IS_LSB_FIRST, SEND_STOP_BIT);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(JVC_REPEAT_SPACE / MICROS_IN_ONE_MILLI);
        }
    }
    IrReceiver.restartAfterSend();
}

/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 */
bool IRrecv::decodeJVC() {

    // Check we have the right amount of data (36 or 34). The +4 is for initial gap, start bit mark and space + stop bit mark. +2 is for repeats
    if (decodedIRData.rawDataPtr->rawlen != ((2 * JVC_BITS) + 4) && decodedIRData.rawDataPtr->rawlen != ((2 * JVC_BITS) + 2)) {
        IR_DEBUG_PRINT(F("JVC: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 34 or 36"));
        return false;
    }

    if (decodedIRData.rawDataPtr->rawlen == ((2 * JVC_BITS) + 2)) {
        /*
         * Check for repeat
         * Check leading space and first and last mark length
         */
        if (decodedIRData.rawDataPtr->rawbuf[0] < ((JVC_REPEAT_SPACE + (JVC_REPEAT_SPACE / 4) / MICROS_PER_TICK))
                && matchMark(decodedIRData.rawDataPtr->rawbuf[1], JVC_BIT_MARK)
                && matchMark(decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen - 1], JVC_BIT_MARK)) {
            /*
             * We have a repeat here, so do not check for start bit
             */
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            decodedIRData.protocol = JVC;
        }
    } else {

        // Check header "mark" and "space"
        if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], JVC_HEADER_MARK)
                || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], JVC_HEADER_SPACE)) {
            IR_DEBUG_PRINT(F("JVC: "));
            IR_DEBUG_PRINTLN(F("Header mark or space length is wrong"));
            return false;
        }

        if (!decodePulseDistanceData(JVC_BITS, 3, JVC_BIT_MARK, JVC_ONE_SPACE, JVC_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
            IR_DEBUG_PRINT(F("JVC: "));
            IR_DEBUG_PRINTLN(F("Decode failed"));
            return false;
        }

        // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
        uint8_t tCommand = decodedIRData.decodedRawData >> JVC_ADDRESS_BITS;  // upper 8 bits of LSB first value
        uint8_t tAddress = decodedIRData.decodedRawData & 0xFF;    // lowest 8 bit of LSB first value

        decodedIRData.command = tCommand;
        decodedIRData.address = tAddress;
        decodedIRData.numberOfBits = JVC_BITS;
        decodedIRData.protocol = JVC;
    }

    return true;
}

bool IRrecv::decodeJVCMSB(decode_results *aResults) {
    unsigned int offset = 1; // Skip first space

    // Check for repeat
    if ((aResults->rawlen - 1 == 33) && matchMark(aResults->rawbuf[offset], JVC_BIT_MARK)
            && matchMark(aResults->rawbuf[aResults->rawlen - 1], JVC_BIT_MARK)) {
        aResults->bits = 0;
        aResults->value = 0xFFFFFFFF;
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = JVC;
        return true;
    }

    // Initial mark
    if (!matchMark(aResults->rawbuf[offset], JVC_HEADER_MARK)) {
        return false;
    }
    offset++;

    // Check we have enough data - +3 for start bit mark and space + stop bit mark
    if (aResults->rawlen <= (2 * JVC_BITS) + 3) {
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(aResults->rawlen);
        IR_DEBUG_PRINTLN(F(" is too small. >= 36 is required."));

        return false;
    }

    // Initial space
    if (!matchSpace(aResults->rawbuf[offset], JVC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(JVC_BITS, offset, JVC_BIT_MARK, JVC_ONE_SPACE, JVC_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }

    // Stop bit
    if (!matchMark(aResults->rawbuf[offset + (2 * JVC_BITS)], JVC_BIT_MARK)) {
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    aResults->value = decodedIRData.decodedRawData;
    aResults->bits = JVC_BITS;
    aResults->decode_type = JVC;
    decodedIRData.protocol = JVC;

    return true;
}

/**
 * With Send sendJVCMSB() you can send your old 32 bit codes.
 * To convert one into the other, you must reverse the byte positions and then reverse all bit positions of each byte.
 * Or write it as one binary string and reverse/mirror it.
 * Example:
 * 0xCB340102 byte reverse -> 02 01 34 CB bit reverse-> 40 80 2C D3.
 * 0xCB340102 is binary 11001011001101000000000100000010.
 * 0x40802CD3 is binary 01000000100000000010110011010011.
 * If you read the first binary sequence backwards (right to left), you get the second sequence.
 */
void IRsend::sendJVCMSB(unsigned long data, int nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut(JVC_KHZ);

    // Only send the Header if this is NOT a repeat command
    if (!repeat) {
        mark(JVC_HEADER_MARK);
        space(JVC_HEADER_SPACE);
    }

    // Old version with MSB first Data
    sendPulseDistanceWidthData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);
    IrReceiver.restartAfterSend();
}

/** @}*/
#endif // _IR_JVC_HPP
