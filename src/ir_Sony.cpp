/*
 * ir_Sony.cpp
 *
 *  Contains functions for receiving and sending SIRCS/Sony IR Protocol in "raw" and standard format with 5 bit address 7 bit command
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
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

//#define DEBUG // Activate this for lots of lovely debug output.
#include "IRremote.h"

//==============================================================================
//                           SSSS   OOO   N   N  Y   Y
//                          S      O   O  NN  N   Y Y
//                           SSS   O   O  N N N    Y
//                              S  O   O  N  NN    Y
//                          SSSS    OOO   N   N    Y
//==============================================================================
// see https://www.sbprojects.net/knowledge/ir/sirc.php - After counting 12 or 15 bits the receiver must wait at least 10ms to make sure that no more pulses follow.
// Here http://picprojects.org.uk/projects/sirc/ it is claimed, that many Sony remotes repeat each frame a minimum of 3 times

// LSB first, start bit + 7 command + 5 to 13 address, no stop bit
//
#define SONY_ADDRESS_BITS       5
#define SONY_COMMAND_BITS       7
#define SONY_EXTRA_BITS         8
#define SONY_BITS_MIN           (SONY_COMMAND_BITS + SONY_ADDRESS_BITS)        // 12 bits
#define SONY_BITS_15            (SONY_COMMAND_BITS + SONY_ADDRESS_BITS + 3)    // 15 bits
#define SONY_BITS_MAX           (SONY_COMMAND_BITS + SONY_ADDRESS_BITS + SONY_EXTRA_BITS)    // 20 bits == SIRCS_20_PROTOCOL
#define SONY_UNIT               600

#define SONY_HEADER_MARK        (4 * SONY_UNIT) //2400
#define SONY_ONE_MARK           (2 * SONY_UNIT) // 1200
#define SONY_ZERO_MARK          SONY_UNIT
#define SONY_SPACE              SONY_UNIT

#define SONY_AVERAGE_DURATION   21000 // SONY_HEADER_MARK + SONY_SPACE  + 12 * 2,5 * SONY_UNIT  // 2.5 because we assume more zeros than ones
#define SONY_REPEAT_PERIOD      45000 // Commands are repeated every 45 ms (measured from start to start) for as long as the key on the remote control is held down.
#define SONY_REPEAT_SPACE       (SONY_REPEAT_PERIOD - SONY_AVERAGE_DURATION)

/*
 * Repeat commands should be sent in a 45 ms raster.
 * There is NO delay after the last sent command / repeat!
 * @param send8AddressBits if false send only 5 address bits (standard is 12 bit SIRCS protocol)
 */
void IRsend::sendSony(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, uint8_t numberOfBits) {
    // Set IR carrier frequency
    enableIROut(40);

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        noInterrupts();

        // Header
        mark(SONY_HEADER_MARK);
        space(SONY_SPACE);

        // send 7 command bits LSB first
        sendPulseDistanceWidthData(SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE, aCommand, SONY_COMMAND_BITS, false);
        // Address 16 bit LSB first
        if (numberOfBits == SIRCS_20_PROTOCOL) {
            sendPulseDistanceWidthData(SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE, aAddress,
                    (SONY_ADDRESS_BITS + SONY_EXTRA_BITS), false);
        } else {
            sendPulseDistanceWidthData(SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE, aAddress, SONY_ADDRESS_BITS, false);
        }
        interrupts();

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a 45 ms raster
            delay(SONY_REPEAT_SPACE / 1000);
        }
    }
}

//+=============================================================================
#if defined(USE_STANDARD_DECODE)

