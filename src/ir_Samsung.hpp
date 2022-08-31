/*
 * ir_Samsung.hpp
 *
 *  Contains functions for receiving and sending Samsung IR Protocol in "raw" and standard format with 16 bit address and 16 or 32 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2022 Darryl Smith, Armin Joachimsmeyer
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
#ifndef _IR_SAMSUNG_HPP
#define _IR_SAMSUNG_HPP

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
// LSB first, 1 start bit + 16 bit address + 16,32 bit data + 1 stop bit.
// IRP notation: {38k,5553}<1,-1|1,-3>(8,-8,D:8,S:8,F:8,~F:8,1,^110)+  ==> 8 bit data
// IRP notation: {38k,5553}<1,-1|1,-3>(8,-8,D:8,S:8,F:16,1,^110)+  ==> 16 bit data
// IRP notation: {38k,5553}<1,-1|1,-3>(8,-8,D:8,S:8,F:32,1,^110)+  ==> 32 bit data
//
#define SAMSUNG_ADDRESS_BITS        16
#define SAMSUNG_COMMAND16_BITS      16
#define SAMSUNG_COMMAND32_BITS      32
#define SAMSUNG_BITS                (SAMSUNG_ADDRESS_BITS + SAMSUNG_COMMAND16_BITS)
#define SAMSUNG48_BITS              (SAMSUNG_ADDRESS_BITS + SAMSUNG_COMMAND32_BITS)

// except SAMSUNG_HEADER_MARK, values are like NEC
#define SAMSUNG_UNIT                560             // 21.28 periods of 38 kHz, 11.2 ticks TICKS_LOW = 8.358 TICKS_HIGH = 15.0
#define SAMSUNG_HEADER_MARK         (8 * SAMSUNG_UNIT) // 4500 | 180
#define SAMSUNG_HEADER_SPACE        (8 * SAMSUNG_UNIT) // 4500
#define SAMSUNG_BIT_MARK            SAMSUNG_UNIT
#define SAMSUNG_ONE_SPACE           (3 * SAMSUNG_UNIT) // 1690 | 33.8  TICKS_LOW = 25.07 TICKS_HIGH = 45.0
#define SAMSUNG_ZERO_SPACE          SAMSUNG_UNIT

#define SAMSUNG_AVERAGE_DURATION    55000 // SAMSUNG_HEADER_MARK + SAMSUNG_HEADER_SPACE  + 32 * 2,5 * SAMSUNG_UNIT + SAMSUNG_UNIT // 2.5 because we assume more zeros than ones
#define SAMSUNG_REPEAT_DURATION     (SAMSUNG_HEADER_MARK  + SAMSUNG_HEADER_SPACE + SAMSUNG_BIT_MARK + SAMSUNG_ZERO_SPACE + SAMSUNG_BIT_MARK)
#define SAMSUNG_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
#define SAMSUNG_REPEAT_SPACE        (SAMSUNG_REPEAT_PERIOD - SAMSUNG_AVERAGE_DURATION)

struct PulsePauseWidthProtocolConstants SamsungProtocolConstants = { SAMSUNG, SAMSUNG_KHZ, SAMSUNG_HEADER_MARK,
SAMSUNG_HEADER_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST,
        SEND_STOP_BIT, (SAMSUNG_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), &sendSamsungLGSpecialRepeat };

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 * This repeat was sent by an LG 6711R1P071A remote
 */
void IRsend::sendSamsungLGRepeat() {
    enableIROut (SAMSUNG_KHZ);       // 38 kHz
    mark(SAMSUNG_HEADER_MARK);      // + 4500
    space(SAMSUNG_HEADER_SPACE);    // - 4500
    mark(SAMSUNG_BIT_MARK);         // + 560
    space(SAMSUNG_ZERO_SPACE);      // - 560
    mark(SAMSUNG_BIT_MARK);         // + 560
    IrReceiver.restartAfterSend();
}

/**
 * Static function for sending special repeat frame.
 * For use in ProtocolConstants. Saves up to 250 bytes compared to a member function.
 */
void sendSamsungLGSpecialRepeat() {
    IrSender.enableIROut(SAMSUNG_KHZ);      // 38 kHz
    IrSender.mark(SAMSUNG_HEADER_MARK);     // + 4500
    IrSender.space(SAMSUNG_HEADER_SPACE);   // - 4500
    IrSender.mark(SAMSUNG_BIT_MARK);        // + 560
    IrSender.space(SAMSUNG_ZERO_SPACE);     // - 560
    IrSender.mark(SAMSUNG_BIT_MARK);        // + 560
    IrReceiver.restartAfterSend();
}

