/*
 * ir_RC5_RC6.hpp
 *
 *  Contains functions for receiving and sending RC5, RC5X, RC6 protocols
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
#ifndef _IR_RC5_RC6_HPP
#define _IR_RC5_RC6_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
uint8_t sLastSendToggleValue = 1; // To start first command with toggle 0
//uint8_t sLastReceiveToggleValue = 3; // 3 -> start value

//==============================================================================
//     RRRR    CCCC  55555
//     R   R  C      5
//     RRRR   C      5555
//     R  R   C          5
//     R   R   CCCC  5555
//==============================================================================
/*
 Protocol=RC5 Address=0x11 Command=0x36 Raw-Data=0x1476 13 bits MSB first
 + 900,- 900
 +1800,-1750 +1800,- 850 + 900,- 850 + 900,-1750
 + 950,- 850 + 900,- 850 +1800,-1750 + 950,- 850
 +1800
 Sum: 23100

 RC5X with 7.th MSB of command set
 Protocol=RC5 Address=0x11 Command=0x76 Toggle=1 Raw-Data=0xC76 13 bits MSB first
 +1800,-1750
 + 850,- 900 +1800,- 850 + 950,- 850 + 900,-1750
 + 900,- 850 + 950,- 850 +1800,-1750 + 900,- 850
 +1800
 Sum: 23050
 */
//
// see: https://www.sbprojects.net/knowledge/ir/rc5.php
// https://en.wikipedia.org/wiki/Manchester_code
// https://forum.arduino.cc/t/sending-rc-5-extended-code-using-irsender/1045841/10 - Protocol Maranz Extended
// mark->space => 0
// space->mark => 1
// MSB first 1 start bit, 1 field bit, 1 toggle bit + 5 bit address + 6 bit command, no stop bit
// Field bit is 1 for RC5 and inverted 7. command bit for RC5X. That way the first 64 commands of RC5X remain compatible with the original RC5.
// SF TAAA  AACC CCCC
// IR duty factor is 25%,
//
#define RC5_ADDRESS_BITS        5
#define RC5_COMMAND_BITS        6
#define RC5_COMMAND_FIELD_BIT   1
#define RC5_TOGGLE_BIT          1

#define RC5_BITS            (RC5_COMMAND_FIELD_BIT + RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS) // 13

#define RC5_UNIT            889 // 32 periods of 36 kHz (888.8888)

#define MIN_RC5_MARKS       ((RC5_BITS + 1) / 2) // 7. Divided by 2 to handle the bit sequence of 01010101 which gives one mark and space for each 2 bits

#define RC5_DURATION        (15L * RC5_UNIT) // 13335
#define RC5_REPEAT_PERIOD   (128L * RC5_UNIT) // 113792
#define RC5_REPEAT_DISTANCE (RC5_REPEAT_PERIOD - RC5_DURATION) // 100 ms
#define RC5_MAXIMUM_REPEAT_DISTANCE     (RC5_REPEAT_DISTANCE + (RC5_REPEAT_DISTANCE / 4)) // Just a guess

/************************************
 * Start of send and decode functions
 ************************************/

/**
 * @param aCommand If aCommand is >=0x40 then we switch automatically to RC5X.
 * @param aEnableAutomaticToggle Send toggle bit according to the state of the static sLastSendToggleValue variable.
 */
void IRsend::sendRC5(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle) {
    // Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

    uint16_t tIRData = ((aAddress & 0x1F) << RC5_COMMAND_BITS);

    if (aCommand < 0x40) {
        // Auto discovery of RC5X, set field bit to 1
        tIRData |= 1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS);
    } else {
        // Mask bit 7 of command and let field bit 0
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

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // start bit is sent by sendBiphaseData
        sendBiphaseData(RC5_UNIT, tIRData, RC5_BITS);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(RC5_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        }
    }
}

