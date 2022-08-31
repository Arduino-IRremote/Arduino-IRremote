/*
 * ir_LG.hpp
 *
 *  Contains functions for receiving and sending LG IR Protocol in "raw" and standard format with 16 or 8 bit address and 8 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2021 Darryl Smith, Armin Joachimsmeyer
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
#ifndef _IR_LG_HPP
#define _IR_LG_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================
// LG originally added by Darryl Smith (based on the JVC protocol)
// see: https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/examples/LGAirConditionerSendDemo
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#LGAIR
// MSB first, 1 start bit + 8 bit address + 16 bit command + 4 bit checksum + 1 stop bit (28 data bits).
// Bit and repeat timing is like NEC
// LG2 has different header timing and a shorter bit time
/*
 * LG remote IR-LED measurements: Type AKB73315611, Ver1.1 from 2011.03.01
 * Internal crystal: 4 MHz
 * Header:  8.9 ms mark 4.15 ms space
 * Data:    500 / 540 and 500 / 1580;
 * Clock is nor synchronized with gate so you have 19 and sometimes 19 and a spike pulses for mark
 * Duty:    9 us on 17 us off => around 33 % duty
 * NO REPEAT: If value like temperature has changed during long press, the last value is send at button release.
 * If you do a double press, the next value can be sent after around 118 ms. Tested with the fan button.
 *
 * The codes of the LG air conditioner are documented in https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/ac_LG.cpp
 */
#define LG_ADDRESS_BITS          8
#define LG_COMMAND_BITS         16
#define LG_CHECKSUM_BITS         4
#define LG_BITS                 (LG_ADDRESS_BITS + LG_COMMAND_BITS + LG_CHECKSUM_BITS) // 28

#define LG_UNIT                 500 // 19 periods of 38 kHz

#define LG_HEADER_MARK          (18 * LG_UNIT) // 9000
#define LG_HEADER_SPACE         4200           // 84

#define LG2_HEADER_MARK         (6 * LG_UNIT)  // 3000
#define LG2_HEADER_SPACE        (19 * LG_UNIT) // 9500

#define LG_BIT_MARK             LG_UNIT
#define LG_ONE_SPACE            1580  // 60 periods of 38 kHz
#define LG_ZERO_SPACE           550

#define LG_REPEAT_HEADER_SPACE  (4 * LG_UNIT)  // 2250
#define LG_AVERAGE_DURATION     58000 // LG_HEADER_MARK + LG_HEADER_SPACE  + 32 * 2,5 * LG_UNIT) + LG_UNIT // 2.5 because we assume more zeros than ones
#define LG_REPEAT_DURATION      (LG_HEADER_MARK  + LG_REPEAT_HEADER_SPACE + LG_BIT_MARK)
#define LG_REPEAT_PERIOD        110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
#define LG_REPEAT_SPACE         (LG_REPEAT_PERIOD - LG_AVERAGE_DURATION) // 52 ms

//+=============================================================================
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendLGRepeat(bool aUseLG2Protocol) {
    enableIROut(LG_KHZ); // 38 kHz
    if (aUseLG2Protocol) {
        mark(LG2_HEADER_MARK);
    } else {
        mark(LG_HEADER_MARK);
    }
    space(LG_REPEAT_HEADER_SPACE);
    mark(LG_BIT_MARK);
    IrReceiver.restartAfterSend();
}

/**
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 * @param aUseLG2Protocol if true use LG2 protocol, which has a different header
 */
void IRsend::sendLG(uint8_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialLGRepeat, bool aUseLG2Protocol) {
    uint32_t tRawData = ((uint32_t) aAddress << (LG_COMMAND_BITS + LG_CHECKSUM_BITS)) | ((uint32_t) aCommand << LG_CHECKSUM_BITS);
    /*
     * My guess of the 4 bit checksum
     * Addition of all 4 nibbles of the 16 bit command
     */
    uint8_t tChecksum = 0;
    uint16_t tTempForChecksum = aCommand;
    for (int i = 0; i < 4; ++i) {
        tChecksum += tTempForChecksum & 0xF; // add low nibble
        tTempForChecksum >>= 4; // shift by a nibble
    }
    tRawData |= (tChecksum & 0xF);
    sendLGRaw(tRawData, aNumberOfRepeats, aSendOnlySpecialLGRepeat, aUseLG2Protocol);
}

void IRsend::sendLG2(uint8_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialLGRepeat) {
    sendLG(aAddress, aCommand, aNumberOfRepeats, aSendOnlySpecialLGRepeat);
}

/*
 * Here you can put your raw data, even one with "wrong" checksum
 */
