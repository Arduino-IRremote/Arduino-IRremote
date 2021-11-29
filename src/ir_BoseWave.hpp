/*
 * ir_BoseWave.cpp
 *
 *  Contains functions for receiving and sending Bose IR Protocol
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 */
#ifndef IR_BOSEWAVE_HPP
#define IR_BOSEWAVE_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//                           BBBB    OOO    SSSS  EEEEE
//                           B   B  O   O  S      E
//                           BB B   O   O   SSS   EEEE
//                           B   B  O   O      S  E
//                           BBBB    OOO   SSSS   EEEEE
//==============================================================================
// see http://lirc.sourceforge.net/remotes/bose/WAVERADIO
//
// Support for Bose Wave Radio CD initially provided by https://github.com/uvotguy.
//
// As seen on my trusty oscilloscope, there is no repeat code.  Instead, when I
// press and hold a button on my remote, it sends a command, makes a 51.2ms space,
// and resends the command, etc, etc.

// LSB first, 1 start bit + 8 bit data + 8 bit inverted data + 1 stop bit.
#define BOSEWAVE_BITS             16 // Command and inverted command

#define BOSEWAVE_HEADER_MARK    1060
#define BOSEWAVE_HEADER_SPACE   1450
#define BOSEWAVE_BIT_MARK        534
#define BOSEWAVE_ONE_SPACE       468
#define BOSEWAVE_ZERO_SPACE     1447

#define BOSEWAVE_REPEAT_SPACE  52000

//+=============================================================================

void IRsend::sendBoseWave(uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(BOSEWAVE_KHZ); // 38 kHz

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // Header
        mark(BOSEWAVE_HEADER_MARK);
        space(BOSEWAVE_HEADER_SPACE);
        // send 8 command bits and then 8 inverted command bits LSB first
        uint16_t tData = ((~aCommand) << 8) | aCommand;

        sendPulseDistanceWidthData(BOSEWAVE_BIT_MARK, BOSEWAVE_ONE_SPACE, BOSEWAVE_BIT_MARK, BOSEWAVE_ZERO_SPACE, tData,
        BOSEWAVE_BITS, PROTOCOL_IS_LSB_FIRST, SEND_STOP_BIT);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command with a fixed space gap
            delay( BOSEWAVE_REPEAT_SPACE / MICROS_IN_ONE_MILLI);
        }
    }
}

//+=============================================================================
bool IRrecv::decodeBoseWave() {

    // Check header "mark"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], BOSEWAVE_HEADER_MARK)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check we have enough data +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != (2 * BOSEWAVE_BITS) + 4) {
        IR_DEBUG_PRINT("Bose: ");
        IR_DEBUG_PRINT("Data length=");
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(" is not 36");
        return false;
    }
    // Check header "space"
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[2], BOSEWAVE_HEADER_SPACE)) {
        IR_DEBUG_PRINT("Bose: ");
        IR_DEBUG_PRINTLN("Header space length is wrong");
        return false;
    }

    if (!decodePulseDistanceData(BOSEWAVE_BITS, 3, BOSEWAVE_BIT_MARK, BOSEWAVE_ONE_SPACE, BOSEWAVE_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT("Bose: ");
        IR_DEBUG_PRINTLN("Decode failed");
        return false;
    }

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * BOSEWAVE_BITS)], BOSEWAVE_BIT_MARK)) {
        IR_DEBUG_PRINT("Bose: ");
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    uint16_t tDecodedValue = decodedIRData.decodedRawData;
    uint8_t tCommandNotInverted = tDecodedValue & 0xFF;
    uint8_t tCommandInverted = tDecodedValue >> 8;
    // parity check for command. Use this variant to avoid compiler warning "comparison of promoted ~unsigned with unsigned [-Wsign-compare]"
    if ((tCommandNotInverted ^ tCommandInverted) != 0xFF) {
        IR_DEBUG_PRINT("Bose: ");
        IR_DEBUG_PRINT("Command and inverted command check failed");
        return false;
    }

    // check for repeat
    if (decodedIRData.rawDataPtr->rawbuf[0] < ((BOSEWAVE_REPEAT_SPACE + (BOSEWAVE_REPEAT_SPACE / 4)) / MICROS_PER_TICK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
    }

    decodedIRData.command = tCommandNotInverted;
    decodedIRData.protocol = BOSEWAVE;
    decodedIRData.numberOfBits = BOSEWAVE_BITS;

    return true;
}

/** @}*/
#endif // #ifndef IR_BOSEWAVE_HPP
#pragma once
