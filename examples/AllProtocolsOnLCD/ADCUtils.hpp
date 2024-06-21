/*
 * ADCUtils.hpp
 *
 * ADC utility functions. Conversion time is defined as 0.104 milliseconds for 16 MHz Arduinos in ADCUtils.h.
 *
 *  Copyright (C) 2016-2023  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  ArduinoUtils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 */

#ifndef _ADC_UTILS_HPP
#define _ADC_UTILS_HPP

#include "ADCUtils.h"
#if defined(ADC_UTILS_ARE_AVAILABLE) // set in ADCUtils.h, if supported architecture was detected

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

/*
 * By replacing this value with the voltage you measured a the AREF pin after a conversion
 * with INTERNAL you can calibrate your ADC readout. For my Nanos I measured e.g. 1060 mV and 1093 mV.
 */
#if !defined(ADC_INTERNAL_REFERENCE_MILLIVOLT)
#define ADC_INTERNAL_REFERENCE_MILLIVOLT    1100L // Change to value measured at the AREF pin. If value > real AREF voltage, measured values are > real values
#endif

// Union to speed up the combination of low and high bytes to a word
// it is not optimal since the compiler still generates 2 unnecessary moves
// but using  -- value = (high << 8) | low -- gives 5 unnecessary instructions
union WordUnionForADCUtils {
    struct {
        uint8_t LowByte;
        uint8_t HighByte;
    } UByte;
    uint16_t UWord;
    int16_t Word;
    uint8_t *BytePointer;
};

/*
 * Enable this to see information on each call.
 * Since there should be no library which uses Serial, it should only be enabled for development purposes.
 */
#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/*
 * Persistent storage for VCC value
 */
float sVCCVoltage;
uint16_t sVCCVoltageMillivolt;

// for isVCCTooLowMultipleTimes()
long sLastVCCCheckMillis;
uint8_t sVCCTooLowCounter = 0;

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannel(uint8_t aADCChannelNumber) {
    WordUnionForADCUtils tUValue;
    ADMUX = aADCChannelNumber | (DEFAULT << SHIFT_VALUE_FOR_REFERENCE);

    // ADCSRB = 0; // Only active if ADATE is set to 1.
    // ADSC-StartConversion ADIF-Reset Interrupt Flag - NOT free running mode
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADIF) | ADC_PRESCALE);

    // wait for single conversion to finish
    loop_until_bit_is_clear(ADCSRA, ADSC);

    // Get value
    tUValue.UByte.LowByte = ADCL;
    tUValue.UByte.HighByte = ADCH;
    return tUValue.UWord;
    //    return ADCL | (ADCH <<8); // needs 4 bytes more
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannelWithReference(uint8_t aADCChannelNumber, uint8_t aReference) {
    WordUnionForADCUtils tUValue;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    // ADCSRB = 0; // Only active if ADATE is set to 1.
    // ADSC-StartConversion ADIF-Reset Interrupt Flag - NOT free running mode
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADIF) | ADC_PRESCALE);

    // wait for single conversion to finish
    loop_until_bit_is_clear(ADCSRA, ADSC);

    // Get value
    tUValue.UByte.LowByte = ADCL;
    tUValue.UByte.HighByte = ADCH;
    return tUValue.UWord;
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 * Does NOT restore ADMUX after reading
 */
uint16_t waitAndReadADCChannelWithReference(uint8_t aADCChannelNumber, uint8_t aReference) {
    checkAndWaitForReferenceAndChannelToSwitch(aADCChannelNumber, aReference);
    return readADCChannelWithReference(aADCChannelNumber, aReference);
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 * Restores ADMUX after reading
 */
uint16_t waitAndReadADCChannelWithReferenceAndRestoreADMUXAndReference(uint8_t aADCChannelNumber, uint8_t aReference) {
    uint8_t tOldADMUX = checkAndWaitForReferenceAndChannelToSwitch(aADCChannelNumber, aReference);
    uint16_t tResult = readADCChannelWithReference(aADCChannelNumber, aReference);
    checkAndWaitForReferenceAndChannelToSwitch(tOldADMUX & MASK_FOR_ADC_CHANNELS, tOldADMUX >> SHIFT_VALUE_FOR_REFERENCE);
    return tResult;
}

/*
 * To prepare reference and ADMUX for next measurement
 */
void setADCChannelAndReferenceForNextConversion(uint8_t aADCChannelNumber, uint8_t aReference) {
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);
}

