/*
 * ADCUtils.hpp
 *
 * ADC utility functions. Conversion time is defined as 0.104 milliseconds for 16 MHz Arduinos in ADCUtils.h by setting ADC_PRESCALE.
 * Formula for 1.1V reference is: mV = <Reading> * <ADC_INTERNAL_REFERENCE_MILLIVOLT> / READING_FOR_AREF = <Reading> * 1100 / 1024 =  <Reading> * 1.07421875
 *
 *  Copyright (C) 2016-2026  Armin Joachimsmeyer
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  Arduino-Utils is free software: you can redistribute it and/or modify
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
 *  along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef _ADC_UTILS_HPP
#define _ADC_UTILS_HPP

#include "ADCUtils.h"
#if defined(ADC_UTILS_ARE_AVAILABLE) // set in ADCUtils.h, if supported architecture was detected
#define ADC_UTILS_ARE_INCLUDED

// Helper macro for getting a macro definition as string
#if !defined(STR_HELPER) && !defined(STR)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
#if !defined(BITS_PER_BYTE)
#define BITS_PER_BYTE 8
#endif

/*
 * By replacing this value with the voltage you measured a the AREF pin after a conversion
 * with INTERNAL you can calibrate your ADC readout. For my Nanos I measured e.g. 1060 mV and 1093 mV.
 * If value > real AREF voltage, measured values are > real values
 */
#if !defined(ADC_INTERNAL_REFERENCE_MILLIVOLT)
#define ADC_INTERNAL_REFERENCE_MILLIVOLT    1100 // Change to value measured at the AREF pin.
#endif

/*
 * Enable this to see information on each call.
 * Since there should be no library which uses Serial, it should only be enabled for development purposes.
 */
#if defined(DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif
#if defined(INFO)
#define LOCAL_INFO
#else
//#define LOCAL_INFO // This enables debug output only for this file
#endif

/*
 * Persistent storage for VCC value
 */
float sVCCVoltage;
uint16_t sVCCVoltageMillivolt; // Set by readVCCVoltageMillivoltSimple(), readVCCVoltageMillivolt()

// for isVCCTooLowMultipleTimes()
unsigned long sLastVCCCheckMillis;
uint8_t sVCCTooLowCounter = 0;

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 * Use previous settings
 */
uint16_t readADCChannel() {
    // ADCSRB = 0; // Only active if ADATE is set to 1.
    // ADSC-StartConversion ADIF-Reset Interrupt Flag - NOT free running mode
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADIF) | ADC_PRESCALE);

    // wait for single conversion to finish
    loop_until_bit_is_clear(ADCSRA, ADSC);

    return ADCW;
}

/*
 * Use new channel aADCChannelNumber, but do not wait for channel switching
 */
uint16_t readADCChannel(uint8_t aADCChannelNumber) {
    return readADCChannelWithReference(aADCChannelNumber, DEFAULT);
}
/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannelWithReference(uint8_t aADCChannelNumber, uint8_t aReference) {
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    // ADCSRB = 0; // Only active if ADATE is set to 1.
    // ADSC-StartConversion ADIF-Reset Interrupt Flag - NOT free running mode
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADIF) | ADC_PRESCALE);

    // wait for single conversion to finish
    loop_until_bit_is_clear(ADCSRA, ADSC);

    return ADCW;
}

uint16_t readADCChannelWithReferenceUsingInternalReference(uint8_t aADCChannelNumber) {
    return waitAndReadADCChannelWithReference(aADCChannelNumber, INTERNAL);
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
    checkAndWaitForReferenceAndChannelToSwitch(tOldADMUX & ADC_CHANNEL_MUX_MASK,
            (tOldADMUX & MASK_FOR_ADC_REFERENCE) >> SHIFT_VALUE_FOR_REFERENCE);
    return tResult;
}

/*
 * To prepare reference and ADMUX for next measurement
 */
