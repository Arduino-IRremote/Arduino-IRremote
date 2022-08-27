/*
 * IRSend.hpp
 *
 *  Contains common functions for sending
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2009-2022 Ken Shirriff, Rafi Khan, Armin Joachimsmeyer
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
#ifndef _IR_SEND_HPP
#define _IR_SEND_HPP

/*
 * This improves readability of code by avoiding a lot of #if defined clauses
 */
#if defined(IR_SEND_PIN)
#define sendPin IR_SEND_PIN
#endif

/** \addtogroup Sending Sending IR data for multiple protocols
 * @{
 */

// The sender instance
IRsend IrSender;

IRsend::IRsend() { // @suppress("Class members should be properly initialized")
#if !defined(IR_SEND_PIN)
    sendPin = 0;
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, DO_NOT_ENABLE_LED_FEEDBACK);
#endif
}

#if defined(IR_SEND_PIN)
/**
 * Only required to set LED feedback
 * Simple start with defaults - LED feedback enabled! Used if IR_SEND_PIN is defined. Saves program memory.
 */
void IRsend::begin(){
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(USE_DEFAULT_FEEDBACK_LED_PIN, LED_FEEDBACK_ENABLED_FOR_SEND);
#  endif
}

/**
 * Only required to set LED feedback
 * @param aEnableLEDFeedback    If true the feedback LED is activated while receiving or sending a PWM signal /a mark
 * @param aFeedbackLEDPin       If 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRsend::begin(bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin) {
#if !defined(NO_LED_FEEDBACK_CODE)
    bool tEnableLEDFeedback = DO_NOT_ENABLE_LED_FEEDBACK;
    if(aEnableLEDFeedback) {
        tEnableLEDFeedback = LED_FEEDBACK_ENABLED_FOR_SEND;
    }
    setLEDFeedback(aFeedbackLEDPin, tEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif
}

#else // defined(IR_SEND_PIN)
IRsend::IRsend(uint_fast8_t aSendPin) { // @suppress("Class members should be properly initialized")
    sendPin = aSendPin;
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, DO_NOT_ENABLE_LED_FEEDBACK);
#  endif
}

/**
 * Initializes the send pin and enable LED feedback with board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 * @param aSendPin The Arduino pin number, where a IR sender diode is connected.
 */
void IRsend::begin(uint_fast8_t aSendPin) {
    sendPin = aSendPin;
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(USE_DEFAULT_FEEDBACK_LED_PIN, LED_FEEDBACK_ENABLED_FOR_SEND);
#  endif
}

void IRsend::setSendPin(uint_fast8_t aSendPin) {
    sendPin = aSendPin;
}
#endif // defined(IR_SEND_PIN)

/**
 * Initializes the send and feedback pin
 * @param aSendPin The Arduino pin number, where a IR sender diode is connected.
 * @param aEnableLEDFeedback    If true the feedback LED is activated while receiving or sending a PWM signal /a mark
 * @param aFeedbackLEDPin       If 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRsend::begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin) {
#if defined(IR_SEND_PIN)
    (void) aSendPin; // for backwards compatibility
#else
    sendPin = aSendPin;
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    bool tEnableLEDFeedback = DO_NOT_ENABLE_LED_FEEDBACK;
    if (aEnableLEDFeedback) {
        tEnableLEDFeedback = LED_FEEDBACK_ENABLED_FOR_SEND;
    }
    setLEDFeedback(aFeedbackLEDPin, tEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif
}

/**
 * Interprets and sends a IRData structure.
 * @param aIRSendData The values of protocol, address, command and repeat flag are taken for sending.
 * @param aNumberOfRepeats Number of repeats to send after the initial data.
 */
