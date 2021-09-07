/*
 * ir_Samsung.cpp
 *
 *  Contains functions for receiving and sending Samsung IR Protocol in "raw" and standard format with 16 bit address and 16 or 32 bit command
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
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONSAMSUNGTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for DEBUG_PRINT
#include "LongUnion.h"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//==============================================================================
// see http://www.hifi-remote.com/wiki/index.php?title=DecodeIR#Samsung
// https://www.mikrocontroller.net/articles/IRMP_-_english#SAMSUNG32
// LSB first, 1 start bit + 16 bit address + 16,32,20 bit data + 1 stop bit.
// repeats are like NEC but with 2 stop bits

#define SAMSUNG_ADDRESS_BITS        16
#define SAMSUNG_COMMAND16_BITS      16
#define SAMSUNG_COMMAND32_BITS      32
#define SAMSUNG_BITS                (SAMSUNG_ADDRESS_BITS + SAMSUNG_COMMAND16_BITS)
#define SAMSUNG48_BITS              (SAMSUNG_ADDRESS_BITS + SAMSUNG_COMMAND32_BITS)

#define SAMSUNG_UNIT                553 // 21 periods of 38 kHz (552,631)
#define SAMSUNG_HEADER_MARK         (8 * SAMSUNG_UNIT) // 4400
#define SAMSUNG_HEADER_SPACE        (8 * SAMSUNG_UNIT) // 4400
#define SAMSUNG_BIT_MARK            SAMSUNG_UNIT
#define SAMSUNG_ONE_SPACE           (3 * SAMSUNG_UNIT) // 1650
#define SAMSUNG_ZERO_SPACE          SAMSUNG_UNIT

#define SAMSUNG_AVERAGE_DURATION    55000 // SAMSUNG_HEADER_MARK + SAMSUNG_HEADER_SPACE  + 32 * 2,5 * SAMSUNG_UNIT + SAMSUNG_UNIT // 2.5 because we assume more zeros than ones
#define SAMSUNG_REPEAT_DURATION     (SAMSUNG_HEADER_MARK  + SAMSUNG_HEADER_SPACE + SAMSUNG_BIT_MARK + SAMSUNG_ZERO_SPACE + SAMSUNG_BIT_MARK)
#define SAMSUNG_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.


/**
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendSamsungRepeat() {
    enableIROut(SAMSUNG_KHZ); // 38 kHz
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);
    mark(SAMSUNG_BIT_MARK);
    space(SAMSUNG_ZERO_SPACE);
    mark(SAMSUNG_BIT_MARK);
}

void IRsend::sendSamsung(uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat) {
    if (aIsRepeat) {
        sendSamsungRepeat();
        return;
    }

    // Set IR carrier frequency
    enableIROut(SAMSUNG_KHZ);

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Address
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aAddress,
    SAMSUNG_ADDRESS_BITS, PROTOCOL_IS_LSB_FIRST);

    // Command

    // send 8 command bits and then 8 inverted command bits LSB first
    aCommand = aCommand & 0xFF;
    aCommand = ((~aCommand) << 8) | aCommand;

    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aCommand,
    SAMSUNG_COMMAND16_BITS, PROTOCOL_IS_LSB_FIRST, SEND_STOP_BIT);

    for (uint_fast8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        if (i == 0) {
            delay((SAMSUNG_REPEAT_PERIOD - SAMSUNG_AVERAGE_DURATION) / 1000);
        } else {
            delay((SAMSUNG_REPEAT_PERIOD - SAMSUNG_REPEAT_DURATION) / 1000);
        }
        // send repeat
        sendSamsungRepeat();
    }
}

//+=============================================================================
bool IRrecv::decodeSamsung() {

    // Check we have enough data (68). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != ((2 * SAMSUNG_BITS) + 4)
            && decodedIRData.rawDataPtr->rawlen != ((2 * SAMSUNG48_BITS) + 4) && (decodedIRData.rawDataPtr->rawlen != 6)) {
        DEBUG_PRINT(F("Samsung: "));
        DEBUG_PRINT("Data length=");
        DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        DEBUG_PRINTLN(" is not 68 or 100 or 6");
        return false;
    }

    // Check header "mark" + "space"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], SAMSUNG_HEADER_MARK)
            || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], SAMSUNG_HEADER_SPACE)) {
        DEBUG_PRINT("Samsung: ");
        DEBUG_PRINTLN("Header mark or space length is wrong");

        return false;
    }

    // Check for repeat
    if (decodedIRData.rawDataPtr->rawlen == 6) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT| IRDATA_FLAGS_IS_LSB_FIRST;
        decodedIRData.address = lastDecodedAddress;
        decodedIRData.command = lastDecodedCommand;
        decodedIRData.protocol = SAMSUNG;
        return true;
    }

    if (decodedIRData.rawDataPtr->rawlen == (2 * SAMSUNG48_BITS) + 4) {
        /*
         * Samsung48
         */
        // decode address
        if (!decodePulseDistanceData(SAMSUNG_ADDRESS_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
            DEBUG_PRINT("Samsung: ");
            DEBUG_PRINTLN("Decode failed");
            return false;
        }
        decodedIRData.address = decodedIRData.decodedRawData;

        // decode 32 bit command
        if (!decodePulseDistanceData(SAMSUNG_COMMAND32_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
            DEBUG_PRINT("Samsung: ");
            DEBUG_PRINTLN("Decode failed");
            return false;
        }

        // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
        LongUnion tValue;
        tValue.ULong = decodedIRData.decodedRawData;
        // receive 2 * (8 bits then 8 inverted bits) LSB first
        if (tValue.UByte.HighByte != (uint8_t) (~tValue.UByte.MidHighByte)
                && tValue.UByte.MidLowByte != (uint8_t) (~tValue.UByte.LowByte)) {
            decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_LSB_FIRST;
        }
        decodedIRData.command = tValue.UByte.HighByte << 8 | tValue.UByte.MidLowByte;
        decodedIRData.numberOfBits = SAMSUNG48_BITS;

    } else {
        /*
         * Samsung32
         */
        if (!decodePulseDistanceData(SAMSUNG_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
            DEBUG_PRINT("Samsung: ");
            DEBUG_PRINTLN("Decode failed");
            return false;
        }
        LongUnion tValue;
        tValue.ULong = decodedIRData.decodedRawData;
        decodedIRData.address = tValue.UWord.LowWord;

        if (tValue.UByte.MidHighByte == (uint8_t) (~tValue.UByte.HighByte)) {
            // 8 bit command protocol
            decodedIRData.command = tValue.UByte.MidHighByte; // first 8 bit
        } else {
            // 16 bit command protocol
            decodedIRData.command = tValue.UWord.HighWord; // first 16 bit
        }
        decodedIRData.numberOfBits = SAMSUNG_BITS;
    }

    decodedIRData.protocol = SAMSUNG;

    return true;
}

#if !defined(NO_LEGACY_COMPATIBILITY)
bool IRrecv::decodeSAMSUNG(decode_results *aResults) {
    unsigned int offset = 1;  // Skip first space

    // Initial mark
    if (!matchMark(aResults->rawbuf[offset], SAMSUNG_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat -- like a NEC repeat
    if ((aResults->rawlen == 4) && matchSpace(aResults->rawbuf[offset], 2250)
            && matchMark(aResults->rawbuf[offset + 1], SAMSUNG_BIT_MARK)) {
        aResults->bits = 0;
        aResults->value = 0xFFFFFFFF;
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = SAMSUNG;
        return true;
    }
    if (aResults->rawlen < (2 * SAMSUNG_BITS) + 4) {
        return false;
    }

// Initial space
    if (!matchSpace(aResults->rawbuf[offset], SAMSUNG_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(SAMSUNG_BITS, offset, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }

// Success
    aResults->value = decodedIRData.decodedRawData;
    aResults->bits = SAMSUNG_BITS;
    aResults->decode_type = SAMSUNG;
    decodedIRData.protocol = SAMSUNG;
    return true;
}
#endif

// Old version with MSB first
void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(SAMSUNG_KHZ);

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Old version with MSB first Data + stop bit
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);
}

/** @}*/
