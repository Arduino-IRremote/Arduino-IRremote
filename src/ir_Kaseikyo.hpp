/*
 * ir_Kaseikyo.hpp
 *
 *  Contains functions for receiving and sending Kaseikyo/Panasonic IR Protocol in "raw" and standard format with 16 bit address + 8 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
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
#ifndef _IR_KASEIKYO_HPP
#define _IR_KASEIKYO_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT
#include "LongUnion.h"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//==============================================================================
// http://www.hifi-remote.com/johnsfine/DecodeIR.html#Panasonic
// http://www.hifi-remote.com/johnsfine/DecodeIR.html#Kaseikyo
// The first two (8-bit) bytes contains the vendor code.
// There are multiple interpretations of the next fields:
// IRP: {37k,432}<1,-1|1,-3>(8,-4,M:8,N:8,X:4,D:4,S:8,F:8,G:8,1,-173)+ {X=M:4:0^M:4:4^N:4:0^N:4:4}
// 1. interpretation: After the vendor code, the next byte is 4 bit VendorID parity and 4 bit Device and Subdevice
//    The 5th byte is the function and the last (6.th) byte is xor of the three bytes before it.
//    0_______ 1_______  2______ 3_______ 4_______ 5
//    01234567 89ABCDEF  01234567 01234567 01234567 01234567
//    01000000 00100100  Dev____  Sub_Dev_ Fun____  XOR( B2, B3, B4) - showing Panasonic vendor code 0x2002
// see: http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
//
// 2. interpretation: LSB first, start bit + 16 VendorID + 4 VendorID parity + 4 Genre1 + 4 Genre2 + 10 Command + 2 ID + 8 Parity + stop bit
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#KASEIKYO
//
//
// We reduce it to: IRP: {37k,432}<1,-1|1,-3>(8,-4,V:16,X:4,D:4,S:8,F:8,(X^D^S^F):8,1,-173)+ {X=M:4:0^M:4:4^N:4:0^N:4:4}
// start bit + 16 VendorID + 4 VendorID parity + 12 Address + 8 Command + 8 Parity of VendorID parity, Address and Command + stop bit
//
#define KASEIKYO_VENDOR_ID_BITS     16
#define KASEIKYO_VENDOR_ID_PARITY_BITS   4
#define KASEIKYO_ADDRESS_BITS       12
#define KASEIKYO_COMMAND_BITS       8
#define KASEIKYO_PARITY_BITS        8
#define KASEIKYO_BITS               (KASEIKYO_VENDOR_ID_BITS + KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS)
#define KASEIKYO_UNIT               432 // 16 pulses of 37 kHz (432,432432)  - Pronto 0x70 | 0x10

#define KASEIKYO_HEADER_MARK        (8 * KASEIKYO_UNIT) // 3456
#define KASEIKYO_HEADER_SPACE       (4 * KASEIKYO_UNIT) // 1728

#define KASEIKYO_BIT_MARK           KASEIKYO_UNIT
#define KASEIKYO_ONE_SPACE          (3 * KASEIKYO_UNIT) // 1296
#define KASEIKYO_ZERO_SPACE         KASEIKYO_UNIT

#define KASEIKYO_AVERAGE_DURATION   56000
#define KASEIKYO_REPEAT_PERIOD      130000
#define KASEIKYO_REPEAT_SPACE       (KASEIKYO_REPEAT_PERIOD - KASEIKYO_AVERAGE_DURATION) // 74 ms

// for old decoder
#define KASEIKYO_DATA_BITS          32

//+=============================================================================
/*
 * Send with LSB first
 * Address is sub-device << 8 + device
 */
