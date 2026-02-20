/*
 * ir_OpenLASIR.hpp
 *
 *  Contains functions for receiving and sending the OpenLASIR IR Protocol.
 *  OpenLASIR is a modified NEC protocol with 8-bit validated address (with inverted complement)
 *  and 16-bit command (no error check on command). This is the opposite of NEC Extended,
 *  which has 16-bit address and 8-bit validated command.
 *
 *  Protocol details:
 *    - Bits 0-7:   Address (8 bits, Block ID)
 *    - Bits 8-15:  ~Address (8-bit inverted copy, for error checking)
 *    - Bits 16-31: Command (16 bits, no error check)
 *    - Same timing as NEC: 38 kHz carrier, 9ms leading mark, 4.5ms leading space,
 *      562.5us bit mark, 1687.5us "1" space, 562.5us "0" space, 562.5us stop bit
 *    - LSB first, pulse distance encoding
 *    - Special NEC-style repeat frame (9ms mark + 2.25ms space + stop bit)
 *
 *  See: https://github.com/danielweidman/OpenLASIR
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2025-2026 Daniel Weidmann, Armin Joachimsmeyer
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
#ifndef _IR_OPENLASIR_HPP
#define _IR_OPENLASIR_HPP

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
#include "LocalDebugLevelStart.h"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//       OpenLASIR - Modified NEC Extended Protocol
//       8-bit validated address + 16-bit command
//==============================================================================
/*
 * OpenLASIR uses the same timing as NEC but rearranges the address/command structure:
 *   - 8-bit address with 8-bit inverted complement (for error checking)
 *   - 16-bit command with no error check
 *     Bits  0-7:   Block ID              (8 bits)  ← Address low byte
 *     Bits  8-15:  ~Block ID             (8 bits)  ← Address high byte (inverted, for error check)
 *     Bits 16-23:  Device ID             (8 bits)  ← Command bits 0-7
 *     Bits 24-28:  Mode                  (5 bits)  ← Command bits 8-12
 *     Bits 29-31:  Data (color, etc.)    (3 bits)  ← Command bits 13-15
 *
 * This is the opposite of NEC Extended, which uses 16-bit address and 8-bit validated command.
 *
 * The timing constants are shared with NEC (defined in ir_NEC.hpp):
 *   NEC_UNIT, NEC_HEADER_MARK, NEC_HEADER_SPACE, NEC_BIT_MARK,
 *   NEC_ONE_SPACE, NEC_ZERO_SPACE, NEC_REPEAT_HEADER_SPACE, etc.
 *
 * IRP notation:
 * {38.0k,564}<1,-1|1,-3>(16,-8,A:8,~A:8,C:16,1,^108m,(16,-4,1,^108m)*)
 *   A = 8-bit address (Block ID)
 *  ~A = inverted address (error check)
 *   C = 16-bit command (Device ID + Mode + Data)
 */

// OpenLASIR address and command bit widths
#define OPENLASIR_ADDRESS_BITS  16 // 8 bit address + 8 bit inverted address
#define OPENLASIR_COMMAND_BITS  16 // 16 bit command, no error check
#define OPENLASIR_BITS          (OPENLASIR_ADDRESS_BITS + OPENLASIR_COMMAND_BITS) // 32 bits total

/*
 * Constants for Mode
 */
#define OPENLASIR_MODE_LASER_TAG_FIRE                               0 // Laser tag shot. Data = color.
#define OPENLASIR_MODE_USER_PRESENCE_ANNOUNCEMENT                   1 // User device saying "I'm here." No response expected.
#define OPENLASIR_MODE_BASE_STATION_PRESENCE_ANNOUNCEMENT           2 // Fixed base station saying "I'm here."
#define OPENLASIR_MODE_USER_TO_USER_HANDSHAKE_INITIATION            3 // User badge initiates handshake with another user badge.
#define OPENLASIR_MODE_USER_TO_USER_HANDSHAKE_RESPONSE              4 // Response to a user-to-user handshake initiation.
#define OPENLASIR_MODE_USER_TO_BASE_STATION_HANDSHAKE_INITIATION    5 // User badge initiates handshake with a base station.
#define OPENLASIR_MODE_USER_TO_BASE_STATION_HANDSHAKE_RESPONSE      6 // Base station responds to a user-initiated handshake.
#define OPENLASIR_MODE_BASE_STATION_TO_USER_HANDSHAKE_INITIATION    7 // Base station initiates handshake with a user badge.
#define OPENLASIR_MODE_BASE_STATION_TO_USER_HANDSHAKE_RESPONSE      8 // User badge responds to a base-station-initiated handshake.
#define OPENLASIR_MODE_COLOR_SET_TEMPORARY                          9 // Tell a badge to display a color temporarily.
#define OPENLASIR_MODE_COLOR_SET_PERMANENT                         10 // Tell a badge to display a color and "remember" it according to some device-specific logic.
#define OPENLASIR_MODE_GENERAL_INTERACT                            11 // Tell a device to execute a general "interact" action. The specific behavior is defined by the receiver.
// modes 12 to 31 are reserved for future use