size_t IRsend::write(IRData *aIRSendData, uint_fast8_t aNumberOfRepeats) {

    auto tProtocol = aIRSendData->protocol;
    auto tAddress = aIRSendData->address;
    auto tCommand = aIRSendData->command;
    bool tIsRepeat = (aIRSendData->flags & IRDATA_FLAGS_IS_REPEAT);
//    switch (tProtocol) { // 26 bytes bigger than if, else if, else
//    case NEC:
//        sendNEC(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);
//        break;
//    case SAMSUNG:
//        sendSamsung(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case SONY:
//        sendSony(tAddress, tCommand, aNumberOfRepeats, aIRSendData->numberOfBits);
//        break;
//    case PANASONIC:
//        sendPanasonic(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case DENON:
//        sendDenon(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case SHARP:
//        sendSharp(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case JVC:
//        sendJVC((uint8_t) tAddress, (uint8_t) tCommand, aNumberOfRepeats); // casts are required to specify the right function
//        break;
//    case RC5:
//        sendRC5(tAddress, tCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats
//        break;
//    case RC6:
//        // No toggle for repeats//        sendRC6(tAddress, tCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats
//        break;
//    default:
//        break;
//    }

    /*
     * Order of protocols is in guessed relevance :-)
     */
    if (tProtocol == NEC) {
        sendNEC(tAddress, tCommand, aNumberOfRepeats, tIsRepeat);

    } else if (tProtocol == SAMSUNG) {
        sendSamsung(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SAMSUNG_LG) {
        sendSamsungLG(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SONY) {
        sendSony(tAddress, tCommand, aNumberOfRepeats, aIRSendData->numberOfBits);

    } else if (tProtocol == PANASONIC) {
        sendPanasonic(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == DENON) {
        sendDenon(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SHARP) {
        sendSharp(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == LG) {
        sendLG(tAddress, tCommand, aNumberOfRepeats, tIsRepeat);

    } else if (tProtocol == JVC) {
        sendJVC((uint8_t) tAddress, (uint8_t) tCommand, aNumberOfRepeats); // casts are required to specify the right function

    } else if (tProtocol == RC5) {
        sendRC5(tAddress, tCommand, aNumberOfRepeats, !tIsRepeat); // No toggle for repeats

    } else if (tProtocol == RC6) {
        sendRC6(tAddress, tCommand, aNumberOfRepeats, !tIsRepeat); // No toggle for repeats

    } else if (tProtocol == KASEIKYO_JVC) {
        sendKaseikyo_JVC(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_DENON) {
        sendKaseikyo_Denon(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_SHARP) {
        sendKaseikyo_Sharp(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_MITSUBISHI) {
        sendKaseikyo_Mitsubishi(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == NEC2) {
        sendNEC2(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == ONKYO) {
        sendOnkyo(tAddress, tCommand, aNumberOfRepeats, tIsRepeat);

    } else if (tProtocol == APPLE) {
        sendApple(tAddress, tCommand, aNumberOfRepeats, tIsRepeat);

#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    } else if (tProtocol == BOSEWAVE) {
        sendBoseWave(tCommand, aNumberOfRepeats);

    } else if (tProtocol == MAGIQUEST) {
        sendMagiQuest(tAddress, tCommand);

    } else if (tProtocol == LEGO_PF) {
        sendLegoPowerFunctions(tAddress, tCommand, tCommand >> 4, tIsRepeat); // send 5 autorepeats
#endif

    }
    return 1;
}

/**
 * Function using an 16 byte microsecond timing array for every purpose.
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    /*
     * Raw data starts with a mark.
     */
    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        if (i & 1) {
            // Odd
            space(aBufferWithMicroseconds[i]);
        } else {
            mark(aBufferWithMicroseconds[i]);
        }
    }

    IrReceiver.restartAfterSend();
}

/**
 * Function using an 8 byte tick timing array to save program memory
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        if (i & 1) {
            // Odd
            space(aBufferWithTicks[i] * MICROS_PER_TICK);
        } else {
            mark(aBufferWithTicks[i] * MICROS_PER_TICK);
        }
    }
    IRLedOff();  // Always end with the LED off
    IrReceiver.restartAfterSend();
}

/**
 * Function using an 16 byte microsecond timing array in FLASH for every purpose.
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw_P(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer,
        uint_fast8_t aIRFrequencyKilohertz) {
#if !defined(__AVR__)
    sendRaw(aBufferWithMicroseconds, aLengthOfBuffer, aIRFrequencyKilohertz); // Let the function work for non AVR platforms
#else
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);
    /*
     * Raw data starts with a mark
     */
    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        unsigned int duration = pgm_read_word(&aBufferWithMicroseconds[i]);
        if (i & 1) {
            // Odd
            space(duration);
        } else {
            mark(duration);
        }
    }
    IrReceiver.restartAfterSend();
#endif
}

/**
 * New function using an 8 byte tick timing array in FLASH to save program memory
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw_P(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
#if !defined(__AVR__)
    sendRaw(aBufferWithTicks, aLengthOfBuffer, aIRFrequencyKilohertz); // Let the function work for non AVR platforms
#else
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        unsigned int duration = pgm_read_byte(&aBufferWithTicks[i]) * (unsigned int) MICROS_PER_TICK;
        if (i & 1) {
            // Odd
            space(duration);
        } else {
            mark(duration);
        }
    }
    IRLedOff();  // Always end with the LED off
    IrReceiver.restartAfterSend();
#endif
}

/**
 * Sends PulseDistance data from array
 * For LSB First the LSB of array[0] is sent first then all bits until MSB of array[0]. Next is LSB of array[1] and so on.
 * The output always ends with a space
 * Stop bit is always sent
 */
void IRsend::sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, unsigned int aHeaderMarkMicros, unsigned int aHeaderSpaceMicros,
        unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros, unsigned int aZeroSpaceMicros,
        uint32_t *aDecodedRawDataArray, unsigned int aNumberOfBits, bool aMSBfirst, unsigned int aRepeatPeriodMillis,
        uint_fast8_t aNumberOfRepeats) {

    // Set IR carrier frequency
    enableIROut(aFrequencyKHz);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    uint_fast8_t tNumberOf32BitChunks = ((aNumberOfBits - 1) / 32) + 1;

    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        // Header
        mark(aHeaderMarkMicros);
        space(aHeaderSpaceMicros);

        for (uint_fast8_t i = 0; i < tNumberOf32BitChunks; ++i) {
            uint8_t tNumberOfBitsForOneSend;
            if (aNumberOfBits > 32) {
                tNumberOfBitsForOneSend = 32;
            } else {
                tNumberOfBitsForOneSend = aNumberOfBits;
            }
            sendPulseDistanceWidthData(aOneMarkMicros, aOneSpaceMicros, aZeroMarkMicros, aZeroSpaceMicros, aDecodedRawDataArray[i],
                    tNumberOfBitsForOneSend, aMSBfirst, (i == (tNumberOf32BitChunks - 1)));

            aNumberOfBits -= 32;
        }

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            delay(aRepeatPeriodMillis - (millis() - tStartOfFrameMillis));
        }
    }
    IrReceiver.restartAfterSend();
}

/**
 * Sends PulseDistance frames and repeats
 */
void IRsend::sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, unsigned int aHeaderMarkMicros, unsigned int aHeaderSpaceMicros,
        unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros, unsigned int aZeroSpaceMicros,
        uint32_t aData, uint_fast8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit, unsigned int aRepeatPeriodMillis,
        uint_fast8_t aNumberOfRepeats) {

    // Set IR carrier frequency
    enableIROut(aFrequencyKHz);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        // Header
        mark(aHeaderMarkMicros);
        space(aHeaderSpaceMicros);

        sendPulseDistanceWidthData(aOneMarkMicros, aOneSpaceMicros, aZeroMarkMicros, aZeroSpaceMicros, aData, aNumberOfBits,
                aMSBfirst, aSendStopBit);

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            delay(aRepeatPeriodMillis - (millis() - tStartOfFrameMillis));
        }
    }
    IrReceiver.restartAfterSend();
}