/*
 * @return original ADMUX register content for optional later restoring values
 * All experimental values are acquired by using the ADCSwitchingTest example from this library
 */
uint8_t checkAndWaitForReferenceAndChannelToSwitch(uint8_t aADCChannelNumber, uint8_t aReference) {
    uint8_t tOldADMUX = ADMUX;
    /*
     * Must wait >= 7 us if reference has to be switched from 1.1 volt/INTERNAL to VCC/DEFAULT (seen on oscilloscope)
     * This is done after the 2 ADC clock cycles required for Sample & Hold :-)
     *
     * Must wait >= 7600 us for Nano board  >= 6200 for Uno board if reference has to be switched from VCC/DEFAULT to 1.1 volt/INTERNAL
     * Must wait >= 200 us if channel has to be switched to 1.1 volt internal channel if S&H was at 5 Volt
     */
    uint8_t tNewReference = (aReference << SHIFT_VALUE_FOR_REFERENCE);
    ADMUX = aADCChannelNumber | tNewReference;
#if defined(INTERNAL2V56)
    if ((tOldADMUX & MASK_FOR_ADC_REFERENCE) != tNewReference && (aReference == INTERNAL || aReference == INTERNAL2V56)) {
#else
    if ((tOldADMUX & MASK_FOR_ADC_REFERENCE) != tNewReference && aReference == INTERNAL) {
#endif
#if defined(LOCAL_DEBUG)
        Serial.println(F("Switch from DEFAULT to INTERNAL"));
#endif
        /*
         * Switch reference from DEFAULT to INTERNAL
         */
        delayMicroseconds(8000); // experimental value is >= 7600 us for Nano board and 6200 for Uno board
    } else if ((tOldADMUX & ADC_CHANNEL_MUX_MASK) != aADCChannelNumber) {
        if (aADCChannelNumber == ADC_1_1_VOLT_CHANNEL_MUX) {
            /*
             * Internal 1.1 Volt channel requires  <= 200 us for Nano board
             */
            delayMicroseconds(350); // 350 was ok and 300 was too less for UltimateBatteryTester - result was 226 instead of 225
        } else {
            /*
             * 100 kOhm requires < 100 us, 1 MOhm requires 120 us S&H switching time
             */
            delayMicroseconds(120); // experimental value is <= 1100 us for Nano board
        }
    }
    return tOldADMUX;
}

/*
 * Oversample and multiple samples only makes sense if you expect a noisy input signal
 * It does NOT increase the precision of the measurement, since the ADC has insignificant noise
 */
uint16_t readADCChannelWithOversample(uint8_t aADCChannelNumber, uint8_t aOversampleExponent) {
    return readADCChannelWithReferenceOversample(aADCChannelNumber, DEFAULT, aOversampleExponent);
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannelWithReferenceOversample(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    uint16_t tSumValue = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | ADC_PRESCALE);

    uint8_t tCount = _BV(aOversampleExponent);
    for (uint8_t i = 0; i < tCount; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // Add value
        tSumValue += ADCL | (ADCH << 8); // using WordUnionForADCUtils does not save space here
        // tSumValue += (ADCH << 8) | ADCL; // this does NOT work!
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    // return rounded value
    return ((tSumValue + (tCount >> 1)) >> aOversampleExponent);
}

/*
 * Use ADC_PRESCALE32 which gives 26 us conversion time and good linearity for 16 MHz Arduino
 */
uint16_t readADCChannelWithReferenceOversampleFast(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    uint16_t tSumValue = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | ADC_PRESCALE32);

    uint8_t tCount = _BV(aOversampleExponent);
    for (uint8_t i = 0; i < tCount; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // Add value
        tSumValue += ADCL | (ADCH << 8); // using WordUnionForADCUtils does not save space here
        // tSumValue += (ADCH << 8) | ADCL; // this does NOT work!
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return ((tSumValue + (tCount >> 1)) >> aOversampleExponent);
}

/*
 * Returns sum of all sample values
 * Conversion time is defined as 0.104 milliseconds for 16 MHz Arduino by ADC_PRESCALE (=ADC_PRESCALE128) in ADCUtils.h.
 * @ param aNumberOfSamples If > 64 an overflow may occur.
 */
uint16_t readADCChannelMultiSamplesWithReference(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aNumberOfSamples) {
    uint16_t tSumValue = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | ADC_PRESCALE);

    for (uint8_t i = 0; i < aNumberOfSamples; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // Add value
        tSumValue += ADCL | (ADCH << 8); // using WordUnionForADCUtils does not save space here
        // tSumValue += (ADCH << 8) | ADCL; // this does NOT work!
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return tSumValue;
}

/*
 * Returns sum of all sample values
 * Conversion time is defined as 0.104 milliseconds for 16 MHz Arduino for ADC_PRESCALE128 in ADCUtils.h.
 * @ param aPrescale can be one of ADC_PRESCALE2, ADC_PRESCALE4, 8, 16, 32, 64, 128.
 *                   ADC_PRESCALE32 is recommended for excellent linearity and fast readout of 26 microseconds
 * @ param aNumberOfSamples If > 16k an overflow may occur.
 */
uint32_t readADCChannelMultiSamplesWithReferenceAndPrescaler(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aPrescale,
        uint16_t aNumberOfSamples) {
    uint32_t tSumValue = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | aPrescale);

    for (uint16_t i = 0; i < aNumberOfSamples; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // Add value
        tSumValue += ADCL | (ADCH << 8); // using WordUnionForADCUtils does not save space here
        // tSumValue += (ADCH << 8) | ADCL; // this does NOT work!
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return tSumValue;
}

/*
 * Returns sum of all sample values
 * Assumes, that channel and reference are still set to the right values
 * @ param aNumberOfSamples If > 16k an overflow may occur.
 */
uint32_t readADCChannelMultiSamples(uint8_t aPrescale, uint16_t aNumberOfSamples) {
    uint32_t tSumValue = 0;

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | aPrescale);

    for (uint16_t i = 0; i < aNumberOfSamples; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // Add value
        tSumValue += ADCL | (ADCH << 8); // using WordUnionForADCUtils does not save space here
        // tSumValue += (ADCH << 8) | ADCL; // this does NOT work!
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return tSumValue;
}
/*
 * use ADC_PRESCALE32 which gives 26 us conversion time and good linearity
 * @return the maximum value of aNumberOfSamples samples.
 */
uint16_t readADCChannelWithReferenceMax(uint8_t aADCChannelNumber, uint8_t aReference, uint16_t aNumberOfSamples) {
    uint16_t tADCValue = 0;
    uint16_t tMaximum = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | ADC_PRESCALE32);

    for (uint16_t i = 0; i < aNumberOfSamples; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // check value
        tADCValue = ADCL | (ADCH << 8);
        if (tADCValue > tMaximum) {
            tMaximum = tADCValue;
        }
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return tMaximum;
}

/*
 * use ADC_PRESCALE32 which gives 26 us conversion time and good linearity
 * @return the maximum value during aMicrosecondsToAquire measurement.
 */
uint16_t readADCChannelWithReferenceMaxMicros(uint8_t aADCChannelNumber, uint8_t aReference, uint16_t aMicrosecondsToAquire) {
    uint16_t tNumberOfSamples = aMicrosecondsToAquire / 26;
    return readADCChannelWithReferenceMax(aADCChannelNumber, aReference, tNumberOfSamples);
}

/*
 * aMaxRetries = 255 -> try forever
 * @return (tMax + tMin) / 2
 */
uint16_t readUntil4ConsecutiveValuesAreEqual(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aDelay,
        uint8_t aAllowedDifference, uint8_t aMaxRetries) {
    int tValues[4]; // last value is in tValues[3]
    int tMin;
    int tMax;

    /*
     * Initialize first 4 values before checking
     */
    tValues[0] = readADCChannelWithReference(aADCChannelNumber, aReference);
    for (int i = 1; i < 4; ++i) {
        if (aDelay != 0) {
            delay(aDelay); // Minimum is only 3 delays!
        }
        tValues[i] = readADCChannelWithReference(aADCChannelNumber, aReference);
    }

    do {
        /*
         * Get min and max of the last 4 values
         */
        tMin = 1024;
        tMax = 0;
        for (uint_fast8_t i = 0; i < 4; ++i) {
            if (tValues[i] < tMin) {
                tMin = tValues[i];
            }
            if (tValues[i] > tMax) {
                tMax = tValues[i];
            }
        }
        /*
         * check for terminating condition
         */
        if ((tMax - tMin) <= aAllowedDifference) {
            break;
        } else {
            /*
             * Get next value
             */
//            Serial.print("Difference=");
//            Serial.println(tMax - tMin);
            // Move values to front
            for (int i = 0; i < 3; ++i) {
                tValues[i] = tValues[i + 1];
            }
            // and wait before getting next value
            if (aDelay != 0) {
                delay(aDelay);
            }
            tValues[3] = readADCChannelWithReference(aADCChannelNumber, aReference);
        }
        if (aMaxRetries != 255) {
            aMaxRetries--;
        }
    } while (aMaxRetries > 0);

#if defined(LOCAL_DEBUG)
    if(aMaxRetries == 0) {
        Serial.print(F("No 4 equal values for difference "));
        Serial.print(aAllowedDifference);
        Serial.print(F(" found "));
        Serial.print(tValues[0]);
        Serial.print(' ');
        Serial.print(tValues[1]);
        Serial.print(' ');
        Serial.print(tValues[2]);
        Serial.print(' ');
        Serial.println(tValues[3]);
    } else {
        Serial.print(aMaxRetries);
        Serial.println(F(" retries left"));
    }
#endif

    return (tMax + tMin) / 2;
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 * Raw reading of 1.1 V is 225 at 5 V.
 * Raw reading of 1.1 V is 221 at 5.1 V.
 * Raw reading of 1.1 V is 214 at 5.25 V (+5 %).
 * Raw reading of 1.1 V is 204 at 5.5 V (+10 %).
 */
float getVCCVoltageSimple(void) {
    // use AVCC with (optional) external capacitor at AREF pin as reference
    float tVCC = readADCChannelMultiSamplesWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    return ((1023 * 1.1 * 4) / tVCC);
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
uint16_t getVCCVoltageMillivoltSimple(void) {
    // use AVCC with external capacitor at AREF pin as reference
    uint16_t tVCC = readADCChannelMultiSamplesWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    return ((1023L * ADC_INTERNAL_REFERENCE_MILLIVOLT * 4) / tVCC);
}

/*
 * Gets the hypothetical 14 bit reading of VCC using 1.1 volt reference
 * Similar to getVCCVoltageMillivolt() * 1023 / 1100
 */
uint16_t getVCCVoltageReadingFor1_1VoltReference(void) {
    uint16_t tVCC = waitAndReadADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT);
    /*
     * Do not switch back ADMUX to enable checkAndWaitForReferenceAndChannelToSwitch() to work correctly for the next measurement
     */
    return ((1023L * 1023L) / tVCC);
}

/*
 * !!! Resolution is only 20 millivolt !!!
 */
float getVCCVoltage(void) {
    return (getVCCVoltageMillivolt() / 1000.0);
}

/*
 * Read value of 1.1 volt internal channel using VCC (DEFAULT) as reference.
 * Handles reference and channel switching by introducing the appropriate delays.
 * !!! Resolution is only 20 millivolt !!!
 * Raw reading of 1.1 V is 225 at 5 V.
 * Raw reading of 1.1 V is 221 at 5.1 V.
 * Raw reading of 1.1 V is 214 at 5.25 V (+5 %).
 * Raw reading of 1.1 V is 204 at 5.5 V (+10 %).
 */
uint16_t getVCCVoltageMillivolt(void) {
    uint16_t tVCC = waitAndReadADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT);
    /*
     * Do not switch back ADMUX to enable checkAndWaitForReferenceAndChannelToSwitch() to work correctly for the next measurement
     */
    return ((1023L * ADC_INTERNAL_REFERENCE_MILLIVOLT) / tVCC);
}

/*
 * Does not set sVCCVoltageMillivolt
 */
uint16_t printVCCVoltageMillivolt(Print *aSerial) {
    aSerial->print(F("VCC="));
    uint16_t tVCCVoltageMillivolt = getVCCVoltageMillivolt();
    aSerial->print(tVCCVoltageMillivolt);
    aSerial->println(" mV");
    return tVCCVoltageMillivolt;
}

void readAndPrintVCCVoltageMillivolt(Print *aSerial) {
    aSerial->print(F("VCC="));
    sVCCVoltageMillivolt = getVCCVoltageMillivolt();
    aSerial->print(sVCCVoltageMillivolt);
    aSerial->println(" mV");
}
/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
void readVCCVoltageSimple(void) {
    // use AVCC with (optional) external capacitor at AREF pin as reference
    float tVCC = readADCChannelMultiSamplesWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    sVCCVoltage = (1023 * (((float) ADC_INTERNAL_REFERENCE_MILLIVOLT) / 1000) * 4) / tVCC;
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
void readVCCVoltageMillivoltSimple(void) {
    // use AVCC with external capacitor at AREF pin as reference
    uint16_t tVCCVoltageMillivoltRaw = readADCChannelMultiSamplesWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    sVCCVoltageMillivolt = (1023L * ADC_INTERNAL_REFERENCE_MILLIVOLT * 4) / tVCCVoltageMillivoltRaw;
}

/*
 * !!! Resolution is only 20 millivolt !!!
 */
void readVCCVoltage(void) {
    sVCCVoltage = getVCCVoltageMillivolt() / 1000.0;
}

/*
 * Read value of 1.1 volt internal channel using VCC (DEFAULT) as reference.
 * Handles reference and channel switching by introducing the appropriate delays.
 * !!! Resolution is only 20 millivolt !!!
 * Sets also the sVCCVoltageMillivolt variable.
 */
void readVCCVoltageMillivolt(void) {
    uint16_t tVCCVoltageMillivoltRaw = waitAndReadADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT);
    /*
     * Do not switch back ADMUX to enable checkAndWaitForReferenceAndChannelToSwitch() to work correctly for the next measurement
     */
    sVCCVoltageMillivolt = (1023L * ADC_INTERNAL_REFERENCE_MILLIVOLT) / tVCCVoltageMillivoltRaw;
}

/*
 * Get voltage at ADC channel aADCChannelForVoltageMeasurement
 * aVCCVoltageMillivolt is assumed as reference voltage
 */
uint16_t getVoltageMillivolt(uint16_t aVCCVoltageMillivolt, uint8_t aADCChannelForVoltageMeasurement) {
    uint16_t tInputVoltageRaw = waitAndReadADCChannelWithReference(aADCChannelForVoltageMeasurement, DEFAULT);
    return (aVCCVoltageMillivolt * (uint32_t) tInputVoltageRaw) / 1023;
}

/*
 * Get voltage at ADC channel aADCChannelForVoltageMeasurement
 * Reference voltage VCC is determined just before
 */
uint16_t getVoltageMillivolt(uint8_t aADCChannelForVoltageMeasurement) {
    uint16_t tInputVoltageRaw = waitAndReadADCChannelWithReference(aADCChannelForVoltageMeasurement, DEFAULT);
    return (getVCCVoltageMillivolt() * (uint32_t) tInputVoltageRaw) / 1023;
}

uint16_t getVoltageMillivoltWith_1_1VoltReference(uint8_t aADCChannelForVoltageMeasurement) {
    uint16_t tInputVoltageRaw = waitAndReadADCChannelWithReference(aADCChannelForVoltageMeasurement, INTERNAL);
    return (ADC_INTERNAL_REFERENCE_MILLIVOLT * (uint32_t) tInputVoltageRaw) / 1023;
}

/*
 * Return true if sVCCVoltageMillivolt is > 4.3 V and < 4.95 V
 */
bool isVCCUSBPowered() {
    readVCCVoltageMillivolt();
    return (VOLTAGE_USB_POWERED_LOWER_THRESHOLD_MILLIVOLT < sVCCVoltageMillivolt
            && sVCCVoltageMillivolt < VOLTAGE_USB_POWERED_UPPER_THRESHOLD_MILLIVOLT);
}

/*
 * Return true if sVCCVoltageMillivolt is > 4.3 V and < 4.95 V
 */
bool isVCCUSBPowered(Print *aSerial) {
    readVCCVoltageMillivolt();
    aSerial->print(F("USB powered is "));
    bool tReturnValue;
    if (VOLTAGE_USB_POWERED_LOWER_THRESHOLD_MILLIVOLT
            < sVCCVoltageMillivolt&& sVCCVoltageMillivolt < VOLTAGE_USB_POWERED_UPPER_THRESHOLD_MILLIVOLT) {
        tReturnValue = true;
        aSerial->print(F("true "));
    } else {
        tReturnValue = false;
        aSerial->print(F("false "));
    }
    printVCCVoltageMillivolt(aSerial);
    return tReturnValue;
}

/*
 * @ return true only once, when VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP (6) times voltage too low -> shutdown
 */
bool isVCCUndervoltageMultipleTimes() {
    /*
     * Check VCC every VCC_CHECK_PERIOD_MILLIS (10) seconds
     */

    if (millis() - sLastVCCCheckMillis >= VCC_CHECK_PERIOD_MILLIS) {
        sLastVCCCheckMillis = millis();

#  if defined(INFO)
        readAndPrintVCCVoltageMillivolt(&Serial);
#  else
        readVCCVoltageMillivolt();
#  endif

        if (sVCCTooLowCounter < VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP) {
            /*
             * Do not check again if shutdown has happened
             */
            if (sVCCVoltageMillivolt > VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) {
                sVCCTooLowCounter = 0; // reset counter
            } else {
                /*
                 * Voltage too low, wait VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP (6) times and then shut down.
                 */
                if (sVCCVoltageMillivolt < VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) {
                    // emergency shutdown
                    sVCCTooLowCounter = VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP;
#  if defined(INFO)
                    Serial.println(
                            F(
                                    "Voltage < " STR(VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) " mV detected -> emergency shutdown"));
#  endif
                } else {
                    sVCCTooLowCounter++;
#  if defined(INFO)
                    Serial.print(F("Voltage < " STR(VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) " mV detected: "));
                    Serial.print(VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP - sVCCTooLowCounter);
                    Serial.println(F(" tries left"));
#  endif
                }
                if (sVCCTooLowCounter == VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP) {
                    /*
                     * 6 times voltage too low -> return signal for shutdown etc.
                     */
                    return true;
                }
            }
        }
    }
    return false;
}

/*
 * Return true if VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT (3 V) reached
 */
bool isVCCUndervoltage() {
    readVCCVoltageMillivolt();
    return (sVCCVoltageMillivolt < VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT);
}

/*
 * Return true if VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT (3 V) reached
 */
bool isVCCEmergencyUndervoltage() {
    readVCCVoltageMillivolt();
    return (sVCCVoltageMillivolt < VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT);
}

void resetCounterForVCCUndervoltageMultipleTimes() {
    sVCCTooLowCounter = 0;
}

/*
 * Recommended VCC is 1.8 V to 5.5 V, absolute maximum VCC is 6.0 V.
 * Check for 5.25 V, because such overvoltage is quite unlikely to happen during regular operation.
 * Raw reading of 1.1 V is 225 at 5 V.
 * Raw reading of 1.1 V is 221 at 5.1 V.
 * Raw reading of 1.1 V is 214 at 5.25 V (+5 %).
 * Raw reading of 1.1 V is 204 at 5.5 V (+10 %).
 * Raw reading of 1.1 V is 1126000 / VCC_MILLIVOLT
 * @return true if 5 % overvoltage reached
 */
bool isVCCOvervoltage() {
    readVCCVoltageMillivolt();
    return (sVCCVoltageMillivolt > VCC_OVERVOLTAGE_THRESHOLD_MILLIVOLT);
}
bool isVCCOvervoltageSimple() {
    readVCCVoltageMillivoltSimple();
    return (sVCCVoltageMillivolt > VCC_OVERVOLTAGE_THRESHOLD_MILLIVOLT);
}

// Version not using readVCCVoltageMillivoltSimple()
bool isVCCTooHighSimple() {
    ADMUX = ADC_1_1_VOLT_CHANNEL_MUX | (DEFAULT << SHIFT_VALUE_FOR_REFERENCE);
// ADCSRB = 0; // Only active if ADATE is set to 1.
// ADSC-StartConversion ADIF-Reset Interrupt Flag - NOT free running mode
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADIF) | ADC_PRESCALE128); //  128 -> 104 microseconds per ADC conversion at 16 MHz --- Arduino default
// wait for single conversion to finish
    loop_until_bit_is_clear(ADCSRA, ADSC);

// Get value
    uint16_t tRawValue = ADCL | (ADCH << 8);

    return tRawValue < 1126000 / VCC_OVERVOLTAGE_THRESHOLD_MILLIVOLT;
}

