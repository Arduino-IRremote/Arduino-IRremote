/**
 * @file IRFeedbackLED.hpp
 *
 * @brief All Feedback LED specific functions are contained in this file.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021-2022 Armin Joachimsmeyer
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
#ifndef _IR_FEEDBACK_LED_HPP
#define _IR_FEEDBACK_LED_HPP

/** \addtogroup FeedbackLEDFunctions Feedback LED functions
 * @{
 */

/**
 * Contains pin number and enable status of the feedback LED
 */
struct FeedbackLEDControlStruct {
    uint8_t FeedbackLEDPin;         ///< if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
    uint8_t LedFeedbackEnabled; ///< LED_FEEDBACK_ENABLED_FOR_RECEIVE or LED_FEEDBACK_ENABLED_FOR_SEND -> enable blinking of pin on IR processing
};

struct FeedbackLEDControlStruct FeedbackLEDControl; ///< The feedback LED control instance

/**
 * Enable blinking of feedback LED (LED_BUILTIN is taken as default) on IR sending and receiving
 * Cannot disable it here!!! Use disableLEDFeedbackForReceive() or disableLEDFeedbackForSend()
 * @param aFeedbackLEDPin If aFeedbackLEDPin == 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 *                        If FeedbackLEDPin == 0 and no LED_BUILTIN defined, disable LED feedback
 * @param aEnableLEDFeedback If LED_FEEDBACK_ENABLED_FOR_RECEIVE or LED_FEEDBACK_ENABLED_FOR_SEND -> enable blinking of Feedback LED
 */
void setLEDFeedback(uint8_t aFeedbackLEDPin, uint8_t aEnableLEDFeedback) {

    FeedbackLEDControl.FeedbackLEDPin = aFeedbackLEDPin; // default is 0 -> use LED_BUILTIN if available, else disable feedback

    if (aEnableLEDFeedback != DO_NOT_ENABLE_LED_FEEDBACK) {
        FeedbackLEDControl.LedFeedbackEnabled |= aEnableLEDFeedback;
        if (aFeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
            pinModeFast(aFeedbackLEDPin, OUTPUT);
#if defined(LED_BUILTIN)
        } else {
            pinModeFast(LED_BUILTIN, OUTPUT);
#else
            FeedbackLEDControl.LedFeedbackEnabled = LED_FEEDBACK_DISABLED_COMPLETELY; // we have no LED_BUILTIN available
#endif
        }
    }
}

/*
 * Direct replacement for blink13()
 */
void setLEDFeedback(bool aEnableLEDFeedback) {
    bool tEnableLEDFeedback = LED_FEEDBACK_DISABLED_COMPLETELY;
    if (aEnableLEDFeedback) {
        tEnableLEDFeedback = LED_FEEDBACK_ENABLED_FOR_SEND | LED_FEEDBACK_ENABLED_FOR_RECEIVE;
    }
    setLEDFeedback(FeedbackLEDControl.FeedbackLEDPin, tEnableLEDFeedback);
}

void enableLEDFeedback() {
    FeedbackLEDControl.LedFeedbackEnabled |= LED_FEEDBACK_ENABLED_FOR_RECEIVE;
}

void disableLEDFeedback() {
    FeedbackLEDControl.LedFeedbackEnabled &= ~(LED_FEEDBACK_ENABLED_FOR_RECEIVE);
}

void enableLEDFeedbackForSend() {
    FeedbackLEDControl.LedFeedbackEnabled |= LED_FEEDBACK_ENABLED_FOR_SEND;
}

void disableLEDFeedbackForSend() {
    FeedbackLEDControl.LedFeedbackEnabled &= ~(LED_FEEDBACK_ENABLED_FOR_SEND);
}

/**
 * Flash LED while receiving or sending IR data. Does not check if enabled, this must be done by the caller.
 * Handles the 0 value of FeedbackLEDPin and the macro FEEDBACK_LED_IS_ACTIVE_LOW.
 */
#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
#endif
void setFeedbackLED(bool aSwitchLedOn) {
    if (aSwitchLedOn) {
        if (FeedbackLEDControl.FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, LOW); // Turn user defined pin LED on
#else
            digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, HIGH); // Turn user defined pin LED on
#endif
#if defined(LED_BUILTIN) // use fast macros here
        } else {
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            digitalWriteFast(LED_BUILTIN, LOW); // For AVR, this generates a single cbi command
#  else
            digitalWriteFast(LED_BUILTIN, HIGH); // For AVR, this generates a single sbi command
#  endif
#endif
        }
    } else {
        if (FeedbackLEDControl.FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, HIGH); // Turn user defined pin LED off
#else
            digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, LOW); // Turn user defined pin LED off
#endif
#if defined(LED_BUILTIN)
        } else {
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            digitalWriteFast(LED_BUILTIN, HIGH); // For AVR, this generates a single sbi command
#  else
            digitalWriteFast(LED_BUILTIN, LOW); // For AVR, this generates a single cbi command
#  endif
#endif
        }
    }
}

/**
 * Old deprecated function name for setLEDFeedback() or enableLEDFeedback() / disableLEDFeedback()
 */
void IRrecv::blink13(uint8_t aEnableLEDFeedback) {
    setLEDFeedback(FeedbackLEDControl.FeedbackLEDPin, aEnableLEDFeedback);
}
/**
 * Old deprecated function name for setLEDFeedback()
 */
void setBlinkPin(uint8_t aBlinkPin) {
    setLEDFeedback(aBlinkPin, FeedbackLEDControl.LedFeedbackEnabled);
}

/** @}*/

#endif // _IR_FEEDBACK_LED_HPP
