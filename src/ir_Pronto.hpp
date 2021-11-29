/*
 * @file ir_Pronto.hpp
 * @brief In this file, the functions IRrecv::compensateAndPrintPronto and IRsend::sendPronto are defined.
 *
 * See http://www.harctoolbox.org/Glossary.html#ProntoSemantics
 * Pronto database http://www.remotecentral.com/search.htm
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020 Bengt Martensson
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
#ifndef IR_PRONTO_HPP
#define IR_PRONTO_HPP

// The first number, here 0000, denotes the type of the signal. 0000 denotes a raw IR signal with modulation,
// The second number, here 006C, denotes a frequency code
#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT"

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */

// TODO remove 6/2021
#if defined(__STM32F1__) || defined(ARDUINO_ARCH_STM32F1) // Recommended original Arduino_STM32 by Roger Clark.
#  if !defined(strncpy_P)
// this define is not included in the pgmspace.h file see https://github.com/rogerclarkmelbourne/Arduino_STM32/issues/852
#define strncpy_P(dest, src, size) strncpy((dest), (src), (size))
#  endif
#endif

//! @cond
// DO NOT EXPORT from this file
static const uint16_t learnedToken = 0x0000U;
static const uint16_t learnedNonModulatedToken = 0x0100U;
static const unsigned int bitsInHexadecimal = 4U;
static const unsigned int digitsInProntoNumber = 4U;
static const unsigned int numbersInPreamble = 4U;
static const unsigned int hexMask = 0xFU;
static const uint32_t referenceFrequency = 4145146UL;
static const uint16_t fallbackFrequency = 64767U; // To use with frequency = 0;
static const uint32_t microsecondsInSeconds = 1000000UL;
static const uint16_t PRONTO_DEFAULT_GAP = 45000;
//! @endcond

static unsigned int toFrequencyKHz(uint16_t code) {
    return ((referenceFrequency / code) + 500) / 1000;
}

/*
 * Parse the string given as Pronto Hex, and send it a number of times given as argument.
 */
void IRsend::sendPronto(const uint16_t *data, unsigned int length, uint_fast8_t aNumberOfRepeats) {
    unsigned int timebase = (microsecondsInSeconds * data[1] + referenceFrequency / 2) / referenceFrequency;
    unsigned int khz;
    switch (data[0]) {
    case learnedToken: // normal, "learned"
        khz = toFrequencyKHz(data[1]);
        break;
    case learnedNonModulatedToken: // non-demodulated, "learned"
        khz = 0U;
        break;
    default:
        return; // There are other types, but they are not handled yet.
    }
    unsigned int intros = 2 * data[2];
    unsigned int repeats = 2 * data[3];
    IR_DEBUG_PRINT(F("sendPronto intros="));
    IR_DEBUG_PRINT(intros);
    IR_DEBUG_PRINT(F(" repeats="));
    IR_DEBUG_PRINTLN(repeats);
    if (numbersInPreamble + intros + repeats != length) { // inconsistent sizes
        return;
    }

    /*
     * Generate a new microseconds timing array for sendRaw.
     * If recorded by IRremote, intro contains the whole IR data and repeat is empty
     */
    uint16_t durations[intros + repeats];
    for (unsigned int i = 0; i < intros + repeats; i++) {
        uint32_t duration = ((uint32_t) data[i + numbersInPreamble]) * timebase;
        durations[i] = (unsigned int) ((duration <= __UINT16_MAX__) ? duration : __UINT16_MAX__);
    }

    /*
     * Send the intro. intros is even.
     * Do not send the trailing space here, send it if repeats are requested
     */
    if (intros >= 2) {
        sendRaw(durations, intros - 1, khz);
    }

    if (repeats == 0 || aNumberOfRepeats == 0) {
        // only send intro once
        return;
    }

    /*
     * Now send the trailing space/gap of the intro and all the repeats
     */
    if (intros >= 2) {
        delay(durations[intros - 1] / MICROS_IN_ONE_MILLI); // equivalent to space(durations[intros - 1]); but allow bigger values for the gap
    }
    for (unsigned int i = 0; i < aNumberOfRepeats; i++) {
        sendRaw(durations + intros, repeats - 1, khz);
        if ((i + 1) < aNumberOfRepeats) { // skip last trailing space/gap, see above
            delay(durations[intros + repeats - 1] / MICROS_IN_ONE_MILLI);
        }
    }
}

