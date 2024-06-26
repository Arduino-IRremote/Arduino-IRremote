/*
 * ir_DistanceWidthProtocol.hpp
 *
 * Contains only the decoder functions for universal pulse width or pulse distance protocols!
 * The send functions are used by almost all protocols and are therefore located in IRSend.hpp.
 *
 * If RAM is not more than 2k, the decoder only accepts mark or space durations up to 50 * 50 (MICROS_PER_TICK) = 2500 microseconds
 * to save RAM space, otherwise it accepts durations up to 10 ms.
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
 *         unsigned int aZeroSpaceMicros, uint32_t *aDecodedRawDataArray, unsigned int aNumberOfBits, uint8_t aFlags,
 *         unsigned int aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats)
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2024 Armin Joachimsmeyer
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

#if !defined(DISTANCE_WIDTH_MAXIMUM_REPEAT_DISTANCE_MICROS)
#define DISTANCE_WIDTH_MAXIMUM_REPEAT_DISTANCE_MICROS       100000 // 100 ms, bit it is just a guess
#endif

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

#if !defined(DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE)
#  if (defined(RAMEND) && RAMEND <= 0x8FF) || (defined(RAMSIZE) && RAMSIZE < 0x8FF)
#define DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE 50 // To save program space, the decoder only accepts mark or space durations up to 50 * 50 (MICROS_PER_TICK) = 2500 microseconds
#  else
#define DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE 200 // The decoder accepts mark or space durations up to 200 * 50 (MICROS_PER_TICK) = 10 milliseconds
#  endif
#endif

// Switch the decoding according to your needs
//#define USE_MSB_DECODING_FOR_DISTANCE_DECODER // If active, it resembles LG, otherwise LSB first as most other protocols e.g. NEC and Kaseikyo/Panasonic

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//=====================================================================================
// DDD   III   SSS  TTTTTT   AA   N   N   CCC  EEEE     W     W  III  DDD  TTTTTT  H  H
// D  D   I   S       TT    A  A  NN  N  C     E        W     W   I   D  D   TT    H  H
// D  D   I    SSS    TT    AAAA  N N N  C     EEE      W  W  W   I   D  D   TT    HHHH
// D  D   I       S   TT    A  A  N  NN  C     E         W W W    I   D  D   TT    H  H
// DDD   III  SSSS    TT    A  A  N   N   CCC  EEEE       W W    III  DDD    TT    H  H
//=====================================================================================
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#Codings
/*
 Example output of UnitTest.ino for PulseWidth protocol:
 Protocol=PulseWidth Raw-Data=0x87654321 32 bits LSB first
 Send on a 8 bit platform with: IrSender.sendPulseDistanceWidth(38, 950, 550, 600, 300, 300, 300, 0x87654321, 32, PROTOCOL_IS_LSB_FIRST, <RepeatPeriodMillis>, <numberOfRepeats>);
 rawData[66]:
 -1088600
 + 950,- 550
 + 600,- 300 + 300,- 300 + 350,- 250 + 350,- 250
 + 350,- 300 + 600,- 300 + 300,- 300 + 300,- 300
 + 650,- 250 + 650,- 250 + 300,- 300 + 350,- 250
 + 350,- 300 + 300,- 300 + 600,- 300 + 300,- 300
 + 600,- 300 + 350,- 250 + 600,- 300 + 350,- 250
 + 350,- 300 + 600,- 300 + 600,- 300 + 300,- 300
 + 600,- 300 + 600,- 300 + 600,- 300 + 300,- 300
 + 300,- 300 + 300,- 300 + 350,- 250 + 650
 Sum: 24500

 Example output of UnitTest.ino for PulseDistanceWidth protocol:
 Protocol=PulseDistance Raw-Data=0x76 7 bits LSB first
 Send on a 8 bit platform with: IrSender.sendPulseDistanceWidth(38, 5950, 500, 550, 1450, 1550, 500, 0x76, 7, PROTOCOL_IS_LSB_FIRST, <RepeatPeriodMillis>, <numberOfRepeats>);
 rawData[18]:
 -1092450
 +5950,- 500
 +1500,- 500 + 500,-1450 + 550,-1450 +1550,- 450
 + 550,-1450 + 550,-1450 + 550,-1450 + 550
 Sum: 20950
 */

