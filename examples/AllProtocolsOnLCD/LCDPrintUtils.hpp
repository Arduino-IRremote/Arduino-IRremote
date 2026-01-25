/*
 *  LCDPrintUtils.hpp
 *
 *  Contains LCD related variables and functions
 *  Sets symbols USE_SERIAL_LCD or USE_PARALLEL_LCD and LCD_COLUMNS and LCD_ROWS
 *
 *  Copyright (C) 2024-2025  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 */

#ifndef _LCD_PRINT_UTILS_HPP
#define _LCD_PRINT_UTILS_HPP

//#define LOCAL_DEBUG // This enables debug output only for this file - only for development

//#define USE_PARALLEL_2004_LCD // Is default
//#define USE_PARALLEL_1602_LCD
//#define USE_SERIAL_2004_LCD
//#define USE_SERIAL_1602_LCD

#if !defined(USE_PARALLEL_2004_LCD) && !defined(USE_PARALLEL_1602_LCD) && !defined(USE_SERIAL_2004_LCD) && !defined(USE_SERIAL_1602_LCD)
#warning "No LCD type like USE_SERIAL_2004_LCD specified, therefore using USE_PARALLEL_2004_LCD as default"
#define USE_PARALLEL_2004_LCD    // Use parallel 2004 LCD as default
#endif

#if defined(USE_PARALLEL_2004_LCD) || defined(USE_PARALLEL_1602_LCD)
#define USE_PARALLEL_LCD
#include "LiquidCrystal.h"
#else
#define USE_SERIAL_LCD
#include "LiquidCrystal_I2C.hpp" // Here we use an enhanced version, which supports SoftI2CMaster
#endif

#if defined(USE_PARALLEL_1602_LCD) || defined(USE_SERIAL_1602_LCD)
#define LCD_COLUMNS     16
#define LCD_ROWS        2
#else
#define LCD_COLUMNS     20
#define LCD_ROWS        4
#endif

// Helper macro for getting a macro definition as string
#if !defined(STR_HELPER) && !defined(STR)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

#define DEGREE_SIGN_STRING "\xDF"

extern char sStringBuffer[];    // For rendering a LCD row with snprintf_P()

#if defined(USE_PARALLEL_LCD)
void LCDResetCursor(LiquidCrystal *aLCD);
void LCDPrintSpaces(LiquidCrystal *aLCD, uint_fast8_t aNumberOfSpacesToPrint);
void LCDClearLine(LiquidCrystal *aLCD, uint_fast8_t aLineNumber); // Cursor is a start of line
size_t LCDPrintHex(LiquidCrystal *aLCD, uint16_t aHexByteValue);
void LCDPrintAsFloatAs5CharacterString_2_3_Decimals(LiquidCrystal *aLCD, uint16_t aValueInMilliUnits);
void LCDPrintFloatValueRightAligned(LiquidCrystal *aLCD, float aFloatValue, uint8_t aNumberOfCharactersToPrint,
        bool aNoLeadingSpaceForPositiveValues = false);
void LCDUnitTestPrintFloatValueRightAligned(Print *aSerial, LiquidCrystal *aLCD);
void LCDShowSpecialCharacters(LiquidCrystal *aLCD);
void LCDShowCustomCharacters(LiquidCrystal *aLCD);
#else
void LCDResetCursor(LiquidCrystal_I2C *aLCD);
void LCDPrintSpaces(LiquidCrystal_I2C *aLCD, uint_fast8_t aNumberOfSpacesToPrint);
void LCDClearLine(LiquidCrystal_I2C *aLCD, uint_fast8_t aLineNumber);
size_t LCDPrintHex(LiquidCrystal_I2C *aLCD, uint16_t aHexByteValue);
void LCDPrintAsFloatAs5CharacterString_2_3_Decimals(LiquidCrystal_I2C *aLCD, uint16_t aValueInMillivolts);
void LCDPrintFloatValueRightAligned(LiquidCrystal_I2C *aLCD, float aFloatValue, uint8_t aNumberOfCharactersToPrint,
        bool aNoLeadingSpaceForPositiveValues = false);
