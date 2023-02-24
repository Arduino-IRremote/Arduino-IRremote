/*
 * ir_NEC.hpp
 *
 *  Contains functions for receiving and sending NEC IR Protocol in "raw" and standard format with 16 or 8 bit address and 8 bit command
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
#ifndef _IR_NEC_HPP
#define _IR_NEC_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC
//==============================================================================
/*
 Protocol=NEC Address=0x4 Command=0x8 Raw-Data=0xF708FB04 32 bits LSB first
 +8950,-4450
 + 600,- 500 + 650,- 500 + 600,-1650 + 600,- 550
 + 600,- 500 + 600,- 500 + 650,- 500 + 600,- 500
 + 650,-1650 + 600,-1600 + 650,- 500 + 600,-1650
 + 600,-1650 + 600,-1650 + 600,-1600 + 650,-1600
 + 650,- 500 + 600,- 550 + 600,- 500 + 600,-1650
 + 600,- 550 + 600,- 500 + 600,- 550 + 600,- 500
 + 600,-1650 + 600,-1650 + 600,-1650 + 600,- 550
 + 600,-1650 + 600,-1650 + 600,-1650 + 600,-1600
 + 650
 Sum: 68000

 Protocol=NEC Address=0x8 Command=0x7 Repeat gap=40900us
 rawData[4]:
 -40900
 +10450,-2250
 + 700
 Sum: 13400
 */
// http://www.hifi-remote.com/wiki/index.php/NEC
// https://www.sbprojects.net/knowledge/ir/nec.php
// for Apple see https://en.wikipedia.org/wiki/Apple_Remote - Fixed address 0x87EE, 8 bit device ID, 7 bit command, 1 bit parity - untested!
// ONKYO like NEC but 16 independent command bits
// PIONEER (not implemented) is NEC2 with 40 kHz
// LSB first, 1 start bit + 16 bit address (or 8 bit address and 8 bit inverted address) + 8 bit command + 8 bit inverted command + 1 stop bit.
// Standard NEC sends a special fixed repeat frame.
// NEC2 sends the same full frame after the 110 ms. I have a DVD remote with NEC2.
// NEC and NEC 2 only differ in the repeat frames, so the protocol can only be detected correctly after the first repeat.
//
// IRP: NEC  {38.0k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,^108m,(16,-4,1,^108m)*)  ==> "*" means send special repeat frames o ore more times
// IRP: NEC2 {38.0k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,^108m)+   ==> "+" means send frame 1 or more times (special repeat is missing here!)
// {38.0k,564} ==> 38.0k -> Frequency , 564 -> unit in microseconds (we use 560), no "msb", so "lsb" is assumed
// <1,-1|1,-3> ==> Zero is 1 unit mark and space | One is 1 unit mark and 3 units space
// 16,-8 ==> Start bit durations
// D:8,S:8,F:8,~F:8 ==> D:8 -> 8 bit bitfield for Device, S:8 -> 8 bit bitfield for Subdevice, F:8 -> 8 bit bitfield for Function, ~F:8 -> 8 bit inverted bitfield for Function
// 1,^108m ==> 1 -> unit mark Stop bit, ^108m -> wait until 108 milliseconds after start of protocol (we use 110)
//
#define NEC_ADDRESS_BITS        16 // 16 bit address or 8 bit address and 8 bit inverted address
#define NEC_COMMAND_BITS        16 // Command and inverted command

#define NEC_BITS                (NEC_ADDRESS_BITS + NEC_COMMAND_BITS)
#define NEC_UNIT                560             // 21.28 periods of 38 kHz, 11.2 ticks TICKS_LOW = 8.358 TICKS_HIGH = 15.0

#define NEC_HEADER_MARK         (16 * NEC_UNIT) // 9000 | 180
#define NEC_HEADER_SPACE        (8 * NEC_UNIT)  // 4500 | 90

#define NEC_BIT_MARK            NEC_UNIT
#define NEC_ONE_SPACE           (3 * NEC_UNIT)  // 1690 | 33.8  TICKS_LOW = 25.07 TICKS_HIGH = 45.0
#define NEC_ZERO_SPACE          NEC_UNIT

#define NEC_REPEAT_HEADER_SPACE (4 * NEC_UNIT)  // 2250

#define NEC_AVERAGE_DURATION    62000 // NEC_HEADER_MARK + NEC_HEADER_SPACE + 32 * 2,5 * NEC_UNIT + NEC_UNIT // 2.5 because we assume more zeros than ones
#define NEC_MINIMAL_DURATION    49900 // NEC_HEADER_MARK + NEC_HEADER_SPACE + 32 * 2 * NEC_UNIT + NEC_UNIT // 2.5 because we assume more zeros than ones
#define NEC_REPEAT_DURATION     (NEC_HEADER_MARK  + NEC_REPEAT_HEADER_SPACE + NEC_BIT_MARK) // 12 ms
#define NEC_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
#define NEC_REPEAT_DISTANCE         (NEC_REPEAT_PERIOD - NEC_AVERAGE_DURATION) // 48 ms
#define NEC_MAXIMUM_REPEAT_DISTANCE (NEC_REPEAT_PERIOD - NEC_MINIMAL_DURATION + 10000) // 70 ms

