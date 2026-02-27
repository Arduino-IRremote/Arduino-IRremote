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
 * Copyright (c) 2020-2026 Armin Joachimsmeyer
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

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
uint8_t sLastSendToggleValue = 1; // To start first command with toggle 0. Only required for biphase protocols.

//==============================================================================
//     RRRR    CCCC  55555
//     R   R  C      5
//     RRRR   C      5555
//     R  R   C          5
//     R   R   CCCC  5555
//==============================================================================
/*
 Protocol=RC5 Address=0x11, Command=0x36, Raw-Data=0x1476, 13 bits, MSB first
 + 950,- 850
 +1850,-1700 +1850,- 850 + 900,- 850 +1000,-1700
 + 950,- 850 + 950,- 800 +1850,-1750 + 950,- 850
 +1850
 Duration=23300us

 RC5X with 7.th MSB of command set as 1
 Protocol=RC5 Address=0x11, Command=0x76, Toggle=1, Raw-Data=0xC76, 13 bits, MSB first
 +1900,-1700
 + 950,- 850 +1850,- 800 + 950,- 800 +1000,-1700
 +1000,- 750 + 950,- 900 +1800,-1700 +1000,- 800
 +1850
 Duration=23250us

 Protocol=Marantz Address=0x11, Command=0x76, Extra=0x9, Toggle=1, Raw-Data=0x31D89, 19 bits, MSB first
 +1850,-1700
 + 950,- 850 +1850,- 850 + 950,- 800 +1000,-1700
 + 950,-4350 + 950,- 900 +1850,-1700 + 900,- 850
 +1850,- 850 + 950,- 800 +1000,-1700 +1800,- 900
 + 900,-1750 + 950
 Duration=38400us
 */
//
// see: https://www.sbprojects.net/knowledge/ir/rc5.php
// https://en.wikipedia.org/wiki/Manchester_code
// https://en.wikipedia.org/wiki/RC-5
// mark->space => 0
// space->mark => 1
// MSB first, 1 start bit, 1 field bit, 1 toggle bit + 5 bit address + 6 bit command, no stop bit 14 bits incl. field bit and start bit
// Field bit is 1 for RC5 and 0 (=inverted 7. command bit) for RC5X. That way the first 64 commands of RC5X are indistinguishable from RC5.
// SF TAAA  AACC CCCC
// IR duty factor is 25%
//
// MARANTZ
// https://forum.arduino.cc/t/sending-rc-5-extended-code-using-irsender/1045841/10 - Protocol Maranz Extended
// Marantz uses RC5X and adds a a pause after the address / first 8 bits
// After the 6 bit command (of RC5) an additional 6 bit command extension is sent -> 20 bits incl. field bit and start bit
//
#define RC5_ADDRESS_BITS        5
#define RC5_COMMAND_BITS        6
#define RC5_COMMAND_FIELD_BIT   1
#define RC5_TOGGLE_BIT          1

#define RC5_BITS            (RC5_COMMAND_FIELD_BIT + RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS) // 13

#define RC5_UNIT            889 // 32 periods of 36 kHz (888.8888)

#define MIN_RC5_MARKS       ((RC5_BITS + 1) / 2) // 7 - Divided by 2 to handle the bit sequence of 01010101 which gives one mark and space for each 2 bits

#define RC5_DURATION        (26 * RC5_UNIT) // 23114
#define RC5_REPEAT_PERIOD   (128L * RC5_UNIT) // 113792
#define RC5_REPEAT_DISTANCE (RC5_REPEAT_PERIOD - RC5_DURATION) // 90 ms
#define RC5_MAXIMUM_REPEAT_DISTANCE (RC5_REPEAT_DISTANCE + (RC5_REPEAT_DISTANCE / 4)) // Just a guess

#define MARANTZ_COMMAND_EXTENSION_BITS      6
#define MARANTZ_BITS                (RC5_COMMAND_FIELD_BIT + RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS + MARANTZ_COMMAND_EXTENSION_BITS) // 19
#define MARANTZ_PAUSE_BIT_INDEX     (RC5_COMMAND_FIELD_BIT + RC5_TOGGLE_BIT + RC5_ADDRESS_BITS) // 7
#define MARANTZ_PAUSE_DURATION      (4 * RC5_UNIT) // 3556