void IRsend::sendKaseikyo(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, uint16_t aVendorCode) {
    // Set IR carrier frequency
    enableIROut(KASEIKYO_KHZ); // 37 kHz

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // Header
        mark(KASEIKYO_HEADER_MARK);
        space(KASEIKYO_HEADER_SPACE);

        // Vendor ID
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aVendorCode,
        KASEIKYO_VENDOR_ID_BITS, PROTOCOL_IS_LSB_FIRST, SEND_NO_STOP_BIT);

        // Vendor Parity
        uint8_t tVendorParity = aVendorCode ^ (aVendorCode >> 8);
        tVendorParity = (tVendorParity ^ (tVendorParity >> 4)) & 0xF;

        LongUnion tSendValue;
        tSendValue.UWord.LowWord = aAddress << KASEIKYO_VENDOR_ID_PARITY_BITS;
        tSendValue.UByte.LowByte |= tVendorParity; // set low nibble to parity
        tSendValue.UByte.MidHighByte = aCommand;
        tSendValue.UByte.HighByte = aCommand ^ tSendValue.UByte.LowByte ^ tSendValue.UByte.MidLowByte; // Parity

        // Send address (device and subdevice) + command + parity + Stop bit
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, tSendValue.ULong,
        KASEIKYO_ADDRESS_BITS + KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS,
        PROTOCOL_IS_LSB_FIRST, SEND_STOP_BIT);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(KASEIKYO_REPEAT_SPACE / MICROS_IN_ONE_MILLI);
        }
    }
    IrReceiver.restartAfterSend();
}

/**
 * Stub using Kaseikyo with PANASONIC_VENDOR_ID_CODE
 */
