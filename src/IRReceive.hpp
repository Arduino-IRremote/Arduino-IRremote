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
#ifndef _IR_RECEIVE_HPP
#define _IR_RECEIVE_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
//#define LOCAL_DEBUG //
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

#if defined(TRACE) && !defined(LOCAL_TRACE)
#define LOCAL_TRACE
#else
//#define LOCAL_TRACE // This enables debug output only for this file
#endif
/*
 * Low level hardware timing measurement
 */
//#define _IR_MEASURE_TIMING // for ISR
//#define _IR_TIMING_TEST_PIN 7 // "pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);" is executed at start()
//
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
#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, DO_NOT_ENABLE_LED_FEEDBACK);
#endif
}

IRrecv::IRrecv(uint_fast8_t aReceivePin) {
    decodedIRData.rawDataPtr = &irparams; // for decodePulseDistanceData() etc.
    setReceivePin(aReceivePin);
#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, DO_NOT_ENABLE_LED_FEEDBACK);
#endif
}

/**
 * Instantiate the IRrecv class. Multiple instantiation is not supported.
 * @param aReceivePin Arduino pin to use, where a demodulating IR receiver is connected.
 * @param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
IRrecv::IRrecv(uint_fast8_t aReceivePin, uint_fast8_t aFeedbackLEDPin) {
    decodedIRData.rawDataPtr = &irparams; // for decodePulseDistanceData() etc.
    setReceivePin(aReceivePin);
#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(aFeedbackLEDPin, DO_NOT_ENABLE_LED_FEEDBACK);
#else
    (void) aFeedbackLEDPin;
#endif
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
#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif
void IRReceiveTimerInterruptHandler() {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
// 7 - 8.5 us for ISR body (without pushes and pops) for ATmega328 @16MHz

#if defined(TIMER_REQUIRES_RESET_INTR_PENDING)
    timerResetInterruptPending(); // reset TickCounterForISR interrupt flag if required (currently only for Teensy and ATmega4809)
#endif

// Read if IR Receiver -> SPACE [xmt LED off] or a MARK [xmt LED on]
#if defined(__AVR__)
    uint8_t tIRInputLevel = *irparams.IRReceivePinPortInputRegister & irparams.IRReceivePinMask;
#else
    uint_fast8_t tIRInputLevel = (uint_fast8_t) digitalReadFast(irparams.IRReceivePin);
#endif

    /*
     * Increase TickCounter and clip it at maximum 0xFFFF / 3.2 seconds at 50 us ticks
     */
    if (irparams.TickCounterForISR < UINT16_MAX) {
        irparams.TickCounterForISR++;  // One more 50uS tick
    }

    /*
     * Due to a ESP32 compiler bug https://github.com/espressif/esp-idf/issues/1552 no switch statements are possible for ESP32
     * So we change the code to if / else if
     */
//    switch (irparams.StateForISR) {
//
    if (irparams.StateForISR == IR_REC_STATE_IDLE) {
        /*
         * Here we are just resumed and maybe in the middle of a transmission
         */
        if (tIRInputLevel == INPUT_MARK) {
            // check if we did not start in the middle of a transmission by checking the minimum length of leading space
            if (irparams.TickCounterForISR > RECORD_GAP_TICKS) {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
//                digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
                /*
                 * Gap between two transmissions just ended; Record gap duration + start recording transmission
                 * Initialize all state machine variables
                 */
                irparams.OverflowFlag = false;
//                irparams.rawbuf[0] = irparams.TickCounterForISR;
                irparams.initialGapTicks = irparams.TickCounterForISR; // Enabling 8 bit buffer since 4.4
                irparams.rawlen = 1;
                irparams.StateForISR = IR_REC_STATE_MARK;
            } // otherwise stay in idle state
            irparams.TickCounterForISR = 0; // reset counter in both cases
        }

    } else if (irparams.StateForISR == IR_REC_STATE_MARK) {  // Timing mark
        if (tIRInputLevel != INPUT_MARK) {
            /*
             * Mark ended here. Record mark time in rawbuf array
             */
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
//            digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
            irparams.rawbuf[irparams.rawlen++] = irparams.TickCounterForISR; // record mark
            irparams.StateForISR = IR_REC_STATE_SPACE;
            irparams.TickCounterForISR = 0; // This resets the tick counter also at end of frame :-)
        }

    } else if (irparams.StateForISR == IR_REC_STATE_SPACE) {  // Timing space
        if (tIRInputLevel == INPUT_MARK) {
            /*
             * Space ended here. Check for overflow and record space time in rawbuf array
             */
            if (irparams.rawlen >= RAW_BUFFER_LENGTH) {
                // Flag up a read OverflowFlag; Stop the state machine
                irparams.OverflowFlag = true;
                irparams.StateForISR = IR_REC_STATE_STOP;
#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
                /*
                 * Call callback if registered (not NULL)
                 */
                if (irparams.ReceiveCompleteCallbackFunction != NULL) {
                    irparams.ReceiveCompleteCallbackFunction();
                }
#endif
            } else {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
//                digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
                irparams.rawbuf[irparams.rawlen++] = irparams.TickCounterForISR; // record space
                irparams.StateForISR = IR_REC_STATE_MARK;
            }
            irparams.TickCounterForISR = 0;

        } else if (irparams.TickCounterForISR > RECORD_GAP_TICKS) {
            /*
             * Maximum space duration reached here.
             * Current code is ready for processing!
             * We received a long space, which indicates gap between codes.
             * Switch to IR_REC_STATE_STOP
             * Don't reset TickCounterForISR; keep counting width of next leading space
             */
            /*
             * These 2 variables allow to call resume() directly after decode.
             * After resume(), decodedIRData.rawDataPtr->initialGapTicks and decodedIRData.rawDataPtr->rawlen are
             * the first variables, which are overwritten by the next received frame.
             * since 4.3.0.
             */
            IrReceiver.decodedIRData.initialGapTicks = irparams.initialGapTicks;
            IrReceiver.decodedIRData.rawlen = irparams.rawlen;
            irparams.StateForISR = IR_REC_STATE_STOP;
#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
            /*
             * Call callback if registered (not NULL)
             */
            if (irparams.ReceiveCompleteCallbackFunction != NULL) {
                irparams.ReceiveCompleteCallbackFunction();
            }
#endif
        }
    } else if (irparams.StateForISR == IR_REC_STATE_STOP) {
        /*
         * Complete command received
         * stay here until resume() is called, which switches state to IR_REC_STATE_IDLE
         */
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
//        digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
        if (tIRInputLevel == INPUT_MARK) {
            // Reset gap TickCounterForISR, to prepare for detection if we are in the middle of a transmission after call of resume()
            irparams.TickCounterForISR = 0;
        }
    }

#if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_RECEIVE) {
        setFeedbackLED(tIRInputLevel == INPUT_MARK);
    }
#endif

#ifdef _IR_MEASURE_TIMING
    digitalWriteFast(_IR_TIMING_TEST_PIN, LOW); // 2 clock cycles
#endif

}

/*
 * The ISR, which calls the interrupt handler
 */
#if defined(TIMER_INTR_NAME) || defined(ISR)
#  if defined(TIMER_INTR_NAME)
ISR (TIMER_INTR_NAME) // for ISR definitions
#  elif defined(ISR)
ISR()
// for functions definitions which are called by separate (board specific) ISR
#  endif
{
    IRReceiveTimerInterruptHandler();
}
#endif

/**********************************************************************************************************************
 * Stream like API
 **********************************************************************************************************************/