/**
 * Sends PulseDistance data
 * The output always ends with a space
 */
void IRsend::sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
        unsigned int aZeroSpaceMicros, uint32_t aData, uint_fast8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit) {

    if (aMSBfirst) {  // Send the MSB first.
        // send data from MSB to LSB until mask bit is shifted out
        for (uint32_t tMask = 1UL << (aNumberOfBits - 1); tMask; tMask >>= 1) {
            if (aData & tMask) {
                IR_TRACE_PRINT('1');
                mark(aOneMarkMicros);
                space(aOneSpaceMicros);
            } else {
                IR_TRACE_PRINT('0');
                mark(aZeroMarkMicros);
                space(aZeroSpaceMicros);
            }
        }
    } else {  // Send the Least Significant Bit (LSB) first / MSB last.
        for (uint_fast8_t bit = 0; bit < aNumberOfBits; bit++, aData >>= 1)
            if (aData & 1) {  // Send a 1
                IR_TRACE_PRINT('1');
                mark(aOneMarkMicros);
                space(aOneSpaceMicros);
            } else {  // Send a 0
                IR_TRACE_PRINT('0');
                mark(aZeroMarkMicros);
                space(aZeroSpaceMicros);
            }
    }
    if (aSendStopBit) {
        IR_TRACE_PRINT('S');
        mark(aZeroMarkMicros); // Use aZeroMarkMicros for stop bits. This seems to be correct for all protocols :-)
    }
    IR_TRACE_PRINTLN(F(""));
}

