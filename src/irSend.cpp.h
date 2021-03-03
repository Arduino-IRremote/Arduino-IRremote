/*
 * irSend.cpp.h
 *
 *  Contains common functions for sending
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
#include "IRremoteInt.h"

// The sender instance
IRsend IrSender;

//#define USE_CUSTOM_DELAY // Use old custom_delay_usec() function for mark and space delays.

#if defined(USE_SOFT_SEND_PWM) || defined(USE_NO_SEND_PWM)
IRsend::IRsend(uint8_t aSendPin) {
    sendPin = aSendPin;
}
void IRsend::setSendPin(uint8_t aSendPin) {
    sendPin = aSendPin;
}

__attribute((error("Version > 3.0.1"))) void UsageError(const char *details);

/*
 * @ param aBlinkPin if 0, then take board BLINKLED_ON() and BLINKLED_OFF() functions
 */
void IRsend::begin(uint8_t aSendPin, bool aEnableLEDFeedback, uint8_t aLEDFeedbackPin) {
#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_ARCH_SAMD)
    if(aSendPin==true || aSendPin==false){
        UsageError("Error: You must use begin(<sendPin>, <EnableLEDFeedback>, <LEDFeedbackPin>) if USE_SOFT_SEND_PWM or USE_NO_SEND_PWM is defined!");
    }
#endif
    
    sendPin = aSendPin;

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
}
#else
IRsend::IRsend() {
    sendPin = IR_SEND_PIN;
}

/*
 * @ param aBlinkPin if 0, then take board BLINKLED_ON() and BLINKLED_OFF() functions
 */
void IRsend::begin(bool aEnableLEDFeedback, uint8_t aLEDFeedbackPin) {

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
}
#endif

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

    } else if (tProtocol == APPLE) {
        sendApple(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);

#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    } else if (tProtocol == BOSEWAVE) {
        sendBoseWave(tCommand, aNumberOfRepeats);

    } else if (tProtocol == LEGO_PF) {
        sendLegoPowerFunctions(aIRSendData); // send 5 autorepeats
#endif

    }
    return 1;
}

#ifdef SENDING_SUPPORTED // from IRremoteBoardDefs.h
//+=============================================================================
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

    ledOff();  // Always end with the LED off
}

/*
 * New function using an 8 byte buffer
 * Raw data starts with a Mark. No leading space any more.
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
    ledOff();  // Always end with the LED off
}

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
    ledOff();  // Always end with the LED off
#endif
}

/*
 * New function using an 8 byte buffer
 * Raw data starts with a Mark. No leading space any more.
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
    ledOff();  // Always end with the LED off
#endif
}

//+=============================================================================
/*
 * Sends PulseDistance data
 * always ends with a space
 */
void IRsend::sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
        unsigned int aZeroSpaceMicros, uint32_t aData, uint8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit) {

    if (aMSBfirst) {  // Send the MSB first.
        // send data from MSB to LSB until mask bit is shifted out
        for (uint32_t tMask = 1UL << (aNumberOfBits - 1); tMask; tMask >>= 1) {
            if (aData & tMask) {
                TRACE_PRINT('1');
                mark(aOneMarkMicros);
                space(aOneSpaceMicros);
            } else {
                TRACE_PRINT('0');
                mark(aZeroMarkMicros);
                space(aZeroSpaceMicros);
            }
        }
    } else {  // Send the Least Significant Bit (LSB) first / MSB last.
        for (uint_fast8_t bit = 0; bit < aNumberOfBits; bit++, aData >>= 1)
            if (aData & 1) {  // Send a 1
                TRACE_PRINT('1');
                mark(aOneMarkMicros);
                space(aOneSpaceMicros);
            } else {  // Send a 0
                TRACE_PRINT('0');
                mark(aZeroMarkMicros);
                space(aZeroSpaceMicros);
            }
    }
    if (aSendStopBit) {
        TRACE_PRINT('S');
        mark(aZeroMarkMicros); // seems like this is used for stop bits
        ledOff(); // Always end with the LED off
    }
    TRACE_PRINTLN("");
}

/*
 * Always send start bit, do not send the trailing space of the start bit
 * 0 -> mark+space
 * 1 -> space+mark
 */
