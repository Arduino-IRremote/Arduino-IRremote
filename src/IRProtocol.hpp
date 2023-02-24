/*
 * IRReceive.hpp
 * This file is exclusively included by IRremote.h to enable easy configuration of library switches
 *
 *  Contains all protocol functions used by receiver and sender.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2009-2023 Ken Shirriff, Rafi Khan, Armin Joachimsmeyer
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
#ifndef _IR_PROTOCOL_HPP
#define _IR_PROTOCOL_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/*
 * Check for additional characteristics of timing like length of mark for a constant mark protocol,
 * where space length determines the bit value. Requires up to 194 additional bytes of program memory.
 */
//#define DECODE_STRICT_CHECKS
/** \addtogroup Receiving Receiving IR data for multiple protocols
 * @{
 */

const char string_Unknown[] PROGMEM = "UNKNOWN";
const char string_PulseWidth[] PROGMEM = "PulseWidth";
const char string_PulseDistance[] PROGMEM = "PulseDistance";
const char string_Apple[] PROGMEM = "Apple";
const char string_Denon[] PROGMEM = "Denon";
const char string_JVC[] PROGMEM = "JVC";
const char string_LG[] PROGMEM = "LG";
const char string_LG2[] PROGMEM = "LG2";
const char string_NEC[] PROGMEM = "NEC";
const char string_NEC2[] PROGMEM = "NEC2";
const char string_Onkyo[] PROGMEM = "Onkyo";
const char string_Panasonic[] PROGMEM = "Panasonic";
const char string_Kaseikyo[] PROGMEM = "Kaseikyo";
const char string_Kaseikyo_Denon[] PROGMEM = "Kaseikyo_Denon";
const char string_Kaseikyo_Sharp[] PROGMEM = "Kaseikyo_Sharp";
const char string_Kaseikyo_JVC[] PROGMEM = "Kaseikyo_JVC";
const char string_Kaseikyo_Mitsubishi[] PROGMEM = "Kaseikyo_Mitsubishi";
const char string_RC5[] PROGMEM = "RC5";
const char string_RC6[] PROGMEM = "RC6";
const char string_Samsung[] PROGMEM = "Samsung";
const char string_Samsung48[] PROGMEM = "Samsung48";
const char string_SamsungLG[] PROGMEM = "SamsungLG";
const char string_Sharp[] PROGMEM = "Sharp";
const char string_Sony[] PROGMEM = "Sony";
const char string_BangOlufsen[] PROGMEM = "Bang&Olufsen";
const char string_BoseWave[] PROGMEM = "BoseWave";
const char string_Lego[] PROGMEM = "Lego";
const char string_MagiQuest[] PROGMEM = "MagiQuest";
const char string_Whynter[] PROGMEM = "Whynter";
const char string_FAST[] PROGMEM = "FAST";

/*
 * !!Must be the same order as in decode_type_t in IRProtocol.h!!!
 */
const char *const ProtocolNames[]
PROGMEM = { string_Unknown, string_PulseWidth, string_PulseDistance, string_Apple, string_Denon, string_JVC, string_LG, string_LG2,
        string_NEC, string_NEC2, string_Onkyo, string_Panasonic, string_Kaseikyo, string_Kaseikyo_Denon, string_Kaseikyo_Sharp,
        string_Kaseikyo_JVC, string_Kaseikyo_Mitsubishi, string_RC5, string_RC6, string_Samsung, string_Samsung48, string_SamsungLG,
        string_Sharp, string_Sony
#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
        , string_BangOlufsen, string_BoseWave, string_Lego, string_MagiQuest, string_Whynter, string_FAST
#endif
        };

#if defined(__AVR__)
const __FlashStringHelper* getProtocolString(decode_type_t aProtocol) {
    const char *tProtocolStringPtr = (char*) pgm_read_word(&ProtocolNames[aProtocol]);
    return ((__FlashStringHelper*) (tProtocolStringPtr));
}
#else
const char* getProtocolString(decode_type_t aProtocol) {
    return ProtocolNames[aProtocol];
}
#endif

#if (__INT_WIDTH__ >= 32)
#  if __has_include(<type_traits>)
/*
 * This code to handle the missing print(unsigned long long...) function of seeduino core was contributed by sklott
 * https://stackoverflow.com/questions/74622227/avoid-calling-of-function-size-t-printprintunsigned-long-long-n-int-base-if
 */
#include <type_traits>

// If you have C++17 you can just use std::void_t, or use this for all versions
#if __cpp_lib_void_t >= 201411L
template<typename T>
using void_t = std::void_t<T>;
#else
template<typename ... Ts> struct make_void {
    typedef void type;
};
template<typename ... Ts> using void_t = typename make_void<Ts...>::type;
#endif

// Detecting if we have print(unsigned long long value, int base) / print(0ull, 0) overload
template<typename T, typename = void>
struct has_ull_print: std::false_type {
};
template<typename T>
struct has_ull_print<T, void_t<decltype(std::declval<T>().print(0ull, 0))>> : std::true_type {
};

// Must be namespace, to avoid public and static declarations for class
namespace PrintULL {
template<typename PrintImplType, typename std::enable_if<!has_ull_print<PrintImplType>::value, bool>::type = true>
size_t print(PrintImplType *p, unsigned long long value, int base) {
    size_t tLength = p->print(static_cast<uint32_t>(value >> 32), base);
    tLength += p->print(static_cast<uint32_t>(value), base);
    return tLength;
}

template<typename PrintImplType, typename std::enable_if<has_ull_print<PrintImplType>::value, bool>::type = true>
size_t print(PrintImplType *p, unsigned long long value, int base) {
    return p->print(value, base);
}
}
;
#  else
namespace PrintULL {
    size_t print(Print *aSerial, unsigned long long n, int base) {
        return aSerial->print(n, base);
    }
};
#  endif
#endif

