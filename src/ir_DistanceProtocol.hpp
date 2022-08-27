/*
 * ir_DistanceProtocol.hpp
 *
 * This decoder tries to decode a pulse width or pulse distance protocol.
 * 1. Analyze all space and mark length
 * 2. Decide if we have an pulse width or distance protocol
 * 3. Try to decode with the mark and space data found in step 1
 * No data and address decoding, only raw data as result.
 *
 * Pulse distance data can be sent with the generic function:
 * void sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
 *            unsigned int aZeroSpaceMicros, uint32_t aData, uint8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit = false)
 * The header must be sent manually with:
 *          IrSender.mark(MarkMicros)
 *          IrSender.space(SpaceMicros);
 * see also: SendDemo example line 150
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
#ifndef _IR_DISTANCE_HPP
#define _IR_DISTANCE_HPP

#include <Arduino.h>

// accept durations up to 50 * 50 (MICROS_PER_TICK) 2500 microseconds
#define DURATION_ARRAY_SIZE 50

// Switch the decoding according to your needs
//#define DISTANCE_DO_MSB_DECODING // If active, it resembles the JVC + Denon, otherwise LSB first as e.g. for NEC and Kaseikyo/Panasonic

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT
//#include "LongUnion.h"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#Codings

#if defined(DEBUG)
void printDurations(uint8_t aArray[], uint8_t aMaxIndex) {
    for (uint_fast8_t i = 0; i <= aMaxIndex; i++) {
        if (i % 10 == 0) {
            if (i == 0) {
                Serial.print(' '); // indentation for the 0
            } else {
                Serial.println();
            }
            Serial.print(i);
            Serial.print(F(":"));
        }
        Serial.print(F(" | "));
        Serial.print(aArray[i]);
    }
    Serial.println();
}
#endif

/*
 * @return false if more than 2 distinct duration values found
 */
bool aggregateArrayCounts(uint8_t aArray[], uint8_t aMaxIndex, uint8_t *aShortIndex, uint8_t *aLongIndex) {
    uint8_t tSum = 0;
    uint16_t tWeightedSum = 0;
    for (uint_fast8_t i = 0; i <= aMaxIndex; i++) {
        uint8_t tCurrentDurations = aArray[i];
        if (tCurrentDurations != 0) {
            // Add it to sum and remove array content
            tSum += tCurrentDurations;
            tWeightedSum += (tCurrentDurations * i);
            aArray[i] = 0;
        }
        if ((tCurrentDurations == 0 || i == aMaxIndex) && tSum != 0) {
            // here we have a sum and a gap after the values
            uint8_t tAggregateIndex = (tWeightedSum + (tSum / 2)) / tSum; // with rounding
            aArray[tAggregateIndex] = tSum; // disabling this line increases code size by 2 - unbelievable!
            // store aggregate for later decoding
            if (*aShortIndex == 0) {
                *aShortIndex = tAggregateIndex;
            } else if (*aLongIndex == 0) {
                *aLongIndex = tAggregateIndex;
            } else {
                // we have 3 bins => this is likely no pulse width or distance protocol. e.g. it can be RC5.
                return false;
            }
            // initialize for next aggregation
            tSum = 0;
            tWeightedSum = 0;
        }
    }
    return true;
}

/*
 * Try to decode a pulse width or pulse distance protocol.
 * 1. Analyze all space and mark length
 * 2. Decide if we have an pulse width or distance protocol
 * 3. Try to decode with the mark and space data found in step 1
 * No data and address decoding, only raw data as result.
 */
