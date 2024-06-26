/*
 * ir_Denon.cpp
 *
 *  Contains functions for receiving and sending Denon/Sharp IR Protocol
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2023 Armin Joachimsmeyer
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
#ifndef _IR_DENON_HPP
#define _IR_DENON_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
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
/*
 Protocol=Denon Address=0x11 Command=0x76 Raw-Data=0xED1 15 bits LSB first
 + 200,-1800 + 300,- 750 + 300,- 800 + 200,- 800
 + 250,-1800 + 250,- 800 + 250,-1800 + 300,-1750
 + 300,- 750 + 300,-1800 + 250,-1800 + 250,-1850
 + 250,- 750 + 300,- 800 + 250,- 800 + 250
 Sum: 23050

 Denon/Sharp variant
 Protocol=Denon Address=0x11 Command=0x76 Raw-Data=0x4ED1 15 bits LSB first
 + 200,-1800 + 300,- 750 + 250,- 800 + 250,- 750
 + 300,-1800 + 250,- 800 + 250,-1800 + 300,-1750
 + 300,- 750 + 300,-1800 + 250,-1800 + 250,-1800
 + 300,- 750 + 300,- 750 + 300,-1800 + 250
 Sum: 23050
 */
/*
 * https://www.mikrocontroller.net/articles/IRMP_-_english#DENON
 * Denon published all their IR codes:
 * http://assets.denon.com/documentmaster/us/denon%20master%20ir%20hex.xls
 * Example:
 * 0000 006D 0000 0020 000A 001E 000A 0046 000A 001E 000A 001E 000A 001E // 5 address bits
 *                     000A 001E 000A 001E 000A 0046 000A 0046 000A 0046 000A 001E 000A 0046 000A 0046 // 8 command bits
 *                     000A 001E 000A 001E 000A 0679 // 2 frame bits 0,0 + stop bit + space for AutoRepeat
 *                     000A 001E 000A 0046 000A 001E 000A 001E 000A 001E // 5 address bits
 *                     000A 0046 000A 0046 000A 001E 000A 001E 000A 001E 000A 0046 000A 001E 000A 001E // 8 inverted command bits
 *                     000A 0046 000A 0046 000A 0679 // 2 frame bits 1,1 + stop bit + space for Repeat
 * From analyzing the codes for Tuner preset 1 to 8 in tab Main Zone ID#1 it is obvious, that the protocol is LSB first at least for command.
 * All Denon codes with 32 as 3. value use the Kaseyikyo Denon variant.
 */
// LSB first, no start bit, 5 address + 8 command + 2 frame (0,0) + 1 stop bit - each frame 2 times
// Every frame is auto repeated with a space period of 45 ms and the command and frame inverted to (1,1) or (0,1) for SHARP.
//
#define DENON_ADDRESS_BITS      5
#define DENON_COMMAND_BITS      8
#define DENON_FRAME_BITS        2 // 00/10 for 1. frame Denon/Sharp, inverted for autorepeat frame

#define DENON_BITS              (DENON_ADDRESS_BITS + DENON_COMMAND_BITS + DENON_FRAME_BITS) // 15 - The number of bits in the command
#define DENON_UNIT              260

#define DENON_BIT_MARK          DENON_UNIT  // The length of a Bit:Mark
#define DENON_ONE_SPACE         (7 * DENON_UNIT) // 1820 // The length of a Bit:Space for 1's
#define DENON_ZERO_SPACE        (3 * DENON_UNIT) // 780 // The length of a Bit:Space for 0's

#define DENON_AUTO_REPEAT_DISTANCE  45000 // Every frame is auto repeated with a space period of 45 ms and the command and frame inverted.
#define DENON_REPEAT_PERIOD        110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.

// for old decoder
#define DENON_HEADER_MARK       DENON_UNIT // The length of the Header:Mark
#define DENON_HEADER_SPACE      (3 * DENON_UNIT) // 780 // The length of the Header:Space

struct PulseDistanceWidthProtocolConstants DenonProtocolConstants = { DENON, DENON_KHZ, DENON_HEADER_MARK, DENON_HEADER_SPACE,
DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST,
        (DENON_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), NULL };

/************************************
 * Start of send and decode functions
 ************************************/

void IRsend::sendSharp(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendDenon(aAddress, aCommand, aNumberOfRepeats, true);
}

void IRsend::sendDenon(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aSendSharp) {
    // Set IR carrier frequency
    enableIROut (DENON_KHZ); // 38 kHz

    // Add frame marker for sharp
    uint16_t tCommand = aCommand;
    if (aSendSharp) {
        tCommand |= 0x0200; // the 2 upper bits are 00 for Denon and 10 for Sharp
    }
    uint16_t tData = aAddress | ((uint16_t) tCommand << DENON_ADDRESS_BITS);
    uint16_t tInvertedData = (tData ^ 0x7FE0); // Command and frame (upper 10 bits) are inverted

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // Data
        sendPulseDistanceWidthData(&DenonProtocolConstants, tData, DENON_BITS);

        // Inverted autorepeat frame
        delay(DENON_AUTO_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        sendPulseDistanceWidthData(&DenonProtocolConstants, tInvertedData, DENON_BITS);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command with a fixed space gap
            delay( DENON_AUTO_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        }
    }
}

