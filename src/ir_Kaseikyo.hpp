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
#ifndef _IR_KASEIKYO_HPP
#define _IR_KASEIKYO_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//       K  K   AA    SSS   EEEE  III  K  K  Y   Y   OOO
//       K K   A  A  S      E      I   K K    Y Y   O   O
//       KK    AAAA   SSS   EEE    I   KK      Y    O   O
//       K K   A  A      S  E      I   K K     Y    O   O
//       K  K  A  A  SSSS   EEEE  III  K  K    Y     OOO
//==============================================================================
//==============================================================================
//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//==============================================================================
/*
 Protocol=Panasonic Address=0xFF1 Command=0x76 Raw-Data=0x9976FF10 48 bits LSB first
 +3450,-1700
 + 450,- 400 + 500,-1250 + 450,- 400 + 500,- 400
 + 450,- 400 + 400,- 450 + 500,- 350 + 450,- 450
 + 450,- 400 + 450,- 400 + 500,- 400 + 450,- 400
 + 450,- 400 + 500,-1250 + 450,- 400 + 500,- 350
 + 500,- 400 + 450,- 400 + 450,- 450 + 450,- 400
 + 450,-1250 + 500,- 400 + 450,- 400 + 450,- 400
 + 450,-1300 + 450,-1250 + 450,-1300 + 400,-1300
 + 450,-1300 + 450,-1250 + 450,-1250 + 500,-1250
 + 450,- 450 + 450,-1250 + 450,-1250 + 500,- 400
 + 450,-1250 + 450,-1300 + 450,-1250 + 450,- 450
 + 450,-1250 + 450,- 400 + 450,- 400 + 500,-1250
 + 450,-1250 + 450,- 400 + 500,- 400 + 450,-1250
 + 450
 Sum: 64300
 */
// http://www.hifi-remote.com/johnsfine/DecodeIR.html#Panasonic
// http://www.hifi-remote.com/johnsfine/DecodeIR.html#Kaseikyo
// LSB first
// The first two (8-bit) bytes contains the vendor code.
// The next 4 bit is VendorID parity.
// The last byte is parity (XOR) of the 3 bytes before.
// There are multiple interpretations of the next fields:
// IRP: {37k,432}<1,-1|1,-3>(8,-4,M:8,N:8,X:4,D:4,S:8,F:8,G:8,1,-173)+ {X=M:4:0^M:4:4^N:4:0^N:4:4}
// 1. interpretation: 4 bit Device, 8 bitSubdevice and 8 bit function.
//    0_______ 1_______  2______  3_______ 4_______ 5_______
//    01234567 89ABCDEF  01234567 01234567 01234567 01234567
//    01000000 00100100  0110Dev_ Sub_Dev_ Fun____  XOR( B2, B3, B4) - Byte 0,1 and vendor parity showing Panasonic vendor code 0x2002.
// 1. interpretation: <start bit><VendorID:16><VendorID parity:4><Device:4><Subdevice:8><Function:8><Parity:8><stop bit>
// see: http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
// 2. interpretation (Flipper Zero style): <start bit><VendorID:16><VendorID parity:4><Genre1:4><Genre2:4><Command:10><ID:2><Parity:8><stop bit>
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#KASEIKYO
// Implemented is Samsung style:  <start bit><VendorID:16><VendorID parity:4><Address:12><Command:8><Parity of VendorID parity, Address and Command:8><stop bit>
//                  which is derived from Samsung remotes and may not be optimal for Denon kind of Kaseikyo protokol usage.
//
#define KASEIKYO_VENDOR_ID_BITS     16
#define KASEIKYO_VENDOR_ID_PARITY_BITS   4
#define KASEIKYO_ADDRESS_BITS       12
#define KASEIKYO_COMMAND_BITS       8
#define KASEIKYO_PARITY_BITS        8
#define KASEIKYO_BITS               (KASEIKYO_VENDOR_ID_BITS + KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS) // 48
#define KASEIKYO_UNIT               432 // 16 pulses of 37 kHz (432,432432)  - Pronto 0x70 | 0x10