bool IRrecv::decodeDistance() {
    uint8_t tDurationArray[DURATION_ARRAY_SIZE];

    /*
     * Accept only protocols with at least 8 bits
     */
    if (decodedIRData.rawDataPtr->rawlen < (2 * 8) + 4) {
        IR_DEBUG_PRINT(F("PULSE_DISTANCE: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is less than 20"));
        return false;
    }

    uint_fast8_t i;

    // Reset duration array
    memset(tDurationArray, 0, sizeof(tDurationArray));

    uint8_t tMaxDurationIndex = 0;
    /*
     * Count number of mark durations. Skip leading start and trailing stop bit.
     */
    for (i = 3; i < (uint_fast8_t) decodedIRData.rawDataPtr->rawlen - 2; i += 2) {
        uint8_t tDurationTicks = decodedIRData.rawDataPtr->rawbuf[i];
        if (tDurationTicks < sizeof(tDurationArray)) {
            tDurationArray[tDurationTicks]++;
            if (tMaxDurationIndex < tDurationTicks) {
                tMaxDurationIndex = tDurationTicks;
            }
        }
    }

    /*
     * Aggregate mark counts to one duration bin
     */
    uint8_t tMarkTicksShort = 0;
    uint8_t tMarkTicksLong = 0;
    bool tSuccess = aggregateArrayCounts(tDurationArray, tMaxDurationIndex, &tMarkTicksShort, &tMarkTicksLong);
#if defined(DEBUG)
    Serial.println(F("Mark:"));
    printDurations(tDurationArray, tMaxDurationIndex);
#endif

    if (!tSuccess) {
        IR_DEBUG_PRINT(F("PULSE_DISTANCE: "));
        IR_DEBUG_PRINTLN(F("Mark aggregation failed, more than 2 distinct mark duration values found"));
    }

    // Reset duration array
    memset(tDurationArray, 0, sizeof(tDurationArray));

    /*
     * Count number of space durations. Skip leading start and trailing stop bit.
     */
    tMaxDurationIndex = 0;
    for (i = 4; i < (uint_fast8_t) decodedIRData.rawDataPtr->rawlen - 2; i += 2) {
        uint8_t tDurationTicks = decodedIRData.rawDataPtr->rawbuf[i];
        if (tDurationTicks < sizeof(tDurationArray)) {
            tDurationArray[tDurationTicks]++;
            if (tMaxDurationIndex < tDurationTicks) {
                tMaxDurationIndex = tDurationTicks;
            }
        }
    }

    /*
     * Aggregate space counts to one duration bin
     */
    uint8_t tSpaceTicksShort = 0;
    uint8_t tSpaceTicksLong = 0;
    tSuccess = aggregateArrayCounts(tDurationArray, tMaxDurationIndex, &tSpaceTicksShort, &tSpaceTicksLong);
#if defined(DEBUG)
    Serial.println(F("Space:"));
    printDurations(tDurationArray, tMaxDurationIndex);
#endif

    if (!tSuccess) {
        IR_DEBUG_PRINT(F("PULSE_DISTANCE: "));
        IR_DEBUG_PRINTLN(F("Space aggregation failed, more than 2 distinct space duration values found"));
        return false;
    }

    // skip leading start and trailing stop bit.
    uint16_t tNumberOfBits = (decodedIRData.rawDataPtr->rawlen / 2) - 2;
    // Store data to reproduce frame for sending
    decodedIRData.numberOfBits = tNumberOfBits;
    decodedIRData.extra = (decodedIRData.rawDataPtr->rawbuf[1] << 8) | decodedIRData.rawDataPtr->rawbuf[2];
    decodedIRData.address = (tMarkTicksShort << 8) | tSpaceTicksLong;
    decodedIRData.command = (tMarkTicksShort << 8) | tSpaceTicksShort;

    /*
     * Print characteristics of this protocol. Durations are in ticks.
     * Number of bits, start bit, start pause, short mark, long mark, short space, long space
     *
     * NEC:         32, 180, 90, 11,  0, 11, 34
     * Samsung32:   32,  90, 90, 11,  0, 11, 34
     * LG:          28, 180, 84, 10,  0, 11, 32
     * JVC:         16, 168, 84, 10,  0, 10, 32
     * Kaseikyo:    48.  69, 35,  9,  0,  9, 26
     * Sony:  12|15|20,  48, 12, 12, 24, 12,  0 // the only known pulse width protocol
     */
    IR_DEBUG_PRINT(F("Protocol characteristics for a " STR(MICROS_PER_TICK) " us tick: "));
    IR_DEBUG_PRINT(decodedIRData.numberOfBits);
    IR_DEBUG_PRINT(F(", "));
    IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawbuf[1]);
    IR_DEBUG_PRINT(F(", "));
    IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawbuf[2]);
    IR_DEBUG_PRINT(F(", "));
    IR_DEBUG_PRINT(tMarkTicksShort);
    IR_DEBUG_PRINT(F(", "));
    IR_DEBUG_PRINT(tMarkTicksLong);
    IR_DEBUG_PRINT(F(", "));
    IR_DEBUG_PRINT(tSpaceTicksShort);
    IR_DEBUG_PRINT(F(", "));
    IR_DEBUG_PRINTLN(tSpaceTicksLong);

    uint8_t tStartIndex = 3;
    uint8_t tNumberOfAdditionalLong = (tNumberOfBits - 1) / 32;

    /*
     * decide, if we have an pulse width or distance protocol
     */
    if (tSpaceTicksLong > 0) {
        //        // check if last bit can be decoded as data or not, in this case take it as a stop bit
        //        if (decodePulseDistanceData(1, decodedIRData.rawDataPtr->rawlen - 3, tMarkTicksShort * MICROS_PER_TICK,
        //                tSpaceTicksLong * MICROS_PER_TICK, tSpaceTicksShort * MICROS_PER_TICK, DISTANCE_DO_MSB_DECODING)) {
        //            Serial.print(F("tNumberOfBits++ "));
        //            tNumberOfBits++;
        //        }

        decodedIRData.protocol = PULSE_DISTANCE;

        /*
         * Here short and long space duration found. Decode in 32 bit chunks.
         */
        for (uint_fast8_t i = 0; i <= tNumberOfAdditionalLong; ++i) {
            uint8_t tNumberOfBitsForOneDecode = tNumberOfBits;
            if (tNumberOfBitsForOneDecode > 32) {
                tNumberOfBitsForOneDecode = 32;
            }
            if (!decodePulseDistanceData(tNumberOfBitsForOneDecode, tStartIndex, tMarkTicksShort * MICROS_PER_TICK,
                    tSpaceTicksLong * MICROS_PER_TICK, tSpaceTicksShort * MICROS_PER_TICK,
#if defined(DISTANCE_DO_MSB_DECODING)
                    true
#else
                    false
#endif
                    )) {
                IR_DEBUG_PRINT(F("PULSE_DISTANCE: "));
                IR_DEBUG_PRINTLN(F("Decode failed"));
                return false;
            } else {
                // fill array with decoded data
                decodedIRData.decodedRawDataArray[i] = decodedIRData.decodedRawData;
                tStartIndex += 64;
                tNumberOfBits -= 32;
            }
        }


    } else {
        if (tMarkTicksLong == 0) {
            IR_DEBUG_PRINT(F("PULSE_DISTANCE: "));
            IR_DEBUG_PRINTLN(F("Only 1 distinct duration value for each space and mark found"));
            return false;
        }

//#define SUPPORT_PULSE_WIDTH_DECODING
#if defined(SUPPORT_PULSE_WIDTH_DECODING) // The only known pulse width protocol is Sony

//        // check if last bit can be decoded as data or not, in this case take it as a stop bit
//        if (decodePulseWidthData(1, decodedIRData.rawDataPtr->rawlen - 3, tMarkTicksLong * MICROS_PER_TICK,
//                tMarkTicksShort * MICROS_PER_TICK, tSpaceTicksShort * MICROS_PER_TICK, DISTANCE_DO_MSB_DECODING)) {
//            tNumberOfBits++;
//        }
        // decode without leading start bit. Currently only seen for sony protocol
        for (uint_fast8_t i = 0; i <= tNumberOfAdditionalLong; ++i) {
            uint8_t tNumberOfBitsForOneDecode = tNumberOfBits;
            if (tNumberOfBitsForOneDecode > 32) {
                tNumberOfBitsForOneDecode = 32;
            }
            if (!decodePulseWidthData(tNumberOfBitsForOneDecode, tStartIndex, tMarkTicksLong * MICROS_PER_TICK,
                    tMarkTicksShort * MICROS_PER_TICK, tSpaceTicksShort * MICROS_PER_TICK, DISTANCE_DO_MSB_DECODING)) {
                IR_DEBUG_PRINT(F("PULSE_WIDTH: "));
                IR_DEBUG_PRINTLN(F("Decode failed"));
                return false;
            }
            tStartIndex += 64;
            tNumberOfBits -= 32;
        }

        // Store ticks used for decoding in extra
        decodedIRData.extra = (tMarkTicksShort << 8) | tMarkTicksLong;
        decodedIRData.protocol = PULSE_WIDTH;
#endif
    }

#if defined(DISTANCE_DO_MSB_DECODING)
        decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST | IRDATA_FLAGS_EXTRA_INFO;
#else
        decodedIRData.flags = IRDATA_FLAGS_EXTRA_INFO;
#endif

    return true;
}

/** @}*/
#endif // _IR_DISTANCE_HPP