/**
 * Initializes the receive and feedback pin
 * @param aReceivePin The Arduino pin number, where a demodulating IR receiver is connected.
 * @param aEnableLEDFeedback if true / ENABLE_LED_FEEDBACK, then let the feedback led blink on receiving IR signal
 * @param aFeedbackLEDPin if 0 / USE_DEFAULT_FEEDBACK_LED_PIN, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRrecv::begin(uint_fast8_t aReceivePin, bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin) {

    setReceivePin(aReceivePin);
#if !defined(NO_LED_FEEDBACK_CODE)
    uint_fast8_t tEnableLEDFeedback = DO_NOT_ENABLE_LED_FEEDBACK;
    if (aEnableLEDFeedback) {
        tEnableLEDFeedback = LED_FEEDBACK_ENABLED_FOR_RECEIVE;
    }
    setLEDFeedback(aFeedbackLEDPin, tEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif

#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif
    start();
}

/**
 * Sets / changes the receiver pin number
 */
void IRrecv::setReceivePin(uint_fast8_t aReceivePinNumber) {
    irparams.IRReceivePin = aReceivePinNumber;
#if defined(__AVR__)
#  if defined(__digitalPinToBit)
    if (__builtin_constant_p(aReceivePinNumber)) {
        irparams.IRReceivePinMask = 1UL << (__digitalPinToBit(aReceivePinNumber));
    } else {
        irparams.IRReceivePinMask = digitalPinToBitMask(aReceivePinNumber); // requires 10 bytes PGM, even if not referenced (?because it is assembler code?)
    }
#  else
    irparams.IRReceivePinMask = digitalPinToBitMask(aReceivePinNumber); // requires 10 bytes PGM, even if not referenced (?because it is assembler code?)
#  endif
#  if defined(__digitalPinToPINReg)
    /*
     * This code is 54 bytes smaller, if aReceivePinNumber is a constant :-), but 38 byte longer if it is not constant (,which is not likely).
     */
    if (__builtin_constant_p(aReceivePinNumber)) {
        irparams.IRReceivePinPortInputRegister = __digitalPinToPINReg(aReceivePinNumber);
    } else {
        irparams.IRReceivePinPortInputRegister = portInputRegister(digitalPinToPort(aReceivePinNumber)); // requires 44 bytes PGM, even if not referenced
    }
#  else
    irparams.IRReceivePinPortInputRegister = portInputRegister(digitalPinToPort(aReceivePinNumber)); // requires 44 bytes PGM, even if not referenced
#  endif
#endif
    // Set pin mode once. pinModeFast makes no difference if used, but saves 224 if not referenced :-(
    pinModeFast(aReceivePinNumber, INPUT); // Seems to be at least required by ESP32
}

#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
/**
 * Sets the function to call if a protocol message has arrived
 */
void IRrecv::registerReceiveCompleteCallback(void (*aReceiveCompleteCallbackFunction)(void)) {
    irparams.ReceiveCompleteCallbackFunction = aReceiveCompleteCallbackFunction;
}
#endif

/**
 * Start the receiving process.
 * This configures the timer and the state machine for IR reception
 * and enables the receive sample timer interrupt which consumes a small amount of CPU every 50 us.
 */
void IRrecv::start() {

    // Setup for cyclic 50 us interrupt
    timerConfigForReceive(); // no interrupts enabled here!

    // Initialize state machine state
    resume();

    // Timer interrupt is enabled after state machine reset
    timerEnableReceiveInterrupt(); // Enables the receive sample timer interrupt which consumes a small amount of CPU every 50 us.
#ifdef _IR_MEASURE_TIMING
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif
}

/*
 * Do not resume() reading of IR data
 */
void IRrecv::restartTimer() {
    // Setup for cyclic 50 us interrupt
    timerConfigForReceive(); // no interrupts enabled here!
    // Timer interrupt is enabled after state machine reset
    timerEnableReceiveInterrupt(); // Enables the receive sample timer interrupt which consumes a small amount of CPU every 50 us.
#ifdef _IR_MEASURE_TIMING
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif
}
/**
 * Alias for start().
 */
void IRrecv::enableIRIn() {
    start();
}

/**
 * Configures the timer and the state machine for IR reception.
 * The tick counter value is already at 100 when decode() gets true, because of the 5000 us minimal gap defined in RECORD_GAP_MICROS.
 * @param aMicrosecondsToAddToGapCounter To compensate for the amount of microseconds the timer was stopped / disabled.
 */
void IRrecv::start(uint32_t aMicrosecondsToAddToGapCounter) {
    irparams.TickCounterForISR += aMicrosecondsToAddToGapCounter / MICROS_PER_TICK;
    start();
}
void IRrecv::restartTimer(uint32_t aMicrosecondsToAddToGapCounter) {
    irparams.TickCounterForISR += aMicrosecondsToAddToGapCounter / MICROS_PER_TICK;
    restartTimer();
}
void IRrecv::startWithTicksToAdd(uint16_t aTicksToAddToGapCounter) {
    irparams.TickCounterForISR += aTicksToAddToGapCounter;
    start();
}
void IRrecv::restartTimerWithTicksToAdd(uint16_t aTicksToAddToGapCounter) {
    irparams.TickCounterForISR += aTicksToAddToGapCounter;
    restartTimer();
}

void IRrecv::addTicksToInternalTickCounter(uint16_t aTicksToAddToInternalTickCounter) {
    irparams.TickCounterForISR += aTicksToAddToInternalTickCounter;
}

void IRrecv::addMicrosToInternalTickCounter(uint16_t aMicrosecondsToAddToInternalTickCounter) {
    irparams.TickCounterForISR += aMicrosecondsToAddToInternalTickCounter / MICROS_PER_TICK;
}
/**
 * Restarts receiver after send. Is a NOP if sending does not require a timer.
 */
void IRrecv::restartAfterSend() {
#if defined(SEND_PWM_BY_TIMER) && !defined(SEND_PWM_DOES_NOT_USE_RECEIVE_TIMER)
    start();
#endif
}

/**
 * Disables the timer for IR reception.
 */
void IRrecv::stop() {
    timerDisableReceiveInterrupt();
}

void IRrecv::stopTimer() {
    timerDisableReceiveInterrupt();
}
/**
 * Alias for stop().
 */
void IRrecv::disableIRIn() {
    stop();
}
/**
 * Alias for stop().
 */
void IRrecv::end() {
    stop();
}

/**
 * Returns status of reception
 * @return true if no reception is on-going.
 */
bool IRrecv::isIdle() {
    return (irparams.StateForISR == IR_REC_STATE_IDLE || irparams.StateForISR == IR_REC_STATE_STOP) ? true : false;
}

/**
 * Restart the ISR (Interrupt Service Routine) state machine, to enable receiving of the next IR frame.
 * Internal counting of gap timing is independent of StateForISR and therefore independent of call time of resume().
 */
