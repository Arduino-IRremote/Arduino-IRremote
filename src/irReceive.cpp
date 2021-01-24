/*
 * irReceive.cpp
 *
 *  Contains all IRrecv class functions
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

// The receiver instance
IRrecv IrReceiver;

//+=============================================================================
/**
 * Instantiate the IRrecv class. Multiple instantiation is not supported.
 * @param recvpin Arduino pin to use. No sanity check is made.
 */
IRrecv::IRrecv() {
    irparams.recvpin = 0;
    irparams.blinkflag = false;
}

IRrecv::IRrecv(int recvpin) {
    irparams.recvpin = recvpin;
    irparams.blinkflag = false;
}
/**
 * Instantiate the IRrecv class. Multiple instantiation is not supported.
 * @param recvpin Arduino pin to use, where a demodulating IR receiver is connected.
 * @param blinkpin pin to blink when receiving IR. Not supported by all hardware. No sanity check is made.
 */
IRrecv::IRrecv(int recvpin, int blinkpin) {
    irparams.recvpin = recvpin;
    irparams.blinkpin = blinkpin;
    pinMode(blinkpin, OUTPUT);
    irparams.blinkflag = false;
}

//+=============================================================================
// Stream like API
/*
 * @ param aBlinkPin if 0, then take board BLINKLED_ON() and BLINKLED_OFF() functions
 */
void IRrecv::begin(uint8_t aReceivePin, bool aEnableLEDFeedback, uint8_t aLEDFeedbackPin) {

    irparams.recvpin = aReceivePin;
    irparams.blinkflag = aEnableLEDFeedback;
    irparams.blinkpin = aLEDFeedbackPin; // default is 0

    if (aEnableLEDFeedback) {
        if (irparams.blinkpin != 0) {
            pinMode(irparams.blinkpin, OUTPUT);
#ifdef BLINKLED
        } else {
            pinMode(BLINKLED, OUTPUT);
#endif
        }
    }
    enableIRIn();
}

void IRrecv::start() {
    enableIRIn();
}

void IRrecv::stop() {
    disableIRIn();
}
void IRrecv::end() {
    stop();
    irparams.blinkflag = true;
}

//+=============================================================================
// initialization
//
#ifdef USE_DEFAULT_ENABLE_IR_IN
/**
 * Enable IR reception.
 */
void IRrecv::enableIRIn() {

    noInterrupts();

    // Setup pulse clock timer interrupt
    timerConfigForReceive();
    TIMER_ENABLE_RECEIVE_INTR;  // Timer interrupt enable
    TIMER_RESET_INTR_PENDING;   // NOP for most platforms

    interrupts();

    // Initialize state machine state
    resume();

    // Set pin modes
    pinMode(irparams.recvpin, INPUT);
}

/**
 * Disable IR reception.
 */
void IRrecv::disableIRIn() {
    TIMER_DISABLE_RECEIVE_INTR;
}
#endif // USE_DEFAULT_ENABLE_IR_IN

//+=============================================================================
// Enable/disable blinking of pin 13 on IR processing
//
void IRrecv::blink13(bool aEnableLEDFeedback) {
    irparams.blinkflag = aEnableLEDFeedback;
    if (aEnableLEDFeedback) {
        if (irparams.blinkpin != 0) {
            pinMode(irparams.blinkpin, OUTPUT);
#ifdef BLINKLED
        } else {
            pinMode(BLINKLED, OUTPUT);
#endif
        }
    }
}
void IRrecv::setBlinkPin(uint8_t aBlinkPin) {
    irparams.blinkpin = aBlinkPin;
    pinMode(aBlinkPin, OUTPUT);
}

//+=============================================================================
/**
 * Returns status of reception
 * @return true if no reception is on-going.
 */
bool IRrecv::isIdle() {
    return (irparams.rcvstate == IR_REC_STATE_IDLE || irparams.rcvstate == IR_REC_STATE_STOP) ? true : false;
}

//+=============================================================================
/**
 * Restart the ISR state machine
 * Enable receiving of the next value
 */
