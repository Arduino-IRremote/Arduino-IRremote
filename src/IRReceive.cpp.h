/*
 * IRReceive.cpp.h
 * This file is exclusively included by IRremote.h to enable easy configuration of library switches
 *
 *  Contains all IRrecv class functions as well as other receiver related functions.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
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
/**
 * The receiver instance
 */
IRrecv IrReceiver;

/*
 * The control structure instance
 */
struct irparams_struct irparams; // the irparams instance

/**
 * Instantiate the IRrecv class. Multiple instantiation is not supported.
 * @param IRReceivePin Arduino pin to use. No sanity check is made.
 */
IRrecv::IRrecv() {
    setReceivePin(0);
#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    setLEDFeedback(0, false);
#endif
}

IRrecv::IRrecv(uint8_t aReceivePin) {
    setReceivePin(aReceivePin);
#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    setLEDFeedback(0, false);
#endif
}
/**
 * Instantiate the IRrecv class. Multiple instantiation is not supported.
 * @param IRReceivePin Arduino pin to use, where a demodulating IR receiver is connected.
 * @ param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
IRrecv::IRrecv(uint8_t aReceivePin, uint8_t aFeedbackLEDPin) {
    setReceivePin(aReceivePin);
#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    setLEDFeedback(aFeedbackLEDPin, false);
#else
    (void) aFeedbackLEDPin;
#endif
}

/**********************************************************************************************************************
 * Stream like API
 **********************************************************************************************************************/
/*
 * @param IRReceivePin Arduino pin to use, where a demodulating IR receiver is connected.
 * @ param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRrecv::begin(uint8_t aReceivePin, bool aEnableLEDFeedback, uint8_t aFeedbackLEDPin) {

    setReceivePin(aReceivePin);
#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    setLEDFeedback(aFeedbackLEDPin, aEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif

    enableIRIn();
}

void IRrecv::setReceivePin(uint8_t aReceivePinNumber) {
    irparams.IRReceivePin = aReceivePinNumber;
#if defined(__AVR__)
    irparams.IRReceivePinMask = digitalPinToBitMask(aReceivePinNumber);
    irparams.IRReceivePinPortInputRegister = portInputRegister(digitalPinToPort(aReceivePinNumber));
#endif
}

void IRrecv::start() {
    enableIRIn();
}

void IRrecv::start(uint16_t aMicrosecondsToAddToGapCounter) {
    enableIRIn();
    noInterrupts();
    irparams.TickCounterForISR += aMicrosecondsToAddToGapCounter / MICROS_PER_TICK;
    interrupts();
}

void IRrecv::stop() {
    disableIRIn();
}
void IRrecv::end() {
    stop();
    FeedbackLEDControl.LedFeedbackEnabled = true;
}

/**
 * Enable IR reception.
 */
void IRrecv::enableIRIn() {

    noInterrupts();

    // Setup pulse clock TickCounterForISR interrupt
    timerConfigForReceive();
    TIMER_ENABLE_RECEIVE_INTR;  // Timer interrupt enable
    TIMER_RESET_INTR_PENDING;   // NOP for most platforms

    // Initialize state machine state
    resume();
    interrupts(); // after resume to avoid running through STOP state 1 time before switching to IDLE

    // Set pin modes
    pinMode(irparams.IRReceivePin, INPUT);
}

/**
 * Disable IR reception.
 */
void IRrecv::disableIRIn() {
    TIMER_DISABLE_RECEIVE_INTR;
}

/**
 * Returns status of reception
 * @return true if no reception is on-going.
 */
bool IRrecv::isIdle() {
    return (irparams.StateForISR == IR_REC_STATE_IDLE || irparams.StateForISR == IR_REC_STATE_STOP) ? true : false;
}

/**
 * Restart the ISR state machine
 * Enable receiving of the next value
 */
void IRrecv::resume() {
    // check allows to call resume at arbitrary places or more than once
    if (irparams.StateForISR == IR_REC_STATE_STOP) {
        irparams.StateForISR = IR_REC_STATE_IDLE;
    }
}

/**
 * Is internally called by decode before calling decoders.
 * Must be used to setup data, if you call decoders manually.
 */
