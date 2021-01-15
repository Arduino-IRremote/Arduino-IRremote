/*
 * ir_RC%_RC6.cpp
 *
 *  Contains functions for receiving and sending RC5, RC5X Protocols
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
#include "LongUnion.h"

bool sLastSendToggleValue = false;
//uint8_t sLastReceiveToggleValue = 3; // 3 -> start value

//==============================================================================
// RRRR    CCCC  55555
// R   R  C      5
// RRRR   C      5555
// R  R   C          5
// R   R   CCCC  5555
//
// 0 -> mark+space
// 1 -> space+mark
// 1 start bit, 1 field bit, 1 toggle bit + 5 bit address + 6 bit command, no stop bit
// duty factor is 25%,
//

#define RC5_ADDRESS_BITS        5
#define RC5_COMMAND_BITS        6
#define RC5_COMMAND_FIELD_BIT   1
#define RC5_TOGGLE_BIT          1

#define RC5_BITS                (RC5_COMMAND_FIELD_BIT + RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS) // 13

#define RC5_UNIT                889 // (32 cycles of 36 kHz)

#define MIN_RC5_MARKS           ((RC5_BITS + 1) / 2) // 7

#define RC5_DURATION            (15L * RC5_UNIT) // 13335
#define RC5_REPEAT_PERIOD       (128L * RC5_UNIT) // 113792
#define RC5_REPEAT_SPACE        (RC5_REPEAT_PERIOD - RC5_DURATION) // 100 ms

/*
 * If Command is >=64 then we switch automatically to RC5X
 */
void IRsend::sendRC5(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aEnableAutomaticToggle) {
    // Set IR carrier frequency
    enableIROut(36);

    uint16_t tIRData = ((aAddress & 0x1F) << RC5_COMMAND_BITS);

    if (aCommand < 0x40) {
        // set field bit to lower field / set inverted upper command bit
        tIRData |= 1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS);
    } else {
        // let field bit zero
        aCommand &= 0x3F;
    }

    tIRData |= aCommand;

    if (aEnableAutomaticToggle) {
        if (sLastSendToggleValue == 0) {
            sLastSendToggleValue = 1;
            // set toggled bit
            tIRData |= 1 << (RC5_ADDRESS_BITS + RC5_COMMAND_BITS);
        } else {
            sLastSendToggleValue = 0;
        }
    }

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        noInterrupts();

        // start bit is sent by sendBiphaseData
        sendBiphaseData(RC5_UNIT, tIRData, RC5_BITS);

        interrupts();

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(RC5_REPEAT_SPACE / 1000);
        }
    }
}

#if defined(USE_STANDARD_DECODE)
bool IRrecv::decodeRC5() {

    // Check we have the right amount of data (11 to 26). The +2 is for initial gap and start bit mark.
    if (results.rawlen < MIN_RC5_MARKS + 2 && results.rawlen > ((2 * RC5_BITS) + 2)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    if (!decodeBiPhaseData(RC5_BITS + 1, 1, 1, RC5_UNIT)) {
        // TRACE_PRINT since I saw this too often
        TRACE_PRINT(F("RC5: "));
        TRACE_PRINTLN(F("Decode failed"));
        return false;
    }

    // Success
    LongUnion tValue;
    tValue.ULong = results.value;
    decodedIRData.command = tValue.UByte.LowByte & 0x3F;
    decodedIRData.address = (tValue.UWord.LowWord >> RC5_COMMAND_BITS) & 0x1F;
    if ((tValue.UWord.LowWord & (1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS))) == 0) {
        decodedIRData.command += 0x40;
    }

    if (tValue.UByte.MidLowByte & 0x8) {
        decodedIRData.flags = IRDATA_TOGGLE_BIT_MASK;
    }

    // check for repeat
    if (results.rawbuf[0] < (RC5_REPEAT_PERIOD / MICROS_PER_TICK)) {
        decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
    }

    decodedIRData.protocol = RC5;
    decodedIRData.numberOfBits = RC5_BITS;

    return true;
}