void setADCChannelAndReferenceForNextConversion(uint8_t aADCChannelNumber, uint8_t aReference) {
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);
}

/*
 * 100 kOhm requires < 100 us, 1 MOhm requires 120 us S&H switching time
 */
void setADCChannelForNextConversionAndWaitUsingInternalReference(uint8_t aADCChannelNumber) {
    ADMUX = aADCChannelNumber | (INTERNAL << SHIFT_VALUE_FOR_REFERENCE);
    delayMicroseconds(120); // experimental value is <= 1100 us for Nano board
}

void setADCChannelForNextConversionAndWaitUsingDefaultReference(uint8_t aADCChannelNumber) {
    ADMUX = aADCChannelNumber | (DEFAULT << SHIFT_VALUE_FOR_REFERENCE);
    delayMicroseconds(120); // experimental value is <= 1100 us for Nano board
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
            delayMicroseconds(350); // 350 was OK and 300 was too less for UltimateBatteryTester - result was 226 instead of 225
        } else {
            /*
             * 100 kOhm requires < 100 us, 1 MOhm requires 120 us S&H switching time
             */
            delayMicroseconds(120); // experimental value is <= 1100 us for Nano board
        }
    }
    return tOldADMUX;
}

uint16_t readADCChannelWithReferenceAndPrescalerAndOversample(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aPrescale,
        uint8_t aOversampleExponent) {
    if (aOversampleExponent > 6) {
        aOversampleExponent = 6;
    }

    uint16_t tSumValue = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | aPrescale);

    uint8_t tCount = _BV(aOversampleExponent);
    for (uint8_t i = 0; i < tCount; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // Add value
        tSumValue += ADCW; // using WordUnionForADCUtils does not save space here
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    // return rounded value
    return ((tSumValue + (tCount >> 1)) >> aOversampleExponent);
}

/*
 * Oversample and multiple samples only makes sense if you expect a noisy input signal
 * It does NOT increase the precision of the measurement, since the ADC has insignificant noise
 */
uint16_t readADCChannelWithOversample(uint8_t aADCChannelNumber, uint8_t aOversampleExponent) {
    return readADCChannelWithReferenceAndPrescalerAndOversample(aADCChannelNumber, DEFAULT, ADC_PRESCALE, aOversampleExponent);
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannelWithReferenceOversample(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    return readADCChannelWithReferenceAndPrescalerAndOversample(aADCChannelNumber, aReference, ADC_PRESCALE, aOversampleExponent);
}
/*
 * Use ADC_PRESCALE_26_MICROS which gives 26 us conversion time and good linearity for 16 MHz Arduino
 */
uint16_t readADCChannelWithReferenceOversampleFast(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    return readADCChannelWithReferenceAndPrescalerAndOversample(aADCChannelNumber, aReference, ADC_PRESCALE_26_MICROS,
            aOversampleExponent);
}

/*
 * Returns sum of all sample values
 * Assumes, that channel and reference are still set to the right values
 * !!!We do NOT disable interrupts here!!!
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
        tSumValue += ADCW; // using WordUnionForADCUtils does not save space here
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return tSumValue;
}

/*
 * Returns sum of all sample values
 * Conversion time is defined as 0.104 milliseconds for 16 MHz Arduino for ADC_PRESCALE128 in ADCUtils.h.
 * @ param aPrescale can be one of ADC_PRESCALE2, ADC_PRESCALE4, 8, 16, 32, 64, 128.
 *                   ADC_PRESCALE_26_MICROS is recommended for excellent linearity and fast readout of 26 microseconds
 */
uint32_t readADCChannelMultiSamplesWithReferenceAndPrescaler(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aPrescale,
        uint16_t aNumberOfSamples) {
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);
    return readADCChannelMultiSamples(aPrescale, aNumberOfSamples);
}

/*
 * Returns sum of all sample values
 * Conversion time is defined as 0.104 milliseconds for 16 MHz Arduino by ADC_PRESCALE (=ADC_PRESCALE128) in ADCUtils.h.
 * @ param aNumberOfSamples If > 64 an overflow may occur.
 */