void IRsend::sendPanasonic(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, PANASONIC_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with DENON_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_Denon(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, DENON_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with MITSUBISHI_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_Mitsubishi(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, MITSUBISHI_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with SHARP_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_Sharp(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, SHARP_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with JVC_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_JVC(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, JVC_VENDOR_ID_CODE);
}

/*
 * Tested with my Panasonic DVD/TV remote
 */
bool IRrecv::decodeKaseikyo() {

    decode_type_t tProtocol;
    // Check we have enough data (100)- +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != ((2 * KASEIKYO_BITS) + 4)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 100"));
        return false;
    }

    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], KASEIKYO_HEADER_MARK)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINTLN(F("Header mark length is wrong"));
        return false;
    }

    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[2], KASEIKYO_HEADER_SPACE)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINTLN(F("Header space length is wrong"));
        return false;
    }

    // decode first 16 Vendor ID bits
    if (!decodePulseDistanceData(KASEIKYO_VENDOR_ID_BITS, 3, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_ZERO_SPACE,
    PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINTLN(F("Vendor ID decode failed"));
        return false;
    }

    uint16_t tVendorId = decodedIRData.decodedRawData;
    if (tVendorId == PANASONIC_VENDOR_ID_CODE) {
        tProtocol = PANASONIC;
    } else if (tVendorId == SHARP_VENDOR_ID_CODE) {
        tProtocol = KASEIKYO_SHARP;
    } else if (tVendorId == DENON_VENDOR_ID_CODE) {
        tProtocol = KASEIKYO_DENON;
    } else if (tVendorId == JVC_VENDOR_ID_CODE) {
        tProtocol = KASEIKYO_JVC;
    } else if (tVendorId == MITSUBISHI_VENDOR_ID_CODE) {
        tProtocol = KASEIKYO_MITSUBISHI;
    } else {
        tProtocol = KASEIKYO;
    }

    // Vendor Parity
    uint8_t tVendorParity = tVendorId ^ (tVendorId >> 8);
    tVendorParity = (tVendorParity ^ (tVendorParity >> 4)) & 0xF;

    // decode next 32 bits, 8 VendorID parity parity + 12 address (device and subdevice) + 8 command + 8 parity
    if (!decodePulseDistanceData(
    KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS,
            3 + (2 * KASEIKYO_VENDOR_ID_BITS), KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE,
            KASEIKYO_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINTLN(F("VendorID parity, address, command + parity decode failed"));
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;
    decodedIRData.address = (tValue.UWord.LowWord >> KASEIKYO_VENDOR_ID_PARITY_BITS); // remove 4 bit vendor parity
    decodedIRData.command = tValue.UByte.MidHighByte;
    uint8_t tParity = tValue.UByte.LowByte ^ tValue.UByte.MidLowByte ^ tValue.UByte.MidHighByte;

    if (tVendorParity != (tValue.UByte.LowByte & 0xF)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINT(F("4 bit VendorID parity is not correct. expected=0x"));
        IR_DEBUG_PRINT(tVendorParity, HEX);
        IR_DEBUG_PRINT(F(" received=0x"));
        IR_DEBUG_PRINT(decodedIRData.decodedRawData, HEX);
        IR_DEBUG_PRINT(F(" VendorID=0x"));
        IR_DEBUG_PRINTLN(tVendorId, HEX);
        decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_LSB_FIRST;
    }

    if (tProtocol == KASEIKYO) {
        decodedIRData.flags |= IRDATA_FLAGS_EXTRA_INFO;
        decodedIRData.extra = tVendorId; // Store (unknown) vendor ID
    }

    if (tValue.UByte.HighByte != tParity) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINT(F("8 bit Parity is not correct. expected=0x"));
        IR_DEBUG_PRINT(tParity, HEX);
        IR_DEBUG_PRINT(F(" received=0x"));
        IR_DEBUG_PRINT(decodedIRData.decodedRawData >> KASEIKYO_COMMAND_BITS, HEX);
        IR_DEBUG_PRINT(F(" address=0x"));
        IR_DEBUG_PRINT(decodedIRData.address, HEX);
        IR_DEBUG_PRINT(F(" command=0x"));
        IR_DEBUG_PRINTLN(decodedIRData.command, HEX);
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
    }

    // check for repeat
    if (decodedIRData.rawDataPtr->rawbuf[0] < ((KASEIKYO_REPEAT_SPACE + (KASEIKYO_REPEAT_SPACE / 4)) / MICROS_PER_TICK)) {
        decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
    }

    decodedIRData.protocol = tProtocol;

    decodedIRData.numberOfBits = KASEIKYO_BITS;

    return true;
}

/**
 * Old MSB first decoder
 */
bool IRrecv::decodePanasonicMSB(decode_results *aResults) {
    unsigned int offset = 1;

    if (aResults->rawlen < (2 * KASEIKYO_BITS) + 2) {
        return false;
    }

    if (!matchMark(aResults->rawbuf[offset], KASEIKYO_HEADER_MARK)) {
        return false;
    }
    offset++;
    if (!matchMark(aResults->rawbuf[offset], KASEIKYO_HEADER_SPACE)) {
        return false;
    }
    offset++;

    // decode address
    if (!decodePulseDistanceData(KASEIKYO_ADDRESS_BITS + KASEIKYO_DATA_BITS, offset, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE,
    KASEIKYO_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }

    aResults->bits = KASEIKYO_BITS;
    aResults->value = decodedIRData.decodedRawData;
    aResults->address = PANASONIC_VENDOR_ID_CODE;
    aResults->decode_type = PANASONIC;
    decodedIRData.protocol = PANASONIC;

    return true;
}

/**
 * Old version with MSB first data
 */
void IRsend::sendPanasonic(uint16_t aAddress, uint32_t aData) {
    // Set IR carrier frequency
    enableIROut(KASEIKYO_KHZ); // 36.7kHz is the correct frequency

    // Header
    mark(KASEIKYO_HEADER_MARK);
    space(KASEIKYO_HEADER_SPACE);

    // Old version with MSB first Data Address
    sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aAddress,
    KASEIKYO_ADDRESS_BITS, PROTOCOL_IS_MSB_FIRST, SEND_NO_STOP_BIT);

    // Old version with MSB first Data Data + stop bit
    sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aData,
    KASEIKYO_DATA_BITS, PROTOCOL_IS_MSB_FIRST, SEND_STOP_BIT);
    IrReceiver.restartAfterSend();
}

/** @}*/
#endif // _IR_KASEIKYO_HPP
