/*
 * ir_DistanceWidthProtocol.hpp
 *
 * Contains only the decoder functions!
 *
 * This decoder tries to decode a pulse distance or pulse distance width with constant period (or pulse width - not enabled yet) protocol.
 * 1. Analyze all space and mark length
 * 2. Decide which protocol we have
 * 3. Try to decode with the mark and space data found in step 1
 * 4. Assume one start bit / header and one stop bit, since pulse distance data must have a stop bit!
 * No data and address decoding, only raw data as result.
 *
 * Pulse distance data can be sent with the generic function as in SendDemo example line 155:
 * https://github.com/Arduino-IRremote/Arduino-IRremote/blob/d51b540cb2ddf1424888d2d9e6b62fe1ef46859d/examples/SendDemo/SendDemo.ino#L155
 * void sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
 *            unsigned int aZeroSpaceMicros, uint32_t aData, uint8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit = false)
 * The header must be sent manually with:
 *          IrSender.mark(MarkMicros)
 *          IrSender.space(SpaceMicros);
 *
 * Or send it by filling a DecodedRawDataArray and with the sendPulseDistanceWidthFromArray() function as in SendDemo example line 175:
 * https://github.com/Arduino-IRremote/Arduino-IRremote/blob/d51b540cb2ddf1424888d2d9e6b62fe1ef46859d/examples/SendDemo/SendDemo.ino#L175
 * sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, unsigned int aHeaderMarkMicros,
 *         unsigned int aHeaderSpaceMicros, unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
 *         unsigned int aZeroSpaceMicros, uint32_t *aDecodedRawDataArray, unsigned int aNumberOfBits, bool aMSBFirst,
 *         bool aSendStopBit, unsigned int aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats)
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022 Armin Joachimsmeyer
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
#ifndef _IR_DISTANCE_WIDTH_HPP
#define _IR_DISTANCE_WIDTH_HPP

// accept durations up to 50 * 50 (MICROS_PER_TICK) 2500 microseconds
#define DURATION_ARRAY_SIZE 50

// Switch the decoding according to your needs
//#define DISTANCE_DO_MSB_DECODING // If active, it resembles the JVC + Denon, otherwise LSB first as e.g. for NEC and Kaseikyo/Panasonic

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#Codings
#if defined(LOCAL_DEBUG)
void printDurations(uint8_t aArray[], uint8_t aMaxIndex) {
    for (uint_fast8_t i = 0; i <= aMaxIndex; i++) {
        if (i % 10 == 0) {
            if (i == 0) {
                Serial.print(' '); // indentation for the 0
            } else {
                Serial.println();
            }
            Serial.print(i);
            Serial.print(F(": "));
        }
        Serial.print(aArray[i]);
        if (aArray[i] != 0) {
            Serial.print(' ');
            Serial.print(i * (uint16_t)MICROS_PER_TICK);
        }
        Serial.print(F(" | "));
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
    uint8_t tDurationArray[DURATION_ARRAY_SIZE]; // For up to 49 ticks / 2450 us

    /*
     * Accept only protocols with at least 8 bits
     */
    if (decodedIRData.rawDataPtr->rawlen < (2 * 8) + 4) {
        IR_DEBUG_PRINT(F("PULSE_DISTANCE_WIDTH: "));
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
     * Count number of mark durations up to 49 ticks. Skip leading start and trailing stop bit.
     */
    for (i = 3; i < (uint_fast8_t) decodedIRData.rawDataPtr->rawlen - 2; i += 2) {
        uint8_t tDurationTicks = decodedIRData.rawDataPtr->rawbuf[i];
        if (tDurationTicks < sizeof(tDurationArray)) {
            tDurationArray[tDurationTicks]++; // count duration if less than DURATION_ARRAY_SIZE (50)
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
#if defined(LOCAL_DEBUG)
    Serial.println(F("Mark:"));
    printDurations(tDurationArray, tMaxDurationIndex);
#endif

    if (!tSuccess) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("PULSE_DISTANCE_WIDTH: "));
        Serial.println(F("Mark aggregation failed, more than 2 distinct mark duration values found"));
#endif
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
#if defined(LOCAL_DEBUG)
    Serial.println(F("Space:"));
    printDurations(tDurationArray, tMaxDurationIndex);
#endif

    if (!tSuccess) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("PULSE_DISTANCE_WIDTH: "));
        Serial.println(F("Space aggregation failed, more than 2 distinct space duration values found"));
#endif
        return false;
    }

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
#if defined(LOCAL_DEBUG)
    Serial.print(F("ProtocolConstants: "));
    Serial.print(decodedIRData.rawDataPtr->rawbuf[1] * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(decodedIRData.rawDataPtr->rawbuf[2] * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(tMarkTicksShort * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(tSpaceTicksLong * MICROS_PER_TICK);
    Serial.print(F(", "));
    if (tMarkTicksLong == 0) {
        Serial.print(tMarkTicksShort * MICROS_PER_TICK);
    } else {
        Serial.print(tMarkTicksLong * MICROS_PER_TICK);
    }
    Serial.print(F(", "));
    Serial.print(tSpaceTicksShort * MICROS_PER_TICK);
    Serial.println();
#endif
    uint8_t tStartIndex = 3;
    // skip leading start bit for decoding.
    uint16_t tNumberOfBits = (decodedIRData.rawDataPtr->rawlen / 2) - 1;
    if (tSpaceTicksLong > 0 && tMarkTicksLong == 0) {
        // For PULSE DISTANCE a stop bit is mandatory, for PULSE_DISTANCE_WIDTH it is not required!
        tNumberOfBits--; // Correct for stop bit
    }
    decodedIRData.numberOfBits = tNumberOfBits;
    uint8_t tNumberOfAdditionalLong = (tNumberOfBits - 1) / 32;

    /*
     * decide, if we have an pulse width or distance protocol
     */
    if (tSpaceTicksLong > 0) {
        /*
         * Here short and long space duration found. Decode in 32 bit chunks.
         * Only the last chunk can contain less than 32 bits
         */
        for (uint_fast8_t i = 0; i <= tNumberOfAdditionalLong; ++i) {
            uint8_t tNumberOfBitsForOneDecode = tNumberOfBits;
            if (tNumberOfBitsForOneDecode > 32) {
                tNumberOfBitsForOneDecode = 32;
            }
            bool tResult;
            if (tMarkTicksLong > 0) {
                decodedIRData.protocol = PULSE_DISTANCE_WIDTH;
                /*
                 * Here we have 2 space and 2 mark durations like MagiQuest
                 * The longer mark is mostly the mark of the coded 1
                 */
                tResult = decodeDistanceWidthData(tNumberOfBitsForOneDecode, tStartIndex, tMarkTicksLong * MICROS_PER_TICK,
                        (tMarkTicksLong + tSpaceTicksShort) * MICROS_PER_TICK,
#if defined(DISTANCE_DO_MSB_DECODING)
                        true
#else
                        false
#endif
                        );

            } else {
                decodedIRData.protocol = PULSE_DISTANCE;
                tResult = decodePulseDistanceData(tNumberOfBitsForOneDecode, tStartIndex, tMarkTicksShort * MICROS_PER_TICK,
                        tSpaceTicksLong * MICROS_PER_TICK, tSpaceTicksShort * MICROS_PER_TICK,
#if defined(DISTANCE_DO_MSB_DECODING)
                        true
#else
                        false
#endif
                        );
            }
            if (!tResult) {
#if defined(LOCAL_DEBUG)
                Serial.print(decodedIRData.protocol);
                Serial.println(F(": Decode failed"));
#endif
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
#if defined(LOCAL_DEBUG)
            Serial.print(F("PULSE_DISTANCE: "));
            Serial.println(F("Only 1 distinct duration value for each space and mark found"));
#endif
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
#if defined(LOCAL_DEBUG)
                Serial.print(F("PULSE_WIDTH: "));
                Serial.println(F("Decode failed"));
#endif
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
// Store data to reproduce frame for sending
    decodedIRData.extra = (decodedIRData.rawDataPtr->rawbuf[1] << 8) | decodedIRData.rawDataPtr->rawbuf[2];
    if (tMarkTicksLong == 0) {
        decodedIRData.address = (tMarkTicksShort << 8) | tSpaceTicksLong;
    } else {
        decodedIRData.address = (tMarkTicksLong << 8) | tSpaceTicksLong;
    }
    decodedIRData.command = (tMarkTicksShort << 8) | tSpaceTicksShort;

    return true;
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_DISTANCE_WIDTH_HPP