/*
 * Constants for Data / Color
 */
#define OPENLASIR_COLOR_CYAN        0 // (0, 255, 255)
#define OPENLASIR_COLOR_MAGENTA     1 // (255, 0, 255)
#define OPENLASIR_COLOR_YELLOW      2 // (255, 255, 0)
#define OPENLASIR_COLOR_GREEN       3 // (0, 255, 0)
#define OPENLASIR_COLOR_RED         4 // (255, 0, 0)
#define OPENLASIR_COLOR_BLUE        5 // (0, 0, 255)
#define OPENLASIR_COLOR_ORANGE      6 // (255, 165, 0)
#define OPENLASIR_COLOR_WHITE       7 // (255, 255, 255)

// Timing is identical to NEC (reuse NEC_* constants from ir_NEC.hpp)

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * Send special OpenLASIR repeat frame (same as NEC repeat frame).
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendOpenLASIRRepeat() {
    sendNECRepeat();
}

/**
 * Static function variant of IRsend::sendOpenLASIRRepeat
 * For use in ProtocolConstants. Saves up to 250 bytes compared to a member function.
 */
void sendOpenLASIRSpecialRepeat() {
    sendNECSpecialRepeat();
}

/**
 * Compute the raw 32-bit data for an OpenLASIR frame from 8-bit address and 16-bit command.
 * Address is sent as: [address:8][~address:8] (inverted complement for error checking)
 * Command is sent as: [command:16] (no error check)
 *
 * @param aAddress  8-bit address (Block ID). Only lower 8 bits are used.
 * @param aCommand  16-bit command (contains Device ID, Mode, and Data).
 * @return 32-bit raw data ready for transmission.
 */
uint32_t IRsend::computeOpenLASIRRawDataAndChecksum(uint8_t aAddress, uint16_t aCommand) {
    LongUnion tRawData;

    // Address: 8 bit address + 8 bit inverted address for error checking
    tRawData.UByte.LowByte = aAddress;
    tRawData.UByte.MidLowByte = ~aAddress;

    // Command: full 16 bits, no error check
    tRawData.UWord.HighWord = aCommand;

    return tRawData.ULong;
}

/**
 * Compute the raw 32-bit data for an OpenLASIR frame from 8-bit address, 8-bit DeviceID, 5-bit Mode and 3-bit Data.
 *
 * @param aAddress  8-bit address (Block ID). Only lower 8 bits are used.
 * @param aDeviceID     8-bit Device ID.
 * @param aMode         5-bit Mode.
 * @param aData         3-bit Data (color, etc.).
 * @return 32-bit raw data ready for transmission.
 */
uint16_t IRsend::computeOpenLASIRRawCommand(uint8_t aDeviceID, uint8_t aMode, uint8_t aData) {
    WordUnion tRawCommand;

    tRawCommand.UByte.LowByte = aDeviceID;
    tRawCommand.UByte.HighByte = (aMode & 0x1F) | ((aData & 0x07) << 5);

    return tRawCommand.UWord;
}

/**
 * Send an OpenLASIR frame with special NEC-style repeats.
 * There is NO delay after the last sent repeat!
 *
 * @param aAddress        8-bit address (Block ID). Only lower 8 bits are used.
 * @param aCommand        16-bit command (Device ID + Mode + Data).
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame will be sent.
 */
void IRsend::sendOpenLASIR(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth_P(&NECProtocolConstants, computeOpenLASIRRawDataAndChecksum(aAddress, aCommand), OPENLASIR_BITS,
            aNumberOfRepeats);
}

/**
 * Send an OpenLASIR frame with special NEC-style repeats.
 * There is NO delay after the last sent repeat!
 *
 * @param aAddress      8-bit address (Block ID). Only lower 8 bits are used.
 * @param aDeviceID     8-bit Device ID.
 * @param aMode         5-bit Mode.
 * @param aData         3-bit Data (color, etc.).
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame will be sent.
 */
