/*
 * ir_Sony.hpp
 *
 *  Contains functions for receiving and sending SIRCS/Sony IR Protocol in "raw" and standard format with 5 bit address 7 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
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
#ifndef _IR_SONY_HPP
#define _IR_SONY_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//                           SSSS   OOO   N   N  Y   Y
//                          S      O   O  NN  N   Y Y
//                           SSS   O   O  N N N    Y
//                              S  O   O  N  NN    Y
//                          SSSS    OOO   N   N    Y
//==============================================================================
/*
 * Protocol=Sony Address=0x4B9 Command=0x7 Raw-Data=0x25C87 20 bits LSB first
 +2550,- 400
 +1300,- 450 +1350,- 450 +1300,- 450 + 700,- 450
 + 700,- 450 + 750,- 450 + 700,- 400
 --13 address bits--
 +1300,- 500
 + 700,- 450 + 700,- 450 +1300,- 500 +1300,- 450
 +1300,- 450 + 700,- 450 +1350,- 400 + 750,- 450
 + 700,- 450 +1300,- 450 + 700,- 450 + 700
 Sum: 31100
 */
// see https://www.sbprojects.net/knowledge/ir/sirc.php
// Here http://picprojects.org.uk/projects/sirc/ it is claimed, that many Sony remotes repeat each frame a minimum of 3 times
// LSB first, start bit + 7 command + 5 to 13 address, no stop bit
// IRP: Sony12 {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,^45m)+ ==> 40 kHz, Unit is 600, LSB, One mark is 2 units, Start bit is 4 units, 7 bit Function, 5 bit Device, no Stop bit, every 45 milliseconds
// IRP: Sony15 {40k,600}<1,-1|2,-1>(4,-1,F:7,D:8,^45m)+ ==> 8 bit Device
// IRP: Sony20 {40k,600}<1,-1|2,-1>(4,-1,F:7,D:5,S:8,^45m)+ ==> 5 bit Device, 8 bit Subdevice
//
#define SONY_ADDRESS_BITS       5
#define SONY_COMMAND_BITS       7
#define SONY_EXTRA_BITS         8
#define SONY_BITS_MIN           (SONY_COMMAND_BITS + SONY_ADDRESS_BITS)        // 12 bits
#define SONY_BITS_15            (SONY_COMMAND_BITS + SONY_ADDRESS_BITS + 3)    // 15 bits
#define SONY_BITS_MAX           (SONY_COMMAND_BITS + SONY_ADDRESS_BITS + SONY_EXTRA_BITS)    // 20 bits == SIRCS_20_PROTOCOL
#define SONY_UNIT               600 // 24 periods of 40kHz

#define SONY_HEADER_MARK        (4 * SONY_UNIT) // 2400
#define SONY_ONE_MARK           (2 * SONY_UNIT) // 1200
#define SONY_ZERO_MARK          SONY_UNIT
#define SONY_SPACE              SONY_UNIT

#define SONY_AVERAGE_DURATION_MIN   21000 // SONY_HEADER_MARK + SONY_SPACE  + 12 * 2,5 * SONY_UNIT  // 2.5 because we assume more zeros than ones
#define SONY_AVERAGE_DURATION_MAX   33000 // SONY_HEADER_MARK + SONY_SPACE  + 20 * 2,5 * SONY_UNIT  // 2.5 because we assume more zeros than ones
#define SONY_REPEAT_PERIOD          45000 // Commands are repeated every 45 ms (measured from start to start) for as long as the key on the remote control is held down.
#define SONY_REPEAT_SPACE_MAX       (SONY_REPEAT_PERIOD - SONY_AVERAGE_DURATION_MIN) // 24 ms

/*
 * Repeat commands should be sent in a 45 ms raster.
 * There is NO delay after the last sent command / repeat!
 * @param numberOfBits if == 20 send 13 address bits otherwise only 5 address bits
 */
void IRsend::sendSony(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, uint8_t numberOfBits) {
    uint32_t tData = (uint32_t)aAddress << 7 | aCommand;
    // send 5, 8, 13 address bits LSB first
    sendPulseDistanceWidth(SONY_KHZ, SONY_HEADER_MARK, SONY_SPACE, SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE, tData,
            numberOfBits, PROTOCOL_IS_LSB_FIRST, SEND_NO_STOP_BIT, SONY_REPEAT_PERIOD / MICROS_IN_ONE_MILLI, aNumberOfRepeats);
}