void IRrecv::resume() {
    // This check allows to call resume at arbitrary places or more than once
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
        decodedIRData.flags = IRDATA_FLAGS_WAS_OVERFLOW;
#if defined(LOCAL_DEBUG)
        Serial.print(F("Overflow happened, try to increase the \"RAW_BUFFER_LENGTH\" value of "));
        Serial.print(RAW_BUFFER_LENGTH);
        Serial.println(F(" with #define RAW_BUFFER_LENGTH=<biggerValue>"));
#endif

    } else {
        decodedIRData.flags = IRDATA_FLAGS_EMPTY;
        // save last protocol, command and address for repeat handling (where they are compared or copied back :-))
        lastDecodedProtocol = decodedIRData.protocol; // repeat patterns can be equal between protocols (e.g. NEC, Samsung and LG), so we must keep the original one
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
 * The set of decoders used is determined by active definitions of the DECODE_<PROTOCOL> macros.
 * Results of decoding are stored in IrReceiver.decodedIRData.* like e.g. IrReceiver.decodedIRData.command.
 * @return false if no IR receiver data available, true if data available.
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

#if defined(DECODE_NEC) || defined(DECODE_ONKYO)
    IR_TRACE_PRINTLN(F("Attempting NEC/Onkyo decode"));
    if (decodeNEC()) {
        return true;
    }
#endif

#if defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)
    IR_TRACE_PRINTLN(F("Attempting Panasonic/Kaseikyo decode"));
    if (decodeKaseikyo()) {
        return true;
    }
#endif

#if defined(DECODE_DENON)
    IR_TRACE_PRINTLN(F("Attempting Denon/Sharp decode"));
    if (decodeDenon()) {
        return true;
    }
#endif

#if defined(DECODE_SONY)
    IR_TRACE_PRINTLN(F("Attempting Sony decode"));
    if (decodeSony()) {
        return true;
    }
#endif

#if defined(DECODE_RC5)
    IR_TRACE_PRINTLN(F("Attempting RC5 decode"));
    if (decodeRC5()) {
        return true;
    }
#endif

#if defined(DECODE_RC6)
    IR_TRACE_PRINTLN(F("Attempting RC6 decode"));
    if (decodeRC6()) {
        return true;
    }
#endif

#if defined(DECODE_LG)
    IR_TRACE_PRINTLN(F("Attempting LG decode"));
    if (decodeLG()) {
        return true;
    }
#endif

#if defined(DECODE_JVC)
    IR_TRACE_PRINTLN(F("Attempting JVC decode"));
    if (decodeJVC()) {
        return true;
    }
#endif

#if defined(DECODE_SAMSUNG)
    IR_TRACE_PRINTLN(F("Attempting Samsung decode"));
    if (decodeSamsung()) {
        return true;
    }
#endif
    /*
     * Start of the exotic protocols
     */

#if defined(DECODE_BEO)
    IR_TRACE_PRINTLN(F("Attempting Bang & Olufsen decode"));
    if (decodeBangOlufsen()) {
        return true;
    }
#endif

#if defined(DECODE_FAST)
    IR_TRACE_PRINTLN(F("Attempting FAST decode"));
    if (decodeFAST()) {
        return true;
    }
#endif

#if defined(DECODE_WHYNTER)
    IR_TRACE_PRINTLN(F("Attempting Whynter decode"));
    if (decodeWhynter()) {
        return true;
    }
#endif

#if defined(DECODE_LEGO_PF)
    IR_TRACE_PRINTLN(F("Attempting Lego Power Functions"));
    if (decodeLegoPowerFunctions()) {
        return true;
    }
#endif

#if defined(DECODE_BOSEWAVE)
    IR_TRACE_PRINTLN(F("Attempting Bosewave  decode"));
    if (decodeBoseWave()) {
        return true;
    }
#endif

#if defined(DECODE_MAGIQUEST)
    IR_TRACE_PRINTLN(F("Attempting MagiQuest decode"));
    if (decodeMagiQuest()) {
        return true;
    }
#endif

    /*
     * Try the universal decoder for pulse distance protocols
     */
#if defined(DECODE_DISTANCE_WIDTH)
    IR_TRACE_PRINTLN(F("Attempting universal Distance Width decode"));
    if (decodeDistanceWidth()) {
        return true;
    }
#endif

    /*
     * Last resort is the universal hash decode which always return true
     */
#if defined(DECODE_HASH)
    IR_TRACE_PRINTLN(F("Hash decode"));
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
 * Decode pulse distance width protocols. We only check the mark or space length of a 1, otherwise we always assume a 0!
 *
 * We can have the following protocol timings
 * PULSE_DISTANCE:       Pause/spaces have different length and determine the bit value, longer space is 1. Pulses/marks can be constant, like NEC.
 * PULSE_WIDTH:          Pulses/marks have different length and determine the bit value, longer mark is 1. Pause/spaces can be constant, like Sony.
 * PULSE_DISTANCE_WIDTH: Pulses/marks and pause/spaces have different length, often the bit length is constant, like MagiQuest. Can be decoded by PULSE_DISTANCE decoder.
 *
 * Input is     IrReceiver.decodedIRData.rawDataPtr->rawbuf[]
 * Output is    IrReceiver.decodedIRData.decodedRawData
 *
 * Assume PULSE_DISTANCE if aOneMarkMicros == aZeroMarkMicros
 *
 * @param   aNumberOfBits       Number of bits to decode from decodedIRData.rawDataPtr->rawbuf[] array.
 * @param   aStartOffset        Offset in decodedIRData.rawDataPtr->rawbuf[] to start decoding. Must point to a mark.
 * @param   aOneMarkMicros      Checked if PULSE_WIDTH
 * @param   aZeroMarkMicros     Required for deciding if we have PULSE_DISTANCE.
 * @param   aOneSpaceMicros     Checked if PULSE_DISTANCE.
 * @param   aMSBfirst           If true send Most Significant Bit first, else send Least Significant Bit (lowest bit) first.
 * @return  true                If decoding was successful
 */
bool IRrecv::decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
        uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, bool aMSBfirst) {

    auto *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];

    bool isPulseDistanceProtocol = (aOneMarkMicros == aZeroMarkMicros); // If true, we check aOneSpaceMicros -> pulse distance protocol

    IRRawDataType tDecodedData = 0; // For MSB first tDecodedData is shifted left each loop
    IRRawDataType tMask = 1UL; // Mask is only used for LSB first

    for (uint_fast8_t i = aNumberOfBits; i > 0; i--) {
        // get one mark and space pair
        unsigned int tMarkTicks;
        unsigned int tSpaceTicks;
        bool tBitValue;

        if (isPulseDistanceProtocol) {
            /*
             * PULSE_DISTANCE -including PULSE_DISTANCE_WIDTH- here.
             * !!!We only check variable length space indicating a 1 or 0!!!
             */
            tRawBufPointer++;
            tSpaceTicks = *tRawBufPointer++; // maybe buffer overflow for last bit, but we do not evaluate this value :-)
            tBitValue = matchSpace(tSpaceTicks, aOneSpaceMicros); // Check for variable length space indicating a 1 or 0

        } else {
            /*
             * PULSE_WIDTH here.
             * !!!We only check variable length mark indicating a 1 or 0!!!
             */
            tMarkTicks = *tRawBufPointer++;
            tBitValue = matchMark(tMarkTicks, aOneMarkMicros); // Check for variable length mark indicating a 1 or 0
            tRawBufPointer++;
        }

        if (aMSBfirst) {
            tDecodedData <<= 1;
        }

        if (tBitValue) {
            // It's a 1 -> set the bit
            if (aMSBfirst) {
                tDecodedData |= 1;
            } else {
                tDecodedData |= tMask;
            }
            IR_TRACE_PRINTLN(F("=> 1"));
        } else {
            // do not set the bit
            IR_TRACE_PRINTLN(F("=> 0"));
        }
        tMask <<= 1;
    }
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

/*
 * Old deprecated version with 7 parameters and unused aZeroSpaceMicros parameter
 */
