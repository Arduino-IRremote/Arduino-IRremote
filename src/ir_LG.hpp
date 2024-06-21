/*
 * ir_LG.hpp
 *
 *  Contains functions for receiving and sending LG IR Protocol for air conditioner
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2024 Darryl Smith, Armin Joachimsmeyer
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

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

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
/*
 * Protocol=LG Address=0xF1 Command=0x7776 Raw-Data=0xF17776B 28 bits MSB first
 +8950,-4150
 + 500,-1550 + 550,-1550 + 500,-1550 + 500,-1600
 + 500,- 700 + 350,- 600 + 450,- 600 + 450,-1550
 + 500,- 550 + 500,-1550 + 500,-1600 + 500,-1550
 + 550,- 550 + 500,-1550 + 500,-1550 + 550,-1550
 + 500,- 550 + 500,-1550 + 500,-1600 + 500,-1550
 + 500,- 550 + 500,-1550 + 500,-1600 + 500,- 550
 + 500,-1550 + 500,- 600 + 450,-1600 + 500,-1550
 + 500
 Sum: 62400
 */

// LG originally added by Darryl Smith (based on the JVC protocol)
// see: https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/examples/LGAirConditionerSendDemo
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#LGAIR
// MSB first, 1 start bit + 8 bit address + 16 bit command + 4 bit checksum + 1 stop bit (28 data bits).
// Bit and repeat timing is like NEC
// LG2 has different header timing and a shorter bit time
/*
 * LG remote IR-LED measurements: Type AKB 73315611 for air conditioner, Ver1.1 from 2011.03.01
 * Protocol: LG2
 * Internal crystal: 4 MHz
 * Header:  8.9 ms mark 4.15 ms space
 * Data:    500 / 540 and 500 / 1580;
 * Clock is not synchronized with gate so you have 19 and sometimes 19 and a spike pulses for mark
 * Duty:    9 us on 17 us off => around 33 % duty
 * NO REPEAT: If value like temperature has changed during long press, the last value is send at button release.
 * If you do a double press, the next value can be sent after around 118 ms. Tested with the fan button.

 * LG remote IR-LED measurements: Type AKB 75095308 for LG TV
 * Protocol: NEC!!!
 * Frequency 37.88 kHz
 * Header:  9.0 ms mark 4.5 ms space
 * Data:    560 / 560 and 560 / 1680;
 * Clock is synchronized with gate, mark always starts with a full period
 * Duty:    13 us on 13 us off => 50 % duty
 * Repeat:  110 ms 9.0 ms mark, 2250 us space, 560 stop
 * LSB first!
 *
 * The codes of the LG air conditioner are documented in https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/ac_LG.cpp
 */
#define LG_ADDRESS_BITS          8
#define LG_COMMAND_BITS         16
#define LG_CHECKSUM_BITS         4
#define LG_BITS                 (LG_ADDRESS_BITS + LG_COMMAND_BITS + LG_CHECKSUM_BITS) // 28

#define LG_UNIT                 500 // 19 periods of 38 kHz

#define LG_HEADER_MARK          (18 * LG_UNIT) // 9000
#define LG_HEADER_SPACE         4200           // 4200 | 84

#define LG2_HEADER_MARK         (19 * LG_UNIT) // 9500
#define LG2_HEADER_SPACE        (6 * LG_UNIT)  // 3000

#define LG_BIT_MARK             LG_UNIT
#define LG_ONE_SPACE            1580  // 60 periods of 38 kHz
#define LG_ZERO_SPACE           550

#define LG_REPEAT_HEADER_SPACE  (4 * LG_UNIT)  // 2250
#define LG_REPEAT_PERIOD        110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
//#define LG_AVERAGE_DURATION     58000 // LG_HEADER_MARK + LG_HEADER_SPACE  + 32 * 2,5 * LG_UNIT) + LG_UNIT // 2.5 because we assume more zeros than ones
//#define LG_REPEAT_DURATION      (LG_HEADER_MARK  + LG_REPEAT_HEADER_SPACE + LG_BIT_MARK)
//#define LG_REPEAT_DISTANCE      (LG_REPEAT_PERIOD - LG_AVERAGE_DURATION) // 52 ms

struct PulseDistanceWidthProtocolConstants LGProtocolConstants = { LG, LG_KHZ, LG_HEADER_MARK, LG_HEADER_SPACE, LG_BIT_MARK,
LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST, (LG_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), &sendNECSpecialRepeat };

struct PulseDistanceWidthProtocolConstants LG2ProtocolConstants = { LG2, LG_KHZ, LG2_HEADER_MARK, LG2_HEADER_SPACE, LG_BIT_MARK,
LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST, (LG_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), &sendLG2SpecialRepeat };

/************************************
 * Start of send and decode functions
 ************************************/
/*
 * Send special LG2 repeat - not used internally
 */
void IRsend::sendLG2Repeat() {
    enableIROut (LG_KHZ);            // 38 kHz
    mark(LG2_HEADER_MARK);          // + 3000
    space(LG_REPEAT_HEADER_SPACE);  // - 2250
    mark(LG_BIT_MARK);              // + 500
}