void IRsend::sendSamsung(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {

    // send 16 bit address and  8 command bits and then 8 inverted command bits LSB first
    LongUnion tRawData;
    tRawData.UWord.LowWord = aAddress;
    tRawData.UByte.MidHighByte = aCommand;
    tRawData.UByte.HighByte = ~aCommand;

    sendPulseDistanceWidth(&SamsungProtocolConstants, tRawData.ULong, SAMSUNG_BITS, aNumberOfRepeats);
}

/*
 * Sent e.g. by an LG 6711R1P071A remote
 * @param aNumberOfRepeats If < 0 then only a special repeat frame will be sent
 */
void IRsend::sendSamsungLG(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {
    if (aNumberOfRepeats < 0) {
        sendSamsungLGRepeat();
        return;
    }

    // send 16 bit address and  8 command bits and then 8 inverted command bits LSB first
    LongUnion tRawData;
    tRawData.UWord.LowWord = aAddress;
    tRawData.UByte.MidHighByte = aCommand;
    tRawData.UByte.HighByte = ~aCommand;

    sendPulseDistanceWidth(&SamsungProtocolConstants, tRawData.ULong, SAMSUNG_BITS, aNumberOfRepeats);
}

bool IRrecv::decodeSamsung() {

    // Check we have enough data (68). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != ((2 * SAMSUNG_BITS) + 4)
            && decodedIRData.rawDataPtr->rawlen != ((2 * SAMSUNG48_BITS) + 4) && (decodedIRData.rawDataPtr->rawlen != 6)) {
        IR_DEBUG_PRINT(F("Samsung: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 68 or 100 or 6"));
        return false;
    }

    if (!checkHeader(&SamsungProtocolConstants)) {
        return false;
    }

    // Check for SansungLG style repeat
    if (decodedIRData.rawDataPtr->rawlen == 6) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
        decodedIRData.address = lastDecodedAddress;
        decodedIRData.command = lastDecodedCommand;
        decodedIRData.protocol = SAMSUNG_LG;
        return true;
    }

    /*
     * Decode first 32 bits
     */
    if (!decodePulseDistanceData(&SamsungProtocolConstants, SAMSUNG_BITS)) {
        IR_DEBUG_PRINT(F("Samsung: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;
    decodedIRData.address = tValue.UWord.LowWord;

    if (decodedIRData.rawDataPtr->rawlen == (2 * SAMSUNG48_BITS) + 4) {
        /*
         * Samsung48
         */

        // decode additional 16 bit
        if (!decodePulseDistanceData(&SamsungProtocolConstants, (SAMSUNG_COMMAND32_BITS - SAMSUNG_COMMAND16_BITS),
                3 + SAMSUNG_BITS)) {
            IR_DEBUG_PRINT(F("Samsung: "));
            IR_DEBUG_PRINTLN(F("Decode failed"));
            return false;
        }

//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
        // Put latest (MSB) bits in LowWord, LSB first would have them in HighWord so keep this in mind for decoding below
        tValue.UWord.LowWord = decodedIRData.decodedRawData;

        /*
         * Check parity
         */
        // receive 2 * (8 bits then 8 inverted bits) LSB first
        if (tValue.UByte.HighByte != (uint8_t)(~tValue.UByte.MidHighByte)
                && tValue.UByte.MidLowByte != (uint8_t)(~tValue.UByte.LowByte)) {
            decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_LSB_FIRST;
        }

        decodedIRData.command = tValue.UByte.MidLowByte << 8 | tValue.UByte.HighByte; // low and high word are swapped here, so fetch it this way
        decodedIRData.numberOfBits = SAMSUNG48_BITS;

    } else {
        /*
         * Samsung32
         */
        if (tValue.UByte.MidHighByte == (uint8_t)(~tValue.UByte.HighByte)) {
            // 8 bit command protocol
            decodedIRData.command = tValue.UByte.MidHighByte; // first 8 bit
        } else {
            // 16 bit command protocol
            decodedIRData.command = tValue.UWord.HighWord; // first 16 bit
        }
        decodedIRData.numberOfBits = SAMSUNG_BITS;
    }
    decodedIRData.protocol = SAMSUNG;

    // check for repeat
    checkForRepeatSpaceAndSetFlag(SAMSUNG_REPEAT_SPACE / MICROS_IN_ONE_MILLI);

    return true;
}

// Old version with MSB first
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

    if (!decodePulseDistanceData(SAMSUNG_BITS, offset, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE,
            PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }

    // Success
    aResults->value = decodedIRData.decodedRawData;
    aResults->bits = SAMSUNG_BITS;
    aResults->decode_type = SAMSUNG;
    decodedIRData.protocol = SAMSUNG;
    return true;
}

// Old version with MSB first
void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut (SAMSUNG_KHZ);

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Old version with MSB first Data + stop bit
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, data, nbits,
            PROTOCOL_IS_MSB_FIRST, SEND_STOP_BIT);
    IrReceiver.restartAfterSend();
}

/** @}*/
#endif // _IR_SAMSUNG_HPP