#if defined(LOCAL_DEBUG)
void printDurations(uint8_t aArray[], uint8_t aMaxIndex) {
    for (uint_fast8_t i = 0; i <= aMaxIndex; i++) {
        //Print index at the beginning of a new line
        if (i % 10 == 0) {
            if (i == 0) {
                Serial.print(' '); // indentation for the first index 0
            } else {
                Serial.println(); // new line for next indexes 10, 20 etc.
            }
            Serial.print(i);
            Serial.print(F(": "));
        }
        // Print number of values in array and duration if != 0
        Serial.print(aArray[i]);
        if (aArray[i] != 0) {
            Serial.print('x');
            Serial.print(i * (uint16_t) MICROS_PER_TICK);
        }
        Serial.print(F(" | "));
    }
    Serial.println();
}
#endif

/*
 * We count all consecutive (allow only one gap between) durations and compute the average.
 * @return false if more than 2 distinct duration values found
 */
bool aggregateArrayCounts(uint8_t aArray[], uint8_t aMaxIndex, uint8_t *aShortIndex, uint8_t *aLongIndex) {
    uint8_t tSum = 0;
    uint16_t tWeightedSum = 0;
    uint8_t tGapCount = 0;
    for (uint_fast8_t i = 0; i <= aMaxIndex; i++) {
        uint8_t tCurrentDurations = aArray[i];
        if (tCurrentDurations != 0) {
            // Add it to sum and remove array content
            tSum += tCurrentDurations;
            tWeightedSum += (tCurrentDurations * i);
            aArray[i] = 0;
            tGapCount = 0;
        } else {
            tGapCount++;
        }
        if (tSum != 0 && (i == aMaxIndex || tGapCount > 1)) {
            /*
             * Here we have a sum AND last element OR more than 1 consecutive gap
             */
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
 * Try to decode a pulse distance or pulse width protocol.
 * 1. Analyze all space and mark length
 * 2. Decide if we have an pulse width or distance protocol
 * 3. Try to decode with the mark and space data found in step 1
 * No data and address decoding, only raw data as result.
 *
 * calloc() version is 700 bytes larger :-(
 */
bool IRrecv::decodeDistanceWidth() {
    /*
     * Array for up to 49 ticks / 2500 us (or 199  ticks / 10 ms us if RAM > 2k)
     * 0 tick covers mark or space durations from 0 to 49 us, and 49 ticks from 2450 to 2499 us
     */
    uint8_t tDurationArray[DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE];

    /*
     * Accept only protocols with at least 7 bits
     */
    if (decodedIRData.rawlen < (2 * 7) + 4) {
        IR_DEBUG_PRINT(F("PULSE_DISTANCE_WIDTH: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is less than 18"));
        return false;
    }

    // Reset duration array
    memset(tDurationArray, 0, DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE);

    uint8_t tIndexOfMaxDuration = 0;
    /*
     * Count number of mark durations. Skip leading start and trailing stop bit.
     */
    for (IRRawlenType i = 3; i < decodedIRData.rawlen - 2; i += 2) {
        auto tDurationTicks = decodedIRData.rawDataPtr->rawbuf[i];
        if (tDurationTicks < DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE) {
            tDurationArray[tDurationTicks]++; // count duration if less than DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE
            if (tIndexOfMaxDuration < tDurationTicks) {
                tIndexOfMaxDuration = tDurationTicks;
            }
        } else {
#if defined(LOCAL_DEBUG)
            Serial.print(F("PULSE_DISTANCE_WIDTH: "));
            Serial.print(F("Mark "));
            Serial.print(tDurationTicks * MICROS_PER_TICK);
            Serial.print(F(" is longer than maximum "));
            Serial.print(DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE * MICROS_PER_TICK);
            Serial.print(F(" us. Index="));
            Serial.println(i);
#endif
            return false;
        }
    }

    /*
     * Aggregate mark counts to one duration bin
     */
    uint8_t tMarkTicksShort = 0;
    uint8_t tMarkTicksLong = 0;
    bool tSuccess = aggregateArrayCounts(tDurationArray, tIndexOfMaxDuration, &tMarkTicksShort, &tMarkTicksLong);
#if defined(LOCAL_DEBUG)
    Serial.println(F("Mark:"));
    printDurations(tDurationArray, tIndexOfMaxDuration);
#endif

    if (!tSuccess) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("PULSE_DISTANCE_WIDTH: "));
        Serial.println(F("Mark aggregation failed, more than 2 distinct mark duration values found"));
#endif
        return false;
    }

    // Reset duration array
    memset(tDurationArray, 0, DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE);

    /*
     * Count number of space durations. Skip leading start and trailing stop bit.
     */
    tIndexOfMaxDuration = 0;
    for (IRRawlenType i = 4; i < decodedIRData.rawlen - 2; i += 2) {
        auto tDurationTicks = decodedIRData.rawDataPtr->rawbuf[i];
        if (tDurationTicks < DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE) {
            tDurationArray[tDurationTicks]++;
            if (tIndexOfMaxDuration < tDurationTicks) {
                tIndexOfMaxDuration = tDurationTicks;
            }
        } else {
#if defined(LOCAL_DEBUG)
            Serial.print(F("PULSE_DISTANCE_WIDTH: "));
            Serial.print(F("Space "));
            Serial.print(tDurationTicks * MICROS_PER_TICK);
            Serial.print(F(" is longer than maximum "));
            Serial.print(DISTANCE_WIDTH_DECODER_DURATION_ARRAY_SIZE * MICROS_PER_TICK);
            Serial.print(F(" us. Index="));
            Serial.println(i);
#endif
            return false;
        }
    }

    /*
     * Aggregate space counts to one duration bin
     */
    uint8_t tSpaceTicksShort = 0;
    uint8_t tSpaceTicksLong = 0;
    tSuccess = aggregateArrayCounts(tDurationArray, tIndexOfMaxDuration, &tSpaceTicksShort, &tSpaceTicksLong);
#if defined(LOCAL_DEBUG)
    Serial.println(F("Space:"));
    printDurations(tDurationArray, tIndexOfMaxDuration);
#endif

    if (!tSuccess) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("PULSE_DISTANCE_WIDTH: "));
        Serial.println(F("Space aggregation failed, more than 2 distinct space duration values found"));
#endif
        return false;
    }

    /*
     * Print characteristics of this protocol. Durations are in (50 us) ticks.
     * Number of bits, start bit, start pause, long mark, long space, short mark, short space
     *
     * NEC:         32, 180, 90,  0, 34, 11, 11
     * Samsung32:   32,  90, 90,  0, 34, 11, 11
     * LG:          28, 180, 84,  0, 32, 10, 11
     * JVC:         16, 168, 84,  0, 32, 10, 10
     * Kaseikyo:    48.  69, 35,  0, 26,  9,  9
     * Sony:  12|15|20,  48, 12, 24,  0, 12, 12 // the only known pulse width protocol
     * Disney monorail
     *   model:      7, 120, 10, 30, 30, 10, 10 // PulseDistanceWidth. Can be seen as direct conversion of a 7 bit serial timing at 250 baud with a 6 ms start bit.
     */
#if defined(LOCAL_DEBUG)
    Serial.print(F("DistanceWidthTimingInfoStruct: "));
    Serial.print(decodedIRData.rawDataPtr->rawbuf[1] * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(decodedIRData.rawDataPtr->rawbuf[2] * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(tMarkTicksLong * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(tSpaceTicksLong * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.print(tMarkTicksShort * MICROS_PER_TICK);
    Serial.print(F(", "));
    Serial.println(tSpaceTicksShort * MICROS_PER_TICK);
#endif
#if RAW_BUFFER_LENGTH <= (512 -4)
    uint_fast8_t tNumberOfBits;
#else
    uint16_t tNumberOfBits;
#endif
    tNumberOfBits = (decodedIRData.rawlen / 2) - 1;
    if (tSpaceTicksLong > 0) {
        // For PULSE_DISTANCE -including PULSE_DISTANCE_WIDTH- a stop bit is mandatory, for PULSE_WIDTH it is not required!
        tNumberOfBits--; // Correct for stop bit
    }
    decodedIRData.numberOfBits = tNumberOfBits;
    uint8_t tNumberOfAdditionalArrayValues = (tNumberOfBits - 1) / BITS_IN_RAW_DATA_TYPE;

    /*
     * We can have the following protocol timings
     * PULSE_DISTANCE:       Pause/spaces have different length and determine the bit value, longer space is 1. Pulses/marks can be constant, like NEC.
     * PULSE_WIDTH:          Pulses/marks have different length and determine the bit value, longer mark is 1. Pause/spaces can be constant, like Sony.
     * PULSE_DISTANCE_WIDTH: Pulses/marks and pause/spaces have different length, often the bit length is constant, like MagiQuest. Can be decoded by PULSE_DISTANCE decoder.
     */

    if (tMarkTicksLong == 0 && tSpaceTicksLong == 0) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("PULSE_DISTANCE: "));
        Serial.println(F("Only 1 distinct duration value for each space and mark found"));
#endif
        return false;
    }
    unsigned int tSpaceMicrosShort;
#if defined DECODE_STRICT_CHECKS
        if(tMarkTicksLong > 0 && tSpaceTicksLong > 0) {
            // We have different mark and space length here, so signal decodePulseDistanceWidthData() not to check against constant length decodePulseDistanceWidthData
            tSpaceMicrosShort = 0;
        }
#endif
    tSpaceMicrosShort = tSpaceTicksShort * MICROS_PER_TICK;
    unsigned int tMarkMicrosShort = tMarkTicksShort * MICROS_PER_TICK;
    unsigned int tMarkMicrosLong = tMarkTicksLong * MICROS_PER_TICK;
    unsigned int tSpaceMicrosLong = tSpaceTicksLong * MICROS_PER_TICK;
    IRRawlenType tStartIndex = 3;  // skip leading start bit for decoding.

    for (uint_fast8_t i = 0; i <= tNumberOfAdditionalArrayValues; ++i) {
        uint8_t tNumberOfBitsForOneDecode = tNumberOfBits;
        /*
         * Decode in 32/64 bit chunks. Only the last chunk can contain less than 32/64 bits
         */
        if (tNumberOfBitsForOneDecode > BITS_IN_RAW_DATA_TYPE) {
            tNumberOfBitsForOneDecode = BITS_IN_RAW_DATA_TYPE;
        }
        bool tResult;
        if (tSpaceTicksLong > 0) {
            /*
             * Here short and long space durations found.
             * Since parameters aOneMarkMicros and aOneSpaceMicros are equal, we only check tSpaceMicrosLong here.
             */
            decodedIRData.protocol = PULSE_DISTANCE; // Sony + PULSE_DISTANCE_WIDTH
            tResult = decodePulseDistanceWidthData(tNumberOfBitsForOneDecode, tStartIndex, tMarkMicrosShort, tSpaceMicrosLong,
                    tMarkMicrosShort,
#if defined(USE_MSB_DECODING_FOR_DISTANCE_DECODER)
                    true
#else
                    false
#endif
                    );
        } else {
            /*
             * Here no long space duration found. => short and long mark durations found, check tMarkMicrosLong here
             * This else case will most likely never be used, but it only requires 12 bytes additional programming space :-)
             */
            decodedIRData.protocol = PULSE_WIDTH; // NEC etc.
            tResult = decodePulseDistanceWidthData(tNumberOfBitsForOneDecode, tStartIndex, tMarkMicrosLong, tSpaceMicrosShort,
                    tMarkMicrosShort,
#if defined(USE_MSB_DECODING_FOR_DISTANCE_DECODER)
                    true
#else
                    false
#endif
                    );

        }
        if (!tResult) {
#if defined(LOCAL_DEBUG)
            Serial.print(F("PULSE_WIDTH: "));
            Serial.println(F("Decode failed"));
#endif
            return false;
        }
#if defined(LOCAL_DEBUG)
        Serial.print(F("PULSE_WIDTH: "));
        Serial.print(F("decodedRawData=0x"));
        Serial.println(decodedIRData.decodedRawData, HEX);
#endif
        // fill array with decoded data
        decodedIRData.decodedRawDataArray[i] = decodedIRData.decodedRawData;
        tStartIndex += (2 * BITS_IN_RAW_DATA_TYPE);
        tNumberOfBits -= BITS_IN_RAW_DATA_TYPE;
    }

#if defined(USE_MSB_DECODING_FOR_DISTANCE_DECODER)
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
#endif

    // Check for repeat
    checkForRepeatSpaceTicksAndSetFlag(DISTANCE_WIDTH_MAXIMUM_REPEAT_DISTANCE_MICROS / MICROS_PER_TICK);

    /*
     * Store timing data to reproduce frame for sending
     */
    decodedIRData.DistanceWidthTimingInfo.HeaderMarkMicros = (decodedIRData.rawDataPtr->rawbuf[1] * MICROS_PER_TICK);
    decodedIRData.DistanceWidthTimingInfo.HeaderSpaceMicros = (decodedIRData.rawDataPtr->rawbuf[2] * MICROS_PER_TICK);
    decodedIRData.DistanceWidthTimingInfo.ZeroMarkMicros = tMarkMicrosShort;
    decodedIRData.DistanceWidthTimingInfo.ZeroSpaceMicros = tSpaceMicrosShort;
    if (tMarkMicrosLong != 0) {
        if (tSpaceMicrosLong == 0) {
            // PULSE_DISTANCE, Sony
            decodedIRData.DistanceWidthTimingInfo.OneMarkMicros = tMarkMicrosLong;
            decodedIRData.DistanceWidthTimingInfo.OneSpaceMicros = tSpaceMicrosShort;
        } else {
            // PULSE_DISTANCE_WIDTH, we have 4 distinct values here
            // Assume long space for a one when we have PulseDistanceWidth like for RS232, where a long inactive period (high) is a 1
            decodedIRData.DistanceWidthTimingInfo.OneSpaceMicros = tSpaceMicrosLong;
            decodedIRData.DistanceWidthTimingInfo.OneMarkMicros = tMarkMicrosShort;
            decodedIRData.DistanceWidthTimingInfo.ZeroMarkMicros = tMarkMicrosLong;
//            // Assume long mark for a one when we have PulseDistanceWidth
//            decodedIRData.DistanceWidthTimingInfo.OneSpaceMicros = tSpaceMicrosShort;
//            decodedIRData.DistanceWidthTimingInfo.ZeroSpaceMicros = tSpaceMicrosLong;
//            decodedIRData.DistanceWidthTimingInfo.OneMarkMicros = tMarkMicrosLong;
        }
    } else {
        // PULSE_WIDTH, NEC etc.
        // Here tMarkMicrosLong is 0 => tSpaceMicrosLong != 0
        decodedIRData.DistanceWidthTimingInfo.OneMarkMicros = tMarkMicrosShort;
        decodedIRData.DistanceWidthTimingInfo.OneSpaceMicros = tSpaceMicrosLong;
    }

#if defined(LOCAL_DEBUG)
    Serial.print(F("DistanceWidthTimingInfo="));
    IrReceiver.printDistanceWidthTimingInfo(&Serial, &decodedIRData.DistanceWidthTimingInfo);
    Serial.println();
#endif
    return true;
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_DISTANCE_WIDTH_HPP