void LCDTestPrintFloatValueRightAligned(LiquidCrystal_I2C *aLCD);
void LCDShowSpecialCharacters(LiquidCrystal_I2C *aLCD);
void LCDShowCustomCharacters(LiquidCrystal_I2C *aLCD);
#endif
uint8_t getNumberOfDecimalsFor16BitValues(uint16_t a16BitValue);

/*
 * Code starts here
 */

#if defined(USE_PARALLEL_LCD)
void LCDResetCursor(LiquidCrystal *aLCD)
#else
void LCDResetCursor(LiquidCrystal_I2C *aLCD)
#endif
        {
    aLCD->setCursor(0, 0);
}

#if defined(USE_PARALLEL_LCD)
void LCDPrintSpaces(LiquidCrystal *aLCD, uint_fast8_t aNumberOfSpacesToPrint)
#else
void LCDPrintSpaces(LiquidCrystal_I2C *aLCD, uint_fast8_t aNumberOfSpacesToPrint)
#endif
        {
    for (uint_fast8_t i = 0; i < aNumberOfSpacesToPrint; ++i) {
        aLCD->print(' ');
    }
}

#if defined(USE_PARALLEL_LCD)
void LCDClearLine(LiquidCrystal *aLCD, uint_fast8_t aLineNumber)
#else
void LCDClearLine(LiquidCrystal_I2C *aLCD, uint_fast8_t aLineNumber)
#endif
        {
    aLCD->setCursor(0, aLineNumber);
    LCDPrintSpaces(aLCD, LCD_COLUMNS);
    aLCD->setCursor(0, aLineNumber);
}

#if defined(USE_PARALLEL_LCD)
size_t LCDPrintHex(LiquidCrystal *aLCD, uint16_t aHexByteValue)
#else
size_t LCDPrintHex(LiquidCrystal_I2C *aLCD, uint16_t aHexByteValue)
#endif
        {
    aLCD->print(F("0x"));
    size_t tPrintSize = 2;
    if (aHexByteValue < 0x10 || (aHexByteValue > 0x100 && aHexByteValue < 0x1000)) {
        aLCD->print('0'); // leading 0
        tPrintSize++;
    }
    return aLCD->print(aHexByteValue, HEX) + tPrintSize;
}

/*
 * Use 2 decimals, if value is >= 10
 */
#if defined(USE_PARALLEL_LCD)
void LCDPrintAsFloatAs5CharacterString_2_3_Decimals(LiquidCrystal *aLCD, uint16_t aValueInMilliUnits)
#else
void LCDPrintAsFloatAs5CharacterString_2_3_Decimals(LiquidCrystal_I2C *aLCD, uint16_t aValueInMilliUnits)
#endif
        {
    if (aValueInMilliUnits < 10000) {
        aLCD->print(((float) (aValueInMilliUnits)) / 1000, 3);
    } else {
        aLCD->print(((float) (aValueInMilliUnits)) / 1000, 2);
    }
}

/*
 * @return 1 for values from 0 to 9, 2 for 10 to 99 up to 5 for >= 10,000
 */
uint8_t getNumberOfDecimalsFor16BitValues(uint16_t a16BitValue) {
    uint16_t tCompareValue = 1;
    /*
     * Check for 10, 100, 1000
     */
    for (uint_fast8_t tNumberOfDecimals = 0; tNumberOfDecimals < 5; ++tNumberOfDecimals) {
        if (a16BitValue < tCompareValue) {
            return tNumberOfDecimals;
        }
        tCompareValue *= 10;
    }
    // here we have values >= 10,000
    return 5;
}

/*
 * @return 1 for values from 0 to 9, 2 for 10 to 99 up to 10 for >= 1,000,000,000
 * Requires 26 bytes more program space than getNumberOfDecimalsFor16BitValues()
 */