//+=============================================================================

bool IRrecv::decodeSony() {

    // Check header "mark"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], SONY_HEADER_MARK)) {
        return false;
    }

    // Check we have enough data. +2 for initial gap and start bit mark and space minus the last/MSB space. NO stop bit! 26, 32, 42
    if (decodedIRData.rawDataPtr->rawlen != (2 * SONY_BITS_MIN) + 2 && decodedIRData.rawDataPtr->rawlen != (2 * SONY_BITS_MAX) + 2
            && decodedIRData.rawDataPtr->rawlen != (2 * SONY_BITS_15) + 2) {
        // ??? IR_TRACE_PRINT since I saw this too often
        IR_DEBUG_PRINT(F("Sony: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 12, 15 or 20"));
        return false;
    }

    // Check header "space"
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[2], SONY_SPACE)) {
        IR_DEBUG_PRINT(F("Sony: "));
        IR_DEBUG_PRINTLN(F("Header space length is wrong"));
        return false;
    }

    if (!decodePulseWidthData((decodedIRData.rawDataPtr->rawlen - 1) / 2, 3, SONY_ONE_MARK, SONY_ZERO_MARK, SONY_SPACE,
    PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Sony: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    uint8_t tCommand = decodedIRData.decodedRawData & 0x7F;  // first 7 bits
    uint16_t tAddress = decodedIRData.decodedRawData >> 7;    // next 5 or 8 or 13 bits

    /*
     *  Check for repeat
     */
    if (decodedIRData.rawDataPtr->rawbuf[0] < ((SONY_REPEAT_SPACE_MAX + (SONY_REPEAT_SPACE_MAX / 4)) / MICROS_PER_TICK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
    }
    decodedIRData.command = tCommand;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = (decodedIRData.rawDataPtr->rawlen - 1) / 2;
    decodedIRData.protocol = SONY;

    return true;
}

#define SONY_DOUBLE_SPACE_USECS    500 // usually see 713 - not using ticks as get number wrap around
bool IRrecv::decodeSonyMSB(decode_results *aResults) {
    long data = 0;
    uint8_t bits = 0;
    unsigned int offset = 0;  // Dont skip first space, check its size

    if (aResults->rawlen < (2 * SONY_BITS_MIN) + 2) {
        return false;
    }

    // Some Sony's deliver repeats fast after first
    // unfortunately can't spot difference from of repeat from two fast clicks
    if (aResults->rawbuf[0] < (SONY_DOUBLE_SPACE_USECS / MICROS_PER_TICK)) {
        IR_DEBUG_PRINTLN(F("IR Gap found"));
        aResults->bits = 0;
        aResults->value = 0xFFFFFFFF;
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = SONY;
        return true;
    }
    offset++;

    // Check header "mark"
    if (!matchMark(aResults->rawbuf[offset], SONY_HEADER_MARK)) {
        return false;
    }
    offset++;

    // MSB first - Not compatible to standard, which says LSB first :-(
    while (offset + 1 < aResults->rawlen) {

        // First check for the constant space length, we do not have a space at the end of raw data
        // we are lucky, since the start space is equal the data space.
        if (!matchSpace(aResults->rawbuf[offset], SONY_SPACE)) {
            return false;
        }
        offset++;

        // bit value is determined by length of the mark
        if (matchMark(aResults->rawbuf[offset], SONY_ONE_MARK)) {
            data = (data << 1) | 1;
        } else if (matchMark(aResults->rawbuf[offset], SONY_ZERO_MARK)) {
            data = (data << 1) | 0;
        } else {
            return false;
        }
        offset++;
        bits++;

    }

    aResults->bits = bits;
    aResults->value = data;
    aResults->decode_type = SONY;
    decodedIRData.protocol = SONY;
    return true;
}

/**
 * Old version with MSB first data
 */
void IRsend::sendSony(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(SONY_KHZ);

    // Header
    mark(SONY_HEADER_MARK);
    space(SONY_SPACE);

    // Old version with MSB first Data
    sendPulseDistanceWidthData(SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST,
    SEND_NO_STOP_BIT);
    IrReceiver.restartAfterSend();
}

/** @}*/
#endif // _IR_SONY_HPP