/**
 * Sends Biphase data MSB first
 * Always send start bit, do not send the trailing space of the start bit
 * 0 -> mark+space
 * 1 -> space+mark
 * The output always ends with a space
 * can only send 31 bit data, since we put the start bit as 32th bit on front
 */
void IRsend::sendBiphaseData(unsigned int aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits) {

    IR_TRACE_PRINT(F("0x"));
    IR_TRACE_PRINT(aData, HEX);

    IR_TRACE_PRINT(F(" S"));

    // Data - Biphase code MSB first
    // prepare for start with sending the start bit, which is 1
    uint32_t tMask = 1UL << aNumberOfBits;    // mask is now set for the virtual start bit
    uint_fast8_t tLastBitValue = 1;    // Start bit is a 1
    bool tNextBitIsOne = 1;    // Start bit is a 1
    for (uint_fast8_t i = aNumberOfBits + 1; i > 0; i--) {
        bool tCurrentBitIsOne = tNextBitIsOne;
        tMask >>= 1;
        tNextBitIsOne = ((aData & tMask) != 0) || (i == 1); // true for last bit to avoid extension of mark
        if (tCurrentBitIsOne) {
            IR_TRACE_PRINT('1');
            space(aBiphaseTimeUnit);
            if (tNextBitIsOne) {
                mark(aBiphaseTimeUnit);
            } else {
                // if next bit is 0, extend the current mark in order to generate a continuous signal without short breaks
                mark(2 * aBiphaseTimeUnit);
            }
            tLastBitValue = 1;

        } else {
            IR_TRACE_PRINT('0');
            if (!tLastBitValue) {
                mark(aBiphaseTimeUnit);
            }
            space(aBiphaseTimeUnit);
            tLastBitValue = 0;
        }
    }
    IR_TRACE_PRINTLN(F(""));
}

/**
 * Sends an IR mark for the specified number of microseconds.
 * The mark output is modulated at the PWM frequency if USE_NO_SEND_PWM is not defined.
 * The output is guaranteed to be OFF / inactive after after the call of the function.
 * This function may affect the state of feedback LED.
 */
void IRsend::mark(unsigned int aMarkMicros) {

#if defined(SEND_PWM_BY_TIMER) || defined(USE_NO_SEND_PWM)
#  if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
        setFeedbackLED(true);
    }
#  endif
#endif

#if defined(SEND_PWM_BY_TIMER)
    /*
     * Generate hardware PWM signal
     */
    ENABLE_SEND_PWM_BY_TIMER; // Enable timer or ledcWrite() generated PWM output
    customDelayMicroseconds(aMarkMicros);
    IRLedOff();// disables hardware PWM and manages feedback LED
    return;

#elif defined(USE_NO_SEND_PWM)
    /*
     * Here we generate no carrier PWM, just simulate an active low receiver signal.
     */
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
    pinModeFast(sendPin, OUTPUT); // active state for mimicking open drain
