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
#ifndef IR_FEEDBACK_LED_HPP
#define IR_FEEDBACK_LED_HPP

/** \addtogroup FeedbackLEDFunctions Feedback LED functions
 * @{
 */

/**
 * Contains pin number and enable status of the feedback LED
 */
struct FeedbackLEDControlStruct {
    uint8_t FeedbackLEDPin;         ///< if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
    bool LedFeedbackEnabled;        ///< true -> enable blinking of pin on IR processing
};

struct FeedbackLEDControlStruct FeedbackLEDControlSend; ///< The feedback LED control instance for sending
struct FeedbackLEDControlStruct FeedbackLEDControlRecv; ///< Separate feedback LED control instance for receiving

/**
 * Enable/disable blinking of Feedback LED (LED_BUILTIN is taken as default) on IR sending
 * @param aFeedbackLEDPin If aFeedbackLEDPin == 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 *                        If FeedbackLEDPin == 0 and no LED_BUILTIN defined, disable LED feedback
 * @param aEnableLEDFeedback true -> enable blinking of Feedback LED
 */
void setLEDFeedback(uint8_t aFeedbackLEDPin, bool aEnableLEDFeedback) {

    FeedbackLEDControlSend.FeedbackLEDPin = aFeedbackLEDPin; // default is 0 -> use LED_BUILTIN if available, else disable feedback

    FeedbackLEDControlSend.LedFeedbackEnabled = aEnableLEDFeedback;
    if (aEnableLEDFeedback) {
        if (aFeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
            pinMode(aFeedbackLEDPin, OUTPUT);
#ifdef LED_BUILTIN
        } else {
            pinMode(LED_BUILTIN, OUTPUT);
#else
            FeedbackLEDControlSend.LedFeedbackEnabled = false; // we have no LED_BUILTIN available
#endif
        }
    }
}

/**
 * Enable/disable blinking of Feedback LED (LED_BUILTIN is taken as default) on IR receiving
 * @param aFeedbackLEDPin If aFeedbackLEDPin == 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 * @param aEnableLEDFeedback true -> enable blinking of Feedback LED
 */
void setLEDFeedbackRecv(uint8_t aFeedbackLEDPin, bool aEnableLEDFeedback) {
    FeedbackLEDControlRecv.FeedbackLEDPin = aFeedbackLEDPin; // default is 0

    FeedbackLEDControlRecv.LedFeedbackEnabled = aEnableLEDFeedback;
    if (aEnableLEDFeedback) {
        if (aFeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
            pinMode(aFeedbackLEDPin, OUTPUT);
#ifdef LED_BUILTIN
        } else {
            pinMode(LED_BUILTIN, OUTPUT);
#endif
        }
    }
}

/*
 * Direct replacement for blink13()
 */
void setLEDFeedback(bool aEnableLEDFeedback) {
    setLEDFeedback(FeedbackLEDControlSend.FeedbackLEDPin, aEnableLEDFeedback);
}

void enableLEDFeedback() {
    FeedbackLEDControlSend.LedFeedbackEnabled = true;
}

void disableLEDFeedback() {
    FeedbackLEDControlSend.LedFeedbackEnabled = false;
}

/**
 * Flash LED while sending IR data, if enabled.
 * Handles the LedFeedbackEnabled flag as well as the 0 value of FeedbackLEDPin and the macro FEEDBACK_LED_IS_ACTIVE_LOW.
 */
#if defined(ESP32)
IRAM_ATTR
#endif
void setFeedbackLED(bool aSwitchLedOn) {
    if (FeedbackLEDControlSend.LedFeedbackEnabled) {
        if (aSwitchLedOn) {
            if (FeedbackLEDControlSend.FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWrite(FeedbackLEDControlSend.FeedbackLEDPin, LOW); // Turn user defined pin LED on
#else
                digitalWrite(FeedbackLEDControlSend.FeedbackLEDPin, HIGH); // Turn user defined pin LED on
#endif
#ifdef LED_BUILTIN // use fast macros here
            } else {
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWriteFast(LED_BUILTIN, LOW); // For AVR, this generates a single cbi command
#  else
                digitalWriteFast(LED_BUILTIN, HIGH); // For AVR, this generates a single sbi command
#  endif
#endif
            }
        } else {
            if (FeedbackLEDControlSend.FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWrite(FeedbackLEDControlSend.FeedbackLEDPin, HIGH); // Turn user defined pin LED off
#else
                digitalWrite(FeedbackLEDControlSend.FeedbackLEDPin, LOW); // Turn user defined pin LED off
#endif
#ifdef LED_BUILTIN // use fast macros here
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
}

/**
 * Flash LED while receiving IR data, if enabled.
 * Handles the LedFeedbackEnabled flag as well as the 0 value of FeedbackLEDPin and the macro FEEDBACK_LED_IS_ACTIVE_LOW.
 */
#if defined(ESP32)
IRAM_ATTR
#endif
void setFeedbackLEDRecv(bool aSwitchLedOn) {
    if (FeedbackLEDControlRecv.LedFeedbackEnabled) {
        if (aSwitchLedOn) {
            if (FeedbackLEDControlRecv.FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWrite(FeedbackLEDControlRecv.FeedbackLEDPin, LOW); // Turn user defined pin LED on
#else
                digitalWrite(FeedbackLEDControlRecv.FeedbackLEDPin, HIGH); // Turn user defined pin LED on
#endif
#ifdef FEEDBACK_LED_ON
            } else {
                FEEDBACK_LED_ON();   // if no user defined LED pin, turn default LED pin for the hardware on
#endif
            }
        } else {
            if (FeedbackLEDControlRecv.FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
                digitalWrite(FeedbackLEDControlRecv.FeedbackLEDPin, HIGH); // Turn user defined pin LED off
#else
                digitalWrite(FeedbackLEDControlRecv.FeedbackLEDPin, LOW); // Turn user defined pin LED off
#endif
#ifdef FEEDBACK_LED_OFF
            } else {
                FEEDBACK_LED_OFF();   // if no user defined LED pin, turn default LED pin for the hardware on
#endif
            }
        }
    }
}

/**
 * Old deprecated function name for setLEDFeedback() or enableLEDFeedback() / disableLEDFeedback()
 */
void IRrecv::blink13(bool aEnableLEDFeedback) {
    setLEDFeedback(FeedbackLEDControlSend.FeedbackLEDPin, aEnableLEDFeedback);
}
/**
 * Old deprecated function name for setLEDFeedback()
 */
void setBlinkPin(uint8_t aBlinkPin) {
    setLEDFeedback(aBlinkPin, FeedbackLEDControlSend.LedFeedbackEnabled);
}

/** @}*/

#endif // #ifndef IR_FEEDBACK_LED_HPP
#pragma once