#define KASEIKYO_HEADER_MARK        (8 * KASEIKYO_UNIT) // 3456
#define KASEIKYO_HEADER_SPACE       (4 * KASEIKYO_UNIT) // 1728

#define KASEIKYO_BIT_MARK           KASEIKYO_UNIT
#define KASEIKYO_ONE_SPACE          (3 * KASEIKYO_UNIT) // 1296
#define KASEIKYO_ZERO_SPACE         KASEIKYO_UNIT

#define KASEIKYO_AVERAGE_DURATION   56000
#define KASEIKYO_REPEAT_PERIOD      130000
#define KASEIKYO_REPEAT_DISTANCE    (KASEIKYO_REPEAT_PERIOD - KASEIKYO_AVERAGE_DURATION) // 74 ms
#define KASEIKYO_MAXIMUM_REPEAT_DISTANCE    (KASEIKYO_REPEAT_DISTANCE + (KASEIKYO_REPEAT_DISTANCE / 4)) // Just a guess

#define PANASONIC_VENDOR_ID_CODE    0x2002
#define DENON_VENDOR_ID_CODE        0x3254
#define MITSUBISHI_VENDOR_ID_CODE   0xCB23
#define SHARP_VENDOR_ID_CODE        0x5AAA
#define JVC_VENDOR_ID_CODE          0x0103

struct PulseDistanceWidthProtocolConstants KaseikyoProtocolConstants = { KASEIKYO, KASEIKYO_KHZ, KASEIKYO_HEADER_MARK,
KASEIKYO_HEADER_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST
       , (KASEIKYO_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), NULL };

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * Address can be interpreted as sub-device << 4 + 4 bit device
 */
void IRsend::sendKaseikyo(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint16_t aVendorCode) {
    // Set IR carrier frequency
    enableIROut (KASEIKYO_KHZ); // 37 kHz

    // Vendor Parity
    uint8_t tVendorParity = aVendorCode ^ (aVendorCode >> 8);
    tVendorParity = (tVendorParity ^ (tVendorParity >> 4)) & 0xF;

#if __INT_WIDTH__ < 32
    LongUnion tSendValue;
    // Compute parity
    tSendValue.UWord.LowWord = (aAddress << KASEIKYO_VENDOR_ID_PARITY_BITS) | tVendorParity; // set low nibble with vendor parity
    tSendValue.UBytes[2] = aCommand;
    tSendValue.UBytes[3] = aCommand ^ tSendValue.UBytes[0] ^ tSendValue.UBytes[1]; // 8 bit parity of 3 bytes command, address and vendor parity
    IRRawDataType tRawKaseikyoData[2];
    tRawKaseikyoData[0] = (uint32_t) tSendValue.UWord.LowWord << 16 | aVendorCode; // LSB of tRawKaseikyoData[0] is sent first
    tRawKaseikyoData[1] = tSendValue.UWord.HighWord;
    sendPulseDistanceWidthFromArray(&KaseikyoProtocolConstants, &tRawKaseikyoData[0], KASEIKYO_BITS, aNumberOfRepeats);
#else
    LongLongUnion tSendValue;
    tSendValue.UWords[0] = aVendorCode;
    // Compute parity
    tSendValue.UWords[1] = (aAddress << KASEIKYO_VENDOR_ID_PARITY_BITS) | tVendorParity; // set low nibble to parity
    tSendValue.UBytes[4] = aCommand;
    tSendValue.UBytes[5] = aCommand ^ tSendValue.UBytes[2] ^ tSendValue.UBytes[3]; // Parity
    sendPulseDistanceWidth(&KaseikyoProtocolConstants, tSendValue.ULongLong, KASEIKYO_BITS, aNumberOfRepeats);
#endif
}

/**
 * Stub using Kaseikyo with PANASONIC_VENDOR_ID_CODE
 */
