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
 * Copyright (c) 2017-2026 Kristian Lauszus, Armin Joachimsmeyer
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

#include "LocalDebugLevelStart.h"

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
/*
 +8400,-4150
 + 550,-1550 + 550,- 500 + 550,- 500 + 550,- 500
 + 550,-1500 + 600,-1500 + 600,-1500 + 550,-1550
 + 550,- 500 + 550,-1550 + 550,-1550 + 550,- 500
 + 500,-1600 + 550,-1550 + 550,-1500 + 600,- 500
 + 550
 Sum: 40350
 */
// https://www.sbprojects.net/knowledge/ir/jvc.php
// http://www.hifi-remote.com/johnsfine/DecodeIR.html#JVC
// IRP: {38k,525}<1,-1|1,-3>(16,-8,(D:8,F:8,1,-45)+)
// LSB first, 1 start bit + 8 bit address + 8 bit command + 1 stop bit.
// The JVC protocol repeats by skipping the header mark and space -> this leads to a poor repeat detection for JVC protocol.
// Some JVC devices require to send 3 repeats.
#define JVC_ADDRESS_BITS      8 // 8 bit address
#define JVC_COMMAND_BITS      8 // Command

#define JVC_BITS              (JVC_ADDRESS_BITS + JVC_COMMAND_BITS) // 16 - The number of bits in the protocol
#define JVC_UNIT              526 // 20 periods of 38 kHz (526.315789)

#define JVC_HEADER_MARK       (16 * JVC_UNIT) // 8400
#define JVC_HEADER_SPACE      (8 * JVC_UNIT)  // 4200

#define JVC_BIT_MARK          JVC_UNIT        // The length of a Bit:Mark
#define JVC_ONE_SPACE         (3 * JVC_UNIT)  // 1578 - The length of a Bit:Space for 1's
#define JVC_ZERO_SPACE        JVC_UNIT        // The length of a Bit:Space for 0's

#define JVC_REPEAT_DISTANCE   (uint16_t)(45 * JVC_UNIT)  // 23670 - Commands are repeated with a distance of 23 ms for as long as the key on the remote control is held down.
#define JVC_REPEAT_PERIOD     65000 // assume around 40 ms for a JVC frame. JVC IR Remotes: RM-SA911U, RM-SX463U have 45 ms period

struct PulseDistanceWidthProtocolConstants const JVCProtocolConstants PROGMEM = {JVC, JVC_KHZ, JVC_HEADER_MARK, JVC_HEADER_SPACE, JVC_BIT_MARK,
    JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE, (JVC_REPEAT_PERIOD / MICROS_IN_ONE_MILLISECOND), nullptr};

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * The JVC protocol repeats by skipping the header mark and space -> this leads to a poor repeat detection for JVC protocol.
 */
void IRsend::sendJVC(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut (JVC_KHZ); // 38 kHz

    if (aNumberOfRepeats < 0) {
        // The JVC protocol repeats by skipping the header.
        aNumberOfRepeats = 0;
    } else {
        mark(JVC_HEADER_MARK);
        space(JVC_HEADER_SPACE);
    }

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // Address + command
        sendPulseDistanceWidthData_P(&JVCProtocolConstants, aAddress | (aCommand << JVC_ADDRESS_BITS), JVC_BITS);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(JVC_REPEAT_DISTANCE / MICROS_IN_ONE_MILLISECOND);
        }
    }
}

bool IRrecv::decodeJVC() {

//    uint_fast8_t tRawlen = decodedIRData.rawlen; // Using a local variable does not improve code size

    // Check we have the right amount of data (36 or 34). The +4 is for initial gap, start bit mark and space + stop bit mark.
    // +4 is for first frame, +2 is for repeats
    if (!(decodedIRData.rawlen == ((2 * JVC_BITS) + 2) || decodedIRData.rawlen == ((2 * JVC_BITS) + 4))) {
        DEBUG_PRINT(F("JVC: Data length="));
        DEBUG_PRINT(decodedIRData.rawlen);
        DEBUG_PRINTLN(F(" is not 34 or 36"));
        return false;
    }

    if (decodedIRData.rawlen == ((2 * JVC_BITS) + 2)) {
        /*
         * 34 -> check for repeat distance and check leading space and first and last mark length
         */
        if (lastDecodedProtocol == JVC
                && decodedIRData.initialGapTicks < ((JVC_REPEAT_DISTANCE + (JVC_REPEAT_DISTANCE / 4)) / MICROS_PER_TICK)
                && matchMark(irparams.rawbuf[1], JVC_BIT_MARK)
                && matchMark(irparams.rawbuf[decodedIRData.rawlen - 1], JVC_BIT_MARK)) {
            /*
             * We have a repeat here, so do not check for start bit
             */
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            decodedIRData.protocol = JVC;
        } else {
            return false;
        }
    } else {

        // 36 -> check for new start frame
        if (!checkHeader_P(&JVCProtocolConstants)) {
            return false;
        }

        decodePulseDistanceWidthData_P(&JVCProtocolConstants, JVC_BITS);

//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
        decodedIRData.command = decodedIRData.decodedRawData >> JVC_ADDRESS_BITS;  // upper 8 bits of LSB first value
        decodedIRData.address = decodedIRData.decodedRawData & 0xFF;    // lowest 8 bit of LSB first value
        decodedIRData.numberOfBits = JVC_BITS;
        decodedIRData.protocol = JVC;
    }

    return true;
}

/** @}*/
#include "LocalDebugLevelEnd.h"

#endif // _IR_JVC_HPP
