/*
 * IRReceive.hpp
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
#ifndef IR_RECEIVE_HPP
#define IR_RECEIVE_HPP

/** \addtogroup Receiving Receiving IR data for multiple protocols
 * @{
 */
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
    decodedIRData.rawDataPtr = &irparams; // for decodePulseDistanceData() etc.
    setReceivePin(0);
#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    setLEDFeedback(0, false);
#endif
}

IRrecv::IRrecv(uint8_t aReceivePin) {
    decodedIRData.rawDataPtr = &irparams; // for decodePulseDistanceData() etc.
    setReceivePin(aReceivePin);
#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    setLEDFeedback(0, false);
#endif
}
/**
 * Instantiate the IRrecv class. Multiple instantiation is not supported.
 * @param aReceivePin Arduino pin to use, where a demodulating IR receiver is connected.
 * @param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
IRrecv::IRrecv(uint8_t aReceivePin, uint8_t aFeedbackLEDPin) {
    decodedIRData.rawDataPtr = &irparams; // for decodePulseDistanceData() etc.
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
/**
 * Initializes the receive and feedback pin
 * @param aReceivePin The Arduino pin number, where a demodulating IR receiver is connected.
 * @param aEnableLEDFeedback if true / ENABLE_LED_FEEDBACK, then let the feedback led blink on receiving IR signal
 * @param aFeedbackLEDPin if 0 / USE_DEFAULT_FEEDBACK_LED_PIN, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
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

/**
 * Sets / changes the receiver pin number
 */
void IRrecv::setReceivePin(uint8_t aReceivePinNumber) {
    irparams.IRReceivePin = aReceivePinNumber;
#if defined(__AVR__)
    irparams.IRReceivePinMask = digitalPinToBitMask(aReceivePinNumber);
    irparams.IRReceivePinPortInputRegister = portInputRegister(digitalPinToPort(aReceivePinNumber));
#endif
}

/**
 * Configures the timer and the state machine for IR reception.
 */
void IRrecv::start() {
    enableIRIn();
}

/**
 * Configures the timer and the state machine for IR reception.
 * @param aMicrosecondsToAddToGapCounter To compensate for microseconds the timer was stopped / disabled.
 */
void IRrecv::start(uint32_t aMicrosecondsToAddToGapCounter) {
    enableIRIn();
    noInterrupts();
    irparams.TickCounterForISR += aMicrosecondsToAddToGapCounter / MICROS_PER_TICK;
    interrupts();
}

/**
 * Disables the timer for IR reception.
 * Alias for
 */
void IRrecv::stop() {
    disableIRIn();
}

/**
 * Disables the timer for IR reception.
 */
void IRrecv::end() {
    stop();
}

/**
 * Configures the timer and the state machine for IR reception.
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
 * Disables the timer for IR reception.
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

    if (irparams.OverflowFlag) {
        // Copy overflow flag to decodedIRData.flags and reset it
        irparams.OverflowFlag = false;
        irparams.rawlen = 0; // otherwise we have OverflowFlag again at next ISR call
        decodedIRData.flags = IRDATA_FLAGS_WAS_OVERFLOW;
        DEBUG_PRINTLN("Overflow happened");

    } else {
        decodedIRData.flags = IRDATA_FLAGS_EMPTY;
        // save last protocol, command and address for repeat handling (where the are copied back :-))
        lastDecodedProtocol = decodedIRData.protocol; // repeat patterns can be equal between protocols (e.g. NEC and LG), so we must keep the original one
        lastDecodedCommand = decodedIRData.command;
        lastDecodedAddress = decodedIRData.address;

    }
    decodedIRData.protocol = UNKNOWN;
    decodedIRData.command = 0;
    decodedIRData.address = 0;
    decodedIRData.decodedRawData = 0;
    decodedIRData.numberOfBits = 0;
}

/**
 * Returns true if IR receiver data is available.
 */
bool IRrecv::available() {
    return (irparams.StateForISR == IR_REC_STATE_STOP);
}

/**
 * If IR receiver data is available, returns pointer to IrReceiver.decodedIRData, else NULL.
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

/**
 * The main decode function, attempts to decode the recently receive IR signal.
 * @return false if no IR receiver data available, true if data available. Results of decoding are stored in IrReceiver.decodedIRData.
 * The set of decoders used is determined by active definitions of the DECODE_<PROTOCOL> macros.
 */