/**
 * Static function for sending special repeat frame.
 * For use in ProtocolConstants. Saves up to 250 bytes compared to a member function.
 */
void sendLG2SpecialRepeat() {
    IrSender.enableIROut(LG_KHZ);            // 38 kHz
    IrSender.mark(LG2_HEADER_MARK);          // + 3000
    IrSender.space(LG_REPEAT_HEADER_SPACE);  // - 2250
    IrSender.mark(LG_BIT_MARK);              // + 500
}

uint32_t IRsend::computeLGRawDataAndChecksum(uint8_t aAddress, uint16_t aCommand) {
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
    return (tRawData | (tChecksum & 0xF));
}

/**
 * LG uses the NEC repeat.
 */
void IRsend::sendLG(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&LGProtocolConstants, computeLGRawDataAndChecksum(aAddress, aCommand), LG_BITS, aNumberOfRepeats);
}

/**
 * LG2 uses a special repeat.
 */
void IRsend::sendLG2(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&LG2ProtocolConstants, computeLGRawDataAndChecksum(aAddress, aCommand), LG_BITS, aNumberOfRepeats);
}

bool IRrecv::decodeLG() {
    decode_type_t tProtocol = LG;
    uint16_t tHeaderSpace = LG_HEADER_SPACE;

    /*
     * First check for right data length
     * Next check start bit
     * Next try the decode
     */

// Check we have the right amount of data (60). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawlen != ((2 * LG_BITS) + 4) && (decodedIRData.rawlen != 4)) {
        IR_DEBUG_PRINT(F("LG: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 60 or 4"));
        return false;
    }

// Check header "mark" this must be done for repeat and data
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], LG_HEADER_MARK)) {
        if (matchMark(decodedIRData.rawDataPtr->rawbuf[1], LG2_HEADER_MARK)) {
            tProtocol = LG2;
            tHeaderSpace = LG2_HEADER_SPACE;
        } else {
#if defined(LOCAL_DEBUG)
            Serial.print(F("LG: "));
            Serial.println(F("Header mark is wrong"));
#endif
            return false; // neither LG nor LG2 header
        }
    }

// Check for repeat - here we have another header space length
    if (decodedIRData.rawlen == 4) {
        if (matchSpace(decodedIRData.rawDataPtr->rawbuf[2], LG_REPEAT_HEADER_SPACE)
                && matchMark(decodedIRData.rawDataPtr->rawbuf[3], LG_BIT_MARK)) {
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_MSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            decodedIRData.protocol = lastDecodedProtocol;
            return true;
        }
#if defined(LOCAL_DEBUG)
        Serial.print(F("LG: "));
        Serial.print(F("Repeat header space is wrong"));
#endif
        return false;
    }

// Check command header space
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[2], tHeaderSpace)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("LG: "));
        Serial.println(F("Header space length is wrong"));
#endif
        return false;
    }

    if (!decodePulseDistanceWidthData(&LGProtocolConstants, LG_BITS)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("LG: "));
        Serial.println(F("Decode failed"));
#endif
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
#if defined(LOCAL_DEBUG)
        Serial.print(F("LG: "));
        Serial.print(F("4 bit checksum is not correct. expected=0x"));
        Serial.print(tChecksum, HEX);
        Serial.print(F(" received=0x"));
        Serial.print((decodedIRData.decodedRawData & 0xF), HEX);
        Serial.print(F(" data=0x"));
        Serial.println(decodedIRData.command, HEX);
#endif
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
    }

    decodedIRData.protocol = tProtocol; // LG or LG2
    decodedIRData.numberOfBits = LG_BITS;

    return true;
}

/*********************************************************************************
 * Old deprecated functions, kept for backward compatibility to old 2.0 tutorials
 *********************************************************************************/

/**
 * Here you can put your raw data, even one with "wrong" checksum.
 * @param aRawData  The lowest 28 (LG_BITS) bit of this value are sent MSB first.
 * @param aNumberOfRepeats If < 0 then only a special repeat frame will be sent.
 */
void IRsend::sendLGRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth(&LGProtocolConstants, aRawData, LG_BITS, aNumberOfRepeats);
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

    if (!decodePulseDistanceWidthData(LG_BITS, offset, LG_BIT_MARK, LG_ONE_SPACE, 0, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }
// Stop bit
    if (!matchMark(aResults->rawbuf[offset + (2 * LG_BITS)], LG_BIT_MARK)) {
#if defined(LOCAL_DEBUG)
        Serial.println(F("Stop bit mark length is wrong"));
#endif
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
    enableIROut (LG_KHZ);
#if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__))
//    Serial.println(F(
//            "The function sendLG(data, nbits) is deprecated and may not work as expected! Use sendLGRaw(data, NumberOfRepeats) or better sendLG(Address, Command, NumberOfRepeats)."));
#endif
// Header
    mark(LG_HEADER_MARK);
    space(LG_HEADER_SPACE);
//    mark(LG_BIT_MARK);

// Data + stop bit
    sendPulseDistanceWidthData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST);
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_LG_HPP
