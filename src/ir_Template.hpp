/*
 Assuming the protocol we are adding is for the (imaginary) manufacturer:  Shuzu

 Our fantasy protocol is a standard protocol, so we can use this standard
 template without too much work. Some protocols are quite unique and will require
 considerably more work in this file! It is way beyond the scope of this text to
 explain how to reverse engineer "unusual" IR protocols. But, unless you own an
 oscilloscope, the starting point is probably to use the ReceiveDump.ino sketch and
 try to spot the pattern!

 Before you start, make sure the IR library is working OK:
 # Open up the Arduino IDE
 # Load up the ReceiveDump.ino example sketch
 # Run it
 # Analyze your data to have an idea, what is the header timing, the bit timing, the address, the command and the checksum of your protocol.

 Now we can start to add our new protocol...

 1. Copy this file to : ir_<YourProtocolName>.hpp

 2. Replace all occurrences of "SHUZU" with the name of your protocol.

 3. Tweak the #defines to suit your protocol.

 4. If you're lucky, tweaking the #defines will make the decode and send() function
 work.

 You have now written the code to support your new protocol!

 To integrate it into the IRremote library, you must search for "BOSEWAVE"
 and add your protocol in the same way as it is already done for BOSEWAVE.

 You have to change the following files:
 IRSend.hpp     IRsend::write(IRData *aIRSendData + int_fast8_t aNumberOfRepeats)
 IRProtocol.h   Add it to decode_type_t
 IRReceive.hpp  IRrecv::decode() + printActiveIRProtocols(Print *aSerial) + getProtocolString(decode_type_t aProtocol)
 IRremote.hpp   At 3 occurrences of DECODE_XXX
 IRremoteInt.h  Add the declaration of the decode and send function

 Now open the Arduino IDE, load up the ReceiveDump.ino sketch, and run it.
 Hopefully it will compile and upload.
 If it doesn't, you've done something wrong. Check your work and look carefully at the error messages.

 If you get this far, I will assume you have successfully added your new protocol

 At last, delete this giant instructional comment.

 If you want us to include your work in the library so others may benefit
 from your hard work, you have to extend the examples
 IRremoteInfo, SmallReceiver, simple Receiver, SendDemo and UnitTest too
 as well as the Readme.md
 It is not an act, but required for completeness.

 Thanks
 The maintainer
 */

/*
 * ir_Shuzu.hpp
 *
 *  Contains functions for receiving and sending Shuzu IR Protocol ...
 *
 *  Copyright (C) 2022  Shuzu Guru
 *  shuzu.guru@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022 Unknown Contributor :-)
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
#ifndef _IR_SHUZU_HPP
#define _IR_SHUZU_HPP

//==============================================================================
//
//
//                              S H U Z U
//
//
//==============================================================================
// see: https://www....

// LSB first, 1 start bit + 16 bit address + 8 bit command + 1 stop bit.
#define SHUZU_ADDRESS_BITS      16 // 16 bit address
#define SHUZU_COMMAND_BITS      8 // Command

#define SHUZU_BITS              (SHUZU_ADDRESS_BITS + SHUZU_COMMAND_BITS) // The number of bits in the protocol
#define SHUZU_UNIT              560               // All timings are in microseconds

#define SHUZU_HEADER_MARK       (16 * SHUZU_UNIT) // The length of the Header:Mark
#define SHUZU_HEADER_SPACE      (8 * SHUZU_UNIT)  // The length of the Header:Space

#define SHUZU_BIT_MARK          SHUZU_UNIT        // The length of a Bit:Mark
#define SHUZU_ONE_SPACE         (3 * SHUZU_UNIT)  // The length of a Bit:Space for 1's
#define SHUZU_ZERO_SPACE        SHUZU_UNIT        // The length of a Bit:Space for 0's

#define SHUZU_REPEAT_HEADER_SPACE (4 * SHUZU_UNIT)  // 2250

#define SHUZU_REPEAT_PERIOD     110000            // From start to start
#define SHUZU_REPEAT_SPACE      45000             // SHUZU_REPEAT_PERIOD - default frame duration. Used for repeat detection.

#define SHUZU_OTHER             1234  // Other things you may need to define

// use BOSEWAVE, we have no SHUZU code
struct PulseDistanceWidthProtocolConstants ShuzuProtocolConstants = { BOSEWAVE, 38, SHUZU_HEADER_MARK, SHUZU_HEADER_SPACE,
SHUZU_BIT_MARK, SHUZU_ONE_SPACE, SHUZU_BIT_MARK, SHUZU_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST, (SHUZU_REPEAT_PERIOD
        / MICROS_IN_ONE_MILLI), NULL };

/************************************
 * Start of send and decode functions
 ************************************/

void IRsend::sendShuzu(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats) {

    sendPulseDistanceWidth(&ShuzuProtocolConstants, (uint32_t) aCommand << 8 | aCommand, SHUZU_BITS, aNumberOfRepeats);
}

bool IRrecv::decodeShuzu() {
    /*
     * First check for right data length
     * Next check start bit / header
     * Next try the decode
     */
    // Check we have the right amount of data (28). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != (2 * SHUZU_BITS) + 4) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header
    if (!checkHeader(&ShuzuProtocolConstants)) {
        return false;
    }

    // Decode
    if (!decodePulseDistanceData(&ShuzuProtocolConstants, SHUZU_BITS)) {
        IR_DEBUG_PRINT(F("Shuzu: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Success, interpret raw data
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    decodedIRData.command = decodedIRData.decodedRawData >> SHUZU_ADDRESS_BITS;  // upper 8 bits of LSB first value
    decodedIRData.address = decodedIRData.decodedRawData & 0xFFFF;    // lowest 16 bit of LSB first value
    decodedIRData.numberOfBits = SHUZU_BITS;
    decodedIRData.protocol = BOSEWAVE; // we have no SHUZU code

    //Check for repeat
    checkForRepeatSpaceAndSetFlag(SHUZU_REPEAT_SPACE / MICROS_IN_ONE_MILLI);

    return true;
}
#endif // _IR_SHUZU_HPP