void IRrecv::initDecodedIRData() {

    decodedIRData.rawDataPtr = &irparams;

    if (irparams.OverflowFlag) {
        irparams.OverflowFlag = false;
        irparams.rawlen = 0; // otherwise we have OverflowFlag again at next ISR call
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
    return (irparams.StateForISR == IR_REC_STATE_STOP);
}

/**
 *@return decoded IRData,
 */
IRData* IRrecv::read() {
    if (irparams.StateForISR != IR_REC_STATE_STOP) {
        return NULL;
    }
    if (decode()) {
        return &decodedIRData;
    } else {
        return NULL;
    }
}

/**********************************************************************************************************************
 * The main decode function
 * Attempt to decode the recently receive IR signal
 * @param results decode_results instance returning the decode, if any.
 * @return 0 if no data ready, 1 if data ready. Results of decoding are stored in results
 **********************************************************************************************************************/
bool IRrecv::decode() {
    if (irparams.StateForISR != IR_REC_STATE_STOP) {
        return false;
    }

#if defined(USE_OLD_DECODE)
    // Copy 3 values from irparams for legacy compatibility
    results.rawbuf = irparams.rawbuf;
    results.rawlen = irparams.rawlen;
    results.overflow = irparams.OverflowFlag;
#endif

    initDecodedIRData(); // sets IRDATA_FLAGS_WAS_OVERFLOW

    if (decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
        /*
         * Set OverflowFlag flag and return true here, to let the loop call resume or print raw data.
         */
        decodedIRData.protocol = UNKNOWN;
        return true;
    }

#if defined(DECODE_NEC)
    TRACE_PRINTLN("Attempting NEC decode");
    if (decodeNEC()) {
        return true;
    }
#endif

#if defined(DECODE_PANASONIC)
    TRACE_PRINTLN("Attempting Panasonic/Kaseikyo decode");
    if (decodeKaseikyo()) {
        return true;
    }
#endif

#if defined(DECODE_KASEIKYO) && defined(USE_OLD_DECODE) // if not USE_OLD_DECODE enabled, decodeKaseikyo() is already called by decodePanasonic()
        TRACE_PRINTLN("Attempting Panasonic/Kaseikyo decode");
        if (decodeKaseikyo()) {
            return true;
        }
#endif

#if defined(DECODE_DENON)
    TRACE_PRINTLN("Attempting Denon/Sharp decode");
    if (decodeDenon()) {
        return true;
    }
#endif

#if defined(DECODE_SONY)
    TRACE_PRINTLN("Attempting Sony decode");
    if (decodeSony()) {
        return true;
    }
#endif

#if defined(DECODE_SHARP) && ! defined(DECODE_DENON)
        TRACE_PRINTLN("Attempting Denon/Sharp decode");
        if (decodeSharp()) {
            return true;
        }
#endif

#if defined(DECODE_RC5)
    TRACE_PRINTLN("Attempting RC5 decode");
    if (decodeRC5()) {
        return true;
    }
#endif

#if defined(DECODE_RC6)
    TRACE_PRINTLN("Attempting RC6 decode");
    if (decodeRC6()) {
        return true;
    }
#endif

#if defined(DECODE_LG)
    TRACE_PRINTLN("Attempting LG decode");
    if (decodeLG()) {
        return true;
    }
#endif

#if defined(DECODE_JVC)
    TRACE_PRINTLN("Attempting JVC decode");
    if (decodeJVC()) {
        return true;
    }
#endif

#if defined(DECODE_SAMSUNG)
    TRACE_PRINTLN("Attempting Samsung decode");
#if !defined(USE_OLD_DECODE)
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

#if defined(DECODE_WHYNTER)
    TRACE_PRINTLN("Attempting Whynter decode");
    if (decodeWhynter()) {
        return true;
    }
#endif

#if defined(DECODE_LEGO_PF)
    TRACE_PRINTLN("Attempting Lego Power Functions");
    if (decodeLegoPowerFunctions()) {
        return true;
    }
#endif

#if defined(DECODE_BOSEWAVE)
    TRACE_PRINTLN("Attempting Bosewave  decode");
    if (decodeBoseWave()) {
        return true;
    }
#endif

#if defined(DECODE_MAGIQUEST)
    TRACE_PRINTLN("Attempting MagiQuest decode");
    if (decodeMagiQuest()) {
        return true;
    }
#endif

    /*
     * Last resort is the universal hash decode which always return true
     */
#if defined(DECODE_HASH)
    TRACE_PRINTLN("Hash decode");
    // decodeHash returns a hash on any input.
    // Thus, it needs to be last in the list.
    // If you add any decodes, add them before this.
    if (decodeHash()) {
        return true;
    }
#endif

    /*
     * Return true here, to let the loop decide to call resume or to print raw data.
     */
    return true;
}

/**********************************************************************************************************************
 * Common decode functions
 **********************************************************************************************************************/
/*
 * Decode pulse width protocols.
 * The space (pause) has constant length, the length of the mark determines the bit value.
 *      Each bit looks like: MARK_1 + SPACE -> 1 or : MARK_0 + SPACE -> 0
 *
 * Data is read MSB first if not otherwise enabled.
 * Input is     results.rawbuf
 * Output is    results.value
 */
bool IRrecv::decodePulseWidthData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint16_t aOneMarkMicros, uint16_t aZeroMarkMicros,
        uint16_t aBitSpaceMicros, bool aMSBfirst) {

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    uint32_t tDecodedData = 0;

    if (aMSBfirst) {
        for (uint_fast8_t i = 0; i < aNumberOfBits; i++) {
            // Check for variable length mark indicating a 0 or 1
            if (matchMark(*tRawBufPointer, aOneMarkMicros)) {
                tDecodedData = (tDecodedData << 1) | 1;
                TRACE_PRINT('1');
            } else if (matchMark(*tRawBufPointer, aZeroMarkMicros)) {
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
                if (!matchSpace(*tRawBufPointer, aBitSpaceMicros)) {
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
            if (matchMark(*tRawBufPointer, aOneMarkMicros)) {
                tDecodedData |= tMask; // set the bit
                TRACE_PRINT('1');
            } else if (matchMark(*tRawBufPointer, aZeroMarkMicros)) {
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
                if (!matchSpace(*tRawBufPointer, aBitSpaceMicros)) {
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
bool IRrecv::decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint16_t aBitMarkMicros, uint16_t aOneSpaceMicros,
        uint16_t aZeroSpaceMicros, bool aMSBfirst) {

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    uint32_t tDecodedData = 0;

    if (aMSBfirst) {
        for (uint_fast8_t i = 0; i < aNumberOfBits; i++) {
            // Check for constant length mark
            if (!matchMark(*tRawBufPointer, aBitMarkMicros)) {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBitMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            // Check for variable length space indicating a 0 or 1
            if (matchSpace(*tRawBufPointer, aOneSpaceMicros)) {
                tDecodedData = (tDecodedData << 1) | 1;
                TRACE_PRINT('1');
            } else if (matchSpace(*tRawBufPointer, aZeroSpaceMicros)) {
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
            if (!matchMark(*tRawBufPointer, aBitMarkMicros)) {
                DBG_PRINT(F("Mark="));
                DBG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DBG_PRINT(F(" is not "));
                DBG_PRINT(aBitMarkMicros);
                DBG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            // Check for variable length space indicating a 0 or 1
            if (matchSpace(*tRawBufPointer, aOneSpaceMicros)) {
                tDecodedData |= tMask; // set the bit
                TRACE_PRINT('1');
            } else if (matchSpace(*tRawBufPointer, aZeroSpaceMicros)) {
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
        uint16_t aBiphaseTimeUnit) {

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    bool tCheckMark = aStartOffset & 1;
    uint_fast8_t tClockCount = 0; // assume that first transition is significant
    aValueOfSpaceToMarkTransition &= 1; // only 0 or 1 are valid
    uint32_t tDecodedData = 0;

    for (uint_fast8_t tBitIndex = 0; tBitIndex < aNumberOfBits;) {
        if (tCheckMark) {
            /*
             *  Check mark and determine current (and next) bit value
             */
            if (matchMark(*tRawBufPointer, aBiphaseTimeUnit)) {
                // we have a transition here from space to mark
                tClockCount++;
                // for BiPhaseCode, we have a transition at every odd clock count.
                if (tClockCount & 1) {
                    // valid clock edge
                    tDecodedData = (tDecodedData << 1) | aValueOfSpaceToMarkTransition;
                    TRACE_PRINT(aValueOfSpaceToMarkTransition);
                    tBitIndex++;
                }
            } else if (matchMark(*tRawBufPointer, 2 * aBiphaseTimeUnit)) {
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
                    || matchSpace(*tRawBufPointer, aBiphaseTimeUnit)) {
                // we have a transition here from mark to space
                tClockCount++;
                if (tClockCount & 1) {
                    // valid clock edge
                    tDecodedData = (tDecodedData << 1) | (aValueOfSpaceToMarkTransition ^ 1);
                    TRACE_PRINT((aValueOfSpaceToMarkTransition ^ 1));
                    tBitIndex++;
                }
            } else if (matchSpace(*tRawBufPointer, 2 * aBiphaseTimeUnit)) {
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

#if defined(DECODE_HASH)
/**********************************************************************************************************************
 * Internal Hash decode function
 **********************************************************************************************************************/
/*
 * Compare two (tick) values
 * Use a tolerance of 20% to enable e.g. 500 and 600 (NEC timing) to be equal
 * @return:  0 if newval is shorter, 1 if newval is equal, and 2 if newval is longer
 */
uint8_t IRrecv::compare(unsigned int oldval, unsigned int newval) {
    if (newval * 10 < oldval * 8) {
        return 0;
    }
    if (oldval * 10 < newval * 8) {
        return 2;
    }
    return 1;
}
/**
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hash the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
 * Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 *
 * see: http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
*/
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

#  if !defined(USE_OLD_DECODE)
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
#  endif // !defined(USE_OLD_DECODE)
#endif // DECODE_HASH

/**********************************************************************************************************************
 * Match functions
 **********************************************************************************************************************/
/*
 * Match function without compensating for marks exceeded or spaces shortened by demodulator hardware
 * Currently not used
 */
bool matchTicks(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
#ifdef TRACE
    Serial.print(F("Testing: "));
    Serial.print(TICKS_LOW(aMatchValueMicros), DEC);
    Serial.print(F(" <= "));
    Serial.print(aMeasuredTicks, DEC);
    Serial.print(F(" <= "));
    Serial.print(TICKS_HIGH(aMatchValueMicros), DEC);
#endif
    bool passed = ((aMeasuredTicks >= TICKS_LOW(aMatchValueMicros)) && (aMeasuredTicks <= TICKS_HIGH(aMatchValueMicros)));
#ifdef TRACE
    if (passed) {
        Serial.println(F("?; passed"));
    } else {
        Serial.println(F("?; FAILED"));
    }
#endif
    return passed;
}

bool MATCH(uint16_t measured_ticks, uint16_t desired_us) {
    return matchTicks(measured_ticks, desired_us);
}

/*
 * Compensate for marks exceeded by demodulator hardware
 */
bool matchMark(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
#ifdef TRACE
    Serial.print(F("Testing mark (actual vs desired): "));
    Serial.print(aMeasuredTicks * MICROS_PER_TICK, DEC);
    Serial.print(F("us vs "));
    Serial.print(aMatchValueMicros, DEC);
    Serial.print(F("us: "));
    Serial.print(TICKS_LOW(aMatchValueMicros + MARK_EXCESS_MICROS) * MICROS_PER_TICK, DEC);
    Serial.print(F(" <= "));
    Serial.print(aMeasuredTicks * MICROS_PER_TICK, DEC);
    Serial.print(F(" <= "));
    Serial.print(TICKS_HIGH(aMatchValueMicros + MARK_EXCESS_MICROS) * MICROS_PER_TICK, DEC);
#endif
    // compensate for marks exceeded by demodulator hardware
    bool passed = ((aMeasuredTicks >= TICKS_LOW(aMatchValueMicros + MARK_EXCESS_MICROS))
            && (aMeasuredTicks <= TICKS_HIGH(aMatchValueMicros + MARK_EXCESS_MICROS)));
#ifdef TRACE
    if (passed) {
        Serial.println(F("?; passed"));
    } else {
        Serial.println(F("?; FAILED"));
    }
#endif
    return passed;
}

bool MATCH_MARK(uint16_t measured_ticks, uint16_t desired_us) {
    return matchMark(measured_ticks, desired_us);
}

/*
 * Compensate for spaces shortened by demodulator hardware
 */
bool matchSpace(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
#ifdef TRACE
    Serial.print(F("Testing space (actual vs desired): "));
    Serial.print(aMeasuredTicks * MICROS_PER_TICK, DEC);
    Serial.print(F("us vs "));
    Serial.print(aMatchValueMicros, DEC);
    Serial.print(F("us: "));
    Serial.print(TICKS_LOW(aMatchValueMicros - MARK_EXCESS_MICROS) * MICROS_PER_TICK, DEC);
    Serial.print(F(" <= "));
    Serial.print(aMeasuredTicks * MICROS_PER_TICK, DEC);
    Serial.print(F(" <= "));
    Serial.print(TICKS_HIGH(aMatchValueMicros - MARK_EXCESS_MICROS) * MICROS_PER_TICK, DEC);
#endif
    // compensate for spaces shortened by demodulator hardware
    bool passed = ((aMeasuredTicks >= TICKS_LOW(aMatchValueMicros - MARK_EXCESS_MICROS))
            && (aMeasuredTicks <= TICKS_HIGH(aMatchValueMicros - MARK_EXCESS_MICROS)));
#ifdef TRACE
    if (passed) {
        Serial.println(F("?; passed"));
    } else {
        Serial.println(F("?; FAILED"));
    }
#endif
    return passed;
}

bool MATCH_SPACE(uint16_t measured_ticks, uint16_t desired_us) {
    return matchSpace(measured_ticks, desired_us);
}

// Function used for ir_Pronto
int getMarkExcessMicros() {
    return MARK_EXCESS_MICROS;
}

/**********************************************************************************************************************
 * Print functions
 * Since a library should not allocate the "Serial" object, all functions require a pointer to a Print object.
 **********************************************************************************************************************/
void printIRResultShort(Print *aSerial, IRData *aIRDataPtr, uint16_t aLeadingSpaceTicks) {
    aSerial->print(F("Protocol="));
    aSerial->print(getProtocolString(aIRDataPtr->protocol));
    if (aIRDataPtr->protocol == UNKNOWN) {
#if defined(DECODE_HASH)
        aSerial->print(F(" Hash=0x"));
        aSerial->print(aIRDataPtr->decodedRawData, HEX);
#endif
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

        if (aIRDataPtr->flags & IRDATA_FLAGS_EXTRA_INFO) {
            aSerial->print(F(" Extra=0x"));
            aSerial->print(aIRDataPtr->extra, HEX);
        }

        if (aIRDataPtr->flags & IRDATA_FLAGS_PARITY_FAILED) {
            aSerial->print(F(" Parity fail"));
        }

        if (aIRDataPtr->flags & IRDATA_TOGGLE_BIT_MASK) {
            aSerial->print(F(" Toggle=1"));
        }

        if (aIRDataPtr->flags & (IRDATA_FLAGS_IS_AUTO_REPEAT | IRDATA_FLAGS_IS_REPEAT)) {
            aSerial->print(' ');
            if (aIRDataPtr->flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
                aSerial->print(F("Auto-"));
            }
            aSerial->print(F("Repeat"));
            if (aLeadingSpaceTicks != 0) {
                aSerial->print(F(" gap="));
                aSerial->print((uint32_t) aLeadingSpaceTicks * MICROS_PER_TICK);
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
            aSerial->print(' ');
            aSerial->print(aIRDataPtr->numberOfBits, DEC);
            aSerial->print(F(" bits"));

#if !defined(USE_OLD_DECODE)
            if (aIRDataPtr->flags & IRDATA_FLAGS_IS_MSB_FIRST) {
                aSerial->println(F(" MSB first"));
            } else {
                aSerial->println(F(" LSB first"));
            }
#else
            aSerial->println();
#endif

        } else {
            aSerial->println();
        }
    }
}

void IRrecv::printIRResultShort(Print *aSerial) {
    ::printIRResultShort(aSerial, &decodedIRData, decodedIRData.rawDataPtr->rawbuf[0]);
}

void IRrecv::printIRResultMinimal(Print *aSerial) {
    aSerial->print(F("P="));
    aSerial->print(getProtocolString(decodedIRData.protocol));
    if (decodedIRData.protocol == UNKNOWN) {
#if defined(DECODE_HASH)
        aSerial->print(F(" #=0x"));
        aSerial->print(decodedIRData.decodedRawData, HEX);
#endif
        aSerial->print(' ');
        aSerial->print((decodedIRData.rawDataPtr->rawlen + 1) / 2, DEC);
        aSerial->println(F(" bits received"));
    } else {
        /*
         * New decoders have address and command
         */
        aSerial->print(F(" A=0x"));
        aSerial->print(decodedIRData.address, HEX);

        aSerial->print(F(" C=0x"));
        aSerial->print(decodedIRData.command, HEX);
        aSerial->print(F(" Raw=0x"));
        aSerial->print(decodedIRData.decodedRawData, HEX);
        if (decodedIRData.flags & (IRDATA_FLAGS_IS_AUTO_REPEAT | IRDATA_FLAGS_IS_REPEAT)) {
            aSerial->print(F(" R"));
        }
    }
}

/**
 * Dump out the decode_results structure
 */
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

    for (uint8_t i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
        if (aOutputMicrosecondsInsteadOfTicks) {
            tDurationMicros = decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
        } else {
            tDurationMicros = decodedIRData.rawDataPtr->rawbuf[i];
        }
        if (!(i & 1)) {  // even
            aSerial->print('-');
        } else {  // odd
            aSerial->print(F("     +"));
        }
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

        if ((i & 1) && (i + 1) < decodedIRData.rawDataPtr->rawlen) {
            aSerial->print(','); //',' not required for last one
        }

        if (!(i % 8)) {
            aSerial->println("");
        }
    }
    aSerial->println("");                    // Newline
}

/**
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

/**
 * Store the decode_results structure to be used for sendRaw().
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding!
 *
 * Maximum for uint8_t is 255*50 microseconds = 12750 microseconds = 12.75 ms, which hardly ever occurs inside an IR sequence.
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
        aSerial->print(F("uint16_t"));
        aSerial->print(F(" address = 0x"));
        aSerial->print(decodedIRData.address, HEX);
        aSerial->println(';');

        aSerial->print(F("uint16_t"));
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

const char* getProtocolString(decode_type_t aProtocol) {
    switch (aProtocol) {
    default:
    case UNKNOWN:
        return ("UNKNOWN");
        break;
    case DENON:
        return ("DENON");
        break;
    case SHARP:
        return ("SHARP");
        break;
    case JVC:
        return ("JVC");
        break;
    case LG:
        return ("LG");
        break;
    case NEC:
        return ("NEC");
        break;
    case PANASONIC:
        return ("PANASONIC");
        break;
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
    case RC5:
        return ("RC5");
        break;
    case RC6:
        return ("RC6");
        break;
    case SAMSUNG:
        return ("SAMSUNG");
        break;
    case SONY:
        return ("SONY");
        break;
    case APPLE:
        return ("APPLE");
        break;
#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    case BOSEWAVE:
        return ("BOSEWAVE");
        break;
    case LEGO_PF:
        return ("LEGO_PF");
        break;
    case MAGIQUEST:
        return ("MAGIQUEST");
        break;
    case WHYNTER:
        return ("WHYNTER");
        break;
#endif
    }
}

/**********************************************************************************************************************
 * Interrupt Service Routine - Called every 50 us
 *
 * Duration in ticks of 50 us of alternating SPACE, MARK are recorded in irparams.rawbuf array.
 * 'rawlen' counts the number of entries recorded so far.
 * First entry is the SPACE between transmissions.
 *
 * As soon as one SPACE entry gets longer than RECORD_GAP_TICKS, state switches to STOP (frame received). Timing of SPACE continues.
 * A call of resume() switches from STOP to IDLE.
 * As soon as first MARK arrives in IDLE, gap width is recorded and new logging starts.
 *
 * With digitalRead and Feedback LED
 * 15 pushs, 1 in, 1 eor before start of code = 2 us @16MHz + * 7.2 us computation time + * pop + reti = 2.25 us @16MHz => 11.5 us @16MHz
 * With portInputRegister and mask and Feedback LED code commented
 * 9 pushs, 1 in, 1 eor before start of code = 1.25 us @16MHz + * 2.25 us computation time + * pop + reti = 1.5 us @16MHz => 5 us @16MHz
 *
 **********************************************************************************************************************/
//#define IR_MEASURE_TIMING
//#define IR_TIMING_TEST_PIN 7 // do not forget to execute:  pinMode(IR_TIMING_TEST_PIN, OUTPUT);
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
#include "digitalWriteFast.h"
#endif
#if defined(TIMER_INTR_NAME)
ISR (TIMER_INTR_NAME) // for ISR definitions
#else
ISR () // for functions definitions which are called by separate (board specific) ISR
#endif
{
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
    digitalWriteFast(IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
    // 7 - 8.5 us for ISR body (without pushes and pops) for ATmega328 @16MHz

    TIMER_RESET_INTR_PENDING;// reset TickCounterForISR interrupt flag if required (currently only for Teensy and ATmega4809)

    // Read if IR Receiver -> SPACE [xmt LED off] or a MARK [xmt LED on]
#if defined(__AVR__)
    uint8_t irdata = *irparams.IRReceivePinPortInputRegister & irparams.IRReceivePinMask;
#else
    uint8_t irdata = (uint8_t) digitalRead(irparams.IRReceivePin);
#endif

    // clip TickCounterForISR at maximum 0xFFFF / 3.2 seconds at 50 us ticks
    if (irparams.TickCounterForISR < 0xFFFF) {
        irparams.TickCounterForISR++;  // One more 50uS tick
    }

    /*
     * Due to a ESP32 compiler bug https://github.com/espressif/esp-idf/issues/1552 no switch statements are possible for ESP32
     * So we change the code to if / else if
     */
//    switch (irparams.StateForISR) {
    //......................................................................
    if (irparams.StateForISR == IR_REC_STATE_IDLE) { // In the middle of a gap
        if (irdata == MARK) {
            // check if we did not start in the middle of an command by checking the minimum length of leading space
            if (irparams.TickCounterForISR > RECORD_GAP_TICKS) {
                // Gap just ended; Record gap duration + start recording transmission
                // Initialize all state machine variables
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
//                digitalWriteFast(IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
                irparams.OverflowFlag = false;
                irparams.rawbuf[0] = irparams.TickCounterForISR;
                irparams.rawlen = 1;
                irparams.StateForISR = IR_REC_STATE_MARK;
            }
            irparams.TickCounterForISR = 0;
        }

    } else if (irparams.StateForISR == IR_REC_STATE_MARK) {  // Timing Mark
        if (irdata != MARK) {   // Mark ended; Record time
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
//            digitalWriteFast(IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
            irparams.rawbuf[irparams.rawlen++] = irparams.TickCounterForISR;
            irparams.StateForISR = IR_REC_STATE_SPACE;
            irparams.TickCounterForISR = 0;
        }

    } else if (irparams.StateForISR == IR_REC_STATE_SPACE) {  // Timing Space
        if (irdata == MARK) {  // Space just ended; Record time
            if (irparams.rawlen >= RAW_BUFFER_LENGTH) {
                // Flag up a read OverflowFlag; Stop the State Machine
                irparams.OverflowFlag = true;
                irparams.StateForISR = IR_REC_STATE_STOP;
            } else {
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
//                digitalWriteFast(IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
                irparams.rawbuf[irparams.rawlen++] = irparams.TickCounterForISR;
                irparams.StateForISR = IR_REC_STATE_MARK;
            }
            irparams.TickCounterForISR = 0;

        } else if (irparams.TickCounterForISR > RECORD_GAP_TICKS) {
            /*
             * Current code is ready for processing!
             * We received a long space, which indicates gap between codes.
             * Switch to IR_REC_STATE_STOP
             * Don't reset TickCounterForISR; keep counting width of next leading space
             */
            irparams.StateForISR = IR_REC_STATE_STOP;
        }
    } else if (irparams.StateForISR == IR_REC_STATE_STOP) {
        /*
         * Complete command received
         * stay here until resume() is called, which switches state to IR_REC_STATE_IDLE
         */
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
//        digitalWriteFast(IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
        if (irdata == MARK) {
            irparams.TickCounterForISR = 0;  // Reset gap TickCounterForISR, to prepare for call of resume()
        }
    }

#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    if (FeedbackLEDControl.LedFeedbackEnabled) {
        setFeedbackLED(irdata == MARK);
    }
#endif

#ifdef IR_MEASURE_TIMING
    digitalWriteFast(IR_TIMING_TEST_PIN, LOW); // 2 clock cycles
#endif
}

/**********************************************************************************************************************
 * The DEPRECATED decode function with parameter aResults for backwards compatibility
 **********************************************************************************************************************/
bool IRrecv::decode(decode_results *aResults) {
    Serial.println(
            "The function decode(&results)) is deprecated and may not work as expected! Just use decode() - without any parameter.");
    (void) aResults;
    return decode();
}