#else

#warning "Old decoder functions decodeRC5() and decodeRC5(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeRC5() instead."

//+=============================================================================
// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
//
#if (DECODE_RC5 || DECODE_RC6)
int getRClevel(decode_results *results, unsigned int *offset, uint8_t *used, int t1) {
    unsigned int width;
    int val;
    int correction;
    uint8_t avail;

    if (*offset >= results->rawlen) {
        return SPACE;  // After end of recorded buffer, assume SPACE.
    }
    width = results->rawbuf[*offset];
    val = ((*offset) % 2) ? MARK : SPACE;
    correction = (val == MARK) ? MARK_EXCESS_MICROS : - MARK_EXCESS_MICROS;

    if (MATCH(width, (t1) + correction)) {
        avail = 1;
    } else if (MATCH(width, (2 * t1) + correction)) {
        avail = 2;
    } else if (MATCH(width, (3 * t1) + correction)) {
        avail = 3;
    } else {
        return -1;
    }

    (*used)++;
    if (*used >= avail) {
        *used = 0;
        (*offset)++;
    }

    TRACE_PRINTLN((val == MARK) ? "MARK" : "SPACE");

    return val;
}
#endif

//+=============================================================================
bool IRrecv::decodeRC5() {
    uint8_t nbits;
    unsigned long data = 0;
    uint8_t used = 0;
    unsigned int offset = 1;  // Skip gap space

    if (results.rawlen < MIN_RC5_MARKS + 2) {
        return false;
    }

// Get start bits
    if (getRClevel(&results, &offset, &used, RC5_UNIT) != MARK) {
        return false;
    }
    if (getRClevel(&results, &offset, &used, RC5_UNIT) != SPACE) {
        return false;
    }
    if (getRClevel(&results, &offset, &used, RC5_UNIT) != MARK) {
        return false;
    }

    /*
     * Get data bits - MSB first
     */
    for (nbits = 0; offset < results.rawlen; nbits++) {
        int levelA = getRClevel(&results, &offset, &used, RC5_UNIT);
        int levelB = getRClevel(&results, &offset, &used, RC5_UNIT);

        if ((levelA == SPACE) && (levelB == MARK)) {
            data = (data << 1) | 1;
        } else if ((levelA == MARK) && (levelB == SPACE)) {
            data = (data << 1) | 0;
        } else {
            return false;
        }
    }

// Success
    results.bits = nbits;
    results.value = data;
    decodedIRData.protocol = RC5;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;
    return true;
}
bool IRrecv::decodeRC5(decode_results *aResults) {
    bool aReturnValue = decodeRC5();
    *aResults = results;
    return aReturnValue;
}
#endif

//+=============================================================================
// RRRR    CCCC   6666
// R   R  C      6
// RRRR   C      6666
// R  R   C      6   6
// R   R   CCCC   666
//
//
#define MIN_RC6_SAMPLES         1

#define RC6_RPT_LENGTH      46000

#define RC6_LEADING_BIT         1
#define RC6_MODE_BITS           3
#define RC6_TOGGLE_BIT          1
#define RC6_ADDRESS_BITS        8
#define RC6_COMMAND_BITS        8

#define RC6_BITS                (RC6_LEADING_BIT + RC6_MODE_BITS + RC6_TOGGLE_BIT + RC6_ADDRESS_BITS + RC6_COMMAND_BITS) // 13

#define RC6_UNIT                444 // (16 cycles of 36 kHz)

#define RC6_HEADER_MARK         (6 * RC6_UNIT) // 2666
#define RC6_HEADER_SPACE        (2 * RC6_UNIT) // 889

#define RC6_TRAILING_SPACE      (6 * RC6_UNIT) // 2666
#define MIN_RC6_MARKS           4 + ((RC6_ADDRESS_BITS + RC6_COMMAND_BITS) / 2) // 12, 4 are for preamble