void IRsend::sendOpenLASIR(uint8_t aAddress, uint8_t aDeviceID, uint8_t aMode, uint8_t aData, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth_P(&NECProtocolConstants,
            computeOpenLASIRRawDataAndChecksum(aAddress, computeOpenLASIRRawCommand(aDeviceID, aMode, aData)),
            OPENLASIR_BITS, aNumberOfRepeats);
}

/**
 * Send raw 32-bit OpenLASIR data.
 * @param aRawData          The pre-computed 32-bit raw data.
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame will be sent.
 */
void IRsend::sendOpenLASIRRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth_P(&NECProtocolConstants, aRawData, OPENLASIR_BITS, aNumberOfRepeats);
}

/**
 * Decode an OpenLASIR frame.
 *
 * OpenLASIR uses the same physical layer as NEC but with different address/command structure:
 *   - 8-bit address with inverted complement (error check on address)
 *   - 16-bit command without error check
 *
 * The decoder validates the address by checking that the second byte is the bitwise
 * inverse of the first byte. The full 16-bit command is returned as-is.
 *
 * @return true if a valid OpenLASIR frame was decoded.
 */
bool IRrecv::decodeOpenLASIR() {
    /*
     * First check for right data length
     * Next check start bit
     * Next try the decode
     */

    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawlen != ((2 * OPENLASIR_BITS) + 4) && (decodedIRData.rawlen != 4)) {
        DEBUG_PRINT(F("OpenLASIR: Data length="));
        DEBUG_PRINT(decodedIRData.rawlen);
        DEBUG_PRINTLN(F(" is not 68 or 4"));
        return false;
    }

    // Check header "mark" - this must be done for repeat and data
    if (!matchMark(irparams.rawbuf[1], NEC_HEADER_MARK)) {
        return false;
    }

    // Check for repeat - here we have another header space length
    if (decodedIRData.rawlen == 4) {
        // Only claim this repeat if the last decoded protocol was OpenLASIR
        if (lastDecodedProtocol == OPENLASIR && matchSpace(irparams.rawbuf[2], NEC_REPEAT_HEADER_SPACE)
                && matchMark(irparams.rawbuf[3], NEC_BIT_MARK)) {
            decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
            decodedIRData.address = lastDecodedAddress;
            decodedIRData.command = lastDecodedCommand;
            decodedIRData.protocol = OPENLASIR;
            return true;
        }
        return false;
    }

    // Check command header space
    if (!matchSpace(irparams.rawbuf[2], NEC_HEADER_SPACE)) {
        DEBUG_PRINTLN(F("OpenLASIR: Header space length is wrong"));
        return false;
    }

    // Decode the pulse distance data using NEC timing
    decodePulseDistanceWidthData_P(&NECProtocolConstants, OPENLASIR_BITS);

    // Success - now interpret the 32 raw bits
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;

    // Validate address: second byte must be inverted first byte
    if (tValue.UByte.LowByte != (uint8_t)(~tValue.UByte.MidLowByte)) {
        // Address validation failed - this is not a valid OpenLASIR frame
        DEBUG_PRINTLN(F("OpenLASIR: Address validation failed"));
        return false;
    }

    /*
     * Disambiguation with standard NEC:
     * If the command also has a valid inverted pattern (upper 8 bits == ~lower 8 bits),
     * then this looks like a standard NEC frame, not OpenLASIR.
     * By design, OpenLASIR uses 16-bit commands that should NOT have this property,
     * so we let the NEC decoder handle these frames instead.
     * (See OpenLASIR spec: "This is to prevent OpenLASIR packets from causing issues
     *  with NEC devices except in very rare cases of collisions.")
     */
    if (tValue.UByte.MidHighByte == (uint8_t)(~tValue.UByte.HighByte)) {
        DEBUG_PRINTLN(F("OpenLASIR: Command has valid inverse - looks like standard NEC, skipping"));
        return false;
    }

    // Valid OpenLASIR frame
    decodedIRData.protocol = OPENLASIR;
    decodedIRData.address = tValue.UByte.LowByte;        // 8-bit validated address (Block ID)
    decodedIRData.command = tValue.UWord.HighWord;       // 16-bit command (Device ID + Mode + Data)
    decodedIRData.numberOfBits = OPENLASIR_BITS;

    // Check for repeat (same frame sent again within repeat window)
    checkForRepeatSpaceTicksAndSetFlag(NEC_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}

/** @}*/
#include "LocalDebugLevelEnd.h"

#endif // _IR_OPENLASIR_HPP
