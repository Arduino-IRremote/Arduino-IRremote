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
#ifndef IR_SEND_HPP
#define IR_SEND_HPP

/*
 * This improves readability of code by avoiding a lot of #if defined clauses
 */
#if defined(IR_SEND_PIN)
#define sendPin IR_SEND_PIN
#endif

#include "digitalWriteFast.h"

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
    setLEDFeedback(0, false);
#endif
}

#if defined(IR_SEND_PIN)
/**
 * Simple start with defaults for constant send pin
 */
void IRsend::begin(){
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(USE_DEFAULT_FEEDBACK_LED_PIN, true);
#  endif
}

/**
 * @param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRsend::begin(bool aEnableLEDFeedback, uint8_t aFeedbackLEDPin) {
#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(aFeedbackLEDPin, aEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif
}

#else // defined(IR_SEND_PIN)
IRsend::IRsend(uint8_t aSendPin) { // @suppress("Class members should be properly initialized")
    sendPin = aSendPin;
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, false);
#  endif
}

/**
 * Initializes the send pin and enable LED feedback with board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 * @param aSendPin The Arduino pin number, where a IR sender diode is connected.
 */
void IRsend::begin(uint8_t aSendPin) {
    sendPin = aSendPin;
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(USE_DEFAULT_FEEDBACK_LED_PIN, true);
#  endif
}

void IRsend::setSendPin(uint8_t aSendPin) {
    sendPin = aSendPin;
}
#endif // defined(IR_SEND_PIN)

/**
 * Initializes the send and feedback pin
 * @param aSendPin The Arduino pin number, where a IR sender diode is connected.
 * @param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRsend::begin(uint8_t aSendPin, bool aEnableLEDFeedback, uint8_t aFeedbackLEDPin) {
#if defined(IR_SEND_PIN)
    (void) aSendPin; // for backwards compatibility
#else
    sendPin = aSendPin;
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(aFeedbackLEDPin, aEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif
}

/**
 * @param aIRSendData The values of protocol, address, command and repeat flag are taken for sending.
 * @param aNumberOfRepeats Number of repeats to send after the initial data.
 */
