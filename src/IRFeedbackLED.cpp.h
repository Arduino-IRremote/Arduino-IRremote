/**
 * @file IRFeedbackLed.cpp.h
 *
 * @brief All Feedback LED specific definitions are contained in this file.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021 Armin Joachimsmeyer
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
#include "private/IRFeedbackLEDDefs.h"

/**
 * Enable/disable blinking of Feedback LED (LED_BUILTIN is taken as default) on IR processing
 * If FeedbackLEDPin == 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void LEDFeedback(bool aEnableLEDFeedback) {
    irparams.LedFeedbackEnabled = aEnableLEDFeedback;
    if (aEnableLEDFeedback) {
        if (irparams.FeedbackLEDPin != 0) {
            pinMode(irparams.FeedbackLEDPin, OUTPUT);
#ifdef FEEDBACK_LED
        } else {
            pinMode(FEEDBACK_LED, OUTPUT);
#endif
        }
    }
}

void enableLEDFeedback() {
    irparams.LedFeedbackEnabled = true;
}

void disableLEDFeedback() {
    irparams.LedFeedbackEnabled = false;
}

/*
 * @ param aFeedbackLEDPin if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void setFeedbackLEDPin(uint8_t aFeedbackLEDPin) {
    irparams.FeedbackLEDPin = aFeedbackLEDPin;
}

/*
 * Flash LED while receiving IR data, if enabled
 */
#if defined(ESP32)
IRAM_ATTR
#endif
void setFeedbackLED(bool aSwitchLedOn) {
    if (irparams.LedFeedbackEnabled) {
        if (aSwitchLedOn) {
            if (irparams.FeedbackLEDPin != 0) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWrite(irparams.FeedbackLEDPin, LOW); // Turn user defined pin LED on
#else
                digitalWrite(irparams.FeedbackLEDPin, HIGH); // Turn user defined pin LED on
#endif
#ifdef FEEDBACK_LED_ON
            } else {
                FEEDBACK_LED_ON();   // if no user defined LED pin, turn default LED pin for the hardware on
#endif
            }
        } else {
            if (irparams.FeedbackLEDPin != 0) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWrite(irparams.FeedbackLEDPin, HIGH); // Turn user defined pin LED off
#else
                digitalWrite(irparams.FeedbackLEDPin, LOW); // Turn user defined pin LED off
#endif
#ifdef FEEDBACK_LED_OFF
            } else {
                FEEDBACK_LED_OFF();   // if no user defined LED pin, turn default LED pin for the hardware on
#endif
            }
        }
    }
}

/*
 * Old deprecated function names
 */
void blink13(bool aEnableLEDFeedback) {
    LEDFeedback(aEnableLEDFeedback);
}
void setBlinkPin(uint8_t aBlinkPin) {
    setFeedbackLEDPin(aBlinkPin);
}