/**
 * Try to decode data as RC5 protocol
 *                             _   _   _   _   _   _   _   _   _   _   _   _   _
 * Clock                 _____| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
 *                                ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^    End of each data bit period
 *                               _   _     - Mark
 * 2 Start bits for RC5    _____| |_| ...  - Data starts with a space->mark bit
 *                                         - Space
 *                               _
 * 1 Start bit for RC5X    _____| ...
 *
 */
bool IRrecv::decodeRC5() {
    uint8_t tBitIndex;
    uint32_t tDecodedRawData = 0;

    // Set Biphase decoding start values
    initBiphaselevel(1, RC5_UNIT); // Skip gap space

    // Check we have the right amount of data (11 to 26). The +2 is for initial gap and start bit mark.
    if (decodedIRData.rawDataPtr->rawlen < ((RC5_BITS + 1) / 2) + 2 && (RC5_BITS + 2) < decodedIRData.rawDataPtr->rawlen) {
        // no debug output, since this check is mainly to determine the received protocol
        IR_DEBUG_PRINT(F("RC5: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not between 9 and 15"));
        return false;
    }

// Check start bit, the first space is included in the gap
    if (getBiphaselevel() != MARK) {
        IR_DEBUG_PRINT(F("RC5: "));
        IR_DEBUG_PRINTLN(F("first getBiphaselevel() is not MARK"));
        return false;
    }

    /*
     * Get data bits - MSB first
     */
    for (tBitIndex = 0; sBiphaseDecodeRawbuffOffset < decodedIRData.rawDataPtr->rawlen; tBitIndex++) {
        // get next 2 levels and check for transition
        uint8_t tStartLevel = getBiphaselevel();
        uint8_t tEndLevel = getBiphaselevel();

        if ((tStartLevel == SPACE) && (tEndLevel == MARK)) {
            // we have a space to mark transition here
            tDecodedRawData = (tDecodedRawData << 1) | 1;
        } else if ((tStartLevel == MARK) && (tEndLevel == SPACE)) {
            // we have a mark to space transition here
            tDecodedRawData = (tDecodedRawData << 1) | 0;
        } else {
#if defined(LOCAL_DEBUG)
            Serial.print(F("RC5: "));
            Serial.println(F("no transition found, decode failed"));
#endif
            return false;
        }
    }

    // Success
    decodedIRData.numberOfBits = tBitIndex; // must be RC5_BITS

    LongUnion tValue;
    tValue.ULong = tDecodedRawData;
    decodedIRData.decodedRawData = tDecodedRawData;
    decodedIRData.command = tValue.UByte.LowByte & 0x3F;
    decodedIRData.address = (tValue.UWord.LowWord >> RC5_COMMAND_BITS) & 0x1F;

    // Get the inverted 7. command bit for RC5X, the inverted value is always 1 for RC5 and serves as a second start bit.
    if ((tValue.UWord.LowWord & (1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS))) == 0) {
        decodedIRData.command += 0x40;
    }

    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    if (tValue.UByte.MidLowByte & 0x8) {
        decodedIRData.flags = IRDATA_FLAGS_TOGGLE_BIT | IRDATA_FLAGS_IS_MSB_FIRST;
    }
    decodedIRData.protocol = RC5;

    // check for repeat
    checkForRepeatSpaceTicksAndSetFlag(RC5_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}

//+=============================================================================
// RRRR    CCCC   6666
// R   R  C      6
// RRRR   C      6666
// R  R   C      6   6
// R   R   CCCC   666
//+=============================================================================
//
/*
 Protocol=RC6 Address=0xF1 Command=0x76 Raw-Data=0xF176 20 bits MSB first
 +2650,- 850
 + 500,- 850 + 500,- 400 + 450,- 450 + 450,- 850
 +1400,- 400 + 450,- 450 + 450,- 450 + 450,- 900
 + 450,- 450 + 450,- 400 + 950,- 850 + 900,- 450
 + 450,- 450 + 450,- 850 + 950,- 400 + 450,- 900
 + 450
 Sum: 23150
 */