bool IRrecv::decodeSharp() {
    return decodeDenon();
}

bool IRrecv::decodeDenon() {

    // we have no start bit, so check for the exact amount of data bits
    // Check we have the right amount of data (32). The + 2 is for initial gap + stop bit mark
    if (decodedIRData.rawlen != (2 * DENON_BITS) + 2) {
        IR_DEBUG_PRINT(F("Denon: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 32"));
        return false;
    }

    // Try to decode as Denon protocol
    if (!decodePulseDistanceWidthData(&DenonProtocolConstants, DENON_BITS, 1)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Denon: "));
        Serial.println(F("Decode failed"));
#endif
        return false;
    }

    // Check for stop mark
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[(2 * DENON_BITS) + 1], DENON_HEADER_MARK)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Denon: "));
        Serial.println(F("Stop bit mark length is wrong"));
#endif
        return false;
    }

    // Success
    decodedIRData.address = decodedIRData.decodedRawData & 0x1F;
    decodedIRData.command = decodedIRData.decodedRawData >> DENON_ADDRESS_BITS;
    uint8_t tFrameBits = (decodedIRData.command >> 8) & 0x03;
    decodedIRData.command &= 0xFF;

    // Check for (auto) repeat
    if (decodedIRData.initialGapTicks < ((DENON_AUTO_REPEAT_DISTANCE + (DENON_AUTO_REPEAT_DISTANCE / 4)) / MICROS_PER_TICK)) {
        repeatCount++;
        if (repeatCount > 1) { // skip first auto repeat
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        }

        if (tFrameBits & 0x01) {
            /*
             * Here we are in the auto repeated frame with the inverted command
             */
#if defined(LOCAL_DEBUG)
                Serial.print(F("Denon: "));
                Serial.println(F("Autorepeat received="));
#endif
            decodedIRData.flags |= IRDATA_FLAGS_IS_AUTO_REPEAT;
            // Check parity of consecutive received commands. There is no parity in one data set.
            if ((uint8_t) lastDecodedCommand != (uint8_t)(~decodedIRData.command)) {
                decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
#if defined(LOCAL_DEBUG)
                Serial.print(F("Denon: "));
                Serial.print(F("Parity check for repeat failed. Last command="));
                Serial.print(lastDecodedCommand, HEX);
                Serial.print(F(" current="));
                Serial.println(~decodedIRData.command, HEX);
#endif
            }
            // always take non inverted command
            decodedIRData.command = lastDecodedCommand;
        }

        // repeated command here
        if (tFrameBits == 1 || tFrameBits == 2) {
            decodedIRData.protocol = SHARP;
        } else {
            decodedIRData.protocol = DENON;
        }
    } else {
        repeatCount = 0;
        // first command here
        if (tFrameBits == 2) {
            decodedIRData.protocol = SHARP;
        } else {
            decodedIRData.protocol = DENON;
        }
    }

    decodedIRData.numberOfBits = DENON_BITS;

    return true;
}

/*********************************************************************************
 * Old deprecated functions, kept for backward compatibility to old 2.0 tutorials
 *********************************************************************************/
/*
 * Only for backwards compatibility
 */
void IRsend::sendDenonRaw(uint16_t aRawData, int_fast8_t aNumberOfRepeats) {
    sendDenon(aRawData >> (DENON_COMMAND_BITS + DENON_FRAME_BITS), (aRawData >> DENON_FRAME_BITS) & 0xFF, aNumberOfRepeats);
}

/*
 * Old function with parameter data
 */
void IRsend::sendDenon(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut (DENON_KHZ);
#if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__))
//    Serial.println(
//            "The function sendDenon(data, nbits) is deprecated and may not work as expected! Use sendDenonRaw(data, NumberOfRepeats) or better sendDenon(Address, Command, NumberOfRepeats).");
#endif

    // Header
    mark(DENON_HEADER_MARK);
    space(DENON_HEADER_SPACE);

    // Data
    sendPulseDistanceWidthData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE, data, nbits,
            PROTOCOL_IS_MSB_FIRST);
}

/*
 * Old function without parameter aNumberOfRepeats
 */
void IRsend::sendSharp(uint16_t aAddress, uint16_t aCommand) {
    sendDenon(aAddress, aCommand, true, 0);
}

bool IRrecv::decodeDenonOld(decode_results *aResults) {

    // Check we have the right amount of data
    if (decodedIRData.rawlen != 1 + 2 + (2 * DENON_BITS) + 1) {
        return false;
    }

    // Check initial Mark+Space match
    if (!matchMark(aResults->rawbuf[1], DENON_HEADER_MARK)) {
        return false;
    }

    if (!matchSpace(aResults->rawbuf[2], DENON_HEADER_SPACE)) {
        return false;
    }

    // Try to decode as Denon protocol.
    if (!decodePulseDistanceWidthData(DENON_BITS, 3, DENON_BIT_MARK, DENON_ONE_SPACE, 0, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }

    // Success
    aResults->value = decodedIRData.decodedRawData;
    aResults->bits = DENON_BITS;
    aResults->decode_type = DENON;
    decodedIRData.protocol = DENON;
    return true;
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_DENON_HPP
