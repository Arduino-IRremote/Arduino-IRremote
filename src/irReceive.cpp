/*
 * irReceive.cpp
 *
 *  Contains common functions for receiving
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2009-2021 Ken Shirriff, Rafi Khan, Armin Joachimsmeyer
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

//#define DEBUG
#include "IRremote.h"

void IRrecv::initDecodedIRData() {
    lastDecodedCommand = decodedIRData.command;
    lastDecodedAddress = decodedIRData.address;
    decodedIRData.command = 0;
    decodedIRData.address = 0;

    decodedIRData.numberOfBits = 0;
    if (irparams.overflow) {
        decodedIRData.flags = IRDATA_FLAGS_WAS_OVERFLOW;
    } else {
        decodedIRData.flags = IRDATA_FLAGS_EMPTY;
    }
    results.value = 0;
}

//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
//
bool IRrecv::decode() {
    if (irparams.rcvstate != IR_REC_STATE_STOP) {
        return false;
    }
    if (irparams.overflow) {
        /*
         * Do resume here, since the loop will not process any IR data if we return false.
         */
        results.overflow = irparams.overflow;
        irparams.overflow = false;
        irparams.rawlen = 0;
        DBG_PRINTLN("Skip overflowed buffer");
        resume();
        return false;
    }

    /*
     * First copy 3 values from irparams to internal results structure
     */
    results.rawbuf = irparams.rawbuf;
    results.rawlen = irparams.rawlen;
    results.overflow = irparams.overflow;

    initDecodedIRData();

#if DECODE_NEC
    TRACE_PRINTLN("Attempting NEC decode");
    if (decodeNEC()) {
        return true;
    }
#endif

#if DECODE_PANASONIC
    TRACE_PRINTLN("Attempting Panasonic/Kaseikyo decode");
    if (decodeKaseikyo()) {
        return true;
    }
#endif

#if DECODE_KASEIKYO && !defined(USE_STANDARD_DECODE) // if USE_STANDARD_DECODE enabled, decodeKaseikyo() is already called by decodePanasonic()
    TRACE_PRINTLN("Attempting Panasonic/Kaseikyo decode");
    if (decodeKaseikyo()) {
        return true;
    }
#endif

#if DECODE_DENON
    TRACE_PRINTLN("Attempting Denon/Sharp decode");
    if (decodeDenon()) {
        return true;
    }
#endif

#if DECODE_SONY
    TRACE_PRINTLN("Attempting Sony decode");
    if (decodeSony()) {
        return true;
    }
#endif

#if DECODE_SHARP && ! DECODE_DENON
    TRACE_PRINTLN("Attempting Denon/Sharp decode");
    if (decodeSharp()) {
        return true;
    }
#endif

#if DECODE_RC5
    TRACE_PRINTLN("Attempting RC5 decode");
    if (decodeRC5()) {
        return true;
    }
#endif

#if DECODE_RC6
    TRACE_PRINTLN("Attempting RC6 decode");
    if (decodeRC6()) {
        return true;
    }
#endif

#if DECODE_LG
    TRACE_PRINTLN("Attempting LG decode");
    if (decodeLG()) {
        return true;
    }
#endif

#if DECODE_JVC
    TRACE_PRINTLN("Attempting JVC decode");
    if (decodeJVC()) {
        return true;
    }
#endif

#if DECODE_SAMSUNG
    TRACE_PRINTLN("Attempting Samsung decode");
    if (decodeSamsung()) {
        return true;
    }
#endif
    /*
     * Start of the exotic protocols
     */

#if DECODE_SANYO
    TRACE_PRINTLN("Attempting Sanyo decode");
    if (decodeSanyo()) {
        return true;
    }
#endif

#if DECODE_WHYNTER
    TRACE_PRINTLN("Attempting Whynter decode");
    if (decodeWhynter()) {
        return true;
    }
#endif

#if DECODE_LEGO_PF
    TRACE_PRINTLN("Attempting Lego Power Functions");
    if (decodeLegoPowerFunctions()) {
        return true;
    }
#endif

#if DECODE_BOSEWAVE
    TRACE_PRINTLN("Attempting Bosewave  decode");
    if (decodeBoseWave()) {
        return true;
    }
