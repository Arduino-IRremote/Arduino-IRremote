/*
 * ADCUtils.hpp
 *
 * ADC utility functions. Conversion time is defined as 0.104 milliseconds for 16 MHz Arduinos in ADCUtils.h.
 *
 *  Copyright (C) 2016-2022  Armin Joachimsmeyer
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
#if defined(ADC_UTILS_ARE_AVAILABLE)

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
 * Persistent storage for VCC value
 */
float sVCCVoltage;
uint16_t sVCCVoltageMillivolt;

// for isVCCTooLowMultipleTimes()
long sLastVoltageCheckMillis;
uint8_t sVoltageTooLowCounter = 0;

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannel(uint8_t aChannelNumber) {
    WordUnionForADCUtils tUValue;
    ADMUX = aChannelNumber | (DEFAULT << SHIFT_VALUE_FOR_REFERENCE);

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
uint16_t readADCChannelWithReference(uint8_t aChannelNumber, uint8_t aReference) {
    WordUnionForADCUtils tUValue;
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

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
uint16_t waitAndReadADCChannelWithReference(uint8_t aChannelNumber, uint8_t aReference) {
    checkAndWaitForReferenceAndChannelToSwitch(aChannelNumber, aReference);
    return readADCChannelWithReference(aChannelNumber, aReference);
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 * Restores ADMUX after reading
 */
uint16_t waitAndReadADCChannelWithReferenceAndRestoreADMUXAndReference(uint8_t aChannelNumber, uint8_t aReference) {
    uint8_t tOldADMUX = checkAndWaitForReferenceAndChannelToSwitch(aChannelNumber, aReference);
    uint16_t tResult = readADCChannelWithReference(aChannelNumber, aReference);
    checkAndWaitForReferenceAndChannelToSwitch(tOldADMUX & MASK_FOR_ADC_CHANNELS, tOldADMUX >> SHIFT_VALUE_FOR_REFERENCE);
    return tResult;
}

/*
 * To prepare reference and ADMUX for next measurement
 */
void setADCMultiplexerAndReferenceForNextConversion(uint8_t aChannelNumber, uint8_t aReference) {
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);
}

/*
 * @return original ADMUX register content for optional later restoring values
 * All experimental values are acquired by using the ADCSwitchingTest example from this library
 */
uint8_t checkAndWaitForReferenceAndChannelToSwitch(uint8_t aChannelNumber, uint8_t aReference) {
    uint8_t tOldADMUX = ADMUX;
    /*
     * Must wait >= 7 us if reference has to be switched from 1.1 volt/INTERNAL to VCC/DEFAULT (seen on oscilloscope)
     * This is done after the 2 ADC clock cycles required for Sample & Hold :-)
     *
     * Must wait >= 7600 us for Nano board  >= 6200 for Uno board if reference has to be switched from VCC/DEFAULT to 1.1 volt/INTERNAL
     * Must wait >= 200 us if channel has to be switched to 1.1 volt internal channel if S&H was at 5 Volt
     */
    uint8_t tNewReference = (aReference << SHIFT_VALUE_FOR_REFERENCE);
    ADMUX = aChannelNumber | tNewReference;
#if defined(INTERNAL2V56)
    if ((tOldADMUX & MASK_FOR_ADC_REFERENCE) != tNewReference && (aReference == INTERNAL || aReference == INTERNAL2V56)) {
#else
    if ((tOldADMUX & MASK_FOR_ADC_REFERENCE) != tNewReference && aReference == INTERNAL) {
#endif
        /*
         * Switch reference from DEFAULT to INTERNAL
         */
        delayMicroseconds(8000); // experimental value is >= 7600 us for Nano board and 6200 for UNO board
    } else if ((tOldADMUX & 0x0F) != aChannelNumber) {
        if (aChannelNumber == ADC_1_1_VOLT_CHANNEL_MUX) {
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
uint16_t readADCChannelWithOversample(uint8_t aChannelNumber, uint8_t aOversampleExponent) {
    return readADCChannelWithReferenceOversample(aChannelNumber, DEFAULT, aOversampleExponent);
}

/*
 * Conversion time is defined as 0.104 milliseconds by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannelWithReferenceOversample(uint8_t aChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    uint16_t tSumValue = 0;
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

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
uint16_t readADCChannelWithReferenceOversampleFast(uint8_t aChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    uint16_t tSumValue = 0;
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

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
 * Conversion time is defined as 0.104 milliseconds for 16 MHz Arduino by ADC_PRESCALE in ADCUtils.h.
 */
uint16_t readADCChannelWithReferenceMultiSamples(uint8_t aChannelNumber, uint8_t aReference, uint8_t aNumberOfSamples) {
    uint16_t tSumValue = 0;
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

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
 * use ADC_PRESCALE32 which gives 26 us conversion time and good linearity
 * @return the maximum of aNumberOfSamples measurements.
 */
uint16_t readADCChannelWithReferenceMax(uint8_t aChannelNumber, uint8_t aReference, uint16_t aNumberOfSamples) {
    uint16_t tADCValue = 0;
    uint16_t tMaximum = 0;
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

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
 */
uint16_t readADCChannelWithReferenceMaxMicros(uint8_t aChannelNumber, uint8_t aReference, uint16_t aMicrosecondsToAquire) {
    uint16_t tNumberOfSamples = aMicrosecondsToAquire / 26;
    return readADCChannelWithReferenceMax(aChannelNumber, aReference, tNumberOfSamples);
}

/*
 * aMaxRetries = 255 -> try forever
 * @return (tMax + tMin) / 2
 */
uint16_t readUntil4ConsecutiveValuesAreEqual(uint8_t aChannelNumber, uint8_t aDelay, uint8_t aAllowedDifference,
        uint8_t aMaxRetries) {
    int tValues[4];
    int tMin;
    int tMax;

    tValues[0] = readADCChannel(aChannelNumber);
    for (int i = 1; i < 4; ++i) {
        delay(aDelay); // Only 3 delays!
        tValues[i] = readADCChannel(aChannelNumber);
    }

    do {
        // find min and max
        tMin = 1024;
        tMax = 0;
        for (int i = 0; i < 4; ++i) {
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
//            Serial.print("Difference=");
//            Serial.println(tMax - tMin);

            // move values
            for (int i = 0; i < 3; ++i) {
                tValues[i] = tValues[i + 1];
            }
            // and wait
            delay(aDelay);
            tValues[3] = readADCChannel(aChannelNumber);
        }
        if (aMaxRetries != 255) {
            aMaxRetries--;
        }
    } while (aMaxRetries > 0);

    return (tMax + tMin) / 2;
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
float getVCCVoltageSimple(void) {
    // use AVCC with (optional) external capacitor at AREF pin as reference
    float tVCC = readADCChannelWithReferenceMultiSamples(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    return ((1023 * 1.1 * 4) / tVCC);
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
uint16_t getVCCVoltageMillivoltSimple(void) {
    // use AVCC with external capacitor at AREF pin as reference
    uint16_t tVCC = readADCChannelWithReferenceMultiSamples(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    return ((1023L * ADC_INTERNAL_REFERENCE_MILLIVOLT * 4) / tVCC);
}

/*
 * Gets the hypothetical 14 bit reading of VCC using 1.1 volt reference
 * Similar to getVCCVoltageMillivolt() * 1023 / 1100
 */
uint16_t getVCCVoltageReadingFor1_1VoltReference(void) {
    uint16_t tVCC = waitAndReadADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT); // 225 for 1.1 V at 5 V VCC
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
 */
uint16_t getVCCVoltageMillivolt(void) {
    uint16_t tVCC = waitAndReadADCChannelWithReference(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT);
    /*
     * Do not switch back ADMUX to enable checkAndWaitForReferenceAndChannelToSwitch() to work correctly for the next measurement
     */
    return ((1023L * ADC_INTERNAL_REFERENCE_MILLIVOLT) / tVCC);
}

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
    float tVCC = readADCChannelWithReferenceMultiSamples(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
    sVCCVoltage = (1023 * 1.1 * 4) / tVCC;
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only call getVCCVoltageSimple() or getVCCVoltageMillivoltSimple() in your program.
 * !!! Resolution is only 20 millivolt !!!
 */
void readVCCVoltageMillivoltSimple(void) {
    // use AVCC with external capacitor at AREF pin as reference
    uint16_t tVCCVoltageMillivoltRaw = readADCChannelWithReferenceMultiSamples(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 4);
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
 * Default values are suitable for Li-ion batteries.
 * We normally have voltage drop at the connectors, so the battery voltage is assumed slightly higher, than the Arduino VCC.
 * But keep in mind that the ultrasonic distance module HC-SR04 may not work reliable below 3.7 volt.
 */
#if !defined(VCC_STOP_THRESHOLD_MILLIVOLT)
#define VCC_STOP_THRESHOLD_MILLIVOLT    3400 // Do not stress your battery and we require some power for standby
#endif
#if !defined(VCC_EMERGENCY_STOP_MILLIVOLT)
#define VCC_EMERGENCY_STOP_MILLIVOLT    3000 // Many Li-ions are specified down to 3.0 volt
#endif
#if !defined(VCC_CHECK_PERIOD_MILLIS)
#define VCC_CHECK_PERIOD_MILLIS        10000 // Period of VCC checks
#endif
#if !defined(VCC_CHECKS_TOO_LOW_BEFORE_STOP)
#define VCC_CHECKS_TOO_LOW_BEFORE_STOP     6 // Shutdown after 6 times (60 seconds) VCC below VCC_STOP_THRESHOLD_MILLIVOLT or 1 time below VCC_EMERGENCY_STOP_MILLIVOLT
#endif

/*
 * @ return true only once, when VCC_CHECKS_TOO_LOW_BEFORE_STOP (6) times voltage too low -> shutdown
 */
bool isVCCTooLowMultipleTimes() {
    /*
     * Check VCC every VCC_CHECK_PERIOD_MILLIS (10) seconds
     */

    if (millis() - sLastVoltageCheckMillis >= VCC_CHECK_PERIOD_MILLIS) {
        sLastVoltageCheckMillis = millis();

#  if defined(INFO)
        readAndPrintVCCVoltageMillivolt(&Serial);
#  else
        readVCCVoltageMillivolt();
#  endif

        if (sVoltageTooLowCounter < VCC_CHECKS_TOO_LOW_BEFORE_STOP) {
            /*
             * Do not check again if shutdown has happened
             */
            if (sVCCVoltageMillivolt > VCC_STOP_THRESHOLD_MILLIVOLT) {
                sVoltageTooLowCounter = 0; // reset counter
            } else {
                /*
                 * Voltage too low, wait VCC_CHECKS_TOO_LOW_BEFORE_STOP (6) times and then shut down.
                 */
                if (sVCCVoltageMillivolt < VCC_EMERGENCY_STOP_MILLIVOLT) {
                    // emergency shutdown
                    sVoltageTooLowCounter = VCC_CHECKS_TOO_LOW_BEFORE_STOP;
#  if defined(INFO)
                    Serial.println(F("Voltage < " STR(VCC_EMERGENCY_STOP_MILLIVOLT) " mV detected -> emergency shutdown"));
#  endif
                } else {
                    sVoltageTooLowCounter++;
#  if defined(INFO)
                    Serial.print(F("Voltage < " STR(VCC_STOP_THRESHOLD_MILLIVOLT) " mV detected: "));
                    Serial.print(VCC_CHECKS_TOO_LOW_BEFORE_STOP - sVoltageTooLowCounter);
                    Serial.println(F(" tries left"));
#  endif
                }
                if (sVoltageTooLowCounter == VCC_CHECKS_TOO_LOW_BEFORE_STOP) {
                    /*
                     * 6 times voltage too low -> shutdown
                     */
                    return true;
                }
            }
        }
    }
    return false;
}

void resetVCCTooLowMultipleTimes(){
    sVoltageTooLowCounter = 0;
}

bool isVoltageTooLow(){
    return (sVoltageTooLowCounter >= VCC_CHECKS_TOO_LOW_BEFORE_STOP);
}

/*
 * !!! Function without handling of switched reference and channel.!!!
 * Use it ONLY if you only use INTERNAL reference (call getTemperatureSimple()) in your program.
 */
float getTemperatureSimple(void) {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    return 0.0;
#else
// use internal 1.1 volt as reference
    float tTemp = (readADCChannelWithReferenceMultiSamples(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL, 2) - 317);
    return (tTemp * (4 / 1.22));
#endif
}

/*
 * Handles reference and channel switching by introducing the appropriate delays.
 */
float getTemperature(void) {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    return 0.0;
#else
    // use internal 1.1 volt as reference
    checkAndWaitForReferenceAndChannelToSwitch(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL);
    // assume the signal has noise, but never verified :-(
    float tTemp = (readADCChannelWithReferenceOversample(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL, 1) - 317);
    return (tTemp / 1.22);
#endif
}

#elif defined(ARDUINO_ARCH_APOLLO3) // defined(ADC_UTILS_ARE_AVAILABLE)
    void ADCUtilsDummyToAvoidBFDAssertions(){
        ;
    }
#endif // defined(ADC_UTILS_ARE_AVAILABLE)

#endif // _ADC_UTILS_HPP
