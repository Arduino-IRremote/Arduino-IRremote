/**
 * @file irPronto.cpp
 * @brief In this file, the functions IRrecv::dumpPronto and
 * IRsend::sendPronto are defined.
 * See http://www.harctoolbox.org/Glossary.html#ProntoSemantics
 */

#include "IRremote.h"

// DO NOT EXPORT from this file
static const uint16_t MICROSECONDS_T_MAX = 0xFFFFU;
static const uint16_t learnedToken = 0x0000U;
static const uint16_t learnedNonModulatedToken = 0x0100U;
static const unsigned int bitsInHexadecimal = 4U;
static const unsigned int digitsInProntoNumber = 4U;
static const unsigned int numbersInPreamble = 4U;
static const unsigned int hexMask = 0xFU;
static const uint32_t referenceFrequency = 4145146UL;
static const uint16_t fallbackFrequency = 64767U; // To use with frequency = 0;
static const uint32_t microsecondsInSeconds = 1000000UL;
static const unsigned int RESULT_JUNK_COUNT = 1U;

static unsigned int toFrequencyKHz(uint16_t code) {
    return ((referenceFrequency / code) + 500) / 1000;
}

/*
 * Parse the string given as Pronto Hex, and send it a number of times given as argument.
 */
void IRsend::sendPronto(const uint16_t *data, unsigned int size, unsigned int times) {
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
    if (numbersInPreamble + intros + repeats != size) { // inconsistent sizes
        return;
    }

    uint16_t durations[intros + repeats];
    for (unsigned int i = 0; i < intros + repeats; i++) {
        uint32_t duration = ((uint32_t) data[i + numbersInPreamble]) * timebase;
        durations[i] = (unsigned int) ((duration <= MICROSECONDS_T_MAX) ? duration : MICROSECONDS_T_MAX);
    }

    unsigned int numberRepeats = intros > 0 ? times - 1 : times;

    /*
     * Send the intro. intros is even.
     * Do not send the trailing space here, send it if repeats are requested
     */
    if (intros >= 2) {
        sendRaw(durations, intros - 1, khz);
    }

    if (repeats == 0 || numberRepeats == 0) {
        return;
    }

    /*
     * Now send the trailing space/gap of the intro and all the repeats
     */
    delay(durations[intros - 1] / 1000U); // equivalent to space(durations[intros - 1]); but allow bigger values for the gap
    for (unsigned int i = 0; i < numberRepeats; i++) {
        sendRaw(&durations[0] + intros, repeats - 1, khz);
        if (i < numberRepeats - 1) { // skip last trailing space/gap, see above
            delay(durations[intros + repeats - 1] / 1000U);
        }
    }
}

void IRsend::sendPronto(const char *str, unsigned int times) {
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
    sendPronto(data, len, times);
}

#if HAS_FLASH_READ
void IRsend::sendPronto_PF(uint_farptr_t str, unsigned int times) {
    size_t len = strlen_PF(STRCPY_PF_CAST(str));
    char work[len + 1];
    strncpy_PF(work, STRCPY_PF_CAST(str), len);
    sendPronto(work, times);
}

void IRsend::sendPronto_PF(const char *str, unsigned int times) {
    sendPronto_PF(reinterpret_cast<uint_farptr_t>(str), times); // to avoid infinite recursion
}

void IRsend::sendPronto(const __FlashStringHelper *str, unsigned int times) {
    return sendPronto_PF(reinterpret_cast<uint_farptr_t>(str), times);
}
#endif

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

static bool dumpDigit(String *aString, unsigned int number) {
    return aString->concat(hexDigit(number));
}

static void dumpNumber(Print *aSerial, uint16_t number) {
    for (unsigned int i = 0; i < digitsInProntoNumber; i++) {
        unsigned int shifts = bitsInHexadecimal * (digitsInProntoNumber - 1 - i);
        dumpDigit(aSerial, (number >> shifts) & hexMask);
    }
    aSerial->print(' ');
}

static size_t dumpNumber(String *aString, uint16_t number) {

    size_t size = 0;

    for (unsigned int i = 0; i < digitsInProntoNumber; i++) {
        unsigned int shifts = bitsInHexadecimal * (digitsInProntoNumber - 1 - i);
        size += dumpDigit(aString, (number >> shifts) & hexMask);
    }
    size += aString->concat(' ');

    return size;
}

static void dumpDuration(Print *aSerial, uint16_t duration, uint16_t timebase) {
    dumpNumber(aSerial, (duration * MICROS_PER_TICK + timebase / 2) / timebase);
}

static size_t dumpDuration(String *aString, uint16_t duration, uint16_t timebase) {
    return dumpNumber(aString, (duration * MICROS_PER_TICK + timebase / 2) / timebase);
}

static void dumpSequence(Print *aSerial, const volatile uint16_t *data, size_t length, uint16_t timebase) {
    for (unsigned int i = 0; i < length; i++)
        dumpDuration(aSerial, data[i], timebase);

    dumpDuration(aSerial, _GAP, timebase);
}

static size_t dumpSequence(String *aString, const volatile uint16_t *data, size_t length, uint16_t timebase) {

    size_t size = 0;

    for (unsigned int i = 0; i < length; i++)
        size += dumpDuration(aString, data[i], timebase);
    
    size += dumpDuration(aString, _GAP, timebase);

    return size;
}

/*
 * Using Print instead of Stream saves 1020 bytes program memory
 * Changed from & to * parameter type to be more transparent and consistent with other code of IRremote
 */
void IRrecv::dumpPronto(Print *aSerial, unsigned int frequency) {
    dumpNumber(aSerial, frequency > 0 ? learnedToken : learnedNonModulatedToken);
    dumpNumber(aSerial, toFrequencyCode(frequency));
    dumpNumber(aSerial, (results.rawlen + 1) / 2);
    dumpNumber(aSerial, 0);
    unsigned int timebase = toTimebase(frequency);
    dumpSequence(aSerial, results.rawbuf + RESULT_JUNK_COUNT, results.rawlen - RESULT_JUNK_COUNT, timebase);
}


/*
 * Writes Pronto HEX to a String object.
 * Returns the amount of characters added to the string.
 */

size_t IRrecv::dumpPronto(String *aString, unsigned int frequency) {

    size_t size = 0;
    unsigned int timebase = toTimebase(frequency);

    size += dumpNumber(aString, frequency > 0 ? learnedToken : learnedNonModulatedToken);
    size += dumpNumber(aString, toFrequencyCode(frequency));
    size += dumpNumber(aString, (results.rawlen + 1) / 2);
    size += dumpNumber(aString, 0);
    size += dumpSequence(aString, results.rawbuf + RESULT_JUNK_COUNT, results.rawlen - RESULT_JUNK_COUNT, timebase);

    return size;
}

//+=============================================================================
// Dump out the raw data as Pronto Hex.
// I know Stream * is locally inconsistent, but all global print functions use it
//
void IRrecv::printIRResultAsPronto(Print *aSerial, unsigned int frequency) {
    aSerial->println("Pronto Hex as string");
    aSerial->print("char ProntoData[] = \"");
    dumpPronto(aSerial, frequency);
    aSerial->println("\"");
}