uint8_t getNumberOfDecimalsFor32BitValues(uint32_t a32BitValue) {
    uint_fast8_t tNumberOfDecimals = 1;
    uint32_t tCompareValue = 10;
    /*
     * Check for 10, 100, 1000 up to 100,000,000
     */
    for (; tNumberOfDecimals < 10; ++tNumberOfDecimals) {
        if (a32BitValue < tCompareValue) {
            return tNumberOfDecimals;
        }
        tCompareValue *= 10;
    }
    // here we have values >= 1,000,000,000
    return 10;
}
/*
 * !!! We internally use uint16_t, thus for bigger values (> 65,536 or < -65,536) we have an overflow.
 * @param aNoLeadingSpaceForPositiveValues - Normally each positive value has a leading space as placeholder for the minus sign.
 *   If we know, that values are always positive, this space can be omitted by aNoLeadingSpaceForPositiveValues == true.
 * @param aNumberOfCharactersToPrint - The characters to be used for the most negative value to show. !!! MUST BE < 8, This is NOT checked!!!
 *   I.e. 4 => max negative value is "-999" max positive value is " 999".
 *   Values below 10 and -10 are displayed as floats with decimal point " 9.9" and "-9.9".
 *   Values below 1 and -1 are displayed as floats without 0 before decimal point " .9" and "-.9".
 *   If positive, there is a leading space, which improves readability if directly concatenated to another value.
 *     e.g. "71V1.189A" is not readable, "71V 1.18A" is as well as "71V-1.18A".
 *
 * Saves programming space if used more than 1 times for printing floats if used instead of myLCD.print(JKComputedData.BatteryVoltageFloat, 2);
 */
#if defined(USE_PARALLEL_LCD)
void LCDPrintFloatValueRightAligned(LiquidCrystal *aLCD, float aFloatValue, uint8_t aNumberOfCharactersToPrint,
        bool aNoLeadingSpaceForPositiveValues)
#else
void LCDPrintFloatValueRightAligned(LiquidCrystal_I2C *aLCD, float aFloatValue, uint8_t aNumberOfCharactersToPrint,
        bool aNoLeadingSpaceForPositiveValues)
#endif
        {

    uint8_t tNumberOfDecimals = getNumberOfDecimalsFor16BitValues(abs(aFloatValue)); // remove sign for length computation
    int8_t tNumberOfDecimalPlaces = (aNumberOfCharactersToPrint - 2) - tNumberOfDecimals;
    if (aNoLeadingSpaceForPositiveValues && aFloatValue >= 0) {
        tNumberOfDecimalPlaces++; // Use this increased value internally, since we do not eventually print the '-'
    }
#if defined(LOCAL_DEBUG)
    Serial.print(F("NumberOfDecimalPlaces("));
    Serial.print(tNumberOfDecimals);
    Serial.print(F(", "));
    Serial.print(aNumberOfCharactersToPrint);
    Serial.print(F(")="));
    Serial.print(tNumberOfDecimalPlaces);
    Serial.print(F(" NumberOfDecimals="));
    Serial.println(tNumberOfDecimals);
#endif
    if (tNumberOfDecimalPlaces < 0) {
        tNumberOfDecimalPlaces = 0;
    }

    char tStringBuffer[9]; // For aNumberOfCharactersToPrint up to 8
    uint8_t tStartIndex = 0;
    if (tNumberOfDecimals == 0 && tNumberOfDecimalPlaces > 0) {
        if (aFloatValue >= 0) {
            if (!aNoLeadingSpaceForPositiveValues) {
                aLCD->print(' ');
            }
            tStartIndex = 1;
        } else {
            aLCD->print('-');
            tStartIndex = 2;
        }
    }
#if defined(__AVR__)
    dtostrf(aFloatValue, aNumberOfCharactersToPrint, tNumberOfDecimalPlaces, tStringBuffer);
#else
    snprintf(sStringBuffer, sizeof(tStringBuffer), "%f", aFloatValue);
#endif
    aLCD->print(&tStringBuffer[tStartIndex]);
}
/**
 * Unit test for LCDPrintFloatValueRightAligned()
 * @param aSerial
 * @param aLCD
 */
