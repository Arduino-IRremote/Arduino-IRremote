/*
 * ir_Kaseikyo.cpp
 *
 *  Contains functions for receiving and sending Kaseikyo/Panasonic IR Protocol in "raw" and standard format with 16 bit address + 8 bit command
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

//#define DEBUG // Activate this  for lots of lovely debug output.
#include "IRremote.h"
#include "LongUnion.h"

//==============================================================================
//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//==============================================================================
// see: http://www.hifi-remote.com/johnsfine/DecodeIR.html#Panasonic
// IRP notation: {37k,432}<1,-1|1,-3>(8,-4,M:8,N:8,X:4,D:4,S:8,F:8,G:8,1,-173)+ {X=M:4:0^M:4:4^N:4:0^N:4:4}
// see: http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
// The first two (8-bit) bytes are always 2 and 32 (These identify Panasonic within the Kaseikyo standard)
// The next two bytes are 4 independent 4-bit fields or Device and Subdevice
// The second to last byte is the function and the last byte is xor of the three bytes before it.
// 0_______ 1_______  2______ 3_______ 4_______ 5
// 76543210 76543210 76543210 76543210 76543210 76543210
// 00000010 00100000 Dev____  Sub Dev  Fun____  XOR( B2, B3, B4)

// LSB first, start bit + 16 Vendor + 4 Parity(of vendor) + 4 Genre1 + 4 Genre2 + 10 Command + 2 ID + 8 Parity + stop bit
// We reduce it to: start bit + 16 Vendor + 16 Address + 8 Command + 8 Parity + stop bit
//
#define KASEIKYO_VENDOR_ID_BITS     16
#define KASEIKYO_VENDOR_ID_PARITY_BITS   4
#define KASEIKYO_ADDRESS_BITS       12
#define KASEIKYO_COMMAND_BITS       8
#define KASEIKYO_PARITY_BITS        8
#define KASEIKYO_BITS               (KASEIKYO_VENDOR_ID_BITS + KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS)
#define KASEIKYO_UNIT               432 // Pronto 0x70 / 0x10 - I measured 17 pulses

#define KASEIKYO_HEADER_MARK        (8 * KASEIKYO_UNIT) // 3456
#define KASEIKYO_HEADER_SPACE       (4 * KASEIKYO_UNIT) // 1728

#define KASEIKYO_BIT_MARK           KASEIKYO_UNIT
#define KASEIKYO_ONE_SPACE          (3 * KASEIKYO_UNIT) // 1296
#define KASEIKYO_ZERO_SPACE         KASEIKYO_UNIT

#define KASEIKYO_AVERAGE_DURATION   56000
#define KASEIKYO_REPEAT_PERIOD      130000
#define KASEIKYO_REPEAT_SPACE       (KASEIKYO_REPEAT_PERIOD - KASEIKYO_AVERAGE_DURATION)

// for old decoder
#define KASEIKYO_DATA_BITS          32

//+=============================================================================
/*
 * Send with LSB first
 * Address is sub-device << 8 + device
 */
void IRsend::sendKaseikyo(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, uint16_t aVendorCode) {
    // Set IR carrier frequency
    enableIROut(37); // 36.7kHz is the correct frequency

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        noInterrupts();
        // Header
        mark(KASEIKYO_HEADER_MARK);
        space(KASEIKYO_HEADER_SPACE);

        // Vendor ID
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aVendorCode,
        KASEIKYO_VENDOR_ID_BITS, LSB_FIRST);

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
        KASEIKYO_ADDRESS_BITS + KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS, LSB_FIRST, SEND_STOP_BIT);

        interrupts();

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(KASEIKYO_REPEAT_SPACE / 1000);
        }
    }
}

void IRsend::sendPanasonic(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    sendKaseikyo(aAddress, aCommand, aNumberOfRepeats, PANASONIC_VENDOR_ID_CODE);
}

/*
 * Tested with my Panasonic DVD/TV remote
 */