#  else
    digitalWriteFast(sendPin, LOW); // Set output to active low.
#  endif

    customDelayMicroseconds(aMarkMicros);
    IRLedOff();
#  if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
        setFeedbackLED(false);
    }
    return;
#  endif

#else // defined(SEND_PWM_BY_TIMER)
    /*
     * Generate PWM by bit banging
     */
    unsigned long tStartMicros = micros();
    unsigned long tNextPeriodEnding = tStartMicros;
    unsigned long tMicros;
#  if !defined(NO_LED_FEEDBACK_CODE)
    bool FeedbackLedIsActive = false;
#  endif

    do {
//        digitalToggleFast(_IR_TIMING_TEST_PIN);
        /*
         * Output the PWM pulse
         */
        noInterrupts(); // do not let interrupts extend the short on period
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, LOW); // set output with pin mode OUTPUT_OPEN_DRAIN to active low
#    else
        pinModeFast(sendPin, OUTPUT); // active state for mimicking open drain
#    endif
#  else
        // 3.5 us from FeedbackLed on to pin setting. 5.7 us from call of mark() to pin setting incl. setting of feedback pin.
        // 4.3 us from do{ to pin setting if sendPin is no constant
        digitalWriteFast(sendPin, HIGH);
#  endif
        delayMicroseconds (periodOnTimeMicros); // this is normally implemented by a blocking wait

        /*
         * Output the PWM pause
         */
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, HIGH); // Set output with pin mode OUTPUT_OPEN_DRAIN to inactive high.
#    else
        pinModeFast(sendPin, INPUT); // to mimic the open drain inactive state
#    endif

#  else
        digitalWriteFast(sendPin, LOW);
#  endif
        interrupts(); // Enable interrupts - to keep micros correct- for the longer off period 3.4 us until receive ISR is active (for 7 us + pop's)

#  if !defined(NO_LED_FEEDBACK_CODE)
        /*
         * Delayed call of setFeedbackLED() to get better timing
         */
        if (!FeedbackLedIsActive) {
            FeedbackLedIsActive = true;
            if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
                setFeedbackLED(true);
            }
        }
#  endif
        /*
         * PWM pause timing
         */
        tNextPeriodEnding += periodTimeMicros;
        do {
            tMicros = micros(); // we have only 4 us resolution for AVR @16MHz
            /*
             * Exit the forever loop if aMarkMicros has reached
             */
            unsigned int tDeltaMicros = tMicros - tStartMicros;
#if defined(__AVR__)
//            tDeltaMicros += (160 / CLOCKS_PER_MICRO); // adding this once increases program size !
#  if !defined(NO_LED_FEEDBACK_CODE)
            if (tDeltaMicros >= aMarkMicros - (30 + (112 / CLOCKS_PER_MICRO))) { // 30 to be constant. Using periodTimeMicros increases program size too much.
            // reset feedback led in the last pause before end
                if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
                    setFeedbackLED(false);
                }
            }
#  endif
            if (tDeltaMicros >= aMarkMicros - (112 / CLOCKS_PER_MICRO)) { // To compensate for call duration - 112 is an empirical value
#else
                if (tDeltaMicros >= aMarkMicros) {
#  if !defined(NO_LED_FEEDBACK_CODE)
                    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
                        setFeedbackLED(false);
                    }
#  endif
#endif
                return;
            }
//            digitalToggleFast(_IR_TIMING_TEST_PIN); // 3.0 us per call @16MHz
        } while (tMicros < tNextPeriodEnding);  // 3.4 us @16MHz
    } while (true);
#  endif
}

/**
 * Just switch the IR sending LED off to send an IR space
 * A space is "no output", so the PWM output is disabled.
 * This function may affect the state of feedback LED.
 */
void IRsend::IRLedOff() {
#if defined(SEND_PWM_BY_TIMER)
        DISABLE_SEND_PWM_BY_TIMER; // Disable PWM output
#elif defined(USE_NO_SEND_PWM)
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, LOW); // prepare for all next active states.
        pinModeFast(sendPin, INPUT);// inactive state for open drain