bool IRrecv::decode() {
    if (irparams.StateForISR != IR_REC_STATE_STOP) {
        return false;
    }

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

#if defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)
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
    if (decodeSamsung()) {
        return true;
    }
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
     * Try the universal decoder for pulse width or pulse distance protocols
     */
#if defined(DECODE_DISTANCE)
    TRACE_PRINTLN("Attempting universal Distance decode");
    if (decodeDistance()) {
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
/**
 * Decode pulse width protocols. Currently only used for sony protocol.
 * The space (pause) has constant length, the length of the mark determines the bit value.
 *      Each bit looks like: MARK_1 + SPACE -> 1 or : MARK_0 + SPACE -> 0
 *
 * Input is     IrReceiver.decodedIRData.rawDataPtr->rawbuf[]
 * Output is    IrReceiver.decodedIRData.decodedRawData
 *
 * @param aStartOffset must point to a mark
 * @return true if decoding was successful
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
                DEBUG_PRINT(F("Mark="));
                DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DEBUG_PRINT(F(" is not "));
                DEBUG_PRINT(aOneMarkMicros);
                DEBUG_PRINT(F(" or "));
                DEBUG_PRINT(aZeroMarkMicros);
                DEBUG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            if (tRawBufPointer < &decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen]) {
                // Assume that last space, which is not recorded, is correct, since we can not check it
                // Check for constant length space
                if (!matchSpace(*tRawBufPointer, aBitSpaceMicros)) {
                    DEBUG_PRINT(F("Space="));
                    DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                    DEBUG_PRINT(F(" is not "));
                    DEBUG_PRINT(aBitSpaceMicros);
                    DEBUG_PRINT(' ');
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
                DEBUG_PRINT(F("Mark="));
                DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DEBUG_PRINT(F(" is not "));
                DEBUG_PRINT(aOneMarkMicros);
                DEBUG_PRINT(F(" or "));
                DEBUG_PRINT(aZeroMarkMicros);
                DEBUG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;

            if (tRawBufPointer < &decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen]) {
                // Assume that last space, which is not recorded, is correct, since we can not check it
                // Check for constant length space
                if (!matchSpace(*tRawBufPointer, aBitSpaceMicros)) {
                    DEBUG_PRINT(F("Space="));
                    DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                    DEBUG_PRINT(F(" is not "));
                    DEBUG_PRINT(aBitSpaceMicros);
                    DEBUG_PRINT(' ');
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

/**
 * Decode pulse distance protocols.
 * The mark (pulse) has constant length, the length of the space determines the bit value.
 * Each bit looks like: MARK + SPACE_1 -> 1
 *                 or : MARK + SPACE_0 -> 0
 *
 * Input is     IrReceiver.decodedIRData.rawDataPtr->rawbuf[]
 * Output is    IrReceiver.decodedIRData.decodedRawData
 *
 * @param aStartOffset must point to a mark
 * @return true if decoding was successful
 */
bool IRrecv::decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint16_t aBitMarkMicros, uint16_t aOneSpaceMicros,
        uint16_t aZeroSpaceMicros, bool aMSBfirst) {

    uint16_t *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    uint32_t tDecodedData = 0;

    if (aMSBfirst) {
        for (uint_fast8_t i = 0; i < aNumberOfBits; i++) {
            // Check for constant length mark
            if (!matchMark(*tRawBufPointer, aBitMarkMicros)) {
                DEBUG_PRINT(F("Mark="));
                DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DEBUG_PRINT(F(" is not "));
                DEBUG_PRINT(aBitMarkMicros);
                DEBUG_PRINT(' ');
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
                DEBUG_PRINT(F("Space="));
                DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DEBUG_PRINT(F(" is not "));
                DEBUG_PRINT(aOneSpaceMicros);
                DEBUG_PRINT(F(" or "));
                DEBUG_PRINT(aZeroSpaceMicros);
                DEBUG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;
        }
        TRACE_PRINTLN("");

    } else {
        for (uint32_t tMask = 1UL; aNumberOfBits > 0; tMask <<= 1, aNumberOfBits--) {
            // Check for constant length mark
            if (!matchMark(*tRawBufPointer, aBitMarkMicros)) {
                DEBUG_PRINT(F("Mark="));
                DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DEBUG_PRINT(F(" is not "));
                DEBUG_PRINT(aBitMarkMicros);
                DEBUG_PRINT(' ');
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
                DEBUG_PRINT(F("Space="));
                DEBUG_PRINT(*tRawBufPointer * MICROS_PER_TICK);
                DEBUG_PRINT(F(" is not "));
                DEBUG_PRINT(aOneSpaceMicros);
                DEBUG_PRINT(F(" or "));
                DEBUG_PRINT(aZeroSpaceMicros);
                DEBUG_PRINT(' ');
                return false;
            }
            tRawBufPointer++;
        }
        TRACE_PRINTLN("");
    }
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

/*
 * Static variables for the getBiphaselevel function
 */
uint8_t sBiphaseDecodeRawbuffOffset; // Index into raw timing array
uint16_t sCurrentTimingIntervals; // Number of aBiphaseTimeUnit intervals of the current rawbuf[sBiphaseDecodeRawbuffOffset] timing.
uint8_t sUsedTimingIntervals;       // Number of already used intervals of sCurrentTimingIntervals.
uint16_t sBiphaseTimeUnit;

void IRrecv::initBiphaselevel(uint8_t aRCDecodeRawbuffOffset, uint16_t aBiphaseTimeUnit) {
    sBiphaseDecodeRawbuffOffset = aRCDecodeRawbuffOffset;
    sBiphaseTimeUnit = aBiphaseTimeUnit;
    sUsedTimingIntervals = 0;
}

/**
 * Gets the level of one time interval (aBiphaseTimeUnit) at a time from the raw buffer.
 * The RC5/6 decoding is easier if the data is broken into time intervals.
 * E.g. if the buffer has mark for 2 time intervals and space for 1,
 * successive calls to getBiphaselevel will return 1, 1, 0.
 *
 *               _   _   _   _   _   _   _   _   _   _   _   _   _
 *         _____| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
 *                ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^    Significant clock edge
 *               _     _   _   ___   _     ___     ___   _   - Mark
 * Data    _____| |___| |_| |_|   |_| |___|   |___|   |_| |  - Data starts with a mark->space bit
 *                1   0   0   0   1   1   0   1   0   1   1  - Space
 * A mark to space at a significant clock edge results in a 1
 * A space to mark at a significant clock edge results in a 0 (for RC6)
 * Returns current level [MARK or SPACE] or -1 for error (measured time interval is not a multiple of sBiphaseTimeUnit).
 */
uint8_t IRrecv::getBiphaselevel() {
    uint8_t tLevelOfCurrentInterval; // 0 (SPACE) or 1 (MARK)

    if (sBiphaseDecodeRawbuffOffset >= decodedIRData.rawDataPtr->rawlen) {
        return SPACE;  // After end of recorded buffer, assume space.
    }

    tLevelOfCurrentInterval = (sBiphaseDecodeRawbuffOffset) & 1; // on odd rawbuf offsets we have mark timings

    /*
     * Setup data if sUsedTimingIntervals is 0
     */
    if (sUsedTimingIntervals == 0) {
        uint16_t tCurrentTimingWith = decodedIRData.rawDataPtr->rawbuf[sBiphaseDecodeRawbuffOffset];
        uint16_t tMarkExcessCorrection = (tLevelOfCurrentInterval == MARK) ? MARK_EXCESS_MICROS : -MARK_EXCESS_MICROS;

        if (matchTicks(tCurrentTimingWith, (sBiphaseTimeUnit) + tMarkExcessCorrection)) {
            sCurrentTimingIntervals = 1;
        } else if (matchTicks(tCurrentTimingWith, (2 * sBiphaseTimeUnit) + tMarkExcessCorrection)) {
            sCurrentTimingIntervals = 2;
        } else if (matchTicks(tCurrentTimingWith, (3 * sBiphaseTimeUnit) + tMarkExcessCorrection)) {
            sCurrentTimingIntervals = 3;
        } else {
            return -1;
        }
    }

    // We use another interval from tCurrentTimingIntervals
    sUsedTimingIntervals++;

    // keep track of current timing offset
    if (sUsedTimingIntervals >= sCurrentTimingIntervals) {
        // we have used all intervals of current timing, switch to next timing value
        sUsedTimingIntervals = 0;
        sBiphaseDecodeRawbuffOffset++;
    }

    TRACE_PRINTLN(tLevelOfCurrentInterval);

    return tLevelOfCurrentInterval;
}

#if defined(DECODE_HASH)
/**********************************************************************************************************************
 * Internal Hash decode function
 **********************************************************************************************************************/
/**
 * Compare two (tick) values
 * Use a tolerance of 20% to enable e.g. 500 and 600 (NEC timing) to be equal
 * @return  0 if newval is shorter, 1 if newval is equal, and 2 if newval is longer
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

#define FNV_PRIME_32 16777619   ///< used for decodeHash()
#define FNV_BASIS_32 2166136261 ///< used for decodeHash()

/**
 * decodeHash - decode an arbitrary IR code.
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
bool IRrecv::decodeHash() {
    long hash = FNV_BASIS_32;

// Require at least 6 samples to prevent triggering on noise
    if (decodedIRData.rawDataPtr->rawlen < 6) {
        return false;
    }
#if RAW_BUFFER_LENGTH <= 254        // saves around 75 bytes program space and speeds up ISR
    uint8_t i;
#else
    uint16_t i;
#endif
    for (i = 1; (i + 2) < decodedIRData.rawDataPtr->rawlen; i++) {
        uint8_t value = compare(decodedIRData.rawDataPtr->rawbuf[i], decodedIRData.rawDataPtr->rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    decodedIRData.decodedRawData = hash;
    decodedIRData.numberOfBits = 32;
    decodedIRData.protocol = UNKNOWN;

    return true;
}

#  if !defined(NO_LEGACY_COMPATIBILITY)
bool IRrecv::decodeHashOld(decode_results *aResults) {
    long hash = FNV_BASIS_32;

// Require at least 6 samples to prevent triggering on noise
    if (aResults->rawlen < 6) {
        return false;
    }

    for (unsigned int i = 1; (i + 2) < aResults->rawlen; i++) {
        uint8_t value = compare(aResults->rawbuf[i], aResults->rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    aResults->value = hash;
    aResults->bits = 32;
    aResults->decode_type = UNKNOWN;
    decodedIRData.protocol = UNKNOWN;

    return true;
}
#  endif
#endif // DECODE_HASH

/**********************************************************************************************************************
 * Match functions
 **********************************************************************************************************************/
/**
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

/**
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

/**
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

/**
 * Getter function for MARK_EXCESS_MICROS
 */
int getMarkExcessMicros() {
    return MARK_EXCESS_MICROS;
}

/*
 * Check if protocol is not detected and detected space between two transmissions
 * is smaller than known value for protocols (Sony with around 24 ms)
 */
void CheckForRecordGapsMicros(Print *aSerial, IRData *aIRDataPtr) {
    /*
     * Check if protocol is not detected and detected space between two transmissions
     * is smaller than known value for protocols (Sony with around 24 ms)
     */
    if (aIRDataPtr->protocol <= PULSE_WIDTH && aIRDataPtr->rawDataPtr->rawbuf[0] < RECORD_GAP_MICROS_WARNING_THRESHOLD) {
        aSerial->println();
        aSerial->print(F("Space between two detected transmission is greater than "));
        aSerial->print(RECORD_GAP_MICROS);
        aSerial->print(F(" but smaller than the minimal gap of "));
        aSerial->print(RECORD_GAP_MICROS_WARNING_THRESHOLD);
        aSerial->println(F(" known for a protocol."));
        aSerial->println(F("If you get unexpected results, try to increase the RECORD_GAP_MICROS in IRremote.h."));
        aSerial->println();
    }
}

/**********************************************************************************************************************
 * Print functions
 * Since a library should not allocate the "Serial" object, all functions require a pointer to a Print object.
 **********************************************************************************************************************/
/**
 * Internal function to print decoded result and flags in one line.
 * Ends with println().
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 */
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

            if (aIRDataPtr->flags & IRDATA_FLAGS_IS_MSB_FIRST) {
                aSerial->println(F(" MSB first"));
            } else {
                aSerial->println(F(" LSB first"));
            }

        } else {
            aSerial->println();
        }

        CheckForRecordGapsMicros(aSerial, aIRDataPtr);
    }
}