// Frame RC6:   1 start bit + 1 Bit "1" + 3 mode bits (000) + 1 toggle bit + 8 address + 8 command bits + 2666us pause
// Frame RC6A:  1 start bit + 1 Bit "1" + 3 mode bits (110) + 1 toggle bit + "1" + 14 customer bits + 8 system bits + 8 command bits (=31bits) + 2666us pause
// !!! toggle bit has another timing :-( !!!
// mark->space => 1
// space->mark => 0
// https://www.sbprojects.net/knowledge/ir/rc6.php
// https://www.mikrocontroller.net/articles/IRMP_-_english#RC6_.2B_RC6A
// https://en.wikipedia.org/wiki/Manchester_code
#define MIN_RC6_SAMPLES         1

#define RC6_RPT_LENGTH      46000

#define RC6_LEADING_BIT         1
#define RC6_MODE_BITS           3 // never seen others than all 0 for Philips TV
#define RC6_TOGGLE_BIT          1 // toggles at every key press. Can be used to distinguish repeats from 2 key presses and has another timing :-(.
#define RC6_TOGGLE_BIT_INDEX    RC6_MODE_BITS //  fourth position, index = 3
#define RC6_ADDRESS_BITS        8
#define RC6_COMMAND_BITS        8

#define RC6_BITS            (RC6_LEADING_BIT + RC6_MODE_BITS + RC6_TOGGLE_BIT + RC6_ADDRESS_BITS + RC6_COMMAND_BITS) // 21

#define RC6_UNIT            444 // 16 periods of 36 kHz (444.4444)

#define RC6_HEADER_MARK     (6 * RC6_UNIT) // 2666
#define RC6_HEADER_SPACE    (2 * RC6_UNIT) // 889

#define RC6_TRAILING_SPACE  (6 * RC6_UNIT) // 2666
#define MIN_RC6_MARKS       4 + ((RC6_ADDRESS_BITS + RC6_COMMAND_BITS) / 2) // 12, 4 are for preamble

#define RC6_REPEAT_DISTANCE 107000 // just a guess but > 2.666ms
#define RC6_MAXIMUM_REPEAT_DISTANCE     (RC6_REPEAT_DISTANCE + (RC6_REPEAT_DISTANCE / 4)) // Just a guess

/**
 * Main RC6 send function
 */
void IRsend::sendRC6(uint32_t aRawData, uint8_t aNumberOfBitsToSend) {
    sendRC6Raw(aRawData, aNumberOfBitsToSend);
}
void IRsend::sendRC6Raw(uint32_t aRawData, uint8_t aNumberOfBitsToSend) {
// Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

// Header
    mark(RC6_HEADER_MARK);
    space(RC6_HEADER_SPACE);

// Start bit
    mark(RC6_UNIT);
    space(RC6_UNIT);

// Data MSB first
    uint32_t mask = 1UL << (aNumberOfBitsToSend - 1);
    for (uint_fast8_t i = 1; mask; i++, mask >>= 1) {
        // The fourth bit we send is the "double width toggle bit"
        unsigned int t = (i == (RC6_TOGGLE_BIT_INDEX + 1)) ? (RC6_UNIT * 2) : (RC6_UNIT);
        if (aRawData & mask) {
            mark(t);
            space(t);
        } else {
            space(t);
            mark(t);
        }
    }
}

/**
 * Send RC6 64 bit raw data
 * Can be used to send RC6A with ?31? data bits
 */
void IRsend::sendRC6(uint64_t aRawData, uint8_t aNumberOfBitsToSend) {
    sendRC6Raw(aRawData, aNumberOfBitsToSend);
}
void IRsend::sendRC6Raw(uint64_t aRawData, uint8_t aNumberOfBitsToSend) {
// Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

// Header
    mark(RC6_HEADER_MARK);
    space(RC6_HEADER_SPACE);

// Start bit
    mark(RC6_UNIT);
    space(RC6_UNIT);

// Data MSB first
    uint64_t mask = 1ULL << (aNumberOfBitsToSend - 1);
    for (uint_fast8_t i = 1; mask; i++, mask >>= 1) {
        // The fourth bit we send is the "double width toggle bit"
        unsigned int t = (i == (RC6_TOGGLE_BIT_INDEX + 1)) ? (RC6_UNIT * 2) : (RC6_UNIT);
        if (aRawData & mask) {
            mark(t);
            space(t);
        } else {
            space(t);
            mark(t);
        }
    }
}