#endif

#if DECODE_MAGIQUEST
    TRACE_PRINTLN("Attempting MagiQuest decode");
    if (decodeMagiQuest()) {
        return true;
    }
#endif

    /*
     * Last resort is the universal hash decode which always return true
     */
#if DECODE_HASH
    TRACE_PRINTLN("Hash decode");
    // decodeHash returns a hash on any input.
    // Thus, it needs to be last in the list.
    // If you add any decodes, add them before this.
    if (decodeHash()) {
        return true;
    }
#endif

    /*
     * Not reached, if Hash is enabled!!!
     * Throw away received data and start over
     */
    resume();
    return false;
}

//+=============================================================================
IRrecv::IRrecv(int recvpin) {
    irparams.recvpin = recvpin;
    irparams.blinkflag = 0;
}

IRrecv::IRrecv(int recvpin, int blinkpin) {
    irparams.recvpin = recvpin;
    irparams.blinkpin = blinkpin;
    pinMode(blinkpin, OUTPUT);
    irparams.blinkflag = 0;
}

//+=============================================================================
// initialization
//
#ifdef USE_DEFAULT_ENABLE_IR_IN
void IRrecv::enableIRIn() {
// the interrupt Service Routine fires every 50 uS
    noInterrupts();
    // Setup pulse clock timer interrupt
    // Prescale /8 (16M/8 = 0.5 microseconds per tick)
    // Therefore, the timer interval can range from 0.5 to 128 microseconds
    // Depending on the reset value (255 to 0)
    timerConfigForReceive();

    // Timer2 Overflow Interrupt Enable
    TIMER_ENABLE_RECEIVE_INTR;

    TIMER_RESET_INTR_PENDING;

    interrupts();

    // Initialize state machine state
    irparams.rcvstate = IR_REC_STATE_IDLE;
    //    irparams.rawlen = 0; // not required

    // Set pin modes
    pinMode(irparams.recvpin, INPUT);
}

void IRrecv::disableIRIn() {
    TIMER_DISABLE_RECEIVE_INTR;
}

#endif // USE_DEFAULT_ENABLE_IR_IN

//+=============================================================================
// Enable/disable blinking of pin 13 on IR processing
//
void IRrecv::blink13(int blinkflag) {
#ifdef BLINKLED
    irparams.blinkflag = blinkflag;
    if (blinkflag) {
        pinMode(BLINKLED, OUTPUT);
    }
#endif
}

//+=============================================================================
// Return if receiving new IR signals
//
bool IRrecv::isIdle() {
    return (irparams.rcvstate == IR_REC_STATE_IDLE || irparams.rcvstate == IR_REC_STATE_STOP) ? true : false;
}

bool IRrecv::available() {
    if (irparams.rcvstate != IR_REC_STATE_STOP) {
        return false;
    }
    results.rawbuf = irparams.rawbuf;
    results.rawlen = irparams.rawlen;

    results.overflow = irparams.overflow;
    if (!results.overflow) {
        return true;
    }
    resume(); //skip overflowed buffer
    return false;
}

//+=============================================================================
// Restart the ISR state machine
//
void IRrecv::resume() {
    irparams.rcvstate = IR_REC_STATE_IDLE;
}

# if DECODE_HASH
//+=============================================================================
// hashdecode - decode an arbitrary IR code.
// Instead of decoding using a standard encoding scheme
// (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
//
// The algorithm: look at the sequence of MARK signals, and see if each one
// is shorter (0), the same length (1), or longer (2) than the previous.
// Do the same with the SPACE signals.  Hash the resulting sequence of 0's,
// 1's, and 2's to a 32-bit value.  This will give a unique value for each
// different code (probably), for most code systems.
//
// http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
//
// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20% to enable 500 and 600 (NEC timing) to be equal
//
unsigned int IRrecv::compare(unsigned int oldval, unsigned int newval) {
    if (newval * 10 < oldval * 8) {
        return 0;
    }
    if (oldval * 10 < newval * 8) {
        return 2;
    }
    return 1;
}