void IRsend::sendBiphaseData(unsigned int aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits) {

// do not send the trailing space of the start bit
    mark(aBiphaseTimeUnit);

    TRACE_PRINT('S');
    uint8_t tLastBitValue = 1; // Start bit is a 1

// Data - Biphase code MSB first
    for (uint32_t tMask = 1UL << (aNumberOfBits - 1); tMask; tMask >>= 1) {
        if (aData & tMask) {
            TRACE_PRINT('1');
            space(aBiphaseTimeUnit);
            mark(aBiphaseTimeUnit);
            tLastBitValue = 1;

        } else {
            TRACE_PRINT('0');
#ifdef USE_SOFT_SEND_PWM
            (void)tLastBitValue; // to avoid compiler warnings
            mark(aBiphaseTimeUnit); // can not eventually delay here, we must call mark to generate the signal
#else
            if (tLastBitValue) {
                // Extend the current mark in order to generate a continuous signal without short breaks
                delayMicroseconds(aBiphaseTimeUnit);
            } else {
                mark(aBiphaseTimeUnit);
            }
#endif
            space(aBiphaseTimeUnit);
            tLastBitValue = 0;
        }
    }
    ledOff();  // Always end with the LED off
    TRACE_PRINTLN("");
}

//+=============================================================================
// Sends an IR mark for the specified number of microseconds.
// The mark output is modulated at the PWM frequency.
//
void IRsend::mark(unsigned int aMarkMicros) {
#ifdef USE_SOFT_SEND_PWM
    unsigned long start = micros();
    unsigned long nextPeriodEnding = start;
    while (micros() - start < aMarkMicros) {
        noInterrupts(); // do not change the short on period
        SENDPIN_ON(sendPin);
        delayMicroseconds(periodOnTimeMicros); // this is normally implemented by a blocking wait and therefore guarantees a constant on period
        SENDPIN_OFF(sendPin);
        interrupts(); // Enable interrupts for the longer off period
        nextPeriodEnding += periodTimeMicros;
        while (micros() < nextPeriodEnding)
            ;
    }

#else

#  if defined(USE_NO_SEND_PWM)
    digitalWrite(sendPin, LOW); // Set output to active low.

#  else
    TIMER_ENABLE_SEND_PWM
    ; // Enable pin 3 PWM output
#  endif //  USE_SOFT_SEND_PWM

    setFeedbackLED(true);
    customDelayMicroseconds(aMarkMicros);
#endif // USE_SOFT_SEND_PWM
}

void IRsend::ledOff() {
#if defined(USE_NO_SEND_PWM)
    digitalWrite(sendPin, HIGH); // Set output to inactive high.
#else
    TIMER_DISABLE_SEND_PWM; // Disable PWM output
#endif // defined(USE_NO_SEND_PWM)

    setFeedbackLED(false);
}

//+=============================================================================
// Leave pin off for time (given in microseconds)
// Sends an IR space for the specified number of microseconds.
// A space is no output, so the PWM output is disabled.
//
void IRsend::space(unsigned int aSpaceMicros) {
#if defined(USE_NO_SEND_PWM)
    digitalWrite(sendPin, HIGH); // Set output to inactive high.
#else
    TIMER_DISABLE_SEND_PWM; // Disable PWM output
#endif // defined(USE_NO_SEND_PWM)

    setFeedbackLED(false);
    customDelayMicroseconds(aSpaceMicros);
}

//+=============================================================================
// Custom delay function that circumvents Arduino's delayMicroseconds 16 bit limit
// and is (mostly) not extended by the duration of interrupt codes like the millis() interrupt

void IRsend::customDelayMicroseconds(unsigned long aMicroseconds) {
    unsigned long start = micros();
    // overflow invariant comparison :-)
    while (micros() - start < aMicroseconds) {
    }
}

#ifdef USE_DEFAULT_ENABLE_IR_OUT
//+=============================================================================
// Enables IR output.  The kHz value controls the modulation frequency in kilohertz.
// The IR output will be on pin 3 (OC2B).
// This routine is designed for 36-40 kHz; if you use it for other values, it's up to you
// to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
// TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
// controlling the duty cycle.
// There is no prescaling, so the output frequency is 16 MHz / (2 * OCR2A)
// To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
// A few hours staring at the ATmega documentation and this will all make sense.
// See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
//
void IRsend::enableIROut(uint8_t aFrequencyKHz) {
#ifdef USE_SOFT_SEND_PWM
    periodTimeMicros = (1000U + aFrequencyKHz / 2) / aFrequencyKHz; // = 1000/kHz + 1/2 = round(1000.0/kHz)
    periodOnTimeMicros = ((periodTimeMicros * IR_SEND_DUTY_CYCLE) / 100U) - PULSE_CORRECTION_MICROS;
#endif

#if defined(USE_NO_SEND_PWM)
    (void) aFrequencyKHz;
    pinMode(sendPin, OUTPUT);
    digitalWrite(sendPin, HIGH); // Set output to inactive high.
#else
    TIMER_DISABLE_RECEIVE_INTR;

    pinMode(sendPin, OUTPUT);
    SENDPIN_OFF(sendPin); // When not sending, we want it low

    timerConfigForSend(aFrequencyKHz);
#endif // defined(USE_NO_SEND_PWM)
}
#endif // USE_DEFAULT_ENABLE_IR_OUT

#endif // SENDING_SUPPORTED