/**
 * Assemble raw data for RC6 from parameters and toggle state and send
 * We do not wait for the minimal trailing space of 2666 us
 * @param aEnableAutomaticToggle Send toggle bit according to the state of the static sLastSendToggleValue variable.
 */
void IRsend::sendRC6(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle) {

    LongUnion tIRRawData;
    tIRRawData.UByte.LowByte = aCommand;
    tIRRawData.UByte.MidLowByte = aAddress;

    tIRRawData.UWord.HighWord = 0; // must clear high word
    if (aEnableAutomaticToggle) {
        if (sLastSendToggleValue == 0) {
            sLastSendToggleValue = 1;
            // set toggled bit
            IR_DEBUG_PRINT(F("Set Toggle "));
            tIRRawData.UByte.MidHighByte = 1; // 3 Mode bits are 0
        } else {
            sLastSendToggleValue = 0;
        }
    }

#if defined(LOCAL_DEBUG)
    Serial.print(F("RC6: "));
    Serial.print(F("sLastSendToggleValue="));
    Serial.print (sLastSendToggleValue);
    Serial.print(F(" RawData="));
    Serial.println(tIRRawData.ULong, HEX);
#endif

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // start and leading bits are sent by sendRC6
        sendRC6Raw(tIRRawData.ULong, RC6_BITS - 1); // -1 since the leading bit is additionally sent by sendRC6

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(RC6_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        }
    }
}

/**
 * Try to decode data as RC6 protocol
 */
bool IRrecv::decodeRC6() {
    uint8_t tBitIndex;
    uint32_t tDecodedRawData = 0;

    // Check we have the right amount of data (). The +3 for initial gap, start bit mark and space
    if (decodedIRData.rawDataPtr->rawlen < MIN_RC6_MARKS + 3 && (RC6_BITS + 3) < decodedIRData.rawDataPtr->rawlen) {
        IR_DEBUG_PRINT(F("RC6: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not between 15 and 25"));
        return false;
    }

    // Check header "mark" and "space", this must be done for repeat and data
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], RC6_HEADER_MARK)
            || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], RC6_HEADER_SPACE)) {
        // no debug output, since this check is mainly to determine the received protocol
        IR_DEBUG_PRINT(F("RC6: "));
        IR_DEBUG_PRINTLN(F("Header mark or space length is wrong"));
        return false;
    }

    // Set Biphase decoding start values
    initBiphaselevel(3, RC6_UNIT); // Skip gap-space and start-bit mark + space