size_t IRsend::write(IRData *aIRSendData, uint_fast8_t aNumberOfRepeats) {

    auto tProtocol = aIRSendData->protocol;
    auto tAddress = aIRSendData->address;
    auto tCommand = aIRSendData->command;
    bool tSendRepeat = (aIRSendData->flags & IRDATA_FLAGS_IS_REPEAT);
//    switch (tProtocol) { // 26 bytes bigger than if, else if, else
//    case NEC:
//        sendNEC(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);
//        break;
//    case SAMSUNG:
//        sendSamsung(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);
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
        sendNEC(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);

    } else if (tProtocol == SAMSUNG) {
        sendSamsung(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);

    } else if (tProtocol == SONY) {
        sendSony(tAddress, tCommand, aNumberOfRepeats, aIRSendData->numberOfBits);

    } else if (tProtocol == PANASONIC) {
        sendPanasonic(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == DENON) {
        sendDenon(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SHARP) {
        sendSharp(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == LG) {
        sendLG(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);

    } else if (tProtocol == JVC) {
        sendJVC((uint8_t) tAddress, (uint8_t) tCommand, aNumberOfRepeats); // casts are required to specify the right function

    } else if (tProtocol == RC5) {
        sendRC5(tAddress, tCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats

    } else if (tProtocol == RC6) {
        sendRC6(tAddress, tCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats

    } else if (tProtocol == KASEIKYO_JVC) {
        sendKaseikyo_JVC(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_DENON) {
        sendKaseikyo_Denon(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_SHARP) {
        sendKaseikyo_Sharp(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_MITSUBISHI) {
        sendKaseikyo_Mitsubishi(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == ONKYO) {
        sendOnkyo(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);

    } else if (tProtocol == APPLE) {
        sendApple(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);

#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    } else if (tProtocol == BOSEWAVE) {
        sendBoseWave(tCommand, aNumberOfRepeats);

    } else if (tProtocol == LEGO_PF) {
        sendLegoPowerFunctions(tAddress, tCommand, tCommand >> 4, tSendRepeat); // send 5 autorepeats
#endif

    }
    return 1;
}

/**
 * Function using an 16 byte microsecond timing array for every purpose.
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    /*
     * Raw data starts with a mark.
     */
    for (uint_fast8_t i = 0; i < aLengthOfBuffer; i++) {
        if (i & 1) {
            // Odd
            space(aBufferWithMicroseconds[i]);
        } else {
            mark(aBufferWithMicroseconds[i]);
        }
    }

//    ledOff();  // Always end with the LED off
}

/**
 * New function using an 8 byte tick timing array to save program space
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw(const uint8_t aBufferWithTicks[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    for (uint_fast8_t i = 0; i < aLengthOfBuffer; i++) {
        if (i & 1) {
            // Odd
            space(aBufferWithTicks[i] * MICROS_PER_TICK);
        } else {
            mark(aBufferWithTicks[i] * MICROS_PER_TICK);
        }
    }
    IRLedOff();  // Always end with the LED off
}

/**
 * Function using an 16 byte microsecond timing array in FLASH for every purpose.
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw_P(const uint16_t aBufferWithMicroseconds[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
#if !defined(__AVR__)
    sendRaw(aBufferWithMicroseconds, aLengthOfBuffer, aIRFrequencyKilohertz); // Let the function work for non AVR platforms
#else
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);
    /*
     * Raw data starts with a mark
     */
    for (uint_fast8_t i = 0; i < aLengthOfBuffer; i++) {
        uint16_t duration = pgm_read_word(&aBufferWithMicroseconds[i]);
        if (i & 1) {
            // Odd
            space(duration);
        } else {
            mark(duration);
        }
    }
//    ledOff();  // Always end with the LED off
#endif
}

/**
 * New function using an 8 byte tick timing array in FLASH to save program space
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw_P(const uint8_t aBufferWithTicks[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
#if !defined(__AVR__)
    sendRaw(aBufferWithTicks, aLengthOfBuffer, aIRFrequencyKilohertz); // Let the function work for non AVR platforms
#else
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    for (uint_fast8_t i = 0; i < aLengthOfBuffer; i++) {
        uint16_t duration = pgm_read_byte(&aBufferWithTicks[i]) * (uint16_t) MICROS_PER_TICK;
        if (i & 1) {
            // Odd
            space(duration);
        } else {
            mark(duration);
        }
    }
    IRLedOff();  // Always end with the LED off
#endif
}

/**
 * Sends PulseDistance data
 * The output always ends with a space
 */
void IRsend::sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
        unsigned int aZeroSpaceMicros, uint32_t aData, uint8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit) {

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
        mark(aZeroMarkMicros); // seems like this is used for stop bits
    }
    IR_TRACE_PRINTLN("");
}

/*
 * Sends Biphase data MSB first
 * Always send start bit, do not send the trailing space of the start bit
 * 0 -> mark+space
 * 1 -> space+mark
 * The output always ends with a space
 */
void IRsend::sendBiphaseData(unsigned int aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits) {

// do not send the trailing space of the start bit
    mark(aBiphaseTimeUnit);

    IR_TRACE_PRINT('S');
    uint8_t tLastBitValue = 1; // Start bit is a 1

// Data - Biphase code MSB first
    for (uint32_t tMask = 1UL << (aNumberOfBits - 1); tMask; tMask >>= 1) {
        if (aData & tMask) {
            IR_TRACE_PRINT('1');
            space(aBiphaseTimeUnit);
            mark(aBiphaseTimeUnit);
            tLastBitValue = 1;

        } else {
            IR_TRACE_PRINT('0');
#if defined(SEND_PWM_BY_TIMER) || defined(USE_NO_SEND_PWM)
            if (tLastBitValue) {
                // Extend the current mark in order to generate a continuous signal without short breaks
                delayMicroseconds(aBiphaseTimeUnit);
            } else {
                mark(aBiphaseTimeUnit);
            }
#else
            (void) tLastBitValue; // to avoid compiler warnings
            mark(aBiphaseTimeUnit); // can not eventually delay here, we must call mark to generate the signal
#endif
            space(aBiphaseTimeUnit);
            tLastBitValue = 0;
        }
    }
    IR_TRACE_PRINTLN("");
}

/**
 * Sends an IR mark for the specified number of microseconds.
 * The mark output is modulated at the PWM frequency if USE_NO_SEND_PWM is not defined.
 * The output is guaranteed to be OFF / inactive after after the call of the function.
 * This function may affect the state of feedback LED.
 */
void IRsend::mark(unsigned int aMarkMicros) {

#if defined(SEND_PWM_BY_TIMER)
#  if !defined(NO_LED_FEEDBACK_CODE)
    setFeedbackLED(true);
#  endif
    ENABLE_SEND_PWM_BY_TIMER; // Enable timer or ledcWrite() generated PWM output
    customDelayMicroseconds(aMarkMicros);
    IRLedOff();
#  if !defined(NO_LED_FEEDBACK_CODE)
    setFeedbackLED(false);
#  endif

#elif defined(USE_NO_SEND_PWM)
#  if !defined(NO_LED_FEEDBACK_CODE)
    setFeedbackLED(true);
#  endif
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
    pinModeFast(sendPin, OUTPUT); // active state for mimicking open drain
#  else
    digitalWriteFast(sendPin, LOW); // Set output to active low.
#  endif

    customDelayMicroseconds(aMarkMicros);
    IRLedOff();
#  if !defined(NO_LED_FEEDBACK_CODE)
    setFeedbackLED(false);
#  endif

#else
    unsigned long startMicros = micros();
    unsigned long nextPeriodEnding = startMicros;
    unsigned long tMicros;
#  if !defined(NO_LED_FEEDBACK_CODE)
    bool FeedbackLedIsActive = false;
#  endif

    do {
//        digitalToggleFast(_IR_TIMING_TEST_PIN);
        // Output the PWM pulse
        noInterrupts(); // do not let interrupts extend the short on period
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, LOW); // active state for open drain
#    else
        pinModeFast(sendPin, OUTPUT); // active state for mimicking open drain
#    endif
#  else
        // 3.5 us from FeedbackLed on to pin setting. 5.7 us from call of mark() to pin setting incl. setting of feedback pin.
        // 4.3 us from do{ to pin setting if sendPin is no constant
        digitalWriteFast(sendPin, HIGH);
#  endif
        delayMicroseconds (periodOnTimeMicros); // this is normally implemented by a blocking wait

        // Output the PWM pause
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, HIGH); // Set output with pin mode OUTPUT_OPEN_DRAIN to inactive high.
#    else
        pinModeFast(sendPin, INPUT); // to mimic the open drain inactive state
#    endif

#  else
        digitalWriteFast(sendPin, LOW);
#  endif
        interrupts(); // Enable interrupts -to keep micros correct- for the longer off period 3.4 us until receive ISR is active (for 7 us + pop's)

#  if !defined(NO_LED_FEEDBACK_CODE)
        /*
         * Delayed call of setFeedbackLED() to get better timing
         */
        if (!FeedbackLedIsActive) {
            FeedbackLedIsActive = true;
            setFeedbackLED(true);
        }
#  endif
        /*
         * Pause timing
         */
        nextPeriodEnding += periodTimeMicros;
        do {
            tMicros = micros(); // we have only 4 us resolution for AVR @16MHz
            // check for aMarkMicros to be gone
            unsigned int tDeltaMicros = tMicros - startMicros;
#if defined(__AVR__)
//            tDeltaMicros += (160 / CLOCKS_PER_MICRO); // adding this once increases program size !
#  if !defined(NO_LED_FEEDBACK_CODE)
            if (tDeltaMicros >= aMarkMicros - (30 + (112 / CLOCKS_PER_MICRO))) { // 30 to be constant. Using periodTimeMicros increases program size too much.
            // reset feedback led in the last pause before end
                setFeedbackLED(false);
            }
#  endif
            if (tDeltaMicros >= aMarkMicros - (112 / CLOCKS_PER_MICRO)) { // To compensate for call duration - 112 is an empirical value
#else
            if (tDeltaMicros >= aMarkMicros) {
#  if !defined(NO_LED_FEEDBACK_CODE)
                setFeedbackLED(false);
#  endif
#endif
                return;
            }
//            digitalToggleFast(_IR_TIMING_TEST_PIN); // 3.0 us per call @16MHz
        } while (tMicros < nextPeriodEnding);  // 3.4 us @16MHz
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
    pinModeFast(sendPin, INPUT); // inactive state for open drain
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
    setFeedbackLED(false);
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
 * The IR output will be on pin 3 (OC2B).
 * This routine is designed for 36-40 kHz and for software generation gives 26 us for 38.46 kHz, 27 us for 37.04 kHz and 25 us for 40 kHz.
 * If you use it for other values, it's up to you to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
 * TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
 * controlling the duty cycle.
 * There is no prescaling, so the output frequency is 16 MHz / (2 * OCR2A)
 * To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
 * A few hours staring at the ATmega documentation and this will all make sense.
 * See my Secrets of Arduino PWM at http://www.righto.com/2009/07/secrets-of-arduino-pwm.html for details.
 */
void IRsend::enableIROut(uint8_t aFrequencyKHz) {
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

#if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && defined(OUTPUT_OPEN_DRAIN)
    pinMode(sendPin, OUTPUT_OPEN_DRAIN); // the mode INPUT for mimicking open drain is set at IRLedOff()
#elif !defined(SEND_PWM_BY_TIMER)
// For SEND_PWM_BY_TIMER this is handled by the timerConfigForSend() function
    pinModeFast(sendPin, OUTPUT);
#endif // defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
}

unsigned int IRsend::getPulseCorrectionNanos() {
    return PULSE_CORRECTION_NANOS;
}

/** @}*/
#endif // IR_SEND_HPP
#pragma once