bool IRrecv::decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
        uint16_t aZeroMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst) {

    (void) aZeroSpaceMicros;
    auto *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];
    bool isPulseDistanceProtocol = (aOneMarkMicros == aZeroMarkMicros); // If true, we have a constant mark -> pulse distance protocol

    IRRawDataType tDecodedData = 0; // For MSB first tDecodedData is shifted left each loop
    IRRawDataType tMask = 1UL; // Mask is only used for LSB first

    for (uint_fast8_t i = aNumberOfBits; i > 0; i--) {
        // get one mark and space pair
        unsigned int tMarkTicks;
        unsigned int tSpaceTicks;
        bool tBitValue;

        if (isPulseDistanceProtocol) {
            /*
             * Pulse distance here, it is not required to check constant mark duration (aOneMarkMicros) and zero space duration.
             */

            (void) aZeroSpaceMicros;
            tRawBufPointer++;
            tSpaceTicks = *tRawBufPointer++; // maybe buffer overflow for last bit, but we do not evaluate this value :-)
            tBitValue = matchSpace(tSpaceTicks, aOneSpaceMicros); // Check for variable length space indicating a 1 or 0
        } else {
            /*
             * Pulse width here, it is not required to check (constant) space duration and zero mark duration.
             */
            tMarkTicks = *tRawBufPointer++;
            tBitValue = matchMark(tMarkTicks, aOneMarkMicros); // Check for variable length mark indicating a 1 or 0
            tRawBufPointer++;
        }

        if (aMSBfirst) {
            tDecodedData <<= 1;
        }

        if (tBitValue) {
            // It's a 1 -> set the bit
            if (aMSBfirst) {
                tDecodedData |= 1;
            } else {
                tDecodedData |= tMask;
            }
            IR_TRACE_PRINTLN(F("=> 1"));
        } else {
            // do not set the bit
            IR_TRACE_PRINTLN(F("=> 0"));
        }
        tMask <<= 1;
    }
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

/*
 * Check for additional required characteristics of timing like length of mark for a constant mark protocol,
 * where space length determines the bit value. Requires up to 194 additional bytes of program memory.
 * Only sensible for development or very exotic requirements.
 * @param   aZeroMarkMicros     For strict checks
 * @param   aZeroSpaceMicros    For strict checks
 *
 * Not used yet
 */
bool IRrecv::decodePulseDistanceWidthDataStrict(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
        uint16_t aZeroMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst) {

    auto *tRawBufPointer = &decodedIRData.rawDataPtr->rawbuf[aStartOffset];

    bool isPulseDistanceProtocol = (aOneMarkMicros == aZeroMarkMicros); // If true, we have a constant mark -> pulse distance protocol

    IRRawDataType tDecodedData = 0; // For MSB first tDecodedData is shifted left each loop
    IRRawDataType tMask = 1UL; // Mask is only used for LSB first

    for (uint_fast8_t i = aNumberOfBits; i > 0; i--) {
        // get one mark and space pair
        unsigned int tMarkTicks;
        unsigned int tSpaceTicks;
        bool tBitValue;

        if (isPulseDistanceProtocol) {
            /*
             * PULSE_DISTANCE here, it is not required to check constant mark duration (aOneMarkMicros) and zero space duration.
             */
            tMarkTicks = *tRawBufPointer++;
            tSpaceTicks = *tRawBufPointer++; // maybe buffer overflow for last bit, but we do not evaluate this value :-)
            tBitValue = matchSpace(tSpaceTicks, aOneSpaceMicros); // Check for variable length space indicating a 1 or 0

            // Check for constant length mark
            if (!matchMark(tMarkTicks, aOneMarkMicros)) {
#if defined(LOCAL_DEBUG)
                Serial.print(F("Mark="));
                Serial.print(tMarkTicks * MICROS_PER_TICK);
                Serial.print(F(" is not "));
                Serial.print(aOneMarkMicros);
                Serial.print(F(". Index="));
                Serial.print(aNumberOfBits - i);
                Serial.print(' ');
#endif
                return false;
            }

        } else {
            /*
             * PULSE_DISTANCE -including PULSE_DISTANCE_WIDTH- here.
             * !!!We only check variable length mark indicating a 1 or 0!!!
             * It is not required to check space duration and zero mark duration.
             */
            tMarkTicks = *tRawBufPointer++;
            tBitValue = matchMark(tMarkTicks, aOneMarkMicros); // Check for variable length mark indicating a 1 or 0
            tSpaceTicks = *tRawBufPointer++; // maybe buffer overflow for last bit, but we do not evaluate this value :-)
        }

        if (aMSBfirst) {
            tDecodedData <<= 1;
        }

        if (tBitValue) {
            // It's a 1 -> set the bit
            if (aMSBfirst) {
                tDecodedData |= 1;
            } else {
                tDecodedData |= tMask;
            }
            IR_TRACE_PRINTLN(F("=> 1"));
        } else {
            /*
             * Additionally check length of tSpaceTicks parameter for PULSE_DISTANCE or tMarkTicks for PULSE_WIDTH
             * which determine a zero
             */
            if (isPulseDistanceProtocol) {
                if (!matchSpace(tSpaceTicks, aZeroSpaceMicros)) {
#if defined(LOCAL_DEBUG)
                    Serial.print(F("Space="));
                    Serial.print(tSpaceTicks * MICROS_PER_TICK);
                    Serial.print(F(" is not "));
                    Serial.print(aOneSpaceMicros);
                    Serial.print(F(" or "));
                    Serial.print(aZeroSpaceMicros);
                    Serial.print(F(". Index="));
                    Serial.print(aNumberOfBits - i);
                    Serial.print(' ');
#endif
                    return false;
                }
            } else {
                if (!matchMark(tMarkTicks, aZeroMarkMicros)) {
#if defined(LOCAL_DEBUG)
                    Serial.print(F("Mark="));
                    Serial.print(tMarkTicks * MICROS_PER_TICK);
                    Serial.print(F(" is not "));
                    Serial.print(aOneMarkMicros);
                    Serial.print(F(" or "));
                    Serial.print(aZeroMarkMicros);
                    Serial.print(F(". Index="));
                    Serial.print(aNumberOfBits - i);
                    Serial.print(' ');
#endif
                    return false;
                }
            }
            // do not set the bit
            IR_TRACE_PRINTLN(F("=> 0"));
        }
        // If we have no stop bit, assume that last space, which is not recorded, is correct, since we can not check it
        if (aZeroSpaceMicros == aOneSpaceMicros
                && tRawBufPointer < &decodedIRData.rawDataPtr->rawbuf[decodedIRData.rawDataPtr->rawlen]) {
            // Check for constant length space (of pulse width protocol) here
            if (!matchSpace(tSpaceTicks, aOneSpaceMicros)) {
#if defined(LOCAL_DEBUG)
                Serial.print(F("Space="));
                Serial.print(tSpaceTicks * MICROS_PER_TICK);
                Serial.print(F(" is not "));
                Serial.print(aOneSpaceMicros);
                Serial.print(F(". Index="));
                Serial.print(aNumberOfBits - i);
                Serial.print(' ');
#endif
                return false;
            }
        }
        tMask <<= 1;
    }
    decodedIRData.decodedRawData = tDecodedData;
    return true;
}

/**
 * Decode pulse distance protocols for PulseDistanceWidthProtocolConstants.
 * @return  true if decoding was successful
 */
bool IRrecv::decodePulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, uint_fast8_t aNumberOfBits,
        IRRawlenType aStartOffset) {

    return decodePulseDistanceWidthData(aNumberOfBits, aStartOffset, aProtocolConstants->DistanceWidthTimingInfo.OneMarkMicros,
            aProtocolConstants->DistanceWidthTimingInfo.OneSpaceMicros, aProtocolConstants->DistanceWidthTimingInfo.ZeroMarkMicros,
            aProtocolConstants->Flags);
}

/*
 * Static variables for the getBiphaselevel function
 */
uint_fast8_t sBiphaseDecodeRawbuffOffset;   // Index into raw timing array
uint16_t sBiphaseCurrentTimingIntervals; // 1, 2 or 3. Number of aBiphaseTimeUnit intervals of the current rawbuf[sBiphaseDecodeRawbuffOffset] timing.
uint_fast8_t sBiphaseUsedTimingIntervals;   // Number of already used intervals of sCurrentTimingIntervals.
uint16_t sBiphaseTimeUnit;