#if defined(USE_PARALLEL_LCD)
void LCDUnitTestPrintFloatValueRightAligned(Print *aSerial, LiquidCrystal *aLCD)
#else
void LCDUnitTestPrintFloatValueRightAligned(Print *aSerial, LiquidCrystal_I2C *aLCD)
#endif
        {
    aSerial->println(F("LCDTestPrintFloatValueRightAligned: 1."));

    aLCD->clear();
    float tTestValue = 123.45;
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 6);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 5);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 4); // no leading space here
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 3); // no leading space here
    // Result=" 123.4  123 123123"

    aLCD->setCursor(0, 1);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 6);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 5);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 4);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 3); // requires also 5 character
    // Result="-123.4 -123-123-123"

    aLCD->setCursor(0, 2);
    tTestValue = -1.234;
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 6);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 5);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 4);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 3);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 2);
    // Result="-1.234-1.23-1.2 -1-1"

    aLCD->setCursor(0, 3);
    tTestValue = -0.1234;
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 6);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 5);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 4);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 3);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 2);
    // Result="-.1234-.123-.12-.1-0"

    delay(4000);
    aSerial->println(F("LCDTestPrintFloatValueRightAligned: 2."));

    aLCD->clear();
    tTestValue = 123.45;
    // no leading space here
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 6, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 5, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 4, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 3, true);
    // Result="123.45123.4 123123"

    aLCD->setCursor(0, 1);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 6, true);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 5, true);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 4, true);
    LCDPrintFloatValueRightAligned(aLCD, -tTestValue, 3, true);
    // Result="-123.4 -123-123-123"

    tTestValue = 1.2344; // .12345 leads to rounding up for .1235
    aLCD->setCursor(0, 2);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 6, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 5, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 4, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 3, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 2, true);
    // Result="1.23441.2341.231.2 1"

    tTestValue = 0.12344; // .12345 leads to rounding up for .1235
    aLCD->setCursor(0, 3);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 6, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 5, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 4, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 3, true);
    LCDPrintFloatValueRightAligned(aLCD, tTestValue, 2, true);
    // Result=".12344.1234.123.12.1"

    delay(4000);
    aSerial->println(F("LCDTestPrintFloatValueRightAligned: End"));
}

/*
 * On my 2004 LCD the custom characters are available under 0 to 7 and mirrored to 8 to 15
 * The characters 0x80 to 0x8F are blanks
 */
#if defined(USE_PARALLEL_LCD)
void LCDShowSpecialCharacters(LiquidCrystal *aLCD)
#else
void LCDShowSpecialCharacters(LiquidCrystal_I2C *aLCD)
#endif
        {
    aLCD->setCursor(0, 0);
    // 0 to 7 are mirrored to 8 to 15 as described in datasheet
    for (uint_fast8_t i = 0; i < 0x8; ++i) {
        aLCD->write(i);
    }
    // Print some interesting characters
    aLCD->write(0xA1);
    aLCD->write(0xA5);
    aLCD->write(0xB0);
    aLCD->write(0xDB);
    aLCD->write(0xDF);

    aLCD->setCursor(0, 1);
    // The characters 0x10 to 0x1F seem to be all blanks => ROM Code: A00
    for (uint_fast8_t i = 0x10; i < 0x20; ++i) {
        aLCD->write(i);
    }
    aLCD->setCursor(0, 2);
    // The characters 0x80 to 0x8F seem to be all blanks => ROM Code: A00
    for (uint_fast8_t i = 0x80; i < 0x90; ++i) {
        aLCD->write(i);
    }
    aLCD->setCursor(0, 3);
    // The characters 0x90 to 0x9F seem to be all blanks => ROM Code: A00
    for (uint_fast8_t i = 0x90; i < 0xA0; ++i) {
        aLCD->write(i);
    }
    delay(2000);
}

#if defined(USE_PARALLEL_LCD)
void LCDShowCustomCharacters(LiquidCrystal *aLCD)
#else
void LCDShowCustomCharacters(LiquidCrystal_I2C *aLCD)
#endif
        {
    aLCD->setCursor(0, 0);
    for (uint_fast8_t i = 0; i < 0x08; ++i) {
        aLCD->write(i);
    }
}

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _LCD_PRINT_UTILS_HPP