/*
 * Decode pulse width protocols.
 * The space (pause) has constant length, the length of the mark determines the bit value.
 *      Each bit looks like: MARK_1 + SPACE -> 1 or : MARK_0 + SPACE -> 0
 *
 * Data is read MSB first if not otherwise enabled.
 * Input is     results.rawbuf
 * Output is    results.value
 */
bool IRrecv::decodePulseWidthData(uint8_t aNumberOfBits, uint8_t aStartOffset, unsigned int aOneMarkMicros,
        unsigned int aZeroMarkMicros, unsigned int aBitSpaceMicros, bool aMSBfirst) {
    unsigned long tDecodedData = 0;

    if (aMSBfirst) {
        for (uint8_t i = 0; i < aNumberOfBits; i++) {
            // Check for variable length mark indicating a 0 or 1
            if (MATCH_MARK(results.rawbuf[aStartOffset], aOneMarkMicros)) {
                tDecodedData = (tDecodedData << 1) | 1;
                TRACE_PRINT('1');
            } else if (MATCH_MARK(results.rawbuf[aStartOffset], aZeroMarkMicros)) {
                tDecodedData = (tDecodedData << 1) | 0;
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(results.rawbuf[aStartOffset] * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneMarkMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            aStartOffset++;

            if (aStartOffset < results.rawlen) {
                // Assume that last space, which is not recorded, is correct
                // Check for constant length space
                if (!MATCH_SPACE(results.rawbuf[aStartOffset], aBitSpaceMicros)) {
                    DBG_PRINT(F("Space="));
                    DBG_PRINT(results.rawbuf[aStartOffset] * MICROS_PER_TICK);
                    DBG_PRINT(F(" is not "));
                    DBG_PRINT(aBitSpaceMicros);
                    DBG_PRINT(' ');
                    return false;
                }
                aStartOffset++;
            }
        }
        TRACE_PRINTLN("");
    } else {
        for (unsigned long mask = 1UL; aNumberOfBits > 0; mask <<= 1, aNumberOfBits--) {

            // Check for variable length mark indicating a 0 or 1
            if (MATCH_MARK(results.rawbuf[aStartOffset], aOneMarkMicros)) {
                tDecodedData |= mask; // set the bit
                TRACE_PRINT('1');
            } else if (MATCH_MARK(results.rawbuf[aStartOffset], aZeroMarkMicros)) {
                // do not set the bit
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(results.rawbuf[aStartOffset] * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneMarkMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            aStartOffset++;

            if (aStartOffset < results.rawlen) {
                // Assume that last space, which is not recorded, is correct
                // Check for constant length space
                if (!MATCH_SPACE(results.rawbuf[aStartOffset], aBitSpaceMicros)) {
                    DBG_PRINT(F("Space="));
                    DBG_PRINT(results.rawbuf[aStartOffset] * MICROS_PER_TICK);
                    DBG_PRINT(F(" is not "));
                    DBG_PRINT(aBitSpaceMicros);
                    DBG_PRINT(' ');
                    return false;
                }
                aStartOffset++;
            }
        }
        TRACE_PRINTLN("");
    }
    results.value = tDecodedData;
    return true;
}

/*
 * Decode pulse distance protocols.
 * The mark (pulse) has constant length, the length of the space determines the bit value.
 * Each bit looks like: MARK + SPACE_1 -> 1
 *                 or : MARK + SPACE_0 -> 0
 * Data is read MSB first if not otherwise enabled.
 * Input is     results.rawbuf
 * Output is    results.value
 * @return false if decoding failed
 */
bool IRrecv::decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, unsigned int aBitMarkMicros,
        unsigned int aOneSpaceMicros, unsigned int aZeroSpaceMicros, bool aMSBfirst) {
    unsigned long tDecodedData = 0;
    if (aMSBfirst) {
        for (uint8_t i = 0; i < aNumberOfBits; i++) {
            // Check for constant length mark
            if (!MATCH_MARK(results.rawbuf[aStartOffset], aBitMarkMicros)) {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(results.rawbuf[aStartOffset] * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBitMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            aStartOffset++;

            // Check for variable length space indicating a 0 or 1
            if (MATCH_SPACE(results.rawbuf[aStartOffset], aOneSpaceMicros)) {
                tDecodedData = (tDecodedData << 1) | 1;
                TRACE_PRINT('1');
            } else if (MATCH_SPACE(results.rawbuf[aStartOffset], aZeroSpaceMicros)) {
                tDecodedData = (tDecodedData << 1) | 0;
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Space="));
                DBG_PRINT(results.rawbuf[aStartOffset] * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneSpaceMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroSpaceMicros);
                DBG_PRINT(' ');
                return false;
            }
            aStartOffset++;
        }
        TRACE_PRINTLN("");

    } else {
        for (unsigned long mask = 1UL; aNumberOfBits > 0; mask <<= 1, aNumberOfBits--) {
            // Check for constant length mark
            if (!MATCH_MARK(results.rawbuf[aStartOffset], aBitMarkMicros)) {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(results.rawbuf[aStartOffset]*MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBitMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            aStartOffset++;

            // Check for variable length space indicating a 0 or 1
            if (MATCH_SPACE(results.rawbuf[aStartOffset], aOneSpaceMicros)) {
                tDecodedData |= mask; // set the bit
                TRACE_PRINT('1');
            } else if (MATCH_SPACE(results.rawbuf[aStartOffset], aZeroSpaceMicros)) {
                // do not set the bit
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Space="));
                DBG_PRINT(results.rawbuf[aStartOffset]*MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneSpaceMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroSpaceMicros);
                DBG_PRINT(' ');
                return false;
            }
            aStartOffset++;
        }
        TRACE_PRINTLN("");
    }
    results.value = tDecodedData;
    return true;
}

//+=============================================================================
// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
// Converts the raw code values into a 32-bit hash code.
// Hopefully this code is unique for each button.
// This isn't a "real" decoding, just an arbitrary value.
//
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

bool IRrecv::decodeHash() {
    long hash = FNV_BASIS_32;

// Require at least 6 samples to prevent triggering on noise
    if (results.rawlen < 6) {
        return false;
    }

    for (unsigned int i = 1; (i + 2) < results.rawlen; i++) {
        unsigned int value = compare(results.rawbuf[i], results.rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    results.value = hash;
    results.bits = 32;
    decodedIRData.protocol = UNKNOWN;

    return true;
}

bool IRrecv::decodeHash(decode_results *aResults) {
    bool aReturnValue = decodeHash();
    *aResults = results;
    return aReturnValue;
}

#endif // defined(DECODE_HASH)

const char* IRrecv::getProtocolString() {
    switch (decodedIRData.protocol) {
    default:
    case UNKNOWN:
        return ("UNKNOWN");
        break;
#if DECODE_BOSEWAVE
    case BOSEWAVE:
        return ("BOSEWAVE");
        break;
#endif
#if DECODE_DENON
    case DENON:
        return ("DENON");
        break;
#endif
#if DECODE_SHARP
    case SHARP:
        return ("SHARP");
        break;
#endif
#if DECODE_JVC
    case JVC:
        return ("JVC");
        break;
#endif
#if DECODE_LEGO_PF
    case LEGO_PF:
        return ("LEGO_PF");
        break;
#endif
#if DECODE_LG
    case LG:
        return ("LG");
        break;
#endif
#if DECODE_MAGIQUEST
    case MAGIQUEST:
        return ("MAGIQUEST");
        break;
#endif
#if DECODE_NEC
    case NEC:
        return ("NEC");
        break;
#endif
#if DECODE_PANASONIC
    case PANASONIC:
        return ("PANASONIC");
        break;
#endif
#if DECODE_KASEIKYO
    case KASEIKYO:
        return ("KASEIKYO");
        break;
    case KASEIKYO_DENON:
        return ("KASEIKYO_DENON");
        break;
    case KASEIKYO_SHARP:
        return ("KASEIKYO_SHARP");
        break;
    case KASEIKYO_JVC:
        return ("KASEIKYO_JVC");
        break;
    case KASEIKYO_MITSUBISHI:
        return ("KASEIKYO_MITSUBISHI");
        break;
#endif
#if DECODE_RC5
    case RC5:
        return ("RC5");
        break;
#endif
#if DECODE_RC6
    case RC6:
        return ("RC6");
        break;
#endif
#if DECODE_SAMSUNG
    case SAMSUNG:
        return ("SAMSUNG");
        break;
#endif
#if DECODE_SANYO
    case SANYO:
        return ("SANYO");
        break;
#endif

#if DECODE_SONY
    case SONY:
        return ("SONY");
        break;
#endif
#if DECODE_WHYNTER
    case WHYNTER:
        return ("WHYNTER");
        break;
#endif
    }
}

void IRrecv::printResultShort(Print *aSerial) {
    aSerial->print(F("Protocol="));
    aSerial->print(getProtocolString());
    if (decodedIRData.protocol == UNKNOWN) {
        aSerial->print(' ');
        aSerial->print((results.rawlen + 1) / 2, DEC);
        aSerial->println(F(" bits received"));
    } else {

        if (!(decodedIRData.flags & IRDATA_FLAGS_IS_OLD_DECODER)) {
            /*
             * New decoders have address and command
             */
            aSerial->print(F(" Address=0x"));
            aSerial->print(decodedIRData.address, HEX);

            aSerial->print(F(" Command=0x"));
            aSerial->print(decodedIRData.command, HEX);

            if (decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
                aSerial->print(F(" Parity fail"));
            }

            if (decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
                aSerial->print(F(" Auto-repeat gap="));
                aSerial->print(results.rawbuf[0] * MICROS_PER_TICK);
                aSerial->print(F("us"));
            }

            if (decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
                aSerial->print(F(" Repeat gap="));
                aSerial->print((uint32_t) results.rawbuf[0] * MICROS_PER_TICK);
                aSerial->print(F("us"));
            }
        } else {
            // assume that we have a repeat if the gap is below 200 ms
            if (results.rawbuf[0] < (200000 / MICROS_PER_TICK)) {
                aSerial->print(F(" Repeat gap="));
                aSerial->print((uint32_t) results.rawbuf[0] * MICROS_PER_TICK);
                aSerial->print(F("us"));
            }
        }
        /*
         * Print raw data
         */
        aSerial->print(F(" Raw-Data=0x"));
        aSerial->print(results.value, HEX);

        /*
         * Print number of bits processed
         */
        aSerial->print(F(" ("));
        if (!(decodedIRData.flags & IRDATA_FLAGS_IS_OLD_DECODER)) {
            // New decoder
            aSerial->print(decodedIRData.numberOfBits, DEC);
        } else {
            // Old decoder
            aSerial->print(results.bits, DEC);
        }
        aSerial->println(F(" bits)"));
    }
}

/*
 * Print a c rawData array for later use in sendRaw()
 */
void IRrecv::printIRResultRaw(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {
    // Dumps out the decode_results structure.
    // Call this after IRrecv::decode()
    aSerial->print(F("rawData["));

#if VERSION_IRREMOTE_MAJOR > 2
    aSerial->print(results.rawlen - 1, DEC);
    aSerial->print(F("]: "));
    for (unsigned int i = 1; i < results.rawlen; i++) {
#else
    /*
     * The leading space is required for repeat detection but not for sending raw data
     */
    unsigned int i;
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->print(results.rawlen, DEC);
        i = 0; // We print the leading space to enable backwards compatibility.
    } else {
        aSerial->print(results.rawlen - 1, DEC);
        i = 1; // Skip the leading space.
    }
    aSerial->print(F("]: "));
    for (; i < results.rawlen; i++) {
#endif
        uint32_t tDurationMicros;
        if (aOutputMicrosecondsInsteadOfTicks) {
            tDurationMicros = results.rawbuf[i] * (uint32_t) MICROS_PER_TICK;
        } else {
            tDurationMicros = results.rawbuf[i];
        }
        if (i & 1) {
            aSerial->print(tDurationMicros, DEC);
        } else {
            aSerial->write('-');
            aSerial->print(tDurationMicros, DEC);
        }
        aSerial->print(' ');
    }
    aSerial->println();
}

//+=============================================================================
// Dump out the decode_results structure
//
void IRrecv::printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {
    // Print Raw data
    aSerial->print(F("rawData["));
    aSerial->print(results.rawlen, DEC);
    aSerial->println(F("]: "));

    uint32_t tDurationMicros;

    /*
     * Print initial gap
     */
    if (aOutputMicrosecondsInsteadOfTicks) {
        tDurationMicros = (uint32_t) results.rawbuf[0] * MICROS_PER_TICK;
    } else {
        tDurationMicros = results.rawbuf[0];
    }
    aSerial->print(F("     -"));
    aSerial->println(tDurationMicros, DEC);

    for (unsigned int i = 1; i < results.rawlen; i++) {
        if (aOutputMicrosecondsInsteadOfTicks) {
            tDurationMicros = results.rawbuf[i] * MICROS_PER_TICK;
        } else {
            tDurationMicros = results.rawbuf[i];
        }
        if (!(i & 1)) {  // even
            aSerial->print('-');
            if (tDurationMicros < 1000) {
                aSerial->print(' ');
            }
            if (tDurationMicros < 100) {
                aSerial->print(' ');
            }
            if (tDurationMicros < 10) {
                aSerial->print(' ');
            }
            aSerial->print(tDurationMicros, DEC);
        } else {  // odd
            aSerial->print(F("     "));
            aSerial->print('+');
            if (tDurationMicros < 1000) {
                aSerial->print(' ');
            }
            if (tDurationMicros < 100) {
                aSerial->print(' ');
            }
            if (tDurationMicros < 10) {
                aSerial->print(' ');
            }
            aSerial->print(tDurationMicros, DEC);
            if (i + 1 < results.rawlen) {
                aSerial->print(','); //',' not required for last one
            }
        }
        if (!(i % 8)) {
            aSerial->println("");
        }
    }
    aSerial->println("");                    // Newline
}

/*
 * Dump out the decode_results structure.
 * 255*50microseconds = 12750microseconds = 12.75 ms, which "hardly ever" occurs inside an Ir sequence.
 * Note that 930 is the final silence. Many systems, including Lirc and IrRemote, just ignore the final gap.
 * However, you have to take care if repeating the signal, for example your NEC2 signal (which repeats every 114ms).
 */
void IRrecv::printIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {
    // Start declaration
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->print(F("uint16_t "));            // variable type
        aSerial->print(F("rawData["));             // array name
    } else {
        aSerial->print(F("uint8_t "));             // variable type
        aSerial->print(F("rawTicks["));            // array name
    }

#if VERSION_IRREMOTE_MAJOR > 2
    aSerial->print(results.rawlen - 1, DEC);    // array size
#else
    /*
     * The leading space is required for repeat detection but not for sending raw data
     * We print the leading space to enable backwards compatibility.
     */
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->print(results.rawlen, DEC);    // array size
    } else {
        aSerial->print(results.rawlen - 1, DEC);    // array size without leading space
    }
#endif
    aSerial->print(F("] = {"));                    // Start declaration

// Dump data
#if VERSION_IRREMOTE_MAJOR > 2
    for (unsigned int i = 1; i < results.rawlen; i++) {
#else
    /*
     * We print the leading space to enable backwards compatibility.
     * It is only required for the Sanyo and Sony hack of decoding of repeats, which is incompatible to other protocols!
     */
    unsigned int i;
    if (aOutputMicrosecondsInsteadOfTicks) {
        i = 0; // We print the leading space to enable backwards compatibility.
    } else {
        i = 1; // Skip the leading space.
    }
    for (; i < results.rawlen; i++) {
#endif
        if (aOutputMicrosecondsInsteadOfTicks) {
            aSerial->print(results.rawbuf[i] * MICROS_PER_TICK, DEC);
        } else {
            aSerial->print(results.rawbuf[i]);
        }
        if (i + 1 < results.rawlen)
            aSerial->print(',');                // ',' not required on last one
        if (!(i & 1))
            aSerial->print(' ');
    }

// End declaration
    aSerial->print(F("};")); //

// Comment
    aSerial->print(F("  // "));
    printResultShort(aSerial);

// Newline
    aSerial->println("");
}

void IRrecv::printIRResultAsCVariables(Print *aSerial) {
// Now dump "known" codes
    if (decodedIRData.protocol != UNKNOWN) {

        if (!(decodedIRData.flags & IRDATA_FLAGS_IS_OLD_DECODER)) {
            /*
             * New decoders have address and command
             */
            if (decodedIRData.address > 0xFFFF) {
                aSerial->print(F("uint32_t"));
            } else {
                aSerial->print(F("uint16_t"));
            }
            aSerial->print(F(" address = 0x"));
            aSerial->print(decodedIRData.address, HEX);
            aSerial->println(';');

            if (decodedIRData.command > 0xFFFF) {
                aSerial->print(F("uint32_t"));
            } else {
                aSerial->print(F("uint16_t"));
            }
            aSerial->print(F(" command = 0x"));
            aSerial->print(decodedIRData.command, HEX);
            aSerial->println(';');

        }

        // All protocols have data
        aSerial->print(F("uint32_t data = 0x"));
        aSerial->print(results.value, HEX);
        aSerial->println(';');
        aSerial->println();
    }
}

/*
 * DEPRECATED
 * With parameter aResults for backwards compatibility
 * Contains no new (since 5/2020) protocols.
 */
bool IRrecv::decode(decode_results *aResults) {
    if (irparams.rcvstate != IR_REC_STATE_STOP) {
        return false;
    }

    Serial.println(F("Use of decode(decode_results *aResults) is deprecated! Use decode() instead!"));
    /*
     * First copy 3 values from irparams to internal results structure
     */
    results.rawbuf = irparams.rawbuf;
    results.rawlen = irparams.rawlen;
    results.overflow = irparams.overflow;

    initDecodedIRData();

#if DECODE_NEC
    DBG_PRINTLN("Attempting NEC decode");
    if (decodeNEC(aResults)) {
        return true;
    }
#endif

#if DECODE_SONY
    DBG_PRINTLN("Attempting Sony decode");
    if (decodeSony(aResults)) {
        return true;
    }
#endif

#if DECODE_SANYO
    DBG_PRINTLN("Attempting Sanyo decode");
    if (decodeSanyo(aResults)) {
        return true;
    }
#endif

//#if DECODE_MITSUBISHI
//    DBG_PRINTLN("Attempting Mitsubishi decode");
//    if (decodeMitsubishi(aResults)) {
//        return true;
//    }
//#endif

#if DECODE_RC5
    DBG_PRINTLN("Attempting RC5 decode");
    if (decodeRC5(aResults)) {
        return true;
    }
#endif

#if DECODE_RC6
    DBG_PRINTLN("Attempting RC6 decode");
    if (decodeRC6(aResults)) {
        return true;
    }
#endif

#if DECODE_PANASONIC
    DBG_PRINTLN("Attempting Panasonic decode");
    if (decodePanasonic(aResults)) {
        return true;
    }
#endif

#if DECODE_LG
    DBG_PRINTLN("Attempting LG decode");
    if (decodeLG(aResults)) {
        return true;
    }
#endif

#if DECODE_JVC
    DBG_PRINTLN("Attempting JVC decode");
    if (decodeJVC(aResults)) {
        return true;
    }
#endif

#if DECODE_SAMSUNG
    DBG_PRINTLN("Attempting SAMSUNG decode");
    if (decodeSAMSUNG(aResults)) {
        return true;
    }
#endif

#if DECODE_WHYNTER
    DBG_PRINTLN("Attempting Whynter decode");
    if (decodeWhynter(aResults)) {
        return true;
    }
#endif

//#if DECODE_AIWA_RC_T501
//    DBG_PRINTLN("Attempting Aiwa RC-T501 decode");
//    if (decodeAiwaRCT501(aResults)) {
//        return true;
//    }
//#endif

#if DECODE_DENON
    DBG_PRINTLN("Attempting Denon decode");
    if (decodeDenon(aResults)) {
        return true;
    }
#endif

#if defined(DECODE_HASH)
    DBG_PRINTLN("Hash decode");
// decodeHash returns a hash on any input.
// Thus, it needs to be last in the list.
// If you add any decodes, add them before this.
    if (decodeHash(aResults)) {
        return true;
    }
#endif

// Throw away and start over
    resume();
    return false;
}