void IRrecv::initBiphaselevel(uint_fast8_t aRCDecodeRawbuffOffset, uint16_t aBiphaseTimeUnit) {
    sBiphaseDecodeRawbuffOffset = aRCDecodeRawbuffOffset;
    sBiphaseTimeUnit = aBiphaseTimeUnit;
    sBiphaseUsedTimingIntervals = 0;
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
uint_fast8_t IRrecv::getBiphaselevel() {
    uint_fast8_t tLevelOfCurrentInterval; // 0 (SPACE) or 1 (MARK)

    if (sBiphaseDecodeRawbuffOffset >= decodedIRData.rawlen) {
        return SPACE;  // After end of recorded buffer, assume space.
    }

    tLevelOfCurrentInterval = (sBiphaseDecodeRawbuffOffset) & 1; // on odd rawbuf offsets we have mark timings

    /*
     * Setup data if sUsedTimingIntervals is 0
     */
    if (sBiphaseUsedTimingIntervals == 0) {
        uint16_t tCurrentTimingWith = decodedIRData.rawDataPtr->rawbuf[sBiphaseDecodeRawbuffOffset];
        uint16_t tMarkExcessCorrection = (tLevelOfCurrentInterval == MARK) ? MARK_EXCESS_MICROS : -MARK_EXCESS_MICROS;

        if (matchTicks(tCurrentTimingWith, sBiphaseTimeUnit + tMarkExcessCorrection)) {
            sBiphaseCurrentTimingIntervals = 1;
        } else if (matchTicks(tCurrentTimingWith, (2 * sBiphaseTimeUnit) + tMarkExcessCorrection)) {
            sBiphaseCurrentTimingIntervals = 2;
        } else if (matchTicks(tCurrentTimingWith, (3 * sBiphaseTimeUnit) + tMarkExcessCorrection)) {
            sBiphaseCurrentTimingIntervals = 3;
        } else {
            return -1;
        }
    }

// We use another interval from tCurrentTimingIntervals
    sBiphaseUsedTimingIntervals++;

// keep track of current timing offset
    if (sBiphaseUsedTimingIntervals >= sBiphaseCurrentTimingIntervals) {
        // we have used all intervals of current timing, switch to next timing value
        sBiphaseUsedTimingIntervals = 0;
        sBiphaseDecodeRawbuffOffset++;
    }

    IR_TRACE_PRINTLN(tLevelOfCurrentInterval);

    return tLevelOfCurrentInterval;
}

/**********************************************************************************************************************
 * Internal Hash decode function
 **********************************************************************************************************************/
#define FNV_PRIME_32 16777619   ///< used for decodeHash()
#define FNV_BASIS_32 2166136261 ///< used for decodeHash()

/**
 * Compare two (tick) values for Hash decoder
 * Use a tolerance of 20% to enable e.g. 500 and 600 (NEC timing) to be equal
 * @return  0 if newval is shorter, 1 if newval is equal, and 2 if newval is longer
 */
uint_fast8_t IRrecv::compare(uint16_t oldval, uint16_t newval) {
    if (newval * 10 < oldval * 8) {
        return 0;
    }
    if (oldval * 10 < newval * 8) {
        return 2;
    }
    return 1;
}

/**
 * decodeHash - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK and SPACE signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous MARK or SPACE.
 * Hash the resulting sequence of 0's, 1's, and 2's to a 32-bit value.
 * This will give a unique value for each different code (probably), for most code systems.
 *
 * Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
 * Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 *
 * see: http://www.righto.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */
bool IRrecv::decodeHash() {
    unsigned long hash = FNV_BASIS_32; // the result is the same no matter if we use a long or unsigned long variable

// Require at least 6 samples to prevent triggering on noise
    if (decodedIRData.rawlen < 6) {
        IR_DEBUG_PRINT(F("HASH: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is less than 6"));
        return false;
    }
    for (IRRawlenType i = 1; (i + 2) < decodedIRData.rawlen; i++) {
        // Compare mark with mark and space with space
        uint_fast8_t value = compare(decodedIRData.rawDataPtr->rawbuf[i], decodedIRData.rawDataPtr->rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    decodedIRData.decodedRawData = hash;
    decodedIRData.numberOfBits = 32;
    decodedIRData.protocol = UNKNOWN;

    return true;
}

bool IRrecv::decodeHashOld(decode_results *aResults) {
    unsigned long hash = FNV_BASIS_32;

// Require at least 6 samples to prevent triggering on noise
    if (aResults->rawlen < 6) {
        return false;
    }

    for (uint8_t i = 3; i < aResults->rawlen; i++) {
        uint_fast8_t value = compare(aResults->rawbuf[i - 2], aResults->rawbuf[i]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }

    aResults->value = hash;
    aResults->bits = 32;
    aResults->decode_type = UNKNOWN;
    decodedIRData.protocol = UNKNOWN;

    return true;
}

/**********************************************************************************************************************
 * Match functions
 **********************************************************************************************************************/

/*
 * returns true if values do match
 */
bool IRrecv::checkHeader(PulseDistanceWidthProtocolConstants *aProtocolConstants) {
// Check header "mark" and "space"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], aProtocolConstants->DistanceWidthTimingInfo.HeaderMarkMicros)) {
#if defined(LOCAL_TRACE)
        Serial.print(::getProtocolString(aProtocolConstants->ProtocolIndex));
        Serial.println(F(": Header mark length is wrong"));
#endif
        return false;
    }
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[2], aProtocolConstants->DistanceWidthTimingInfo.HeaderSpaceMicros)) {
#if defined(LOCAL_TRACE)
        Serial.print(::getProtocolString(aProtocolConstants->ProtocolIndex));
        Serial.println(F(": Header space length is wrong"));
#endif
        return false;
    }
    return true;
}

/*
 * Do not check for same address and command, because it is almost not possible to press 2 different buttons on the remote within around 100 ms.
 * And if really required, it can be enabled here, or done manually in user program.
 * And we have still no RC6 toggle bit check for detecting a second press on the same button.
 */
void IRrecv::checkForRepeatSpaceTicksAndSetFlag(uint16_t aMaximumRepeatSpaceTicks) {
    if (decodedIRData.initialGapTicks < aMaximumRepeatSpaceTicks
#if defined(ENABLE_FULL_REPEAT_CHECK)
            && decodedIRData.address == lastDecodedAddress && decodedIRData.command == lastDecodedCommand /* requires around 85 bytes program space */
#endif
            ) {
        decodedIRData.flags |= IRDATA_FLAGS_IS_REPEAT;
    }
}

/**
 * Match function without compensating for marks exceeded or spaces shortened by demodulator hardware
 * @return true, if values match
 * Currently not used
 */
bool matchTicks(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
#if defined(LOCAL_TRACE)
    Serial.print(F("Testing: "));
    Serial.print(TICKS_LOW(aMatchValueMicros), DEC);
    Serial.print(F(" <= "));
    Serial.print(aMeasuredTicks, DEC);
    Serial.print(F(" <= "));
    Serial.print(TICKS_HIGH(aMatchValueMicros), DEC);
#endif
    bool passed = ((aMeasuredTicks >= TICKS_LOW(aMatchValueMicros)) && (aMeasuredTicks <= TICKS_HIGH(aMatchValueMicros)));
#if defined(LOCAL_TRACE)
    if (passed) {
        Serial.println(F(" => passed"));
    } else {
        Serial.println(F(" => FAILED"));
    }
#endif
    return passed;
}

bool MATCH(uint16_t measured_ticks, uint16_t desired_us) {
    return matchTicks(measured_ticks, desired_us);
}

/**
 * Compensate for marks exceeded by demodulator hardware
 * @return true, if values match
 */
bool matchMark(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
#if defined(LOCAL_TRACE)
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
#if defined(LOCAL_TRACE)
    if (passed) {
        Serial.println(F(" => passed"));
    } else {
        Serial.println(F(" => FAILED"));
    }
#endif
    return passed;
}

bool MATCH_MARK(uint16_t measured_ticks, uint16_t desired_us) {
    return matchMark(measured_ticks, desired_us);
}

/**
 * Compensate for spaces shortened by demodulator hardware
 * @return true, if values match
 */
bool matchSpace(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros) {
#if defined(LOCAL_TRACE)
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
#if defined(LOCAL_TRACE)
    if (passed) {
        Serial.println(F(" => passed"));
    } else {
        Serial.println(F(" => FAILED"));
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
 * @return true, if CheckForRecordGapsMicros() has printed a message, i.e. gap < 15ms (RECORD_GAP_MICROS_WARNING_THRESHOLD)
 */
bool IRrecv::checkForRecordGapsMicros(Print *aSerial) {
    /*
     * Check if protocol is not detected and detected space between two transmissions
     * is smaller than known value for protocols (Sony with around 24 ms)
     */
    if (decodedIRData.protocol <= PULSE_DISTANCE
            && decodedIRData.initialGapTicks < (RECORD_GAP_MICROS_WARNING_THRESHOLD / MICROS_PER_TICK)) {
        aSerial->println();
        aSerial->print(F("Space of "));
        aSerial->print(decodedIRData.initialGapTicks * MICROS_PER_TICK);
        aSerial->print(F(" us between two detected transmission is smaller than the minimal gap of "));
        aSerial->print(RECORD_GAP_MICROS_WARNING_THRESHOLD);
        aSerial->println(F(" us known for implemented protocols like NEC, Sony, RC% etc.."));
        aSerial->println(F("But it can be OK for some yet unsupported protocols, and especially for repeats."));
        aSerial->println(F("If you get unexpected results, try to increase the RECORD_GAP_MICROS in IRremote.h."));
        aSerial->println();
        return true;
    }
    return false;
}

/**********************************************************************************************************************
 * Print functions
 * Since a library should not allocate the "Serial" object, all functions require a pointer to a Print object.
 **********************************************************************************************************************/
void IRrecv::printActiveIRProtocols(Print *aSerial) {
// call no class function with same name
    ::printActiveIRProtocols(aSerial);
}
void printActiveIRProtocols(Print *aSerial) {
#if defined(DECODE_ONKYO)
    aSerial->print(F("Onkyo, "));
#elif defined(DECODE_NEC)
    aSerial->print(F("NEC/NEC2/Onkyo/Apple, "));
#endif
#if defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)
    aSerial->print(F("Panasonic/Kaseikyo, "));
#endif
#if defined(DECODE_DENON)
    aSerial->print(F("Denon/Sharp, "));
#endif
#if defined(DECODE_SONY)
    aSerial->print(F("Sony, "));
#endif
#if defined(DECODE_RC5)
    aSerial->print(F("RC5, "));
#endif
#if defined(DECODE_RC6)
    aSerial->print(F("RC6, "));
#endif
#if defined(DECODE_LG)
    aSerial->print(F("LG, "));
#endif
#if defined(DECODE_JVC)
    aSerial->print(F("JVC, "));
#endif
#if defined(DECODE_SAMSUNG)
    aSerial->print(F("Samsung, "));
#endif
    /*
     * Start of the exotic protocols
     */
#if defined(DECODE_BEO)
    aSerial->print(F("Bang & Olufsen, "));
#endif
#if defined(DECODE_FAST)
    aSerial->print(F("FAST, "));
#endif
#if defined(DECODE_WHYNTER)
    aSerial->print(F("Whynter, "));
#endif
#if defined(DECODE_LEGO_PF)
    aSerial->print(F("Lego Power Functions, "));
#endif
#if defined(DECODE_BOSEWAVE)
    aSerial->print(F("Bosewave, "));
#endif
#if defined(DECODE_MAGIQUEST)
    aSerial->print(F("MagiQuest, "));
#endif
#if defined(DECODE_DISTANCE_WIDTH)
    aSerial->print(F("Universal Pulse Distance Width, "));
#endif
#if defined(DECODE_HASH)
    aSerial->print(F("Hash "));
#endif
#if defined(NO_DECODER) // for sending raw only
    (void)aSerial; // to avoid compiler warnings
#endif
}

/**
 * Function to print values and flags of IrReceiver.decodedIRData in one line.
 * Ends with println().
 *
 * @param aSerial   The Print object on which to write, for Arduino you can use &Serial.
 * @param aPrintRepeatGap     If true also print the gap before repeats.
 * @param aCheckForRecordGapsMicros   If true, call CheckForRecordGapsMicros() which may do a long printout,
 *                                    which in turn may block the proper detection of repeats.*
 * @return true, if CheckForRecordGapsMicros() has printed a message, i.e. gap < 15ms (RECORD_GAP_MICROS_WARNING_THRESHOLD).
 */
bool IRrecv::printIRResultShort(Print *aSerial, bool aPrintRepeatGap, bool aCheckForRecordGapsMicros) {
// call no class function with same name
    ::printIRResultShort(aSerial, &decodedIRData, aPrintRepeatGap);
    if (aCheckForRecordGapsMicros && decodedIRData.protocol != UNKNOWN) {
        return checkForRecordGapsMicros(aSerial);
    }
    return false;
}

void IRrecv::printDistanceWidthTimingInfo(Print *aSerial, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo) {
    aSerial->print(aDistanceWidthTimingInfo->HeaderMarkMicros);
    aSerial->print(F(", "));
    aSerial->print(aDistanceWidthTimingInfo->HeaderSpaceMicros);
    aSerial->print(F(", "));
    aSerial->print(aDistanceWidthTimingInfo->OneMarkMicros);
    aSerial->print(F(", "));
    aSerial->print(aDistanceWidthTimingInfo->OneSpaceMicros);
    aSerial->print(F(", "));
    aSerial->print(aDistanceWidthTimingInfo->ZeroMarkMicros);
    aSerial->print(F(", "));
    aSerial->print(aDistanceWidthTimingInfo->ZeroSpaceMicros);
}

/*
 * Get maximum of mark ticks in rawDataPtr.
 * Skip leading start and trailing stop bit.
 */
uint8_t IRrecv::getMaximumMarkTicksFromRawData() {
    uint8_t tMaximumTick = 0;
    for (IRRawlenType i = 3; i < decodedIRData.rawlen - 2; i += 2) { // Skip leading start and trailing stop bit.
        auto tTick = decodedIRData.rawDataPtr->rawbuf[i];
        if (tMaximumTick < tTick) {
            tMaximumTick = tTick;
        }
    }
    return tMaximumTick;
}
uint8_t IRrecv::getMaximumSpaceTicksFromRawData() {
    uint8_t tMaximumTick = 0;
    for (IRRawlenType i = 4; i < decodedIRData.rawlen - 2; i += 2) { // Skip leading start and trailing stop bit.
        auto tTick = decodedIRData.rawDataPtr->rawbuf[i];
        if (tMaximumTick < tTick) {
            tMaximumTick = tTick;
        }
    }
    return tMaximumTick;
}

/*
 * The optimizing compiler internally generates this function, if getMaximumMarkTicksFromRawData() and getMaximumSpaceTicksFromRawData() is used.
 */
uint8_t IRrecv::getMaximumTicksFromRawData(bool aSearchSpaceInsteadOfMark) {
    uint8_t tMaximumTick = 0;
    IRRawlenType i;
    if (aSearchSpaceInsteadOfMark) {
        i = 4;
    } else {
        i = 3;
    }
    for (; i < decodedIRData.rawlen - 2; i += 2) { // Skip leading start and trailing stop bit.
        auto tTick = decodedIRData.rawDataPtr->rawbuf[i];
        if (tMaximumTick < tTick) {
            tMaximumTick = tTick;
        }
    }
    return tMaximumTick;
}

uint32_t IRrecv::getTotalDurationOfRawData() {
    uint16_t tSumOfDurationTicks = 0;

    for (IRRawlenType i = 1; i < decodedIRData.rawlen; i++) {
        tSumOfDurationTicks += decodedIRData.rawDataPtr->rawbuf[i];
    }
    return tSumOfDurationTicks * (uint32_t) MICROS_PER_TICK;
}

/**
 * Function to print values and flags of IrReceiver.decodedIRData in one line.
 * do not print for repeats except IRDATA_FLAGS_IS_PROTOCOL_WITH_DIFFERENT_REPEAT.
 * Ends with println().
 * !!!Attention: The result differs on a 8 bit or 32 bit platform!!!
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 */
void IRrecv::printIRSendUsage(Print *aSerial) {
    if (decodedIRData.protocol != UNKNOWN
            && ((decodedIRData.flags & (IRDATA_FLAGS_IS_AUTO_REPEAT | IRDATA_FLAGS_IS_REPEAT)) == 0x00
                    || (decodedIRData.flags & IRDATA_FLAGS_IS_PROTOCOL_WITH_DIFFERENT_REPEAT))) {
#if defined(DECODE_DISTANCE_WIDTH)
        uint_fast8_t tNumberOfArrayData = 0;
        if (decodedIRData.protocol == PULSE_DISTANCE || decodedIRData.protocol == PULSE_WIDTH) {
#  if __INT_WIDTH__ < 32
            aSerial->print(F("Send on a 8 bit platform with: "));
            tNumberOfArrayData = ((decodedIRData.numberOfBits - 1) / 32) + 1;
            if(tNumberOfArrayData > 1) {
                aSerial->println();
                aSerial->print(F("    uint32_t tRawData[]={0x"));
#  else
                aSerial->print(F("Send on a 32 bit platform with: "));
            tNumberOfArrayData = ((decodedIRData.numberOfBits - 1) / 64) + 1;
            if(tNumberOfArrayData > 1) {
                aSerial->println();
                aSerial->print(F("    uint64_t tRawData[]={0x"));
#  endif
                for (uint_fast8_t i = 0; i < tNumberOfArrayData; ++i) {
#  if (__INT_WIDTH__ < 32)
                    aSerial->print(decodedIRData.decodedRawDataArray[i], HEX);
#  else
                    PrintULL::print(aSerial, decodedIRData.decodedRawDataArray[i], HEX);
#  endif
                    if (i != tNumberOfArrayData - 1) {
                        aSerial->print(F(", 0x"));
                    }
                }
                aSerial->println(F("};"));
                aSerial->print(F("    "));
            }
        } else {
            aSerial->print(F("Send with: "));
        }
        aSerial->print(F("IrSender.send"));

#else
        aSerial->print(F("Send with: IrSender.send"));
#endif

#if defined(DECODE_DISTANCE_WIDTH)
        if (decodedIRData.protocol != PULSE_DISTANCE && decodedIRData.protocol != PULSE_WIDTH) {
#endif
        aSerial->print(getProtocolString());
        aSerial->print(F("(0x"));
#if defined(DECODE_MAGIQUEST)
            if (decodedIRData.protocol == MAGIQUEST) {
#  if (__INT_WIDTH__ < 32)
                aSerial->print(decodedIRData.decodedRawData, HEX);
#  else
                PrintULL::print(aSerial, decodedIRData.decodedRawData, HEX);
#  endif
            } else {
                aSerial->print(decodedIRData.address, HEX);
            }
#else
        /*
         * New decoders have address and command
         */
        aSerial->print(decodedIRData.address, HEX);
#endif

        aSerial->print(F(", 0x"));
        aSerial->print(decodedIRData.command, HEX);
        if (decodedIRData.protocol == SONY) {
            aSerial->print(F(", 2, "));
            aSerial->print(decodedIRData.numberOfBits);
        } else {
            aSerial->print(F(", <numberOfRepeats>"));
        }

#if defined(DECODE_DISTANCE_WIDTH)
        } else {
            /*
             * Pulse distance or pulse width here
             */
            aSerial->print("PulseDistanceWidth");
            if(tNumberOfArrayData > 1) {
                aSerial->print("FromArray(38, ");
            } else {
                aSerial->print("(38, ");
            }
            printDistanceWidthTimingInfo(aSerial, &decodedIRData.DistanceWidthTimingInfo);

            if(tNumberOfArrayData > 1) {
                aSerial->print(F(", &tRawData[0], "));
            } else {
                aSerial->print(F(", 0x"));
#  if (__INT_WIDTH__ < 32)
                aSerial->print(decodedIRData.decodedRawData, HEX);
#  else
                PrintULL::print(aSerial, decodedIRData.decodedRawData, HEX);
#  endif
                aSerial->print(F(", "));
            }
            aSerial->print(decodedIRData.numberOfBits);// aNumberOfBits
            aSerial->print(F(", PROTOCOL_IS_"));

            if (decodedIRData.flags & IRDATA_FLAGS_IS_MSB_FIRST) {
                aSerial->print('M');
            } else {
                aSerial->print('L');
            }
            aSerial->print(F("SB_FIRST, <RepeatPeriodMillis>, <numberOfRepeats>"));
        }
#endif
#if defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)
        if ((decodedIRData.flags & IRDATA_FLAGS_EXTRA_INFO) && decodedIRData.protocol == KASEIKYO) {
            aSerial->print(F(", 0x"));
            aSerial->print(decodedIRData.extra, HEX);
        }
#endif
        aSerial->print(F(");"));
        aSerial->println();
    }
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
#  if (__INT_WIDTH__ < 32)
        aSerial->print(decodedIRData.decodedRawData, HEX);
#  else
        PrintULL::print(aSerial, decodedIRData.decodedRawData, HEX);
#  endif
#endif
        aSerial->print(' ');
        aSerial->print((decodedIRData.rawlen + 1) / 2, DEC);
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
#if (__INT_WIDTH__ < 32)
        aSerial->print(decodedIRData.decodedRawData, HEX);
#else
        PrintULL::print(aSerial, decodedIRData.decodedRawData, HEX);
#endif

        if (decodedIRData.flags & (IRDATA_FLAGS_IS_AUTO_REPEAT | IRDATA_FLAGS_IS_REPEAT)) {
            aSerial->print(F(" R"));
        }
    }
}

/**
 * Dump out the timings in IrReceiver.decodedIRData.rawDataPtr->rawbuf[] array 8 values per line.
 *
 * @param aSerial The Print object on which to write, for Arduino you can use &Serial.
 * @param aOutputMicrosecondsInsteadOfTicks Output the (rawbuf_values * MICROS_PER_TICK) for better readability.
 */
void IRrecv::printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks) {

// Print Raw data
    aSerial->print(F("rawData["));
    aSerial->print(decodedIRData.rawlen, DEC);
    aSerial->println(F("]: "));

    /*
     * Print initial gap
     */
    aSerial->print(F(" -"));
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->println((uint32_t) decodedIRData.initialGapTicks * MICROS_PER_TICK, DEC);
    } else {
        aSerial->println(decodedIRData.initialGapTicks, DEC);
    }

// Newline is printed every 8. value, if tCounterForNewline % 8 == 0
    uint_fast8_t tCounterForNewline = 6; // first newline is after the 2 values of the start bit

// check if we have a protocol with no or 8 start bits
#if defined(DECODE_DENON) || defined(DECODE_MAGIQUEST)
    if (
#  if defined(DECODE_DENON)
            decodedIRData.protocol == DENON || decodedIRData.protocol == SHARP ||
#  endif
#  if defined(DECODE_MAGIQUEST)
            decodedIRData.protocol == MAGIQUEST ||
#  endif
            false) {
        tCounterForNewline = 0; // no or 8 start bits
    }
#endif

    uint32_t tDuration;
    uint16_t tSumOfDurationTicks = 0;
    for (IRRawlenType i = 1; i < decodedIRData.rawlen; i++) {
        auto tCurrentTicks = decodedIRData.rawDataPtr->rawbuf[i];
        if (aOutputMicrosecondsInsteadOfTicks) {
            tDuration = tCurrentTicks * MICROS_PER_TICK;
        } else {
            tDuration = tCurrentTicks;
        }
        tSumOfDurationTicks += tCurrentTicks; // compute length of protocol frame

        if (!(i & 1)) {  // even
            aSerial->print('-');
        } else {  // odd
            aSerial->print(F(" +"));
        }

        // padding only for big values
        if (aOutputMicrosecondsInsteadOfTicks && tDuration < 1000) {
            aSerial->print(' ');
        }
        if (aOutputMicrosecondsInsteadOfTicks && tDuration < 100) {
            aSerial->print(' ');
        }
        if (tDuration < 10) {
            aSerial->print(' ');
        }
        aSerial->print(tDuration, DEC);

        if ((i & 1) && (i + 1) < decodedIRData.rawlen) {
            aSerial->print(','); //',' not required for last one
        }

        tCounterForNewline++;
        if ((tCounterForNewline % 8) == 0) {
            aSerial->println();
        }
    }

    aSerial->println();
    aSerial->print("Sum: ");
    if (aOutputMicrosecondsInsteadOfTicks) {
        aSerial->println((uint32_t) tSumOfDurationTicks * MICROS_PER_TICK, DEC);
    } else {
        aSerial->println(tSumOfDurationTicks, DEC);
    }
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
        aSerial->print(F("uint16_t rawData["));         // variable type, array name
    } else {
        aSerial->print(F("uint8_t rawTicks["));          // variable type, array name
    }

    aSerial->print(decodedIRData.rawlen - 1, DEC);    // array size
    aSerial->print(F("] = {"));    // Start declaration

// Dump data
    for (IRRawlenType i = 1; i < decodedIRData.rawlen; i++) {
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
            unsigned int tTicks = (tDuration + (MICROS_PER_TICK / 2)) / MICROS_PER_TICK;
            /*
             * Clip to 8 bit value
             */
            tTicks = (tTicks > UINT8_MAX) ? UINT8_MAX : tTicks;
            aSerial->print(tTicks);
        }
        if (i + 1 < decodedIRData.rawlen) aSerial->print(',');                // ',' not required on last one
        if (!(i & 1)) aSerial->print(' ');
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
    IRRawlenType i;
    for (i = 1; i < decodedIRData.rawlen; i++) {
        uint32_t tDuration = decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
        if (i & 1) {
            // Mark
            tDuration -= MARK_EXCESS_MICROS;
        } else {
            tDuration += MARK_EXCESS_MICROS;
        }

        unsigned int tTicks = (tDuration + (MICROS_PER_TICK / 2)) / MICROS_PER_TICK;
        *aArrayPtr = (tTicks > UINT8_MAX) ? UINT8_MAX : tTicks; // we store it in an 8 bit array
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

        // All protocols have raw data
#if __INT_WIDTH__ < 32
        aSerial->print(F("uint32_t rawData = 0x"));
#else
        aSerial->print(F("uint64_t rawData = 0x"));
#endif
#if (__INT_WIDTH__ < 32)
        aSerial->print(decodedIRData.decodedRawData, HEX);
#else
        PrintULL::print(aSerial, decodedIRData.decodedRawData, HEX);
#endif
        aSerial->println(';');
        aSerial->println();
    }
}