// Process first bit, which is known to be a 1 (mark->space)
    if (getBiphaselevel() != MARK) {
        IR_DEBUG_PRINT(F("RC6: "));
        IR_DEBUG_PRINTLN(F("first getBiphaselevel() is not MARK"));
        return false;
    }
    if (getBiphaselevel() != SPACE) {
        IR_DEBUG_PRINT(F("RC6: "));
        IR_DEBUG_PRINTLN(F("second getBiphaselevel() is not SPACE"));
        return false;
    }

    for (tBitIndex = 0; sBiphaseDecodeRawbuffOffset < decodedIRData.rawDataPtr->rawlen; tBitIndex++) {
        uint8_t tStartLevel; // start level of coded bit
        uint8_t tEndLevel;   // end level of coded bit

        tStartLevel = getBiphaselevel();
        if (tBitIndex == RC6_TOGGLE_BIT_INDEX) {
            // Toggle bit is double wide; make sure second half is equal first half
            if (tStartLevel != getBiphaselevel()) {
#if defined(LOCAL_DEBUG)
                Serial.print(F("RC6: "));
                Serial.println(F("Toggle mark or space length is wrong"));
#endif
                return false;
            }
        }

        tEndLevel = getBiphaselevel();
        if (tBitIndex == RC6_TOGGLE_BIT_INDEX) {
            // Toggle bit is double wide; make sure second half matches
            if (tEndLevel != getBiphaselevel()) {
#if defined(LOCAL_DEBUG)
                Serial.print(F("RC6: "));
                Serial.println(F("Toggle mark or space length is wrong"));
#endif
                return false;
            }
        }

        /*
         * Determine tDecodedRawData bit value by checking the transition type
         */
        if ((tStartLevel == MARK) && (tEndLevel == SPACE)) {
            // we have a mark to space transition here
            tDecodedRawData = (tDecodedRawData << 1) | 1;  // inverted compared to RC5
        } else if ((tStartLevel == SPACE) && (tEndLevel == MARK)) {
            // we have a space to mark transition here
            tDecodedRawData = (tDecodedRawData << 1) | 0;
        } else {
#if defined(LOCAL_DEBUG)
            Serial.print(F("RC6: "));
            Serial.println(F("Decode failed"));
#endif
            // we have no transition here or one level is -1 -> error
            return false;            // Error
        }
    }

// Success
    decodedIRData.numberOfBits = tBitIndex;

    LongUnion tValue;
    tValue.ULong = tDecodedRawData;
    decodedIRData.decodedRawData = tDecodedRawData;

    if (tBitIndex < 36) {
        // RC6 8 address bits, 8 command bits
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
        decodedIRData.command = tValue.UByte.LowByte;
        decodedIRData.address = tValue.UByte.MidLowByte;
        // Check for toggle flag
        if ((tValue.UByte.MidHighByte & 1) != 0) {
            decodedIRData.flags = IRDATA_FLAGS_TOGGLE_BIT | IRDATA_FLAGS_IS_MSB_FIRST;
        }
        if (tBitIndex > 20) {
            decodedIRData.flags |= IRDATA_FLAGS_EXTRA_INFO;
        }
    } else {
        // RC6A - 32 bits
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
        if ((tValue.UByte.MidLowByte & 0x80) != 0) {
            decodedIRData.flags = IRDATA_FLAGS_TOGGLE_BIT | IRDATA_FLAGS_IS_MSB_FIRST;
        }
        tValue.UByte.MidLowByte &= 0x87F; // mask toggle bit
        decodedIRData.command = tValue.UByte.LowByte;
        decodedIRData.address = tValue.UByte.MidLowByte;
    }

    // check for repeat, do not check toggle bit yet
    checkForRepeatSpaceTicksAndSetFlag(RC6_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    decodedIRData.protocol = RC6;
    return true;
}

/*********************************************************************************
 * Old deprecated functions, kept for backward compatibility to old 2.0 tutorials
 *********************************************************************************/

/**
 * Old version with 32 bit data
 */
void IRsend::sendRC5(uint32_t data, uint8_t nbits) {
    // Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

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
}

/*
 * Not longer required, use sendRC5(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle) instead
 */
void IRsend::sendRC5ext(uint8_t addr, uint8_t cmd, bool toggle) {
// Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

    uint8_t addressBits = 5;
    uint8_t commandBits = 7;
//    unsigned long nbits = addressBits + commandBits;

// Start
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
    for (uint_fast8_t mask = 1UL << (addressBits - 1); mask; mask >>= 1) {
        if (addr & mask) {
            space(RC5_UNIT); // 1 is space, then mark
            mark(RC5_UNIT);
        } else {
            mark(RC5_UNIT);
            space(RC5_UNIT);
        }
    }

// Command
    for (uint_fast8_t mask = 1UL << (commandBits - 1); mask; mask >>= 1) {
        if (cmd & mask) {
            space(RC5_UNIT); // 1 is space, then mark
            mark(RC5_UNIT);
        } else {
            mark(RC5_UNIT);
            space(RC5_UNIT);
        }
    }
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_RC5_RC6_HPP