/**
 * Function to print decoded result and flags in one line.
 * A static function to be able to print data to send or copied received data.
 * Ends with println().
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 * @param aIRDataPtr        Pointer to the data to be printed.
 * @param aPrintRepeatGap   If true also print the gap before repeats.
 *
 */
void printIRResultShort(Print *aSerial, IRData *aIRDataPtr, bool aPrintRepeatGap) {
    if (aIRDataPtr->flags & IRDATA_FLAGS_WAS_OVERFLOW) {
        aSerial->println(F("Overflow"));
        return;
    }
    aSerial->print(F("Protocol="));
    aSerial->print(getProtocolString(aIRDataPtr->protocol));
    if (aIRDataPtr->protocol == UNKNOWN) {
#if defined(DECODE_HASH)
        aSerial->print(F(" Hash=0x"));
#if (__INT_WIDTH__ < 32)
        aSerial->print(aIRDataPtr->decodedRawData, HEX);
#else
        PrintULL::print(aSerial,aIRDataPtr->decodedRawData, HEX);
#endif

#endif
#if !defined(DISABLE_CODE_FOR_RECEIVER)
        aSerial->print(' ');
        aSerial->print((aIRDataPtr->rawDataPtr->rawlen + 1) / 2, DEC);
        aSerial->println(F(" bits (incl. gap and start) received"));
#endif
    } else {
#if defined(DECODE_DISTANCE_WIDTH)
        if (aIRDataPtr->protocol != PULSE_DISTANCE && aIRDataPtr->protocol != PULSE_WIDTH) {
#endif
        /*
         * New decoders have address and command
         */
        aSerial->print(F(" Address=0x"));
        aSerial->print(aIRDataPtr->address, HEX);

        aSerial->print(F(" Command=0x"));
        aSerial->print(aIRDataPtr->command, HEX);

        if (aIRDataPtr->flags & IRDATA_FLAGS_EXTRA_INFO) {
            aSerial->print(F(" Extra=0x"));
            aSerial->print(aIRDataPtr->extra, HEX);
        }

        if (aIRDataPtr->flags & IRDATA_FLAGS_PARITY_FAILED) {
            aSerial->print(F(" Parity fail"));
        }

        if (aIRDataPtr->flags & IRDATA_FLAGS_TOGGLE_BIT) {
            aSerial->print(F(" Toggle=1"));
        }
#if defined(DECODE_DISTANCE_WIDTH)
        }
#endif
        if (aIRDataPtr->flags & (IRDATA_FLAGS_IS_AUTO_REPEAT | IRDATA_FLAGS_IS_REPEAT)) {
            aSerial->print(' ');
            if (aIRDataPtr->flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
                aSerial->print(F("Auto-"));
            }
            aSerial->print(F("Repeat"));
#if !defined(DISABLE_CODE_FOR_RECEIVER)
            if (aPrintRepeatGap) {
                aSerial->print(F(" gap="));
                aSerial->print((uint32_t) aIRDataPtr->rawDataPtr->rawbuf[0] * MICROS_PER_TICK);
                aSerial->print(F("us"));
            }
#else
            (void)aPrintRepeatGap;
#endif
        }

        /*
         * Print raw data
         */
        if (!(aIRDataPtr->flags & IRDATA_FLAGS_IS_REPEAT) || aIRDataPtr->decodedRawData != 0) {
            aSerial->print(F(" Raw-Data=0x"));
#if (__INT_WIDTH__ < 32)
            aSerial->print(aIRDataPtr->decodedRawData, HEX);
#else
            PrintULL::print(aSerial, aIRDataPtr->decodedRawData, HEX);
#endif
            /*
             * Print number of bits processed
             */
            aSerial->print(' ');
            aSerial->print(aIRDataPtr->numberOfBits, DEC);
            aSerial->print(F(" bits"));

            if (aIRDataPtr->flags & IRDATA_FLAGS_IS_MSB_FIRST) {
                aSerial->println(F(" MSB first"));
            } else {
                aSerial->println(F(" LSB first"));
            }

        } else {
            aSerial->println();
        }
    }
}

/**********************************************************************************************************************
 * Function to bit reverse OLD MSB values of e.g. NEC.
 **********************************************************************************************************************/
uint8_t bitreverseOneByte(uint8_t aValue) {
//    uint8_t tReversedValue;
//    return __builtin_avr_insert_bits(0x01234567, aValue, tReversedValue);
// 76543210
    aValue = (aValue >> 4) | (aValue << 4); // Swap in groups of 4
// 32107654
    aValue = ((aValue & 0xcc) >> 2) | ((aValue & 0x33) << 2); // Swap in groups of 2
// 10325476
    aValue = ((aValue & 0xaa) >> 1) | ((aValue & 0x55) << 1); // Swap bit pairs
// 01234567
    return aValue;
}

uint32_t bitreverse32Bit(uint32_t aInput) {
//    __builtin_avr_insert_bits();
    LongUnion tValue;
    tValue.UByte.HighByte = bitreverseOneByte(aInput);
    tValue.UByte.MidHighByte = bitreverseOneByte(aInput >> 8);
    tValue.UByte.MidLowByte = bitreverseOneByte(aInput >> 16);
    tValue.UByte.LowByte = bitreverseOneByte(aInput >> 24);
    return tValue.ULong;
}

/** @}*/

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_PROTOCOL_HPP