void IRrecv::resume() {
    // check allows to call resume at arbitrary places or more than once
    if (irparams.rcvstate == IR_REC_STATE_STOP) {
        irparams.rcvstate = IR_REC_STATE_IDLE;
    }
}

/*
 * Is internally called by decode before calling decoders.
 * Must be used to setup data, if you call decoders manually.
 */
void IRrecv::initDecodedIRData() {

    decodedIRData.rawDataPtr = &irparams;

    if (irparams.overflow) {
        irparams.overflow = false;
        irparams.rawlen = 0; // otherwise we have overflow again at next ISR call
        decodedIRData.flags = IRDATA_FLAGS_WAS_OVERFLOW;
        DBG_PRINTLN("Overflow happened");

    } else {
        decodedIRData.flags = IRDATA_FLAGS_EMPTY;
        // we have no new data so do not need to save old ones
        lastDecodedCommand = decodedIRData.command;
        lastDecodedAddress = decodedIRData.address;

    }
    decodedIRData.command = 0;
    decodedIRData.address = 0;
    decodedIRData.decodedRawData = 0;
    decodedIRData.numberOfBits = 0;
}

/**
 * Returns status of reception and copies IR-data to decode_results buffer if true.
 * @return true if data is available.
 */
bool IRrecv::available() {
    return (irparams.rcvstate == IR_REC_STATE_STOP);
}

/**
 *@return decoded IRData,
 */
IRData* IRrecv::read() {
    if (irparams.rcvstate != IR_REC_STATE_STOP) {
        return NULL;
    }
    if (decode()) {
        return &decodedIRData;
    } else {
        return NULL;
    }
}

//+=============================================================================
/**
 * Attempt to decode the recently receive IR signal
 * @param results decode_results instance returning the decode, if any.
 * @return 0 if no data ready, 1 if data ready. Results of decoding are stored in results
 */