#define MARANTZ_DURATION            (42L * RC5_UNIT) // RC5 + 6 Extension bits + 4 pause units = 37338
#define MARANTZ_REPEAT_DISTANCE     (RC5_REPEAT_PERIOD - MARANTZ_DURATION) // 100 ms

/************************************
 * Start of send and decode functions
 ************************************/

/*
 * Just in case to permanently send it as 0 because start value is 1,
 * or to reset toggle bit after a transmission with automatic toggling.
 * @param aRC5ToggleBitValue 0 or 1, only LSB is taken
 */
void IRsend::setRC5ToggleBitValue(uint8_t aRC5ToggleBitValue){
    sLastSendToggleValue = aRC5ToggleBitValue & 0x01;
}

/**
 * !!! Not tested, because no Marantz remote was at hand and no receive function was contributed!!!
 * Send function for the Marantz version of RC5(X) with a pause of 4 * RC5_UNIT after address / first 8 bits
 * and before the bits of command and command extension.
 * Marantz seems to require at least one repetition with toggle bit set
 * @param aAddress 5 address bits to be sent first (MSB first).
 * @param aCommand 6 or 7 (RC5X) bits. If aCommand is >=0x40 then we switch automatically to RC5X.
 * @param aMarantzExtension 6 bit command extension which is sent after 6 command bits of aCommand. aCommand and aMarantzExtension are sent after a short pause.
 * @param aEnableAutomaticToggle Send toggle bit according to the state of the static sLastSendToggleValue variable.
 */
void IRsend::sendRC5Marantz(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint8_t aMarantzExtension,
        bool aEnableAutomaticToggle) {

    // Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

    uint16_t tIRData = (aAddress & 0x1F);

    /*
     * Process field bit (the bit after start bit)
     * Field bit is 1 for RC5 and 0 (inverted 7. command bit) for RC5X
     */
    if (aCommand >= 0x40) { // Auto discovery of RC5X
        // Mask bit 7 of command to set field bit to 0 (inverted 1) for RC5X
        aCommand &= 0x3F;
    } else {
        // Set field bit to 1 for RC5
        tIRData |= 1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS); // 1 << 6 = 0x40
    }

    // Combine command and command extension for the 2nd part of data to be sent after the pause
    uint16_t tIRExtData = (aCommand << MARANTZ_COMMAND_EXTENSION_BITS);
    // Set the Marantz command extension bits
    tIRExtData |= (aMarantzExtension & 0x3F);

    if (aEnableAutomaticToggle) {
        // invert toggle bit if enabled
        sLastSendToggleValue ^= 1;
    }

    // set toggled bit independent of current state of aEnableAutomaticToggle
    if (sLastSendToggleValue) {
        tIRData |= 1 << RC5_ADDRESS_BITS;
    }

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // start bit is sent by sendBiphaseData followed by the field bit and toggle bit and address
        sendBiphaseData(RC5_UNIT, tIRData, RC5_COMMAND_FIELD_BIT + RC5_TOGGLE_BIT + RC5_ADDRESS_BITS);
        // pause before the bits of command and command extension to indicate that it's Marantz-RC5x
        space(MARANTZ_PAUSE_DURATION); // Marantz-RC5x has a pause before the bits of command and command extension
        // send command and command extension
        sendBiphaseData(RC5_UNIT, tIRExtData, RC5_COMMAND_BITS + MARANTZ_COMMAND_EXTENSION_BITS, false);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(MARANTZ_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        }
    }
}

/**
 * @param aAddress 5 address bits to be sent first (MSB first).
 * @param aCommand 6 or 7 (RC5X) bits. If aCommand is >=0x40 then we switch automatically to RC5X.
 * @param aEnableAutomaticToggle Send toggle bit according to the state of the static sLastSendToggleValue variable.
 */
