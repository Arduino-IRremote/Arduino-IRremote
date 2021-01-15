/*
 * ir_Samsung.cpp
 *
 *  Contains functions for receiving and sending Samsung IR Protocol in "raw" and standard format with 16 bit address and 16 or 32 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
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

//#define DEBUG // Activate this for lots of lovely debug output.
#include "IRremote.h"
#include "LongUnion.h"

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

#define SAMSUNG_UNIT                550
#define SAMSUNG_HEADER_MARK         (8 * SAMSUNG_UNIT) // 4400
#define SAMSUNG_HEADER_SPACE        (8 * SAMSUNG_UNIT) // 4400
#define SAMSUNG_BIT_MARK            SAMSUNG_UNIT
#define SAMSUNG_ONE_SPACE           (3 * SAMSUNG_UNIT) // 1650
#define SAMSUNG_ZERO_SPACE          SAMSUNG_UNIT

#define SAMSUNG_AVERAGE_DURATION    55000 // SAMSUNG_HEADER_MARK + SAMSUNG_HEADER_SPACE  + 32 * 2,5 * SAMSUNG_UNIT + SAMSUNG_UNIT // 2.5 because we assume more zeros than ones
#define SAMSUNG_REPEAT_DURATION     (SAMSUNG_HEADER_MARK  + SAMSUNG_HEADER_SPACE + SAMSUNG_BIT_MARK + SAMSUNG_ZERO_SPACE + SAMSUNG_BIT_MARK)
#define SAMSUNG_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.

//+=============================================================================
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendSamsungRepeat() {
    enableIROut(38);
    noInterrupts();
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);
    mark(SAMSUNG_BIT_MARK);
    space(SAMSUNG_ZERO_SPACE);
    mark(SAMSUNG_BIT_MARK);
    space(0); // Always end with the LED off
    interrupts();
}

void IRsend::sendSamsung(uint16_t aAddress, uint16_t aCommand, uint8_t aNumberOfRepeats, bool aIsRepeat) {
    if(aIsRepeat){
        sendSamsungRepeat();
        return;
    }

    // Set IR carrier frequency
    enableIROut(38);

    noInterrupts();

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Address
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aAddress,
    SAMSUNG_ADDRESS_BITS, false);

    // Command

    // send 8 command bits and then 8 inverted command bits LSB first
    aCommand = aCommand & 0xFF;
    aCommand = ((~aCommand) << 8) | aCommand;

    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aCommand,
    SAMSUNG_COMMAND16_BITS, false, true);

    interrupts();

    for (uint8_t i = 0; i < aNumberOfRepeats; ++i) {
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
#if defined(USE_STANDARD_DECODE)

bool IRrecv::decodeSamsung() {

    // Check we have enough data (68). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != ((2 * SAMSUNG_BITS) + 4) && results.rawlen != ((2 * SAMSUNG48_BITS) + 4) && (results.rawlen != 6)) {
        return false;
    }

    // Check header "mark" + "space"
    if (!MATCH_MARK(results.rawbuf[1], SAMSUNG_HEADER_MARK) || !MATCH_SPACE(results.rawbuf[2], SAMSUNG_HEADER_SPACE)) {
        DBG_PRINT("Samsung: ");
        DBG_PRINTLN("Header mark or space length is wrong");

        return false;
    }

    // Check for repeat
    if (results.rawlen == 6) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.address = lastDecodedAddress;
        decodedIRData.command = lastDecodedCommand;
        return true;
    }

    if (results.rawlen == (2 * SAMSUNG48_BITS) + 4) {
        /*
         * Samsung48
         */
        // decode address
        if (!decodePulseDistanceData(SAMSUNG_ADDRESS_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, false)) {
            DBG_PRINT("Samsung: ");
            DBG_PRINTLN("Decode failed");
            return false;
        }
        decodedIRData.address = results.value;

        // decode 32 bit command
        if (!decodePulseDistanceData(SAMSUNG_COMMAND32_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, false)) {
            DBG_PRINT("Samsung: ");
            DBG_PRINTLN("Decode failed");
            return false;
        }
        LongUnion tValue;
        tValue.ULong = results.value;
        // receive 2 * (8 bits then 8 inverted bits) LSB first
        if (tValue.UByte.HighByte != (uint8_t) (~tValue.UByte.MidHighByte)
                && tValue.UByte.MidLowByte != (uint8_t) (~tValue.UByte.LowByte)) {
            decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
        }
        decodedIRData.command = tValue.UByte.HighByte << 8 | tValue.UByte.MidLowByte;
        decodedIRData.numberOfBits = SAMSUNG48_BITS;

    } else {
        /*
         * Samsung32
         */
        if (!decodePulseDistanceData(SAMSUNG_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, false)) {
            DBG_PRINT("Samsung: ");
            DBG_PRINTLN("Decode failed");
            return false;
        }
        LongUnion tValue;
        tValue.ULong = results.value;
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

#else

#warning "Old decoder functions decodeSAMSUNG() and decodeSAMSUNG(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeSamsung() instead."

bool IRrecv::decodeSAMSUNG() {
    unsigned int offset = 1;  // Skip first space

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], SAMSUNG_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat -- like a NEC repeat
    if ((results.rawlen == 4) && MATCH_SPACE(results.rawbuf[offset], 2250)
            && MATCH_MARK(results.rawbuf[offset + 1], SAMSUNG_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER | IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = SAMSUNG;
        return true;
    }
    if (results.rawlen < (2 * SAMSUNG_BITS) + 4) {
        return false;
    }

// Initial space
    if (!MATCH_SPACE(results.rawbuf[offset], SAMSUNG_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(SAMSUNG_BITS, offset, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE)) {
        return false;
    }

// Success
    results.bits = SAMSUNG_BITS;
    decodedIRData.protocol = SAMSUNG;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;
    return true;
}

bool IRrecv::decodeSAMSUNG(decode_results *aResults) {
    bool aReturnValue = decodeSAMSUNG();
    *aResults = results;
    return aReturnValue;
}
#endif

void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Data + stop bit
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, data, nbits,true,true);
}
