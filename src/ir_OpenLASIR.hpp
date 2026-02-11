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
 * Copyright (c) 2025
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

#if defined(DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

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

// Timing is identical to NEC (reuse NEC_* constants from ir_NEC.hpp)

struct PulseDistanceWidthProtocolConstants const OpenLASIRProtocolConstants PROGMEM = {
    OPENLASIR,
    NEC_KHZ,
    NEC_HEADER_MARK,
    NEC_HEADER_SPACE,
    NEC_BIT_MARK,
    NEC_ONE_SPACE,
    NEC_BIT_MARK,
    NEC_ZERO_SPACE,
    PROTOCOL_IS_LSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE,
    (NEC_REPEAT_PERIOD / MICROS_IN_ONE_MILLI),
    &sendOpenLASIRSpecialRepeat
};

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * Send special OpenLASIR repeat frame (same as NEC repeat frame).
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendOpenLASIRRepeat() {
    enableIROut(NEC_KHZ);            // 38 kHz
    mark(NEC_HEADER_MARK);           // + 9000
    space(NEC_REPEAT_HEADER_SPACE);  // - 2250
    mark(NEC_BIT_MARK);              // + 560
}

/**
 * Static function variant of IRsend::sendOpenLASIRRepeat
 * For use in ProtocolConstants. Saves up to 250 bytes compared to a member function.
 */
void sendOpenLASIRSpecialRepeat() {
    IrSender.enableIROut(NEC_KHZ);            // 38 kHz
    IrSender.mark(NEC_HEADER_MARK);           // + 9000
    IrSender.space(NEC_REPEAT_HEADER_SPACE);  // - 2250
    IrSender.mark(NEC_BIT_MARK);              // + 560
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
 * Send an OpenLASIR frame with special NEC-style repeats.
 * There is NO delay after the last sent repeat!
 *
 * @param aAddress        8-bit address (Block ID). Only lower 8 bits are used.
 * @param aCommand        16-bit command (Device ID + Mode + Data).
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame will be sent.
 */
void IRsend::sendOpenLASIR(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth_P(&OpenLASIRProtocolConstants,
            computeOpenLASIRRawDataAndChecksum(aAddress, aCommand), OPENLASIR_BITS, aNumberOfRepeats);
}

/**
 * Send raw 32-bit OpenLASIR data.
 * @param aRawData          The pre-computed 32-bit raw data.
 * @param aNumberOfRepeats  If < 0 then only a special repeat frame will be sent.
 */
void IRsend::sendOpenLASIRRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth_P(&OpenLASIRProtocolConstants, aRawData, OPENLASIR_BITS, aNumberOfRepeats);
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
        IR_DEBUG_PRINT(F("OpenLASIR: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 68 or 4"));
        return false;
    }

    // Check header "mark" - this must be done for repeat and data
    if (!matchMark(irparams.rawbuf[1], NEC_HEADER_MARK)) {
        return false;
    }

    // Check for repeat - here we have another header space length
    if (decodedIRData.rawlen == 4) {
        // Only claim this repeat if the last decoded protocol was OpenLASIR
        if (lastDecodedProtocol == OPENLASIR
                && matchSpace(irparams.rawbuf[2], NEC_REPEAT_HEADER_SPACE)
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
#if defined(LOCAL_DEBUG)
        Serial.print(F("OpenLASIR: "));
        Serial.println(F("Header space length is wrong"));
#endif
        return false;
    }

    // Decode the pulse distance data using NEC timing
    decodePulseDistanceWidthData_P(&OpenLASIRProtocolConstants, OPENLASIR_BITS);

    // Success - now interpret the 32 raw bits
    LongUnion tValue;
    tValue.ULong = decodedIRData.decodedRawData;

    // Validate address: second byte must be inverted first byte
    if (tValue.UByte.LowByte != (uint8_t)(~tValue.UByte.MidLowByte)) {
        // Address validation failed - this is not a valid OpenLASIR frame
        IR_DEBUG_PRINT(F("OpenLASIR: "));
        IR_DEBUG_PRINTLN(F("Address validation failed"));
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
        IR_DEBUG_PRINT(F("OpenLASIR: "));
        IR_DEBUG_PRINTLN(F("Command has valid inverse - looks like standard NEC, skipping"));
        return false;
    }

    // Valid OpenLASIR frame
    decodedIRData.protocol = OPENLASIR;
    decodedIRData.address = tValue.UByte.LowByte;       // 8-bit validated address (Block ID)
    decodedIRData.command = tValue.UWord.HighWord;       // 16-bit command (Device ID + Mode + Data)
    decodedIRData.numberOfBits = OPENLASIR_BITS;

    // Check for repeat (same frame sent again within repeat window)
    checkForRepeatSpaceTicksAndSetFlag(NEC_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_OPENLASIR_HPP