#if defined(__AVR__)
const __FlashStringHelper* IRrecv::getProtocolString() {
// call no class function with same name
    return ::getProtocolString(decodedIRData.protocol);
}
#else
const char* IRrecv::getProtocolString() {
    // call no class function with same name
    return ::getProtocolString(decodedIRData.protocol);
}
#endif

/**********************************************************************************************************************
 * The OLD and DEPRECATED decode function with parameter aResults, kept for backward compatibility to old 2.0 tutorials
 * This function calls the old MSB first decoders and fills only the 3 variables:
 * aResults->value
 * aResults->bits
 * aResults->decode_type
 **********************************************************************************************************************/
bool IRrecv::decode_old(decode_results *aResults) {

    if (irparams.StateForISR != IR_REC_STATE_STOP) {
        return false;
    }

// copy for usage by legacy programs
    aResults->rawbuf[0] = irparams.initialGapTicks;
    for (int i = 1; i < RAW_BUFFER_LENGTH; ++i) {
        aResults->rawbuf[i] = irparams.rawbuf[i]; // copy 8 bit array into a 16 bit array
    }
    aResults->rawlen = irparams.rawlen;
    if (irparams.OverflowFlag) {
        // Copy overflow flag to decodedIRData.flags
        irparams.OverflowFlag = false;
        irparams.rawlen = 0; // otherwise we have OverflowFlag again at next ISR call
        IR_DEBUG_PRINTLN(F("Overflow happened"));
    }
    aResults->overflow = irparams.OverflowFlag;
    aResults->value = 0;

    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST; // for print

#if defined(DECODE_NEC)
    IR_DEBUG_PRINTLN(F("Attempting old NEC decode"));
    if (decodeNECMSB(aResults)) {
        return true;
    }
#endif

#if defined(DECODE_SONY)
    IR_DEBUG_PRINTLN(F("Attempting old Sony decode"));
    if (decodeSonyMSB(aResults)) {
        return true;
    }
#endif

#if defined(DECODE_RC5)
    IR_DEBUG_PRINTLN(F("Attempting RC5 decode"));
    if (decodeRC5()) {
        aResults->bits = decodedIRData.numberOfBits;
        aResults->value = decodedIRData.decodedRawData;
        aResults->decode_type = RC5;

        return true;
    }
#endif

#if defined(DECODE_RC6)
    IR_DEBUG_PRINTLN(F("Attempting RC6 decode"));
    if (decodeRC6()) {
        aResults->bits = decodedIRData.numberOfBits;
        aResults->value = decodedIRData.decodedRawData;
        aResults->decode_type = RC6;
        return true;
    }
#endif

//    Removed bool IRrecv::decodePanasonicMSB(decode_results *aResults) since  implementations was wrong (wrong length), and nobody recognized it

#if defined(DECODE_LG)
    IR_DEBUG_PRINTLN(F("Attempting old LG decode"));
    if (decodeLGMSB(aResults)) {return true;}
#endif

#if defined(DECODE_JVC)
    IR_DEBUG_PRINTLN(F("Attempting old JVC decode"));
    if (decodeJVCMSB(aResults)) {
        return true;
    }
#endif

#if defined(DECODE_SAMSUNG)
    IR_DEBUG_PRINTLN(F("Attempting old SAMSUNG decode"));
    if (decodeSAMSUNG(aResults)) {
        return true;
    }
#endif

#if defined(DECODE_DENON)
    IR_DEBUG_PRINTLN(F("Attempting old Denon decode"));
    if (decodeDenonOld(aResults)) {
        return true;
    }
#endif

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

/** @}*/
#if defined(_IR_MEASURE_TIMING)
#undef _IR_MEASURE_TIMING
#endif
#if defined(LOCAL_TRACE)
#undef LOCAL_TRACE
#endif
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_RECEIVE_HPP