/*
 * Temperature sensor is enabled by selecting the appropriate channel.
 * Different formula for 328P and 328PB!
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only use INTERNAL reference (e.g. only call getTemperatureSimple()) in your program.
 */
float getCPUTemperatureSimple(void) {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    return 0.0;
#else
    // use internal 1.1 volt as reference. 4 times oversample. Assume the signal has noise, but never verified :-(
    uint16_t tTemperatureRaw = readADCChannelWithReferenceOversample(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL, 2);
#if defined(LOCAL_DEBUG)
    Serial.print(F("TempRaw="));
    Serial.println(tTemperatureRaw);
#endif

#if defined(__AVR_ATmega328PB__)
    tTemperatureRaw -= 245;
    return (float)tTemperatureRaw;
#elif defined(__AVR_ATtiny85__)
    tTemperatureRaw -= 273; // 273 and 1.1666 are values from the datasheet
    return (float)tTemperatureRaw / 1.1666;
#else
    tTemperatureRaw -= 317;
    return (float) tTemperatureRaw / 1.22;
#endif
#endif
}

/*
 * Handles usage of 1.1 V reference and channel switching by introducing the appropriate delays.
 */
float getTemperature(void) {
    return getCPUTemperature();
}
float getCPUTemperature(void) {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    return 0.0;
#else
    // use internal 1.1 volt as reference
    checkAndWaitForReferenceAndChannelToSwitch(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL);
    return getCPUTemperatureSimple();
#endif
}

#else // defined(ADC_UTILS_ARE_AVAILABLE)
// Dummy definition of functions defined in ADCUtils to compile examples for non AVR platforms without errors
/*
 * Persistent storage for VCC value
 */
float sVCCVoltage;
uint16_t sVCCVoltageMillivolt;

uint16_t getVCCVoltageMillivoltSimple(void){
    return 3300;
}

uint16_t readADCChannelWithReferenceOversample(uint8_t aChannelNumber __attribute__((unused)),
        uint8_t aReference __attribute__((unused)), uint8_t aOversampleExponent __attribute__((unused))) {
    return 0;
}
float getCPUTemperature() {
    return 20.0;
}
float getVCCVoltage() {
    return 3.3;
}
#endif // defined(ADC_UTILS_ARE_AVAILABLE)

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _ADC_UTILS_HPP