bool IRrecv::decodeKaseikyo() {

    decode_type_t tProtocol;
    // Check we have enough data (100)- +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != ((2 * KASEIKYO_BITS) + 4)) {
        return false;
    }

    if (!MATCH_MARK(decodedIRData.rawDataPtr->rawbuf[1], KASEIKYO_HEADER_MARK)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Header mark length is wrong");
        return false;
    }

    if (!MATCH_MARK(decodedIRData.rawDataPtr->rawbuf[2], KASEIKYO_HEADER_SPACE)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Header space length is wrong");
        return false;
    }

    // decode Vendor ID
    if (!decodePulseDistanceData(KASEIKYO_VENDOR_ID_BITS, 3, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_ZERO_SPACE, false)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Vendor ID decode failed");
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

    // decode address (device and subdevice) + command + parity
    if (!decodePulseDistanceData(
    KASEIKYO_VENDOR_ID_PARITY_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS,
            3 + (2 * KASEIKYO_VENDOR_ID_BITS), KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE,
            KASEIKYO_ZERO_SPACE, false)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Address, command + parity decode failed");
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;
    decodedIRData.address = (tValue.UWord.LowWord >> KASEIKYO_VENDOR_ID_PARITY_BITS); // remove vendor parity
    decodedIRData.command = tValue.UByte.MidHighByte;
    uint8_t tParity = tValue.UByte.LowByte ^ tValue.UByte.MidLowByte ^ tValue.UByte.MidHighByte;

    if (tVendorParity != (tValue.UByte.LowByte & 0xF)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINT("4 bit VendorID Parity is not correct. expected=0x");
        DBG_PRINT(tVendorParity, HEX);
        DBG_PRINT(" received=0x");
        DBG_PRINT(decodedIRData.decodedRawData, HEX);
        DBG_PRINT(" VendorID=0x");
        DBG_PRINTLN(tVendorId, HEX);
        decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_LSB_FIRST;
    }

    if (tProtocol == KASEIKYO) {
        decodedIRData.flags |= IRDATA_FLAGS_EXTRA_INFO;
#if defined(ENABLE_EXTRA_INFO)
        // Include vendor ID in address
        decodedIRData.extra |= tVendorId;
#endif
    }

    if (tValue.UByte.HighByte != tParity) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINT("8 bit Parity is not correct. expected=0x");
        DBG_PRINT(tParity, HEX);
        DBG_PRINT(" received=0x");
        DBG_PRINT(decodedIRData.decodedRawData >> KASEIKYO_COMMAND_BITS, HEX);
        DBG_PRINT(" address=0x");
        DBG_PRINT(decodedIRData.address, HEX);
        DBG_PRINT(" command=0x");
        DBG_PRINTLN(decodedIRData.command, HEX);
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
    }

    // check for repeat
    if (decodedIRData.rawDataPtr->rawbuf[0] < (KASEIKYO_REPEAT_PERIOD / MICROS_PER_TICK)) {
        decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
    }

    decodedIRData.protocol = tProtocol;

    decodedIRData.numberOfBits = KASEIKYO_BITS;

    return true;
}

//+=============================================================================
#if !defined(USE_STANDARD_DECODE)
bool IRrecv::decodePanasonic() {
    decodedIRData.flags |= IRDATA_FLAGS_IS_OLD_DECODER;
    unsigned int offset = 1;

    if (results.rawlen < (2 * KASEIKYO_BITS) + 2) {
        return false;
    }

    if (!MATCH_MARK(results.rawbuf[offset], KASEIKYO_HEADER_MARK)) {
        return false;
    }
    offset++;
    if (!MATCH_MARK(results.rawbuf[offset], KASEIKYO_HEADER_SPACE)) {
        return false;
    }
    offset++;

    // decode address
    if (!decodePulseDistanceData(KASEIKYO_ADDRESS_BITS + KASEIKYO_DATA_BITS, offset, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE,
    KASEIKYO_ZERO_SPACE)) {
        return false;
    }

    decodedIRData.protocol = PANASONIC;
    decodedIRData.numberOfBits = KASEIKYO_BITS;

    return true;
}

#endif

// Old version with MSB first Data
void IRsend::sendPanasonic(uint16_t aAddress, uint32_t aData) {
    // Set IR carrier frequency
    enableIROut(37); // 36.7kHz is the correct frequency

    // Header
    mark(KASEIKYO_HEADER_MARK);
    space(KASEIKYO_HEADER_SPACE);

    // Old version with MSB first Data Address
    sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aAddress,
    KASEIKYO_ADDRESS_BITS, MSB_FIRST);

    // Old version with MSB first Data Data + stop bit
    sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aData,
    KASEIKYO_DATA_BITS, MSB_FIRST);

}