#define APPLE_ADDRESS           0x87EE

struct PulseDistanceWidthProtocolConstants NECProtocolConstants =
        { NEC, NEC_KHZ, NEC_HEADER_MARK, NEC_HEADER_SPACE, NEC_BIT_MARK,
        NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST, (NEC_REPEAT_PERIOD / MICROS_IN_ONE_MILLI),
                &sendNECSpecialRepeat };

// Like NEC but repeats are full frames instead of special NEC repeats
struct PulseDistanceWidthProtocolConstants NEC2ProtocolConstants = { NEC2, NEC_KHZ, NEC_HEADER_MARK, NEC_HEADER_SPACE, NEC_BIT_MARK,
NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST, (NEC_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), NULL };

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * Send special NEC repeat frame
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendNECRepeat() {
    enableIROut (NEC_KHZ);           // 38 kHz
    mark(NEC_HEADER_MARK);          // + 9000
    space(NEC_REPEAT_HEADER_SPACE); // - 2250
    mark(NEC_BIT_MARK);             // + 560
}

/**
 * Static function for sending special repeat frame.
 * For use in ProtocolConstants. Saves up to 250 bytes compared to a member function.
 */
void sendNECSpecialRepeat() {
    IrSender.enableIROut(NEC_KHZ);           // 38 kHz
    IrSender.mark(NEC_HEADER_MARK);          // + 9000
    IrSender.space(NEC_REPEAT_HEADER_SPACE); // - 2250
    IrSender.mark(NEC_BIT_MARK);             // + 560
}

uint32_t IRsend::computeNECRawDataAndChecksum(uint16_t aAddress, uint16_t aCommand) {
    LongUnion tRawData;

    // Address 16 bit LSB first
    if ((aAddress & 0xFF00) == 0) {
        // assume 8 bit address -> send 8 address bits and then 8 inverted address bits LSB first
        tRawData.UByte.LowByte = aAddress;
        tRawData.UByte.MidLowByte = ~tRawData.UByte.LowByte;
    } else {
        tRawData.UWord.LowWord = aAddress;
    }

    // send 8 command bits and then 8 inverted command bits LSB first
    tRawData.UByte.MidHighByte = aCommand;
    tRawData.UByte.HighByte = ~aCommand;
    return tRawData.ULong;
}

/**
 * NEC Send frame and special repeats
 * There is NO delay after the last sent repeat!
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame without leading and trailing space
 *                          will be sent by calling NECProtocolConstants.SpecialSendRepeatFunction().
 */
void IRsend::sendNEC(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&NECProtocolConstants, computeNECRawDataAndChecksum(aAddress, aCommand), NEC_BITS, aNumberOfRepeats);
}

/*
 * NEC2 Send frame and repeat the frame for each requested repeat
 * There is NO delay after the last sent repeat!
 * @param aNumberOfRepeats  If < 0 then nothing is sent.
 */
void IRsend::sendNEC2(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&NEC2ProtocolConstants, computeNECRawDataAndChecksum(aAddress, aCommand), NEC_BITS, aNumberOfRepeats);
}

/*
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame without leading and trailing space
 *                          will be sent by calling NECProtocolConstants.SpecialSendRepeatFunction().
 */
void IRsend::sendOnkyo(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&NECProtocolConstants, (uint32_t) aCommand << 16 | aAddress, NEC_BITS, aNumberOfRepeats);
}

/*
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 * https://en.wikipedia.org/wiki/Apple_Remote
 * https://gist.github.com/darconeous/4437f79a34e3b6441628
 * @param aAddress is the DeviceId*
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame without leading and trailing space
 *                          will be sent by calling NECProtocolConstants.SpecialSendRepeatFunction().
 */
void IRsend::sendApple(uint8_t aDeviceId, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {
    LongUnion tRawData;

    // Address 16 bit LSB first
    tRawData.UWord.LowWord = APPLE_ADDRESS;

    // send Apple code and then 8 command bits LSB first
    tRawData.UByte.MidHighByte = aCommand;
    tRawData.UByte.HighByte = aDeviceId; // e.g. 0xD7

    sendPulseDistanceWidth(&NECProtocolConstants, tRawData.ULong, NEC_BITS, aNumberOfRepeats);
}

/*
 * Sends NEC1 protocol
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame without leading and trailing space
 *                          will be sent by calling NECProtocolConstants.SpecialSendRepeatFunction().
 */
void IRsend::sendNECRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&NECProtocolConstants, aRawData, NEC_BITS, aNumberOfRepeats);
}

/**
 * Decodes also Onkyo and Apple
 */