void IRsend::sendPanasonic(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, PANASONIC_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with DENON_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_Denon(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, DENON_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with MITSUBISHI_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_Mitsubishi(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, MITSUBISHI_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with SHARP_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_Sharp(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, SHARP_VENDOR_ID_CODE);
}

/**
 * Stub using Kaseikyo with JVC_VENDOR_ID_CODE
 */
void IRsend::sendKaseikyo_JVC(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, JVC_VENDOR_ID_CODE);
}

/*
 * Tested with my Panasonic DVD/TV remote
 */
bool IRrecv::decodeKaseikyo() {

    decode_type_t tProtocol;
    // Check we have enough data (96 + 4) 4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawlen != ((2 * KASEIKYO_BITS) + 4)) {
        IR_DEBUG_PRINT(F("Kaseikyo: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 100"));
        return false;
    }

    if (!checkHeader(&KaseikyoProtocolConstants)) {
        return false;
    }

    // decode first 16 Vendor ID bits
    if (!decodePulseDistanceWidthData(&KaseikyoProtocolConstants, KASEIKYO_VENDOR_ID_BITS)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Kaseikyo: "));
        Serial.println(F("Vendor ID decode failed"));
#endif
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

    /*
     * Decode next 32 bits, 8 VendorID parity parity + 12 address (device and subdevice) + 8 command + 8 parity
     */
    if (!decodePulseDistanceWidthData(&KaseikyoProtocolConstants,
    KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS,
            3 + (2 * KASEIKYO_VENDOR_ID_BITS))) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Kaseikyo: "));
        Serial.println(F("VendorID parity, address, command + parity decode failed"));
#endif
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;
#if __INT_WIDTH__ >= 32
    // workaround until complete refactoring for 64 bit
    decodedIRData.decodedRawData = (decodedIRData.decodedRawData << 16) | tVendorId; // store all 48 bits in decodedRawData
#endif
    decodedIRData.address = (tValue.UWord.LowWord >> KASEIKYO_VENDOR_ID_PARITY_BITS); // remove 4 bit vendor parity
    decodedIRData.command = tValue.UByte.MidHighByte;
    uint8_t tParity = tValue.UByte.LowByte ^ tValue.UByte.MidLowByte ^ tValue.UByte.MidHighByte;

    if (tVendorParity != (tValue.UByte.LowByte & 0xF)) {
        decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_LSB_FIRST;

#if defined(LOCAL_DEBUG)
        Serial.print(F("Kaseikyo: "));
        Serial.print(F("4 bit VendorID parity is not correct. Expected=0x"));
        Serial.print(tVendorParity, HEX);
        Serial.print(F(" received=0x"));
        Serial.print(decodedIRData.decodedRawData, HEX);
        Serial.print(F(" VendorID=0x"));
        Serial.println(tVendorId, HEX);
#endif
    }

    if (tProtocol == KASEIKYO) {
        decodedIRData.flags |= IRDATA_FLAGS_EXTRA_INFO;
        decodedIRData.extra = tVendorId; // Store (unknown) vendor ID
    }

    if (tValue.UByte.HighByte != tParity) {
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;

#if defined(LOCAL_DEBUG)
        Serial.print(F("Kaseikyo: "));
        Serial.print(F("8 bit parity is not correct. Expected=0x"));
        Serial.print(tParity, HEX);
        Serial.print(F(" received=0x"));
        Serial.print(decodedIRData.decodedRawData >> KASEIKYO_COMMAND_BITS, HEX);
        Serial.print(F(" address=0x"));
        Serial.print(decodedIRData.address, HEX);
        Serial.print(F(" command=0x"));
        Serial.println(decodedIRData.command, HEX);
#endif
    }

    decodedIRData.numberOfBits = KASEIKYO_BITS;
    decodedIRData.protocol = tProtocol;

    // check for repeat
    checkForRepeatSpaceTicksAndSetFlag(KASEIKYO_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}

/*
 * Removed void IRsend::sendPanasonic(uint16_t aAddress, uint32_t aData)
 * and bool IRrecv::decodePanasonicMSB(decode_results *aResults)
 * since their implementations were wrong (wrong length), and nobody recognized it
 */

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_KASEIKYO_HPP