/**
 * Function to print values and flags of IrReceiver.decodedIRData in one line.
 * Ends with println().
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 */
void IRrecv::printIRResultShort(Print *aSerial) {
    ::printIRResultShort(aSerial, &decodedIRData, decodedIRData.rawDataPtr->rawbuf[0]);
}

/**
 * Function to print protocol number, address, command, raw data and repeat flag of IrReceiver.decodedIRData in one short line.
 * Does not print a Newline / does not end with println().
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 */
void IRrecv::printIRResultMinimal(Print *aSerial) {
    aSerial->print(F("P="));
    aSerial->print(decodedIRData.protocol);
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
 * Dump out the timings in IrReceiver.decodedIRData.rawDataPtr->rawbuf[] array 8 values per line.
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
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
#if RAW_BUFFER_LENGTH <= 254        // saves around 75 bytes program space and speeds up ISR
    uint8_t i;
#else
    uint16_t i;
#endif
    for (i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
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
 * Dump out the IrReceiver.decodedIRData.rawDataPtr->rawbuf[] to be used as C definition for sendRaw().
 *
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding!
 * Print ticks in 8 bit format to save space.
 * Maximum is 255*50 microseconds = 12750 microseconds = 12.75 ms, which hardly ever occurs inside an IR sequence.
 * Recording of IRremote anyway stops at a gap of RECORD_GAP_MICROS (5 ms).
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 * @param aOutputMicrosecondsInsteadOfTicks Output the (rawbuf_values * MICROS_PER_TICK) for better readability.
 */
void IRrecv::compensateAndPrintIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {
// Start declaration
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->print(F("uint16_t "));         // variable type
        aSerial->print(F("rawData["));          // array name
    } else {
        aSerial->print(F("uint8_t "));          // variable type
        aSerial->print(F("rawTicks["));         // array name
    }

    aSerial->print(decodedIRData.rawDataPtr->rawlen - 1, DEC);    // array size
    aSerial->print(F("] = {"));    // Start declaration

// Dump data
#if RAW_BUFFER_LENGTH <= 254        // saves around 75 bytes program space and speeds up ISR
    uint8_t i;
#else
    uint16_t i;
#endif
    for (i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
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
 * Store the decodedIRData to be used for sendRaw().
 *
 * Compensate received values by MARK_EXCESS_MICROS, like it is done for decoding and store it in an array provided.
 *
 * Maximum for uint8_t is 255*50 microseconds = 12750 microseconds = 12.75 ms, which hardly ever occurs inside an IR sequence.
 * Recording of IRremote anyway stops at a gap of RECORD_GAP_MICROS (5 ms).
 * @param aArrayPtr Address of an array provided by the caller.
 */
void IRrecv::compensateAndStoreIRResultInArray(uint8_t *aArrayPtr) {

// Store data, skip leading space#
#if RAW_BUFFER_LENGTH <= 254        // saves around 75 bytes program space and speeds up ISR
    uint8_t i;
#else
    uint16_t i;
#endif
    for (i = 1; i < decodedIRData.rawDataPtr->rawlen; i++) {
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

/**
 * Print results as C variables to be used for sendXXX()
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 */
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

const __FlashStringHelper* getProtocolString(decode_type_t aProtocol) {
    switch (aProtocol) {
    default:
    case UNKNOWN:
        return (F("UNKNOWN"));
        break;
    case DENON:
        return (F("DENON"));
        break;
    case SHARP:
        return (F("SHARP"));
        break;
    case JVC:
        return (F("JVC"));
        break;
    case LG:
        return (F("LG"));
        break;
    case LG2:
        return (F("LG2"));
        break;
    case NEC:
        return (F("NEC"));
        break;
    case PANASONIC:
        return (F("PANASONIC"));
        break;
    case KASEIKYO:
        return (F("KASEIKYO"));
        break;
    case KASEIKYO_DENON:
        return (F("KASEIKYO_DENON"));
        break;
    case KASEIKYO_SHARP:
        return (F("KASEIKYO_SHARP"));
        break;
    case KASEIKYO_JVC:
        return (F("KASEIKYO_JVC"));
        break;
    case KASEIKYO_MITSUBISHI:
        return (F("KASEIKYO_MITSUBISHI"));
        break;
    case RC5:
        return (F("RC5"));
        break;
    case RC6:
        return (F("RC6"));
        break;
    case SAMSUNG:
        return (F("SAMSUNG"));
        break;
    case SONY:
        return (F("SONY"));
        break;
    case ONKYO:
        return (F("ONKYO"));
        break;
    case APPLE:
        return (F("APPLE"));
        break;
    case PULSE_DISTANCE:
        return (F("PULSE_DISTANCE"));
        break;
    case PULSE_WIDTH:
        return (F("PULSE_WIDTH"));
        break;
#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    case BOSEWAVE:
        return (F("BOSEWAVE"));
        break;
    case LEGO_PF:
        return (F("LEGO_PF"));
        break;
    case MAGIQUEST:
        return (F("MAGIQUEST"));
        break;
    case WHYNTER:
        return (F("WHYNTER"));
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
 * 15 pushs, 1 in, 1 eor before start of code = 2 us @16MHz + * 7.2 us computation time (6us idle time) + * pop + reti = 2.25 us @16MHz => 10.3 to 11.5 us @16MHz
 * With portInputRegister and mask and Feedback LED code commented
 * 9 pushs, 1 in, 1 eor before start of code = 1.25 us @16MHz + * 2.25 us computation time + * pop + reti = 1.5 us @16MHz => 5 us @16MHz
 * => Minimal CPU frequency is 4 MHz
 *
 **********************************************************************************************************************/
//#define IR_MEASURE_TIMING
//#define IR_TIMING_TEST_PIN 7 // do not forget to execute: "pinMode(IR_TIMING_TEST_PIN, OUTPUT);" if activated by line above
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
    uint8_t tIRInputLevel = *irparams.IRReceivePinPortInputRegister & irparams.IRReceivePinMask;
#else
    uint8_t tIRInputLevel = (uint8_t) digitalRead(irparams.IRReceivePin);
#endif

    /*
     * Increase TickCounter and clip it at maximum 0xFFFF / 3.2 seconds at 50 us ticks
     */
    if (irparams.TickCounterForISR < 0xFFFF) {
        irparams.TickCounterForISR++;  // One more 50uS tick
    }

    /*
     * Due to a ESP32 compiler bug https://github.com/espressif/esp-idf/issues/1552 no switch statements are possible for ESP32
     * So we change the code to if / else if
     */
//    switch (irparams.StateForISR) {
//......................................................................
    if (irparams.StateForISR == IR_REC_STATE_IDLE) { // In the middle of a gap or just resumed (and maybe in the middle of a transmission
        if (tIRInputLevel == INPUT_MARK) {
            // check if we did not start in the middle of a transmission by checking the minimum length of leading space
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
            } // otherwise stay in idle state
            irparams.TickCounterForISR = 0;// reset counter in both cases
        }

    } else if (irparams.StateForISR == IR_REC_STATE_MARK) {  // Timing mark
        if (tIRInputLevel != INPUT_MARK) {   // Mark ended; Record time
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
//            digitalWriteFast(IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
            irparams.rawbuf[irparams.rawlen++] = irparams.TickCounterForISR;
            irparams.StateForISR = IR_REC_STATE_SPACE;
            irparams.TickCounterForISR = 0;
        }

    } else if (irparams.StateForISR == IR_REC_STATE_SPACE) {  // Timing space
        if (tIRInputLevel == INPUT_MARK) {  // Space just ended; Record time
            if (irparams.rawlen >= RAW_BUFFER_LENGTH) {
                // Flag up a read OverflowFlag; Stop the state machine
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
        if (tIRInputLevel == INPUT_MARK) {
            // Reset gap TickCounterForISR, to prepare for detection if we are in the middle of a transmission after call of resume()
            irparams.TickCounterForISR = 0;
        }
    }

#if !defined(DISABLE_LED_FEEDBACK_FOR_RECEIVE)
    if (FeedbackLEDControl.LedFeedbackEnabled) {
        setFeedbackLED(tIRInputLevel == INPUT_MARK);
    }
#endif

#ifdef IR_MEASURE_TIMING
    digitalWriteFast(IR_TIMING_TEST_PIN, LOW); // 2 clock cycles
#endif
}

#if !defined(NO_LEGACY_COMPATIBILITY)
/**********************************************************************************************************************
 * The DEPRECATED decode function with parameter aResults ONLY for backwards compatibility!
 * This function calls the old MSB first decoders and fills only the 3 variables:
 * aResults->value
 * aResults->bits
 * aResults->decode_type
 **********************************************************************************************************************/
bool IRrecv::decode(decode_results *aResults) {
    static bool sDeprecationMessageSent = false;

    if (irparams.StateForISR != IR_REC_STATE_STOP) {
        return false;
    }

    if (!sDeprecationMessageSent) {
        Serial.println(
                "The function decode(&results)) is deprecated and may not work as expected! Just use decode() without a parameter and IrReceiver.decodedIRData.<fieldname> .");
        sDeprecationMessageSent = true;
    }

    // copy for usage by legacy programs
    aResults->rawbuf = irparams.rawbuf;
    aResults->rawlen = irparams.rawlen;
    if (irparams.OverflowFlag) {
        // Copy overflow flag to decodedIRData.flags
        irparams.OverflowFlag = false;
        irparams.rawlen = 0; // otherwise we have OverflowFlag again at next ISR call
        DEBUG_PRINTLN("Overflow happened");
    }
    aResults->overflow = irparams.OverflowFlag;
    aResults->value = 0;

    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST; // for print

#if defined(DECODE_NEC)
    DEBUG_PRINTLN("Attempting old NEC decode");
    if (decodeNECMSB(aResults)) {
        return true ;
    }
#endif

#if defined(DECODE_SONY)
    DEBUG_PRINTLN("Attempting old Sony decode");
    if (decodeSonyMSB(aResults))  {
        return true ;
    }
#endif

//#if defined(DECODE_MITSUBISHI)
//    DEBUG_PRINTLN("Attempting Mitsubishi decode");
//    if (decodeMitsubishi(results))  return true ;
//#endif

#if defined(DECODE_RC5)
    DEBUG_PRINTLN("Attempting RC5 decode");
    if (decodeRC5()) {
        aResults->bits = decodedIRData.numberOfBits;
        aResults->value = decodedIRData.decodedRawData;
        aResults->decode_type = RC5;

        return true ;
    }
#endif

#if defined(DECODE_RC6)
    DEBUG_PRINTLN("Attempting RC6 decode");
    if (decodeRC6())  {
        aResults->bits = decodedIRData.numberOfBits;
        aResults->value = decodedIRData.decodedRawData;
        aResults->decode_type = RC6;
        return true ;
    }
#endif

#if defined( DECODE_PANASONIC)
    DEBUG_PRINTLN("Attempting old Panasonic decode");
    if (decodePanasonicMSB(aResults)) {
        return true ;
    }
#endif

#if defined(DECODE_LG)
    DEBUG_PRINTLN("Attempting old LG decode");
    if (decodeLGMSB(aResults)) { return true ;}
#endif

#if defined(DECODE_JVC)
    DEBUG_PRINTLN("Attempting old JVC decode");
    if (decodeJVCMSB(aResults)) {
        return true ;
    }
#endif

#if defined(DECODE_SAMSUNG)
    DEBUG_PRINTLN("Attempting old SAMSUNG decode");
    if (decodeSAMSUNG(aResults)) {
        return true ;
    }
#endif

//#if defined(DECODE_WHYNTER)
//    DEBUG_PRINTLN("Attempting Whynter decode");
//    if (decodeWhynter(results))  return true ;
//#endif

#if defined(DECODE_DENON)
    DEBUG_PRINTLN("Attempting old Denon decode");
    if (decodeDenonOld(aResults)) {
        return true ;
    }
#endif

//#if defined(DECODE_LEGO_PF)
//    DEBUG_PRINTLN("Attempting Lego Power Functions");
//    if (decodeLegoPowerFunctions(results))  return true ;
//#endif

    // decodeHash returns a hash on any input.
    // Thus, it needs to be last in the list.
    // If you add any decodes, add them before this.
    if (decodeHashOld(aResults)) {
        return true;
    }
    // Throw away and start over
    resume();
    return false;
}
#endif

/** @}*/
#endif // #ifndef IR_RECEIVE_HPP
#pragma once