#define RC6_REPEAT_SPACE        107000 // just a guess

void IRsend::sendRC6(uint32_t data, uint8_t nbits) {
// Set IR carrier frequency
    enableIROut(36);

    noInterrupts();

// Header
    mark(RC6_HEADER_MARK);
    space(RC6_HEADER_SPACE);

// Start bit
    mark(RC6_UNIT);
    space(RC6_UNIT);

// Data
    uint32_t mask = 1UL << (nbits - 1);
    for (uint8_t i = 1; mask; i++, mask >>= 1) {
        // The fourth bit we send is a "double width trailer bit"
        unsigned int t = (i == 4) ? (RC6_UNIT * 2) : (RC6_UNIT);
        if (data & mask) {
            mark(t);
            space(t);
        } else {
            space(t);
            mark(t);
        }
    }

    space(0);  // Always end with the LED off
    interrupts();
}

void IRsend::sendRC6(uint64_t data, uint8_t nbits) {
// Set IR carrier frequency
    enableIROut(36);

    noInterrupts();

// Header
    mark(RC6_HEADER_MARK);
    space(RC6_HEADER_SPACE);

// Start bit
    mark(RC6_UNIT);
    space(RC6_UNIT);

// Data
    uint64_t mask = 1ULL << (nbits - 1);
    for (uint8_t i = 1; mask; i++, mask >>= 1) {
        // The fourth bit we send is a "double width trailer bit"
        unsigned int t = (i == 4) ? (RC6_UNIT * 2) : (RC6_UNIT);
        if (data & mask) {
            mark(t);
            space(t);
        } else {
            space(t);
            mark(t);
        }
    }

    space(0);  // Always end with the LED off
    interrupts();
}

/*
 * We do not wait for the minimal trailing space of 2666 us
 */
void IRsend::sendRC6(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aEnableAutomaticToggle) {

    LongUnion tIRRawData;
    tIRRawData.UByte.LowByte = aCommand;
    tIRRawData.UByte.MidLowByte = aAddress;

    tIRRawData.UWord.HighWord = 0; // must clear high word
    if (aEnableAutomaticToggle) {
        if (sLastSendToggleValue == 0) {
            sLastSendToggleValue = 1;
            // set toggled bit
            DBG_PRINT(F("Set Toggle "));
            tIRRawData.UByte.MidHighByte = 1; // 3 Mode bits are 0
        } else {
            sLastSendToggleValue = 0;
        }
    }

    DBG_PRINT(F("RC6: "));
    DBG_PRINT(F("sLastSendToggleValue="));
    DBG_PRINT(sLastSendToggleValue);
    DBG_PRINT(F(" RawData="));
    DBG_PRINTLN(tIRRawData.ULong, HEX);

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // start bit is sent by sendBiphaseData
        sendRC6(tIRRawData.ULong, RC6_BITS - 1); // -1 since the leading bit is additionally sent by sendRC6

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(RC6_REPEAT_SPACE / 1000);
        }
    }
}