uint16_t readADCChannelMultiSamplesWithReference(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aNumberOfSamples) {
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);
    return readADCChannelMultiSamples(ADC_PRESCALE, aNumberOfSamples);
}

/*
 * use ADC_PRESCALE_26_MICROS which gives 26 us conversion time and good linearity
 * @return the maximum value of aNumberOfSamples samples.
 */
uint16_t readADCChannelWithReferenceMax(uint8_t aADCChannelNumber, uint8_t aReference, uint16_t aNumberOfSamples) {
    uint16_t tADCValue = 0;
    uint16_t tMaximum = 0;
    ADMUX = aADCChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

    ADCSRB = 0; // Free running mode. Only active if ADATE is set to 1.
    // ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | ADC_PRESCALE_26_MICROS);

    for (uint16_t i = 0; i < aNumberOfSamples; i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to enable recognizing next conversion has finished
        // check value
        tADCValue = ADCW;
        if (tADCValue > tMaximum) {
            tMaximum = tADCValue;
        }
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return tMaximum;
}

/*
 * ADC_PRESCALE_26_MICROS always gives 26 us conversion time and good linearity
 * @return the maximum value during aMicrosecondsToAquire measurement.
 */
uint16_t readADCChannelWithReferenceMaxMicros(uint8_t aADCChannelNumber, uint8_t aReference, uint16_t aMicrosecondsToAquire) {
    uint16_t tNumberOfSamples = aMicrosecondsToAquire / 26;
    return readADCChannelWithReferenceMax(aADCChannelNumber, aReference, tNumberOfSamples);
}

/*
 * Reads until the last 4 values differ by at most aAllowedDifference.
 * aMaxRetries = number of additional reads after the first 4, 255 -> try forever
 * @return (tMax + tMin) / 2 of the last 4 values
 */
uint16_t readUntil4ConsecutiveValuesAreEqual(uint8_t aADCChannelNumber, uint8_t aReference, uint8_t aDelay,
        uint8_t aAllowedDifference, uint8_t aMaxRetries) {
    uint16_t tValues[4];
    uint16_t tMin, tMax;
    uint_fast8_t tIndex = 0;

    while (true) {
        tValues[tIndex & 0x03] = readADCChannelWithReference(aADCChannelNumber, aReference);
        if (++tIndex >= 4) { // buffer is filled -> check
            if (tIndex == 8) {
                tIndex = 4; // keep index in range 4 to 7 to avoid overflow
            }
            tMin = tMax = tValues[0];
            for (uint_fast8_t i = 1; i < 4; ++i) {
                if (tValues[i] < tMin) {
                    tMin = tValues[i];
                }
                if (tValues[i] > tMax) {
                    tMax = tValues[i];
                }
            }
            if ((tMax - tMin) <= aAllowedDifference || (aMaxRetries != 255 && aMaxRetries-- == 0)) {
                break;
            }
        }
        if (aDelay != 0) {
            delay(aDelay);
        }
    }
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
    return (READING_FOR_AREF * (ADC_INTERNAL_REFERENCE_MILLIVOLT / 1000.0) * 4) / tVCC;
}
void readVCCVoltageSimple(void) {
    sVCCVoltage = getVCCVoltageSimple();
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
uint16_t getVCCVoltageMillivoltSimple(void) {
    // use AVCC with external capacitor at AREF pin as reference
    uint16_t tVCC = readADCChannelMultiSamplesWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    return (((uint32_t) READING_FOR_AREF * ADC_INTERNAL_REFERENCE_MILLIVOLT * 4) / tVCC);
}
void readVCCVoltageMillivoltSimple(void) {
    sVCCVoltageMillivolt = getVCCVoltageMillivoltSimple();
}

/*
 * Gets the hypothetical 14 bit reading of VCC using 1.1 volt reference
 * Similar to getVCCVoltageMillivolt() * 1024 / 1100
 */
uint16_t getVCCVoltageReadingFor1_1VoltReference(void) {
    uint16_t tVCC = waitAndReadADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT);
    /*
     * Do not switch back ADMUX to enable checkAndWaitForReferenceAndChannelToSwitch() to work correctly for the next measurement
     */
    return (((uint32_t) READING_FOR_AREF * READING_FOR_AREF) / tVCC);
}

/*
 * !!! Resolution is only 20 millivolt !!!
 */
float getVCCVoltage(void) {
    return (getVCCVoltageMillivolt() / 1000.0);
}
void readVCCVoltage(void) {
    sVCCVoltage = getVCCVoltage();
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
    return (((uint32_t) READING_FOR_AREF * ADC_INTERNAL_REFERENCE_MILLIVOLT) / tVCC);
}
void readVCCVoltageMillivolt(void) {
    sVCCVoltageMillivolt = getVCCVoltageMillivolt();
}

void printVCCVoltageMillivolt(Print *aSerial, uint16_t aVCCVoltageMillivolt) {
    aSerial->print(F("VCC="));
    aSerial->print(aVCCVoltageMillivolt);
    aSerial->println(F(" mV"));
}
/*
 * Does not set sVCCVoltageMillivolt
 */
uint16_t printVCCVoltageMillivolt(Print *aSerial) {
    uint16_t tVCCVoltageMillivolt = getVCCVoltageMillivolt();
    printVCCVoltageMillivolt(aSerial, tVCCVoltageMillivolt);
    return tVCCVoltageMillivolt;
}
/*
 * Sets sVCCVoltageMillivolt
 */
void readAndPrintVCCVoltageMillivolt(Print *aSerial) {
    sVCCVoltageMillivolt = getVCCVoltageMillivolt();
    printVCCVoltageMillivolt(aSerial, sVCCVoltageMillivolt);
}

/*
 * Get voltage at ADC channel aADCChannelForVoltageMeasurement
 * aVCCVoltageMillivolt is assumed as reference voltage
 */
uint16_t getVoltageMillivolt(uint16_t aVCCVoltageMillivolt, uint8_t aADCChannelForVoltageMeasurement) {
    uint16_t tInputVoltageRaw = waitAndReadADCChannelWithReference(aADCChannelForVoltageMeasurement, DEFAULT);
    return (aVCCVoltageMillivolt * (uint32_t) tInputVoltageRaw) / READING_FOR_AREF;
}

/*
 * Get voltage at ADC channel aADCChannelForVoltageMeasurement
 * Reference voltage VCC is determined just before
 */
uint16_t getVoltageMillivolt(uint8_t aADCChannelForVoltageMeasurement) {
    uint16_t tInputVoltageRaw = waitAndReadADCChannelWithReference(aADCChannelForVoltageMeasurement, DEFAULT);
    return (getVCCVoltageMillivolt() * (uint32_t) tInputVoltageRaw) / READING_FOR_AREF;
}

uint16_t getVoltageMillivoltWith_1_1VoltReference(uint8_t aADCChannelForVoltageMeasurement) {
    uint16_t tInputVoltageRaw = waitAndReadADCChannelWithReference(aADCChannelForVoltageMeasurement, INTERNAL);
    return (ADC_INTERNAL_REFERENCE_MILLIVOLT * (uint32_t) tInputVoltageRaw) / READING_FOR_AREF;
}

/*
 * Return true if sVCCVoltageMillivolt is > 4.3 V and < 4.95 V
 * This does not really work for the UNO board, because it has no series Diode in the USB VCC
 * and therefore a very low voltage drop.
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
    bool tReturnValue = isVCCUSBPowered();
    aSerial->print(F("USB powered is "));
    aSerial->print(tReturnValue ? F("true ") : F("false "));
    printVCCVoltageMillivolt(aSerial, sVCCVoltageMillivolt);
    return tReturnValue;
}

/*
 * It checks every 10 seconds for 6 times, and then returns once true if the undervoltage condition ( <3.4V ) still applies.
 * Uses the following macros: - The default values are suitable for Li-ion batteries.
 * VCC_CHECK_PERIOD_MILLIS, VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP, VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT, VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT
 * @ return true only once, when VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP (6) times voltage too low -> shutdown
 */
bool isVCCUndervoltageMultipleTimes(Print *aSerial) {
#if !defined(LOCAL_INFO)
    (void) aSerial;
#endif
    /*
     * Check VCC every VCC_CHECK_PERIOD_MILLIS - default is 10 seconds
     */
    if (millis() - sLastVCCCheckMillis >= VCC_CHECK_PERIOD_MILLIS) {
        sLastVCCCheckMillis = millis();

#  if defined(LOCAL_INFO)
        readAndPrintVCCVoltageMillivolt(aSerial);
#  else
        readVCCVoltageMillivolt();
#  endif

        /*
         * Do not check again if shutdown signaling (sVCCTooLowCounter >= 6) has happened
         */
        if (sVCCTooLowCounter < VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP) { // VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP = 6
            if (sVCCVoltageMillivolt > VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) {
                sVCCTooLowCounter = 0; // reset counter
            } else {
                /*
                 * Voltage too low, wait VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP (6) times and then signal shut down.
                 */
                if (sVCCVoltageMillivolt < VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) {
                    // emergency shutdown
                    sVCCTooLowCounter = VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP;
#  if defined(LOCAL_INFO)
                    aSerial->println(
                            F(
                                    "Undervoltage < " STR(VCC_EMERGENCY_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) " mV detected -> emergency shutdown"));
#  endif
                } else {
                    sVCCTooLowCounter++;
#  if defined(LOCAL_INFO)
                    aSerial->print(sVCCVoltageMillivolt);
                    aSerial->print(F(" mV < " STR(VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT) " mV -> undervoltage detected: "));

                    aSerial->print(VCC_UNDERVOLTAGE_CHECKS_BEFORE_STOP - sVCCTooLowCounter);
                    aSerial->println(F(" attempts left"));
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
 * Return true if VCC_UNDERVOLTAGE_THRESHOLD_MILLIVOLT (3.4 V) reached
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
    uint16_t tRawValue = readADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT);
    return tRawValue < ((uint32_t) READING_FOR_AREF * ADC_INTERNAL_REFERENCE_MILLIVOLT) / VCC_OVERVOLTAGE_THRESHOLD_MILLIVOLT;
}

/*
 * Temperature sensor is enabled by selecting the appropriate channel.
 * Different formula for 328P and 328PB!
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only use INTERNAL reference (e.g. only call getTemperatureSimple()) in your program.
 */
float getCPUTemperatureSimple(void) {
#if !defined(ADC_TEMPERATURE_CHANNEL_MUX)
    return 0.0; // No temperature sensor available on this CPU
#else
    // use internal 1.1 volt as reference. 4 times oversample. Assume the signal has noise, but never verified :-(
    int16_t tTemperatureRaw = (int16_t) readADCChannelWithReferenceOversample(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL, 2);
#if defined(LOCAL_DEBUG)
    Serial.print(F("TempRaw="));
    Serial.println(tTemperatureRaw);
#endif

#if defined(__AVR_ATmega328PB__)
    return (float)(tTemperatureRaw - 245);
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    return (tTemperatureRaw - 273) / 1.1666; // 273 and 1.1666 are values from the datasheet
#else
    return (tTemperatureRaw - 317) / 1.22;
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
#if !defined(ADC_TEMPERATURE_CHANNEL_MUX)
    return 0.0; // No temperature sensor available on this CPU
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
#if defined(LOCAL_INFO)
#undef LOCAL_INFO
#endif
#endif // _ADC_UTILS_HPP