void IRsend::sendRC5(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle) {

    // Set IR carrier frequency
    enableIROut (RC5_RC6_KHZ);

    uint16_t tIRData = ((aAddress & 0x1F) << (RC5_COMMAND_BITS)); // << 6

    /*
     * Process field bit (the bit after start bit)
     * Field bit is 1 for RC5 and 0 (inverted 7. command bit) for RC5X
     */
    if (aCommand >= 0x40) { // Auto discovery of RC5X
        // Mask bit 7 of command to set field bit to 0 (inverted 1) for RC5X
        aCommand &= 0x3F;
    } else {
        // Set field bit to 1 for RC5
        tIRData |= 1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS); // 1 << 12 = 0x1000
    }
    tIRData |= aCommand;


    if (aEnableAutomaticToggle) {
        // invert toggle bit if enabled
        sLastSendToggleValue ^= 1;
    }

    // set toggled bit independent of current state of aEnableAutomaticToggle
    if (sLastSendToggleValue) {
        tIRData |= 1 << (RC5_ADDRESS_BITS + RC5_COMMAND_BITS);
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

#if defined(DECODE_RC5) || defined(DECODE_MARANTZ) || defined(DECODE_RC6)
void IRrecv::initBiphaselevel(uint_fast8_t aRCDecodeRawbuffOffset, uint16_t aBiphaseTimeUnit) {
    irparams.RawbuffOffsetForNextBiphaseLevel = aRCDecodeRawbuffOffset;
    irparams.BiphaseTimeUnit = aBiphaseTimeUnit;
    irparams.AlreadyUsedTimingIntervalsOfCurrentInterval = 0;
}

/*
 * Margin is 1/2 of unit.
 */
uint8_t IRrecv::getNumberOfUnitsInInterval(uint16_t aCurrentInterval, uint16_t aTimeUnit) {
    return (aCurrentInterval + (aTimeUnit / 2)) / aTimeUnit;
}

/**
 * Gets the level of one time interval (aBiphaseTimeUnit) at a time from the raw buffer.
 * The RC5/6 decoding is easier if the data is broken into time intervals.
 * E.g. if the buffer has mark for 2 time intervals and space for 1,
 * successive calls to getBiphaselevel will return 1, 1, 0.
 *
 *               _   _   _   _   _   _   _   _   _   _   _   _   _
 *         _____| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
 *                ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^    Significant clock edge /sample point for bit value
 *                  ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^      End of clock / each data bit period
 *               _     _   _   ___   _     ___     ___   _   - Mark
 * Data    _____| |___| |_| |_|   |_| |___|   |___|   |_| |  - Data starts with a mark->space bit
 *                1   0   0   0   1   1   0   1   0   1   1  - Space
 * A mark to space at a significant clock edge results in a 1
 * A space to mark at a significant clock edge results in a 0
 * Returns current level [MARK or SPACE] or NO_MARK_OR_SPACE for error (measured time interval is not a multiple of BiphaseTimeUnit).
 */
uint_fast8_t IRrecv::getBiphaselevel() {
    uint_fast8_t tLevelOfCurrentInterval; // Return value: 0 (SPACE) or 1 (MARK)

    if (irparams.RawbuffOffsetForNextBiphaseLevel >= decodedIRData.rawlen) {
        return SPACE;  // After end of recorded buffer, assume space.
    }

    tLevelOfCurrentInterval = (irparams.RawbuffOffsetForNextBiphaseLevel) & 1; // on odd rawbuf offsets we have mark timings

    /*
     * Refill NumberOfTimingIntervalsInCurrentInterval if AlreadyUsedTimingIntervalsOfCurrentInterval is 0
     * i.e. we went to next rawbuf interval
     */
    if (irparams.AlreadyUsedTimingIntervalsOfCurrentInterval == 0) {
        uint16_t tCurrentIntervalWithMicros = irparams.rawbuf[irparams.RawbuffOffsetForNextBiphaseLevel] * MICROS_PER_TICK;
        uint16_t tMarkExcessCorrectionMicros = (tLevelOfCurrentInterval == MARK) ? MARK_EXCESS_MICROS : -MARK_EXCESS_MICROS;
        irparams.NumberOfTimingIntervalsInCurrentInterval = getNumberOfUnitsInInterval(
                tCurrentIntervalWithMicros + tMarkExcessCorrectionMicros, irparams.BiphaseTimeUnit);
        // 666 is 3/4 of RC5 time unit
        TRACE_PRINT(tLevelOfCurrentInterval);
        TRACE_PRINT(F("="));
        TRACE_PRINT(tCurrentIntervalWithMicros + tMarkExcessCorrectionMicros);
        TRACE_PRINT(F(" "));
        TRACE_PRINTLN(irparams.NumberOfTimingIntervalsInCurrentInterval);
    }
    if (irparams.NumberOfTimingIntervalsInCurrentInterval == 0) {
        return NO_MARK_OR_SPACE;
    }

// We use another interval from tCurrentTimingIntervals
    irparams.AlreadyUsedTimingIntervalsOfCurrentInterval++;

    if (irparams.AlreadyUsedTimingIntervalsOfCurrentInterval >= irparams.NumberOfTimingIntervalsInCurrentInterval) {
        /*
         * We have used all intervals of current interval (mark or space duration in rawbuf), go to next rawbuf interval
         */
        irparams.AlreadyUsedTimingIntervalsOfCurrentInterval = 0;
        irparams.RawbuffOffsetForNextBiphaseLevel++;
    }


    return tLevelOfCurrentInterval;
}
# endif // defined(DECODE_RC5) || defined(DECODE_MARANTZ) || defined(DECODE_RC6)

#if defined(DECODE_RC5) || defined(DECODE_MARANTZ)
/**
 * Try to decode data as RC5 protocol
 *  mark->space => 0
 *  space->mark => 1
 *                             _   _   _   _   _   _   _   _   _   _   _   _   _
 * Clock                 _____| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
 *                                ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^  End of clock / each data bit period
 *                              ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^    Significant clock edge /sample point for bit value
 *                               _   _
 * 2 Start bits for RC5    _____| |_| ..|...|..
 *
 *                               _
 * 1 Start bit for RC5X    _____| ..|...|...|..
 *
 */
bool IRrecv::decodeRC5() {
    uint8_t tBitIndex;
    uint32_t tDecodedRawData = 0;

    // Set Biphase decoding start values
    initBiphaselevel(1, RC5_UNIT); // Skip gap space

    // Check we have the right amount of data (11 to 26). The +2 is for initial gap and start bit mark.
    if (decodedIRData.rawlen < ((RC5_BITS + 1) / 2) + 2 && (RC5_BITS + 2) < decodedIRData.rawlen) {
        // no debug output, since this check is mainly to determine the received protocol
        DEBUG_PRINT(F("RC5: Data length="));
        DEBUG_PRINT(decodedIRData.rawlen);
        DEBUG_PRINTLN(F(" is not between 9 and 15"));
        return false;
    }

    // Check length of first mark / header
    if (irparams.rawbuf[1] > (((2 * RC5_UNIT) + (RC5_UNIT / 2)) / MICROS_PER_TICK)) {
        DEBUG_PRINTLN(F("RC5: first MARK is too long"));
        return false;
    }

    // Check start bit, the first space is included in the gap
    if (getBiphaselevel() != MARK) {
        DEBUG_PRINTLN(F("RC5: first getBiphaselevel() is not MARK"));
        return false;
    }

    /*
     * Get data bits - MSB first
     */
#if defined(DECODE_MARANTZ)
    bool RC5Marantz = false;
#endif
    for (tBitIndex = 0; irparams.RawbuffOffsetForNextBiphaseLevel < decodedIRData.rawlen; tBitIndex++) {
        // get next 2 levels and check for transition
        uint8_t tStartLevel = getBiphaselevel();
        uint8_t tEndLevel = getBiphaselevel();

        if ((tStartLevel == SPACE) && (tEndLevel == MARK)) {
            // we have a space to mark transition here
            tDecodedRawData = (tDecodedRawData << 1) | 1;
        } else if ((tStartLevel == MARK) && (tEndLevel == SPACE)) {
            // we have a mark to space transition here
            tDecodedRawData = (tDecodedRawData << 1) | 0;
#if defined(DECODE_MARANTZ)
        } else if (tBitIndex == MARANTZ_PAUSE_BIT_INDEX) {
            /*
             * Check for RC5 Marantz format i.e. long space after 8 bits (including start bit)
             * Check if timing buffer contains a pause, which is longer than 3/4 the expected pause
             * Some dumb compiler throw a warning: comparison is always false due to limited range of data type [-Wtype-limits] which is nonsense
             */
            if (irparams.rawbuf[irparams.RawbuffOffsetForNextBiphaseLevel] > ((MARANTZ_PAUSE_DURATION * 3L) / (MICROS_PER_TICK * 4L))) { // 3556 * 3 / 200 = 53
                RC5Marantz = true;
                DEBUG_PRINTLN(F("Marantz detected"));
                /*
                 * Here we are at the space interval and have 4 cases:
                 * With 0 -> mark+space and 1 -> space+mark and pause = 4*space we get the following space length
                 * 0 | pause | 0 -> 5*space
                 * 0 | pause | 1 -> 6*space
                 * 1 | pause | 0 -> 4*space
                 * 1 | pause | 1 -> 5*space
                 */
                irparams.AlreadyUsedTimingIntervalsOfCurrentInterval += 2; // We already consumed 2 to get here
                if (irparams.AlreadyUsedTimingIntervalsOfCurrentInterval >= irparams.NumberOfTimingIntervalsInCurrentInterval){
                    irparams.RawbuffOffsetForNextBiphaseLevel++;
                }
            }
#endif
        } else {
            DEBUG_PRINT(F("RC5: no transition found, decode failed, BitIndex="));
            DEBUG_PRINTLN(tBitIndex);
            return false;
        }
    }

    // Success

    LongUnion tValue;
    tValue.ULong = tDecodedRawData;
    decodedIRData.decodedRawData = tDecodedRawData;

#if defined(DECODE_MARANTZ)
    if (RC5Marantz) {
        decodedIRData.numberOfBits = tBitIndex - 1; // Above we count space handling as bit

        decodedIRData.extra = tValue.UWord.LowWord & 0x3F;
        decodedIRData.command = (tValue.UWord.LowWord >> MARANTZ_COMMAND_EXTENSION_BITS) & 0x3F;
        decodedIRData.address = (tValue.ULong >> (RC5_COMMAND_BITS + MARANTZ_COMMAND_EXTENSION_BITS)) & 0x1F;

        // Get the inverted 7. command bit to decide if we have RC5X. For RC5, the inverted value is always 1 and serves as a second start bit.
        if ((tValue.ULong & (1L << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS + MARANTZ_COMMAND_EXTENSION_BITS))) == 0) {
            // Here we have detected RC5X!
            decodedIRData.command += 0x40;
        }

        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST | IRDATA_FLAGS_EXTRA_INFO;
        if (tValue.ULong & (1L << (RC5_ADDRESS_BITS + RC5_COMMAND_BITS + MARANTZ_COMMAND_EXTENSION_BITS))) {
            decodedIRData.flags |= IRDATA_FLAGS_TOGGLE_BIT;
        }
        decodedIRData.protocol = MARANTZ;
    } else {
#endif
    decodedIRData.numberOfBits = tBitIndex; // must be RC5_BITS

    decodedIRData.command = tValue.UByte.LowByte & 0x3F;
    decodedIRData.address = (tValue.UWord.LowWord >> RC5_COMMAND_BITS) & 0x1F;

    // Get the inverted 7. command bit to decide if we have RC5X. For RC5, the inverted value is always 1 and serves as a second start bit.
    if ((tValue.UWord.LowWord & (1 << (RC5_TOGGLE_BIT + RC5_ADDRESS_BITS + RC5_COMMAND_BITS))) == 0) {
        // Here we have detected RC5X!
        decodedIRData.command += 0x40;
    }

    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    if (tValue.UByte.MidLowByte & 0x8) {
        decodedIRData.flags = IRDATA_FLAGS_TOGGLE_BIT | IRDATA_FLAGS_IS_MSB_FIRST;
    }
    decodedIRData.protocol = RC5;

#if defined(DECODE_MARANTZ)
    }
#endif
    // check for repeat
    checkForRepeatSpaceTicksAndSetFlag(RC5_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}
#endif // defined(DECODE_RC5) || defined(DECODE_MARANTZ)

//+=============================================================================
// RRRR    CCCC   6666
// R   R  C      6
// RRRR   C      6666
// R  R   C      6   6
// R   R   CCCC   666
//+=============================================================================
//
/*
 Protocol=RC6 Address=0xF1, Command=0x76, Raw-Data=0xF176, 20 bits, MSB first
 +2700,- 850
 + 550,- 800 + 550,- 350 + 550,- 350 + 550,- 800
 +1400,- 400 + 550,- 400 + 550,- 350 + 550,- 800
 + 550,- 350 + 500,- 450 +1000,- 750 +1000,- 450
 + 500,- 400 + 500,- 850 + 950,- 450 + 550,- 750
 + 550
 Duration=23600us

 Protocol=RC6A Address=0xF1, Command=0x76, Extra=0x2711, Toggle=1, Raw-Data=0xA711F176, 35 bits, MSB first
 +2700,- 850
 + 500,- 400 + 500,- 400 + 500,- 850 +1450,-1300
 +1000,- 850 + 550,- 350 +1000,- 400 + 550,- 350
 + 500,- 850 + 500,- 450 + 500,- 400 + 950,- 900
 + 500,- 400 + 550,- 350 +1000,- 400 + 500,- 400
 + 550,- 400 + 500,- 400 + 500,- 850 + 550,- 350
 + 550,- 400 +1000,- 800 +1000,- 400 + 500,- 400
 + 500,- 900 +1000,- 350 + 550,- 850 + 500
 Duration=37450us
 */
// Frame RC6:   1 start bit + 1 Bit "1" + 3 mode bits (000) + 1 toggle bit + 8 address + 8 command bits + 2666us pause - 22 bits incl. start bit and constant bit 1
// Frame RC6A:  1 start bit + 1 Bit "1" + 3 mode bits (110) + 1 toggle bit + "1" + 14 customer bits + 8 system bits + 8 command bits + 2666us pause - 37 bits incl. start bit
// !!! toggle bit has double timing !!!
// mark->space => 1 - Inverse of RC5!
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
#define RC6_CUSTOMER_BITS      14

#define RC6_BITS            (RC6_LEADING_BIT + RC6_MODE_BITS + RC6_TOGGLE_BIT + RC6_ADDRESS_BITS + RC6_COMMAND_BITS) // 21
#define RC6A_BITS           (RC6_LEADING_BIT + RC6_MODE_BITS + RC6_TOGGLE_BIT + 1 + RC6_CUSTOMER_BITS + RC6_ADDRESS_BITS + RC6_COMMAND_BITS) // 36

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
void IRsend::sendRC6(uint32_t aRawData, uint8_t aNumberOfBitsToSend) { // Deprecated
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
void IRsend::sendRC6(uint64_t aRawData, uint8_t aNumberOfBitsToSend) { // Deprecated
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

    tIRRawData.UWord.HighWord = 0; // must clear high word for the 3 mode bits to be 0
    if (aEnableAutomaticToggle) {
        if (sLastSendToggleValue == 0) {
            sLastSendToggleValue = 1;
            // set toggle bit
            tIRRawData.UByte.MidHighByte = 1; // 3 Mode bits are 0
        } else {
            sLastSendToggleValue = 0;
        }
    }

    DEBUG_PRINT(F("RC6: ToggleValue="));
    DEBUG_PRINT(sLastSendToggleValue);
    DEBUG_PRINT(F(" RawData="));
    DEBUG_PRINTLN(tIRRawData.ULong, HEX);

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
 * Assemble raw data for RC6 from parameters and toggle state and send
 * We do not wait for the minimal trailing space of 2666 us
 * @param aEnableAutomaticToggle Send toggle bit according to the state of the static sLastSendToggleValue variable.
 */
void IRsend::sendRC6A(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint16_t aCustomer,
        bool aEnableAutomaticToggle) {

    LongUnion tIRRawData;
    tIRRawData.UByte.LowByte = aCommand;
    tIRRawData.UByte.MidLowByte = aAddress;

    tIRRawData.UWord.HighWord = aCustomer | 0x400; // bit 31 is always 1

    if (aEnableAutomaticToggle) {
        if (sLastSendToggleValue == 0) {
            sLastSendToggleValue = 1;
            // set toggled bit
            tIRRawData.UByte.HighByte |= 0x80; // toggle bit is bit 32
        } else {
            sLastSendToggleValue = 0;
        }
    }

    // Set mode bits
    uint64_t tRawData = tIRRawData.ULong + 0x0600000000;

    DEBUG_PRINT(F("RC6A: ToggleValue="));
    DEBUG_PRINT(sLastSendToggleValue);
    DEBUG_PRINT(F(" RawData="));
    DEBUG_PRINTLN(tIRRawData.ULong, HEX);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {

        // start and leading bits are sent by sendRC6
        sendRC6Raw(tRawData, RC6A_BITS - 1); // -1 since the leading bit is additionally sent by sendRC6

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay(RC6_REPEAT_DISTANCE / MICROS_IN_ONE_MILLI);
        }
    }
}

#if defined(DECODE_RC6)
/**
 * Try to decode data as RC6 protocol
 *  Unit is 444 us and half of RC5 unit 888
 *  mark->space => 1 - Inverse of RC5!
 *  space->mark => 0
 *                           _   _   _   _   _   _   _   _   _   _   _   _   _
 * Clock               _____| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| | 444 us high and low
 *                              ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^  End of clock / each data bit period
 *                            ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^    Significant clock edge /sample point for bit value
 *                          ______   _     _   _   _
 * Start bit for RC6  _____|      |_| |___| |_| |_| ......|.......|..
 *                          Start     1   0   0   0       ^double timing toggle bit + 8 bit address + 8 bit command
 *                          ______   _   _   _     _             _
 * Start bit for RC6A _____|      |_| |_| |_| |___| ......|...... |_..
 *                          Start     1   1   1   0 toggle^ bit   1  + 14 bit extra + 8 bit address + 8 bit command
 */
bool IRrecv::decodeRC6() {
    uint8_t tBitIndex;
    uint32_t tDecodedRawData = 0;

    // Check we have the right amount of data (). The +3 for initial gap, start bit mark and space
    if (decodedIRData.rawlen < MIN_RC6_MARKS + 3 && (RC6_BITS + 3) < decodedIRData.rawlen) {
        DEBUG_PRINT(F("RC6: Data length="));
        DEBUG_PRINT(decodedIRData.rawlen);
        DEBUG_PRINTLN(F(" is not between 15 and 25"));
        return false;
    }

    // Check header "mark" and "space", this must be done for repeat and data
    if (!matchMark(irparams.rawbuf[1], RC6_HEADER_MARK) || !matchSpace(irparams.rawbuf[2], RC6_HEADER_SPACE)) {
        // no debug output, since this check is mainly to determine the received protocol
        DEBUG_PRINTLN(F("RC6: Header mark or space length is wrong"));
        return false;
    }

    // Set Biphase decoding start values
    initBiphaselevel(3, RC6_UNIT); // Skip gap-space and start-bit mark + space

    // Check first bit, which is known to be a 1 (mark->space)
    if (getBiphaselevel() != MARK) {
        DEBUG_PRINTLN(F("RC6: first getBiphaselevel() is not MARK"));
        return false;
    }
    // Check second bit
    if (getBiphaselevel() != SPACE) {
        DEBUG_PRINTLN(F("RC6: second getBiphaselevel() is not SPACE"));
        return false;
    }

    for (tBitIndex = 0; irparams.RawbuffOffsetForNextBiphaseLevel < decodedIRData.rawlen; tBitIndex++) {
        uint8_t tStartLevel; // start level of coded bit
        uint8_t tEndLevel;   // end level of coded bit

        tStartLevel = getBiphaselevel();
        tEndLevel = getBiphaselevel();

        if (tBitIndex == RC6_TOGGLE_BIT_INDEX) {
            /*
             * Toggle bit is double wide; level of 1. and 2. time slot and 3. and 4.time slot must be equal
             */
            if (tStartLevel != tEndLevel) { // 1. and 2. time slot must be equal
                DEBUG_PRINTLN(F("RC6: Toggle mark or space length is wrong"));
                return false;
            }
            tEndLevel = getBiphaselevel();
            if (tEndLevel != getBiphaselevel()) { // 3. and 4. time slot must be equal
                DEBUG_PRINTLN(F("RC6: Toggle mark or space length is wrong"));
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
            DEBUG_PRINTLN(F("RC6: Decode failed"));
            // we have no transition here or one level is -1 -> error
            return false;            // Error
        }
    }

// Success
    decodedIRData.numberOfBits = tBitIndex;

    LongUnion tValue;
    tValue.ULong = tDecodedRawData;
    decodedIRData.decodedRawData = tDecodedRawData;

    if (tBitIndex < 35) {
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
        decodedIRData.protocol = RC6;

    } else {
        // RC6A
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST | IRDATA_FLAGS_EXTRA_INFO;
        decodedIRData.command = tValue.UByte.LowByte;
        decodedIRData.address = tValue.UByte.MidLowByte;
        decodedIRData.extra = tValue.UWord.HighWord & 0x3FFF; // Mask to 14 bits, remove toggle and constant 1
        if ((tValue.UByte.HighByte & 0x80) != 0) {
            decodedIRData.flags |= IRDATA_FLAGS_TOGGLE_BIT;
        }
        decodedIRData.protocol = RC6A;
    }

    // check for repeat, do not check toggle bit yet
    checkForRepeatSpaceTicksAndSetFlag(RC6_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}
#endif // #if defined(DECODE_RC6)

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
 * Deprecated, use sendRC5(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle) instead
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
        // invert static toggle bit
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
#include "LocalDebugLevelEnd.h"

#endif // _IR_RC5_RC6_HPP