bool IRrecv::decodeSony() {

    // Check header "mark"
    if (!MATCH_MARK(results.rawbuf[1], SONY_HEADER_MARK)) {
        return false;
    }

    // Check we have enough data. +2 for initial gap and start bit mark and space minus the last/MSB space. NO stop bit!
    if (results.rawlen != (2 * SONY_BITS_MIN) + 2 && results.rawlen != (2 * SONY_BITS_MAX) + 2
            && results.rawlen != (2 * SONY_BITS_15) + 2) {
        // TRACE_PRINT since I saw this too often
        TRACE_PRINT("Sony: ");
        TRACE_PRINT("Data length=");
        TRACE_PRINT(results.rawlen);
        TRACE_PRINTLN(" is not 12, 15 or 20");
        return false;
    }
    // Check header "space"
    if (!MATCH_SPACE(results.rawbuf[2], SONY_SPACE)) {
        DBG_PRINT("Sony: ");
        DBG_PRINTLN("Header space length is wrong");
        return false;
    }

    if (!decodePulseWidthData((results.rawlen - 1) / 2, 3, SONY_ONE_MARK, SONY_ZERO_MARK, SONY_SPACE, false)) {
        DBG_PRINT("Sony: ");
        DBG_PRINTLN("Decode failed");
        return false;
    }

    // Success
    uint8_t tCommand = results.value & 0x7F;  // first 7 bits
    uint8_t tAddress = results.value >> 7;    // next 5 or 8 bits

    /*
     *  Check for repeat
     */
    if (results.rawbuf[0] < (SONY_REPEAT_PERIOD / MICROS_PER_TICK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
    }
    decodedIRData.command = tCommand;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = (results.rawlen - 1) / 2;
    decodedIRData.protocol = SONY;

    return true;
}

#else

#define SONY_DOUBLE_SPACE_USECS    500 // usually see 713 - not using ticks as get number wrap around

#warning "Old decoder functions decodeSony() and decodeSony(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeSony() instead."

bool IRrecv::decodeSony() {
    long data = 0;
    uint8_t bits = 0;
    unsigned int offset = 0;  // Dont skip first space, check its size

    if (results.rawlen < (2 * SONY_BITS_MIN) + 2) {
        return false;
    }

    // Some Sony's deliver repeats fast after first
    // unfortunately can't spot difference from of repeat from two fast clicks
    if (results.rawbuf[0] < (SONY_DOUBLE_SPACE_USECS / MICROS_PER_TICK)) {
        DBG_PRINTLN("IR Gap found");
        results.bits = 0;
        results.value = REPEAT;
        decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER | IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = UNKNOWN;
        return true;
    }
    offset++;

    // Check header "mark"
    if (!MATCH_MARK(results.rawbuf[offset], SONY_HEADER_MARK)) {
        return false;
    }
    offset++;

    // MSB first - Not compatible to standard, which says LSB first :-(
    while (offset + 1 < results.rawlen) {

        // First check for the constant space length, we do not have a space at the end of raw data
        // we are lucky, since the start space is equal the data space.
        if (!MATCH_SPACE(results.rawbuf[offset], SONY_SPACE)) {
            return false;
        }
        offset++;

        // bit value is determined by length of the mark
        if (MATCH_MARK(results.rawbuf[offset], SONY_ONE_MARK)) {
            data = (data << 1) | 1;
        } else if (MATCH_MARK(results.rawbuf[offset], SONY_ZERO_MARK)) {
            data = (data << 1) | 0;
        } else {
            return false;
        }
        offset++;
        bits++;

    }

    results.bits = bits;
    results.value = data;
    decodedIRData.protocol = SONY;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;
    return true;
}

bool IRrecv::decodeSony(decode_results *aResults) {
    bool aReturnValue = decodeSony();
    *aResults = results;
    return aReturnValue;
}

#endif

//+=============================================================================
void IRsend::sendSony(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(40);

    // Header
    mark(SONY_HEADER_MARK);
    space(SONY_SPACE);

    sendPulseDistanceWidthData(SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE, data, nbits, true, false);
    /*
     * Pulse width coding, the short version.
     * Use this if you need to save program space and only require this protocol.
     */
//    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
//        if (data & mask) {
//            mark(SONY_ONE_MARK);
//            space(SONY_SPACE);
//        } else {
//            mark(SONY_ZERO_MARK);
//            space(SONY_SPACE);
//        }
//    }
}