#  else
        digitalWriteFast(sendPin, HIGH); // Set output to inactive high.
#  endif
#else
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, HIGH); // Set output to inactive high.
#    else
        pinModeFast(sendPin, INPUT); // inactive state to mimic open drain
#    endif
#  else
    digitalWriteFast(sendPin, LOW);
#  endif
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
        setFeedbackLED(false);
    }
#endif
}

/**
 * Sends an IR space for the specified number of microseconds.
 * A space is "no output", so just wait.
 */
void IRsend::space(unsigned int aSpaceMicros) {
    customDelayMicroseconds(aSpaceMicros);
}

/**
 * Custom delay function that circumvents Arduino's delayMicroseconds 16 bit limit
 * and is (mostly) not extended by the duration of interrupt codes like the millis() interrupt
 */
void IRsend::customDelayMicroseconds(unsigned long aMicroseconds) {
#if defined(__AVR__)
    unsigned long start = micros() - (64 / clockCyclesPerMicrosecond()); // - (64 / clockCyclesPerMicrosecond()) for reduced resolution and additional overhead
#else
        unsigned long start = micros();
#endif
    // overflow invariant comparison :-)
    while (micros() - start < aMicroseconds) {
    }
}

/**
 * Enables IR output. The kHz value controls the modulation frequency in kilohertz.
 * IF PWM should be generated by a timer, it uses the platform specific timerConfigForSend() function,
 * otherwise it computes the delays used by the mark() function.
 */
void IRsend::enableIROut(uint_fast8_t aFrequencyKHz) {
#if defined(SEND_PWM_BY_TIMER)
        timerConfigForSend(aFrequencyKHz); // must set output pin mode and disable receive interrupt if required, e.g. uses the same resource

#elif defined(USE_NO_SEND_PWM)
        (void) aFrequencyKHz;

#else
    periodTimeMicros = (1000U + (aFrequencyKHz / 2)) / aFrequencyKHz; // rounded value -> 26 for 38.46 kHz, 27 for 37.04 kHz, 25 for 40 kHz.
#  if defined(IR_SEND_PIN)
        periodOnTimeMicros = (((periodTimeMicros * IR_SEND_DUTY_CYCLE_PERCENT) + 50) / 100U); // +50 for rounding -> 830/100 for 30% and 16 MHz
#  else
    // Heuristics! We require a nanosecond correction for "slow" digitalWrite() functions
    periodOnTimeMicros = (((periodTimeMicros * IR_SEND_DUTY_CYCLE_PERCENT) + 50 - (PULSE_CORRECTION_NANOS / 10)) / 100U); // +50 for rounding -> 530/100 for 30% and 16 MHz
#  endif
#endif // defined(SEND_PWM_BY_TIMER)

#if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && defined(OUTPUT_OPEN_DRAIN) // the mode INPUT for mimicking open drain is set at IRLedOff()
#  if defined(IR_SEND_PIN)
        pinModeFast(IR_SEND_PIN, OUTPUT_OPEN_DRAIN);
#  else
        pinModeFast(sendPin, OUTPUT_OPEN_DRAIN);
#  endif
#else

    // For Non AVR platforms pin mode for SEND_PWM_BY_TIMER must be handled by the timerConfigForSend() function
    // because ESP 2.0.2 ledcWrite does not work if pin mode is set, and RP2040 requires gpio_set_function(IR_SEND_PIN, GPIO_FUNC_PWM);
#  if defined(__AVR__) || !defined(SEND_PWM_BY_TIMER)
#    if defined(IR_SEND_PIN)
        pinModeFast(IR_SEND_PIN, OUTPUT);
#    else
    pinModeFast(sendPin, OUTPUT);
#    endif
#  endif
#endif // defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
}

unsigned int IRsend::getPulseCorrectionNanos() {
    return PULSE_CORRECTION_NANOS;
}

/** @}*/
#endif // _IR_SEND_HPP