void IRsend::sendLGRaw(uint32_t aRawData, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialLGRepeat, bool aUseLG2Protocol) {
    if (aSendOnlySpecialLGRepeat) {
        sendLGRepeat();
        return;
    }
    // Set IR carrier frequency
    enableIROut(LG_KHZ);

    // Header
    if (aUseLG2Protocol) {
        mark(LG2_HEADER_MARK);
        space(LG2_HEADER_SPACE);
    } else {
        mark(LG_HEADER_MARK);
        space(LG_HEADER_SPACE);
    }
    // MSB first
    sendPulseDistanceWidthData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, aRawData, LG_BITS, PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);

    for (uint_fast8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        if (i == 0) {
            delay(LG_REPEAT_SPACE / MICROS_IN_ONE_MILLI);
        } else {
            delay((LG_REPEAT_PERIOD - LG_REPEAT_DURATION) / MICROS_IN_ONE_MILLI);
        }
        // send repeat
        sendLGRepeat(aUseLG2Protocol);
    }
    IrReceiver.restartAfterSend();
}

//+=============================================================================
// LGs has a repeat like NEC
//
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 */
bool IRrecv::decodeLG() {
    decode_type_t tProtocol = LG;
    uint16_t tHeaderSpace = LG_HEADER_SPACE;

// Check we have the right amount of data (60). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawDataPtr->rawlen != ((2 * LG_BITS) + 4) && (decodedIRData.rawDataPtr->rawlen != 4)) {
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 60 or 4"));
        return false;
    }

// Check header "mark" this must be done for repeat and data
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], LG_HEADER_MARK)) {
        if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], LG2_HEADER_MARK)) {
            IR_DEBUG_PRINT(F("LG: "));
            IR_DEBUG_PRINTLN(F("Header mark is wrong"));
            return false;
        } else {
            tProtocol = LG2;
            tHeaderSpace = LG2_HEADER_SPACE;
        }
    }

// Check for repeat - here we have another header space length
    if (decodedIRData.rawDataPtr->rawlen == 4) {
        if (matchSpace(decodedIRData.rawDataPtr->rawbuf[2], LG_REPEAT_HEADER_SPACE)
                && matchMark(decodedIRData.rawDataPtr->rawbuf[3], LG_BIT_MARK)) {
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_MSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            decodedIRData.protocol = lastDecodedProtocol;
            return true;
        }
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINT(F("Repeat header space is wrong"));
        return false;
    }

// Check command header space
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[2], tHeaderSpace)) {
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINTLN(F("Header space length is wrong"));
        return false;
    }

    if (!decodePulseDistanceData(LG_BITS, 3, LG_BIT_MARK, LG_ONE_SPACE, LG_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

// Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * LG_BITS)], LG_BIT_MARK)) {
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

// Success
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.command = (decodedIRData.decodedRawData >> LG_CHECKSUM_BITS) & 0xFFFF;
    decodedIRData.address = decodedIRData.decodedRawData >> (LG_COMMAND_BITS + LG_CHECKSUM_BITS); // first 8 bit

    /*
     * My guess of the checksum
     */
    uint8_t tChecksum = 0;
    uint16_t tTempForChecksum = decodedIRData.command;
    for (int i = 0; i < 4; ++i) {
        tChecksum += tTempForChecksum & 0xF; // add low nibble
        tTempForChecksum >>= 4; // shift by a nibble
    }
// Checksum check
    if ((tChecksum & 0xF) != (decodedIRData.decodedRawData & 0xF)) {
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINT(F("4 bit checksum is not correct. expected=0x"));
        IR_DEBUG_PRINT(tChecksum, HEX);
        IR_DEBUG_PRINT(F(" received=0x"));
        IR_DEBUG_PRINT((decodedIRData.decodedRawData & 0xF), HEX);
        IR_DEBUG_PRINT(F(" data=0x"));
        IR_DEBUG_PRINTLN(decodedIRData.command, HEX);
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
    }

    decodedIRData.protocol = tProtocol; // LG or LG2
    decodedIRData.numberOfBits = LG_BITS;

    return true;
}

bool IRrecv::decodeLGMSB(decode_results *aResults) {
    unsigned int offset = 1; // Skip first space

// Check we have enough data (60) - +4 for initial gap, start bit mark and space + stop bit mark
    if (aResults->rawlen != (2 * LG_BITS) + 4) {
        return false;
    }

// Initial mark/space
    if (!matchMark(aResults->rawbuf[offset], LG_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (!matchSpace(aResults->rawbuf[offset], LG_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(LG_BITS, offset, LG_BIT_MARK, LG_ONE_SPACE, LG_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }
// Stop bit
    if (!matchMark(aResults->rawbuf[offset + (2 * LG_BITS)], LG_BIT_MARK)) {
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

// Success
    aResults->value = decodedIRData.decodedRawData;
    aResults->bits = LG_BITS;
    aResults->decode_type = LG;
    decodedIRData.protocol = LG;
    return true;
}

//+=============================================================================
void IRsend::sendLG(unsigned long data, int nbits) {
// Set IR carrier frequency
    enableIROut(LG_KHZ);
#if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__))
    Serial.println(
            "The function sendLG(data, nbits) is deprecated and may not work as expected! Use sendLGRaw(data, NumberOfRepeats) or better sendLG(Address, Command, NumberOfRepeats).");
#endif
// Header
    mark(LG_HEADER_MARK);
    space(LG_HEADER_SPACE);
//    mark(LG_BIT_MARK);

// Data + stop bit
    sendPulseDistanceWidthData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);
    IrReceiver.restartAfterSend();
}

/** @}*/
#endif // _IR_LG_HPP