#if defined(USE_STANDARD_DECODE)
bool IRrecv::decodeRC6() {

    // Check we have the right amount of data (). The +3 for initial gap, start bit mark and space
    if (results.rawlen < MIN_RC6_MARKS + 3 && results.rawlen > ((2 * RC6_BITS) + 3)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header "mark" and "space", this must be done for repeat and data
    if (!MATCH_MARK(results.rawbuf[1], RC6_HEADER_MARK) || !MATCH_SPACE(results.rawbuf[2], RC6_HEADER_SPACE)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    /*
     * Decode and check preamble
     *   Start    1  0   0   0 Toggle1 0/1 - MSB of address
     * ______    _    _   _   _ __    _
     *       |__| |__| |_| |_|    |__|_|
     *                         Toggle0
     * ______    _    _   _   _    __ _
     *       |__| |__| |_| |_| |__|  |_|
     */
    if (!decodeBiPhaseData(RC6_LEADING_BIT + RC6_MODE_BITS, 3, 0, RC6_UNIT)) {
        DBG_PRINT(F("RC6: "));
        DBG_PRINTLN(F("Preamble mark or space length is wrong"));
        return false;
    }
    if (results.value != 4) {
        DBG_PRINT(F("RC6: "));
        DBG_PRINT(F("Preamble content "));
        DBG_PRINT(results.value);
        DBG_PRINTLN(F(" is not 4"));
        return false;
    }

    /*
     * Check toggle bit which has double unit length
     * Maybe we do not need to check all the timings
     */
    uint8_t tStartOffset;
    if (MATCH_MARK(results.rawbuf[9], RC6_UNIT) && MATCH_SPACE(results.rawbuf[10], 2 * RC6_UNIT)) {
        // toggle = 0
        if (MATCH_MARK(results.rawbuf[11], 2 * RC6_UNIT)) {
            // Address MSB is 0
            tStartOffset = 13;
        } else if (MATCH_MARK(results.rawbuf[11], 3 * RC6_UNIT)) {
            // Address MSB is 1
            tStartOffset = 12;
        } else {
            DBG_PRINT(F("RC6: "));
            DBG_PRINTLN(F("Toggle mark or space length is wrong"));
            return false;
        }
    } else if (MATCH_MARK(results.rawbuf[9], 3 * RC6_UNIT)) {
        // Toggle = 1
        decodedIRData.flags = IRDATA_TOGGLE_BIT_MASK;
        if (MATCH_SPACE(results.rawbuf[10], 2 * RC6_UNIT)) {
            // Address MSB is 1
            tStartOffset = 12;
        } else if (MATCH_SPACE(results.rawbuf[10], 3 * RC6_UNIT)) {
            // Address MSB is 0
            tStartOffset = 11;
        } else {
            DBG_PRINT(F("RC6: "));
            DBG_PRINTLN(F("Toggle mark or space length is wrong"));
            return false;
        }
    } else {
        DBG_PRINT(F("RC6: "));
        DBG_PRINTLN(F("Toggle mark or space length is wrong"));
        return false;
    }

    /*
     * Get address and command
     */
    if (!decodeBiPhaseData(RC6_ADDRESS_BITS + RC6_COMMAND_BITS, tStartOffset, 0, RC6_UNIT)) {
        DBG_PRINT(F("RC6: "));
        DBG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Success
    LongUnion tValue;
    tValue.ULong = results.value;
    decodedIRData.command = tValue.UByte.LowByte;
    decodedIRData.address = tValue.UByte.MidLowByte;

    // check for repeat, do not check toggle bit yet
    if (results.rawbuf[0] < ((RC6_REPEAT_SPACE + (RC6_REPEAT_SPACE / 2)) / MICROS_PER_TICK)) {
        decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
    }

    decodedIRData.protocol = RC6;
    decodedIRData.numberOfBits = RC6_ADDRESS_BITS + RC6_COMMAND_BITS;

    return true;
}

#else

#warning "Old decoder functions decodeRC5() and decodeRC5(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeRC5() instead."

//+=============================================================================
bool IRrecv::decodeRC6() {
    unsigned int nbits;
    uint32_t data = 0;
    uint8_t used = 0;
    unsigned int offset = 1;  // Skip first space

    if (results.rawlen < MIN_RC6_SAMPLES) {
        return false;
    }

// Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], RC6_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (!MATCH_SPACE(results.rawbuf[offset], RC6_HEADER_SPACE)) {
        return false;
    }
    offset++;

// Get start bit (1)
    if (getRClevel(&results, &offset, &used, RC6_UNIT) != MARK) {
        return false;
    }
    if (getRClevel(&results, &offset, &used, RC6_UNIT) != SPACE) {
        return false;
    }

    for (nbits = 0; offset < results.rawlen; nbits++) {
        int levelA, levelB;  // Next two levels

        levelA = getRClevel(&results, &offset, &used, RC6_UNIT);
        if (nbits == 3) {
            // T bit is double wide; make sure second half matches
            if (levelA != getRClevel(&results, &offset, &used, RC6_UNIT)) {
                return false;
            }
        }

        levelB = getRClevel(&results, &offset, &used, RC6_UNIT);
        if (nbits == 3) {
            // T bit is double wide; make sure second half matches
            if (levelB != getRClevel(&results, &offset, &used, RC6_UNIT)) {
                return false;
            }
        }

        if ((levelA == MARK) && (levelB == SPACE)) {
            data = (data << 1) | 1;  // inverted compared to RC5
        } else if ((levelA == SPACE) && (levelB == MARK)) {
            data = (data << 1) | 0;
        } else {
            return false;            // Error
        }
    }

// Success
    results.bits = nbits;
    results.value = data;
    decodedIRData.protocol = RC6;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;
    return true;
}

bool IRrecv::decodeRC6(decode_results *aResults) {
    bool aReturnValue = decodeRC6();
    *aResults = results;
    return aReturnValue;
}
#endif

//+=============================================================================
void IRsend::sendRC5(uint32_t data, uint8_t nbits) {
    // Set IR carrier frequency
    enableIROut(36);

    noInterrupts();

    // Start
    mark(RC5_UNIT);
    space(RC5_UNIT);
    mark(RC5_UNIT);

    // Data - Biphase code MSB first
    for (uint32_t mask = 1UL << (nbits - 1); mask; mask >>= 1) {
        if (data & mask) {
            space(RC5_UNIT); // 1 is space, then mark
            mark(RC5_UNIT);
        } else {
            mark(RC5_UNIT);
            space(RC5_UNIT);
        }
    }

    space(0);  // Always end with the LED off
    interrupts();
}

/*
 * Not longer required, use sendRC5 instead
 */
void IRsend::sendRC5ext(uint8_t addr, uint8_t cmd, boolean toggle) {
// Set IR carrier frequency
    enableIROut(36);

    uint8_t addressBits = 5;
    uint8_t commandBits = 7;
//    unsigned long nbits = addressBits + commandBits;

// Start
    noInterrupts();

    mark(RC5_UNIT);

// Bit #6 of the command part, but inverted!
    uint8_t cmdBit6 = (1UL << (commandBits - 1)) & cmd;
    if (cmdBit6) {
        // Inverted (1 -> 0 = mark-to-space)
        mark(RC5_UNIT);
        space(RC5_UNIT);
    } else {
        space(RC5_UNIT);
        mark(RC5_UNIT);
    }
    commandBits--;

// Toggle bit
    static int toggleBit = 1;
    if (toggle) {
        if (toggleBit == 0) {
            toggleBit = 1;
        } else {
            toggleBit = 0;
        }
    }
    if (toggleBit) {
        space(RC5_UNIT);
        mark(RC5_UNIT);
    } else {
        mark(RC5_UNIT);
        space(RC5_UNIT);
    }

// Address
    for (uint8_t mask = 1UL << (addressBits - 1); mask; mask >>= 1) {
        if (addr & mask) {
            space(RC5_UNIT); // 1 is space, then mark
            mark(RC5_UNIT);
        } else {
            mark(RC5_UNIT);
            space(RC5_UNIT);
        }
    }

// Command
    for (uint8_t mask = 1UL << (commandBits - 1); mask; mask >>= 1) {
        if (cmd & mask) {
            space(RC5_UNIT); // 1 is space, then mark
            mark(RC5_UNIT);
        } else {
            mark(RC5_UNIT);
            space(RC5_UNIT);
        }
    }

    space(0);  // Always end with the LED off
    interrupts();
}