bool IRrecv::decode() {
    if (irparams.rcvstate != IR_REC_STATE_STOP) {
        return false;
    }

    /*
     * First copy 3 values from irparams for legacy compatibility
     */
    results.rawbuf = irparams.rawbuf;
    results.rawlen = irparams.rawlen;
    results.overflow = irparams.overflow;

    initDecodedIRData(); // sets IRDATA_FLAGS_WAS_OVERFLOW

    if (decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
        /*
         * Set overflow flag and return true here, to let the loop call resume or print raw data.
         */
        decodedIRData.protocol = UNKNOWN;
        return true;
    }

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
#if defined(USE_STANDARD_DECODE)
    if (decodeSamsung()) {
        return true;
    }
#else
        if (decodeSAMSUNG()) {
            return true;
        }
#endif
#endif
    /*
     * Start of the exotic protocols
     */

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

// to be removed!
//#if DECODE_SANYO
//    TRACE_PRINTLN("Attempting Sanyo decode");
//    if (decodeSanyo()) {
//        return true;
//    }
//#endif

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
     * Not reached, if Hash is enabled and rawlen >= 6!!!
     * Throw away received data and start over
     */
    resume();
    return false;
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

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    uint32_t tDecodedData = 0;

    if (aMSBfirst) {
        for (uint8_t i = 0; i < aNumberOfBits; i++) {
            // Check for variable length mark indicating a 0 or 1
            if (MATCH_MARK(*tRawBufPointer, aOneMarkMicros)) {
                tDecodedData = (tDecodedData << 1) | 1;
                TRACE_PRINT('1');
            } else if (MATCH_MARK(*tRawBufPointer, aZeroMarkMicros)) {
                tDecodedData = (tDecodedData << 1) | 0;
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneMarkMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            if (tRawBufPointer < &decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen]) {
                // Assume that last space, which is not recorded, is correct, since we can not check it
                // Check for constant length space
                if (!MATCH_SPACE(*tRawBufPointer, aBitSpaceMicros)) {
                    DBG_PRINT(F("Space="));
                    DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                    DBG_PRINT(F(" is not "));
                    DBG_PRINT(aBitSpaceMicros);
                    DBG_PRINT(' ');
                    return false;
                }
                tRawBufPointer++;
            }
        }
        TRACE_PRINTLN("");
    } else {
        for (uint32_t tMask = 1UL; aNumberOfBits > 0; tMask <<= 1, aNumberOfBits--) {

            // Check for variable length mark indicating a 0 or 1
            if (MATCH_MARK(*tRawBufPointer, aOneMarkMicros)) {
                tDecodedData |= tMask; // set the bit
                TRACE_PRINT('1');
            } else if (MATCH_MARK(*tRawBufPointer, aZeroMarkMicros)) {
                // do not set the bit
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneMarkMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            if (tRawBufPointer < &decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen]) {
                // Assume that last space, which is not recorded, is correct, since we can not check it
                // Check for constant length space
                if (!MATCH_SPACE(*tRawBufPointer, aBitSpaceMicros)) {
                    DBG_PRINT(F("Space="));
                    DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                    DBG_PRINT(F(" is not "));
                    DBG_PRINT(aBitSpaceMicros);
                    DBG_PRINT(' ');
                    return false;
                }
                tRawBufPointer++;
            }
        }
        TRACE_PRINTLN("");
    }
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

/*
 * Decode pulse distance protocols.
 * The mark (pulse) has constant length, the length of the space determines the bit value.
 * Each bit looks like: MARK + SPACE_1 -> 1
 *                 or : MARK + SPACE_0 -> 0
 * @param aStartOffset must point to a mark
 *
 * Input is     results.rawbuf
 * Output is    results.value
 * @return false if decoding failed
 */
bool IRrecv::decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, unsigned int aBitMarkMicros,
        unsigned int aOneSpaceMicros, unsigned int aZeroSpaceMicros, bool aMSBfirst) {

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    uint32_t tDecodedData = 0;

    if (aMSBfirst) {
        for (uint8_t i = 0; i < aNumberOfBits; i++) {
            // Check for constant length mark
            if (!MATCH_MARK(*tRawBufPointer, aBitMarkMicros)) {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBitMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            // Check for variable length space indicating a 0 or 1
            if (MATCH_SPACE(*tRawBufPointer, aOneSpaceMicros)) {
                tDecodedData = (tDecodedData << 1) | 1;
                TRACE_PRINT('1');
            } else if (MATCH_SPACE(*tRawBufPointer, aZeroSpaceMicros)) {
                tDecodedData = (tDecodedData << 1) | 0;
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Space="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneSpaceMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroSpaceMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;
        }
        TRACE_PRINTLN("");

    } else {
        for (uint32_t tMask = 1UL; aNumberOfBits > 0; tMask <<= 1, aNumberOfBits--) {
            // Check for constant length mark
            if (!MATCH_MARK(*tRawBufPointer, aBitMarkMicros)) {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBitMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            // Check for variable length space indicating a 0 or 1
            if (MATCH_SPACE(*tRawBufPointer, aOneSpaceMicros)) {
                tDecodedData |= tMask; // set the bit
                TRACE_PRINT('1');
            } else if (MATCH_SPACE(*tRawBufPointer, aZeroSpaceMicros)) {
                // do not set the bit
                TRACE_PRINT('0');
            } else {
                DBG_PRINT(F("Space="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aOneSpaceMicros);
                DBG_PRINT(F(" or "));
                DBG_PRINT(aZeroSpaceMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;
        }
        TRACE_PRINTLN("");
    }
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

//#  define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
//#  define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
//#  define TRACE_PRINT(...)    Serial.print(__VA_ARGS__)
//#  define TRACE_PRINTLN(...)  Serial.println(__VA_ARGS__)
/*
 * We "regenerate" the clock and check changes on the significant clock transition
 * We assume that the transition from (aStartOffset -1) to aStartOffset is a significant clock transition
 *
 * The first bit is assumed as start bit and excluded for result
 * @param aStartOffset must point to a mark
 * Input is     results.rawbuf
 * Output is    results.value
 */
bool IRrecv::decodeBiPhaseData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint8_t aValueOfSpaceToMarkTransition,
        unsigned int aBiphaseTimeUnit) {

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    bool tCheckMark = aStartOffset & 1;
    uint8_t tClockCount = 0; // assume that first transition is significant
    aValueOfSpaceToMarkTransition &= 1; // only 0 or 1 are valid
    uint32_t tDecodedData = 0;

    for (uint8_t tBitIndex = 0; tBitIndex < aNumberOfBits;) {
        if (tCheckMark) {
            /*
             *  Check mark and determine current (and next) bit value
             */
            if (MATCH_MARK(*tRawBufPointer, aBiphaseTimeUnit)) {
                // we have a transition here from space to mark
                tClockCount++;
                // for BiPhaseCode, we have a transition at every odd clock count.
                if (tClockCount & 1) {
                    // valid clock edge
                    tDecodedData = (tDecodedData << 1) | aValueOfSpaceToMarkTransition;
                    TRACE_PRINT(aValueOfSpaceToMarkTransition);
                    tBitIndex++;
                }
            } else if (MATCH_MARK(*tRawBufPointer, 2 * aBiphaseTimeUnit)) {
                tClockCount = 0; // can reset clock count here
                // We have a double length mark this includes two valid clock edges
                tDecodedData = (tDecodedData << 1) | aValueOfSpaceToMarkTransition;
                TRACE_PRINT(aValueOfSpaceToMarkTransition);

                tBitIndex++;

            } else {
                /*
                 * Use TRACE_PRINT here, since this normally checks the length of the start bit and therefore will happen very often
                 */
                TRACE_PRINT(F("Mark="));
                TRACE_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                TRACE_PRINT(F(" is not "));
                TRACE_PRINT(aBiphaseTimeUnit);
                TRACE_PRINT(F(" or "));
                TRACE_PRINT(2 * aBiphaseTimeUnit);
                TRACE_PRINT(' ');
                return false;
            }

        } else {
            /*
             * Check space - simulate last not recorded space
             */
            if (tRawBufPointer == &decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen]
                    || MATCH_SPACE(*tRawBufPointer, aBiphaseTimeUnit)) {
                // we have a transition here from mark to space
                tClockCount++;
                if (tClockCount & 1) {
                    // valid clock edge
                    tDecodedData = (tDecodedData << 1) | (aValueOfSpaceToMarkTransition ^ 1);
                    TRACE_PRINT((aValueOfSpaceToMarkTransition ^ 1));
                    tBitIndex++;
                }
            } else if (MATCH_SPACE(*tRawBufPointer, 2 * aBiphaseTimeUnit)) {
                // We have a double length space -> current bit value is 0 and changes to 1
                tClockCount = 0;                // can reset clock count here
                // We have a double length mark this includes two valid clock edges
                if (tBitIndex == 0) {
                    TRACE_PRINT('S'); // do not put start bit into data
                } else {
                    tDecodedData = (tDecodedData << 1) | (aValueOfSpaceToMarkTransition ^ 1);
                    TRACE_PRINT((aValueOfSpaceToMarkTransition ^ 1));
                }
                tBitIndex++;
            } else {
                DBG_PRINT(F("Space="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBiphaseTimeUnit);
                DBG_PRINT(F(" or "));
                DBG_PRINT(2 * aBiphaseTimeUnit);
                DBG_PRINT(' ');
                return false;
            }
        }
        tRawBufPointer++;
        tCheckMark = !tCheckMark;

    }
    TRACE_PRINTLN("");
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

#if DECODE_HASH
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
uint8_t IRrecv::compare(unsigned int oldval, unsigned int newval) {
    if (newval * 10 < oldval * 8) {
        return 0;
    }
    if (oldval * 10 < newval * 8) {
        return 2;
    }
    return 1;
}
//+=============================================================================
// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
// Converts the raw code values into a 32-bit hash code.
// Hopefully this code is unique for each button.
// This isn't a "real" decoding, just an arbitrary value.
//
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

#  if defined(USE_STANDARD_DECODE)
bool IRrecv::decodeHash() {
    long hash = FNV_BASIS_32;

// Require at least 6 samples to prevent triggering on noise
    if (decodedIRData.rawDataPtr->rawlen < 6) {
        return false;
    }

    for (unsigned int i = 1; (i + 2) < decodedIRData.rawDataPtr->rawlen; i++) {
        uint8_t value = compare(decodedIRData.rawDataPtr->rawbuf[i], decodedIRData.rawDataPtr->rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    decodedIRData.decodedRawData = hash;
    decodedIRData.numberOfBits = 32;
    decodedIRData.protocol = UNKNOWN;

    return true;
}
#  else

#warning "Old decoder function decodeHash() is enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeHash() instead."

bool IRrecv::decodeHash() {
    long hash = FNV_BASIS_32;

// Require at least 6 samples to prevent triggering on noise
    if (results.rawlen < 6) {
        return false;
    }

    for (unsigned int i = 1; (i + 2) < results.rawlen; i++) {
        uint8_t value = compare(results.rawbuf[i], results.rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    results.value = hash;
    results.bits = 32;
    decodedIRData.protocol = UNKNOWN;

    return true;
}
#  endif // defined(USE_STANDARD_DECODE)
#endif // DECODE_HASH

const char* IRrecv::getProtocolString(decode_type_t aProtocol) {
    switch (aProtocol) {
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

void IRrecv::printIRResultShort(Print *aSerial, IRData *aIRDataPtr, uint16_t aLeadingSpaceTicks) {
    aSerial->print(F("Protocol="));
    aSerial->print(getProtocolString(aIRDataPtr->protocol));
    if (aIRDataPtr->protocol == UNKNOWN) {
        aSerial->print(' ');
        aSerial->print((aIRDataPtr->rawDataPtr->rawlen + 1) / 2, DEC);
        aSerial->println(F(" bits received"));
    } else {
        /*
         * New decoders have address and command
         */
        aSerial->print(F(" Address=0x"));
        aSerial->print(aIRDataPtr->address, HEX);

        aSerial->print(F(" Command=0x"));
        aSerial->print(aIRDataPtr->command, HEX);

#if defined(ENABLE_EXTRA_INFO)
        if (aIRDataPtr->flags & IRDATA_FLAGS_EXTRA_INFO) {
            aSerial->print(F(" Extra=0x"));
            aSerial->print(aIRDataPtr->extra, HEX);
        }
#endif

        if (aIRDataPtr->flags & IRDATA_FLAGS_PARITY_FAILED) {
            aSerial->print(F(" Parity fail"));
        }

        if (aIRDataPtr->flags & IRDATA_TOGGLE_BIT_MASK) {
            aSerial->print(F(" Toggle=1"));
        }

        if (aIRDataPtr->flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
            aSerial->print(F(" Auto-repeat gap="));
            aSerial->print(aLeadingSpaceTicks * MICROS_PER_TICK);
            aSerial->print(F("us"));
        }

        if (aIRDataPtr->flags & IRDATA_FLAGS_IS_REPEAT) {
            aSerial->print(F(" Repeat"));
            if (aLeadingSpaceTicks != 0) {
                aSerial->print(F(" gap="));
                aSerial->print(aLeadingSpaceTicks * MICROS_PER_TICK);
                aSerial->print(F("us"));
            }
        }

        /*
         * Print raw data
         */
        if (!(aIRDataPtr->flags & IRDATA_FLAGS_IS_REPEAT) || aIRDataPtr->decodedRawData != 0) {
            aSerial->print(F(" Raw-Data=0x"));
            aSerial->print(aIRDataPtr->decodedRawData, HEX);

            /*
             * Print number of bits processed
             */
            aSerial->print(F(" ("));
            aSerial->print(aIRDataPtr->numberOfBits, DEC);
            aSerial->print(F(" bits)"));

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
void IRrecv::printIRResultShort(Print *aSerial) {
    printIRResultShort(aSerial, &decodedIRData, decodedIRData.rawDataPtr->rawbuf[0]);
}

//+=============================================================================
// Dump out the decode_results structure
//
void IRrecv::printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {
    // Print Raw data
    aSerial->print(F("rawData["));
    aSerial->print(decodedIRData.rawDataPtr->rawlen, DEC);
    aSerial->println(F("]: "));

    uint32_t tDurationMicros;

    /*
     * Print initial gap
     */
    if (aOutputMicrosecondsInsteadOfTicks) {
        tDurationMicros = (uint32_t) decodedIRData.rawDataPtr->rawbuf[0] * MICROS_PER_TICK;
    } else {
        tDurationMicros = decodedIRData.rawDataPtr->rawbuf[0];
    }
    aSerial->print(F("     -"));
    aSerial->println(tDurationMicros, DEC);

    for (unsigned int i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
        if (aOutputMicrosecondsInsteadOfTicks) {
            tDurationMicros = decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
        } else {
            tDurationMicros = decodedIRData.rawDataPtr->rawbuf[i];
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
            if (i + 1 < decodedIRData.rawDataPtr->rawlen) {
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
 * Dump out the decode_results structure to be used for sendRaw().
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding!
 *
 * Print ticks in 8 bit format to save space.
 * Maximum is 255*50 microseconds = 12750 microseconds = 12.75 ms, which hardly ever occurs inside an IR sequence.
 * Recording of IRremote anyway stops at a gap of RECORD_GAP_MICROS (5 ms).
 */
void IRrecv::compensateAndPrintIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {
    // Start declaration
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->print(F("uint16_t "));            // variable type
        aSerial->print(F("rawData["));            // array name
    } else {
        aSerial->print(F("uint8_t "));             // variable type
        aSerial->print(F("rawTicks["));             // array name
    }

    aSerial->print(decodedIRData.rawDataPtr->rawlen - 1, DEC);    // array size
    aSerial->print(F("] = {"));    // Start declaration

// Dump data
    for (unsigned int i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
        uint32_t tDuration = decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;

        if (i & 1) {
            // Mark
            tDuration -= MARK_EXCESS_MICROS;
        } else {
            tDuration += MARK_EXCESS_MICROS;
        }

        if (aOutputMicrosecondsInsteadOfTicks) {
            aSerial->print(tDuration);
        } else {
            uint16_t tTicks = (tDuration + (MICROS_PER_TICK / 2)) / MICROS_PER_TICK;
            tTicks = (tTicks > 0xFF) ? 0xFF : tTicks; // safety net
            aSerial->print(tTicks);
        }
        if (i + 1 < decodedIRData.rawDataPtr->rawlen)
            aSerial->print(',');                // ',' not required on last one
        if (!(i & 1))
            aSerial->print(' ');
    }

// End declaration
    aSerial->print(F("};"));                //

// Comment
    aSerial->print(F("  // "));
    printIRResultShort(aSerial);

// Newline
    aSerial->println("");
}

/*
 * Store the decode_results structure to be used for sendRaw().
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding!
 *
 * Maximum foruint8_t is 255*50 microseconds = 12750 microseconds = 12.75 ms, which hardly ever occurs inside an IR sequence.
 * Recording of IRremote anyway stops at a gap of RECORD_GAP_MICROS (5 ms).
 */
void IRrecv::compensateAndStoreIRResultInArray(uint8_t *aArrayPtr) {

// Store data, skip leading space
    for (unsigned int i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
        uint32_t tDuration = decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
        if (i & 1) {
            // Mark
            tDuration -= MARK_EXCESS_MICROS;
        } else {
            tDuration += MARK_EXCESS_MICROS;
        }

        uint16_t tTicks = (tDuration + (MICROS_PER_TICK / 2)) / MICROS_PER_TICK;
        *aArrayPtr = (tTicks > 0xFF) ? 0xFF : tTicks; // safety net
        aArrayPtr++;
    }
}

void IRrecv::printIRResultAsCVariables(Print *aSerial) {
// Now dump "known" codes
    if (decodedIRData.protocol != UNKNOWN) {

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

        // All protocols have data
        aSerial->print(F("uint32_t data = 0x"));
        aSerial->print(decodedIRData.decodedRawData, HEX);
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
    Serial.println("The function decode(&results)) is deprecated and may not work as expected! Just use decode() - without any parameter.");
    (void) aResults;
    return decode();
}