/**
 * Parse the string given as Pronto Hex, and send it a number of times given
 * as the second argument. Thereby the division of the Pronto Hex into
 * an intro-sequence and a repeat sequence is taken into account:
 * First the intro sequence is sent, then the repeat sequence is sent times-1 times.
 * However, if the intro sequence is empty, the repeat sequence is sent times times.
 * <a href="http://www.harctoolbox.org/Glossary.html#ProntoSemantics">Reference</a>.
 *
 * Note: Using this function is very wasteful for the memory consumption on
 * a small board.
 * Normally it is a much better idea to use a tool like e.g. IrScrutinizer
 * to transform Pronto type signals offline
 * to a more memory efficient format.
 *
 * @param str C type string (null terminated) containing a Pronto Hex representation.
 * @param aNumberOfRepeats Number of times to send the signal.
 */
void IRsend::sendPronto(const char *str, uint_fast8_t aNumberOfRepeats) {
    size_t len = strlen(str) / (digitsInProntoNumber + 1) + 1;
    uint16_t data[len];
    const char *p = str;
    char *endptr[1];
    for (unsigned int i = 0; i < len; i++) {
        long x = strtol(p, endptr, 16);
        if (x == 0 && i >= numbersInPreamble) {
            // Alignment error?, bail immediately (often right result).
            len = i;
            break;
        }
        data[i] = static_cast<uint16_t>(x); // If input is conforming, there can be no overflow!
        p = *endptr;
    }
    sendPronto(data, len, aNumberOfRepeats);
}

#if defined(__AVR__)
/**
 * Version of sendPronto that reads from PROGMEM, saving RAM memory.
 * @param str pronto C type string (null terminated) containing a Pronto Hex representation.
 * @param aNumberOfRepeats Number of times to send the signal.
 */
//far pointer (? for ATMega2560 etc.)
void IRsend::sendPronto_PF(uint_farptr_t str, uint_fast8_t aNumberOfRepeats) {
    size_t len = strlen_PF(str);
    char work[len + 1];
    strncpy_PF(work, str, len);
    sendPronto(work, aNumberOfRepeats);
}

//standard pointer
void IRsend::sendPronto_P(const char *str, uint_fast8_t aNumberOfRepeats) {
    size_t len = strlen_P(str);
    char work[len + 1];
    strncpy_P(work, str, len);
    sendPronto(work, aNumberOfRepeats);
}
#endif

void IRsend::sendPronto(const __FlashStringHelper *str, uint_fast8_t aNumberOfRepeats) {
    size_t len = strlen_P(reinterpret_cast<const char*>(str));
    char work[len + 1];
    strncpy_P(work, reinterpret_cast<const char*>(str), len);
    return sendPronto(work, aNumberOfRepeats);
}

static uint16_t effectiveFrequency(uint16_t frequency) {
    return frequency > 0 ? frequency : fallbackFrequency;
}

static uint16_t toTimebase(uint16_t frequency) {
    return microsecondsInSeconds / effectiveFrequency(frequency);
}

static uint16_t toFrequencyCode(uint16_t frequency) {
    return referenceFrequency / effectiveFrequency(frequency);
}

static char hexDigit(unsigned int x) {
    return (char) (x <= 9 ? ('0' + x) : ('A' + (x - 10)));
}

static void dumpDigit(Print *aSerial, unsigned int number) {
    aSerial->print(hexDigit(number));
}

static void dumpNumber(Print *aSerial, uint16_t number) {
    for (unsigned int i = 0; i < digitsInProntoNumber; i++) {
        unsigned int shifts = bitsInHexadecimal * (digitsInProntoNumber - 1 - i);
        dumpDigit(aSerial, (number >> shifts) & hexMask);
    }
    aSerial->print(' ');
}

static void dumpDuration(Print *aSerial, uint32_t duration, uint16_t timebase) {
    dumpNumber(aSerial, (duration + timebase / 2) / timebase);
}

