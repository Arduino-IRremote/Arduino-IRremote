/*
 Assuming the protocol we are adding is for the (imaginary) manufacturer:  Shuzu

 Our fantasy protocol is a standard protocol, so we can use this standard
 template without too much work. Some protocols are quite unique and will require
 considerably more work in this file! It is way beyond the scope of this text to
 explain how to reverse engineer "unusual" IR protocols. But, unless you own an
 oscilloscope, the starting point is probably to use the rawDump.ino sketch and
 try to spot the pattern!

 Before you start, make sure the IR library is working OK:
 # Open up the Arduino IDE
 # Load up the rawDump.ino example sketch
 # Run it

 Now we can start to add our new protocol...

 1. Copy this file to : ir_Shuzu.cpp

 2. Replace all occurrences of "Shuzu" with the name of your protocol.

 3. Tweak the #defines to suit your protocol.

 4. If you're lucky, tweaking the #defines will make the default send() function
 work.

 5. Again, if you're lucky, tweaking the #defines will have made the default
 decode() function work.

 You have written the code to support your new protocol!

 Now you must do a few things to add it to the IRremote system:

 1. Open IRremote.h and make the following changes:
 REMEMEBER to change occurrences of "SHUZU" with the name of your protocol

 A. At the top, in the section "Supported Protocols", add:
 #define DECODE_SHUZU  1
 #define SEND_SHUZU    1

 B. In the section "An enum consisting of all supported formats", add:
 SHUZU,
 to the end of the list (notice there is a comma after the protocol name)

 C. Further down in "Main class for receiving IR", add:
 //......................................................................
 #if DECODE_SHUZU
 bool  decodeShuzu () ;
 #endif

 D. Further down in "Main class for sending IR", add:
 //......................................................................
 #if SEND_SHUZU
 void  sendShuzuStandard (uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) ;
 #endif

 E. Save your changes and close the file

 2. Now open irReceive.cpp and make the following change:

 A. In the function IRrecv::decode(), add:
 #ifdef DECODE_SHUZU
 DBG_PRINTLN("Attempting Shuzu decode");
 if (decodeShuzu())  return true ;
 #endif

 B. In the function IRrecv::getProtocolString(), add
 #if DECODE_SHUZU
 case SHUZU:
 return ("Shuzu");
 break;
 #endif

 C. Save your changes and close the file

 Now open the Arduino IDE, load up the rawDump.ino sketch, and run it.
 Hopefully it will compile and upload.
 If it doesn't, you've done something wrong. Check your work.
 If you can't get it to work - seek help from somewhere.

 If you get this far, I will assume you have successfully added your new protocol
 There is one last thing to do.

 1. Delete this giant instructional comment.

 2. Send a copy of your work to us so we can include it in the library and
 others may benefit from your hard work and maybe even write a song about how
 great you are for helping them! :)

 Regards,
 BlueChip
 */

/*
 * ir_Shuzu.cpp
 *
 *  Contains functions for receiving and sending Shuzu IR Protocol ...
 *
 *  Copyright (C) 2021  Shuzu Guru
 *  shuzu.guru@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2021 Unknown Contributor :-)
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

//#define DEBUG // Activate this for lots of lovely debug output.
#include "IRremote.h"

//#define SEND_SHUZU  1 // for testing
//#define DECODE_SHUZU  1 // for testing
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
#define SHUZU_UNIT              560

#define SHUZU_HEADER_MARK       (16 * SHUZU_UNIT) // The length of the Header:Mark
#define SHUZU_HEADER_SPACE      (8 * SHUZU_UNIT)  // The lenght of the Header:Space

#define SHUZU_BIT_MARK          SHUZU_UNIT        // The length of a Bit:Mark
#define SHUZU_ONE_SPACE         (3 * SHUZU_UNIT)  // The length of a Bit:Space for 1's
#define SHUZU_ZERO_SPACE        SHUZU_UNIT        // The length of a Bit:Space for 0's

#define SHUZU_REPEAT_HEADER_SPACE (4 * SHUZU_UNIT)  // 2250

#define SHUZU_REPEAT_SPACE      45000

#define SHUZU_OTHER             1234  // Other things you may need to define

//+=============================================================================
//
void IRsend::sendShuzu(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(37); // 36.7kHz is the correct frequency

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        noInterrupts();

        // Header
        mark(SHUZU_HEADER_MARK);
        space(SHUZU_HEADER_SPACE);

        // Address (device and subdevice)
        sendPulseDistanceWidthData(SHUZU_BIT_MARK, SHUZU_ONE_SPACE, SHUZU_BIT_MARK, SHUZU_ZERO_SPACE, aAddress,
        SHUZU_ADDRESS_BITS, false); // false -> LSB first

        // Command + stop bit
        sendPulseDistanceWidthData(SHUZU_BIT_MARK, SHUZU_ONE_SPACE, SHUZU_BIT_MARK, SHUZU_ZERO_SPACE, aCommand,
        SHUZU_COMMAND_BITS, false, true); // false, true -> LSB first, stop bit

        interrupts();

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(SHUZU_REPEAT_SPACE / 1000);
        }
    }
}

//+=============================================================================
//
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 */
bool IRrecv::decodeShuzu() {

    // Check we have the right amount of data (28). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != (2 * SHUZU_BITS) + 4) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header "space"
    if (!MATCH_MARK(results.rawbuf[1], SHUZU_HEADER_MARK) || !MATCH_SPACE(results.rawbuf[2], SHUZU_HEADER_SPACE)) {
        DBG_PRINT("Shuzu: ");
        DBG_PRINTLN("Header mark or space length is wrong");
        return false;
    }

    // false -> LSB first
    if (!decodePulseDistanceData(SHUZU_BITS, 3, SHUZU_BIT_MARK, SHUZU_ONE_SPACE, SHUZU_ZERO_SPACE, false)) {
        DBG_PRINT(F("Shuzu: "));
        DBG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[3 + (2 * SHUZU_BITS)], SHUZU_BIT_MARK)) {
        DBG_PRINT(F("Shuzu: "));
        DBG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    uint8_t tCommand = results.value >> SHUZU_ADDRESS_BITS;  // upper 8 bits of LSB first value
    uint8_t tAddress = results.value & 0xFFFF;    // lowest 16 bit of LSB first value

    /*
     *  Check for repeat
     */
    if (results.rawbuf[0] < ((SHUZU_REPEAT_SPACE + (SHUZU_REPEAT_SPACE / 2)) / MICROS_PER_TICK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
    }
    decodedIRData.command = tCommand;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = SHUZU_BITS;
    decodedIRData.protocol = LG; // we have no SHUZU code

    return true;
}