bool IRrecv::decodeNEC() {
    /*
     * First check for right data length
     * Next check start bit
     * Next try the decode
     */
    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawDataPtr->rawlen != ((2 * NEC_BITS) + 4) && (decodedIRData.rawDataPtr->rawlen != 4)) {
        IR_DEBUG_PRINT(F("NEC: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 68 or 4"));
        return false;
    }

    // Check header "mark" this must be done for repeat and data
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], NEC_HEADER_MARK)) {
        return false;
    }

    // Check for repeat - here we have another header space length
    if (decodedIRData.rawDataPtr->rawlen == 4) {
        if (matchSpace(decodedIRData.rawDataPtr->rawbuf[2], NEC_REPEAT_HEADER_SPACE)
                && matchMark(decodedIRData.rawDataPtr->rawbuf[3], NEC_BIT_MARK)) {
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            decodedIRData.protocol = lastDecodedProtocol;
            return true;
        }
        return false;
    }

    // Check command header space
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[2], NEC_HEADER_SPACE)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("NEC: "));
        Serial.println(F("Header space length is wrong"));
#endif
        return false;
    }

    // Try to decode as NEC protocol
    if (!decodePulseDistanceWidthData(&NECProtocolConstants, NEC_BITS)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("NEC: "));
        Serial.println(F("Decode failed"));
#endif
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;
    decodedIRData.command = tValue.UByte.MidHighByte; // 8 bit
    // Address
    if (tValue.UWord.LowWord == APPLE_ADDRESS) {
        /*
         * Apple
         */
        decodedIRData.protocol = APPLE;
        decodedIRData.address = tValue.UByte.HighByte;

    } else {
        /*
         * NEC LSB first, so first sent bit is also LSB of decodedIRData.decodedRawData
         */
        if (tValue.UByte.LowByte == (uint8_t)(~tValue.UByte.MidLowByte)) {
            // standard 8 bit address NEC protocol
            decodedIRData.address = tValue.UByte.LowByte; // first 8 bit
        } else {
            // extended NEC protocol
            decodedIRData.address = tValue.UWord.LowWord; // first 16 bit
        }
        // Check for command if it is 8 bit NEC or 16 bit ONKYO
        if (tValue.UByte.MidHighByte == (uint8_t)(~tValue.UByte.HighByte)) {
            decodedIRData.protocol = NEC;
        } else {
            decodedIRData.protocol = ONKYO;
            decodedIRData.command = tValue.UWord.HighWord; // 16 bit command
        }
    }
    decodedIRData.numberOfBits = NEC_BITS;

    // check for NEC2 repeat, do not check for same content ;-)
    checkForRepeatSpaceTicksAndSetFlag(NEC_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);
    if (decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
        decodedIRData.protocol = NEC2;
    }
    return true;
}

/*********************************************************************************
 * Old deprecated functions, kept for backward compatibility to old 2.0 tutorials
 *********************************************************************************/

bool IRrecv::decodeNECMSB(decode_results *aResults) {
    unsigned int offset = 1;  // Index in to results; Skip first space.

// Check header "mark"
    if (!matchMark(aResults->rawbuf[offset], NEC_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat
    if ((aResults->rawlen == 4) && matchSpace(aResults->rawbuf[offset], NEC_REPEAT_HEADER_SPACE)
            && matchMark(aResults->rawbuf[offset + 1], NEC_BIT_MARK)) {
        aResults->bits = 0;
        aResults->value = 0xFFFFFFFF;
        decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = NEC;
        return true;
    }

    // Check we have the right amount of data (32). +4 for initial gap, start bit mark and space + stop bit mark
    if (aResults->rawlen != (2 * NEC_BITS) + 4) {
        IR_DEBUG_PRINT(F("NEC MSB: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(aResults->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 68"));
        return false;
    }

// Check header "space"
    if (!matchSpace(aResults->rawbuf[offset], NEC_HEADER_SPACE)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("NEC MSB: "));
        Serial.println(F("Header space length is wrong"));
#endif
        return false;
    }
    offset++;

    if (!decodePulseDistanceWidthData(NEC_BITS, offset, NEC_BIT_MARK, 0, NEC_ONE_SPACE, NEC_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("NEC MSB: "));
        Serial.println(F("Decode failed"));
#endif
        return false;
    }

    // Stop bit
    if (!matchMark(aResults->rawbuf[offset + (2 * NEC_BITS)], NEC_BIT_MARK)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("NEC MSB: "));
        Serial.println(F("Stop bit mark length is wrong"));
#endif
        return false;
    }

// Success
    aResults->value = decodedIRData.decodedRawData;
    aResults->bits = NEC_BITS;
    aResults->decode_type = NEC;
    decodedIRData.protocol = NEC;

    return true;
}

/**
 * With Send sendNECMSB() you can send your old 32 bit codes.
 * To convert one into the other, you must reverse the byte positions and then reverse all bit positions of each byte.
 * Or write it as one binary string and reverse/mirror it.
 * Example:
 * 0xCB340102 byte reverse -> 02 01 34 CB bit reverse-> 40 80 2C D3.
 * 0xCB340102 is binary 11001011001101000000000100000010.
 * 0x40802CD3 is binary 01000000100000000010110011010011.
 * If you read the first binary sequence backwards (right to left), you get the second sequence.
 */
void IRsend::sendNECMSB(uint32_t data, uint8_t nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut (NEC_KHZ);

    if (data == 0xFFFFFFFF || repeat) {
        sendNECRepeat();
        return;
    }

    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);

    // Old version with MSB first Data + stop bit
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST);
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_NEC_HPP