/*
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding!
 */
static void compensateAndDumpSequence(Print *aSerial, const volatile uint16_t *data, size_t length, uint16_t timebase) {
    for (size_t i = 0; i < length; i++) {
        uint32_t tDuration = data[i] * MICROS_PER_TICK;
        if (i & 1) {
            // Mark
            tDuration -= getMarkExcessMicros();
        } else {
            tDuration += getMarkExcessMicros();
        }
        dumpDuration(aSerial, tDuration, timebase);
    }

    // append a gap
    dumpDuration(aSerial, PRONTO_DEFAULT_GAP, timebase);
}

/**
 * Print the result (second argument) as Pronto Hex on the Print supplied as argument.
 * Used in the ReceiveDump example.
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 * @param aFrequencyHertz Modulation frequency in Hz. Often 38000Hz.
 */
void IRrecv::compensateAndPrintIRResultAsPronto(Print *aSerial, unsigned int aFrequencyHertz) {
    aSerial->println(F("Pronto Hex as string"));
    aSerial->print(F("char prontoData[] = \""));
    dumpNumber(aSerial, aFrequencyHertz > 0 ? learnedToken : learnedNonModulatedToken);
    dumpNumber(aSerial, toFrequencyCode(aFrequencyHertz));
    dumpNumber(aSerial, (decodedIRData.rawDataPtr->rawlen + 1) / 2);
    dumpNumber(aSerial, 0);
    unsigned int timebase = toTimebase(aFrequencyHertz);
    compensateAndDumpSequence(aSerial, &decodedIRData.rawDataPtr->rawbuf[1], decodedIRData.rawDataPtr->rawlen - 1, timebase); // skip leading space
    aSerial->println("\";");
}

/*
 * Functions for dumping Pronto to a String. This is not very time and space efficient
 * and can lead to resource problems especially on small processors like AVR's
 */

static bool dumpDigit(String *aString, unsigned int number) {
    aString->concat(hexDigit(number));
    return number;
}

static size_t dumpNumber(String *aString, uint16_t number) {

    size_t size = 0;

    for (unsigned int i = 0; i < digitsInProntoNumber; i++) {
        unsigned int shifts = bitsInHexadecimal * (digitsInProntoNumber - 1 - i);
        size += dumpDigit(aString, (number >> shifts) & hexMask);
    }
    aString->concat(' ');
    size++;

    return size;
}

/*
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding!
 */
static size_t dumpDuration(String *aString, uint32_t duration, uint16_t timebase) {
    return dumpNumber(aString, (duration + timebase / 2) / timebase);
}

static size_t compensateAndDumpSequence(String *aString, const volatile uint16_t *data, size_t length, uint16_t timebase) {

    size_t size = 0;

    for (size_t i = 0; i < length; i++) {
        uint32_t tDuration = data[i] * MICROS_PER_TICK;
        if (i & 1) {
            // Mark
            tDuration -= getMarkExcessMicros();
        } else {
            tDuration += getMarkExcessMicros();
        }
        size += dumpDuration(aString, tDuration, timebase);
    }

    // append minimum gap
    size += dumpDuration(aString, PRONTO_DEFAULT_GAP, timebase);

    return size;
}

/*
 * Writes Pronto HEX to a String object.
 * Returns the amount of characters added to the string.(360 characters for a NEC code!)
 */
size_t IRrecv::compensateAndStorePronto(String *aString, unsigned int frequency) {

    size_t size = 0;
    unsigned int timebase = toTimebase(frequency);

    size += dumpNumber(aString, frequency > 0 ? learnedToken : learnedNonModulatedToken);
    size += dumpNumber(aString, toFrequencyCode(frequency));
    size += dumpNumber(aString, (decodedIRData.rawDataPtr->rawlen + 1) / 2);
    size += dumpNumber(aString, 0);
    size += compensateAndDumpSequence(aString, &decodedIRData.rawDataPtr->rawbuf[1], decodedIRData.rawDataPtr->rawlen - 1,
            timebase); // skip leading space

    return size;
}

/** @}*/
#endif
#pragma once
