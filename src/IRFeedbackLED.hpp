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
    uint8_t FeedbackLEDPin = USE_DEFAULT_FEEDBACK_LED_PIN; ///< if USE_DEFAULT_FEEDBACK_LED_PIN / 0, then use digitalWriteFast(LED_BUILTIN,..) otherwise use digitalWrite(FeedbackLEDPin,..)
    bool LedFeedbackEnabled; ///< Disabled for receive at default. Feedback for send is always enabled and can be disabled by NO_LED_SEND_FEEDBACK_CODE or #define NO_LED_FEEDBACK_CODE macros
};

struct FeedbackLEDControlStruct FeedbackLEDControl; ///< The feedback LED control instance

/**
 * @param aFeedbackLEDPin If FeedbackLEDPin != USE_DEFAULT_FEEDBACK_LED_PIN / 0, then use digitalWrite(FeedbackLEDPin,..)
 *                        If FeedbackLEDPin == USE_DEFAULT_FEEDBACK_LED_PIN / 0 and no LED_BUILTIN defined, disable LED feedback
 */
void setLEDFeedbackPin(uint8_t aFeedbackLEDPin) {
    FeedbackLEDControl.FeedbackLEDPin = aFeedbackLEDPin;
}

/*
 * Direct replacement for blink13()
 */
void setLEDFeedback(bool aEnableLEDFeedback) {
    FeedbackLEDControl.LedFeedbackEnabled = aEnableLEDFeedback;
}

/*
 * Historically this only affects receive LED
 */
void enableLEDFeedback() {
    FeedbackLEDControl.LedFeedbackEnabled = ENABLE_LED_FEEDBACK;
}
void disableLEDFeedback() {
    FeedbackLEDControl.LedFeedbackEnabled = DISABLE_LED_FEEDBACK;
}

/**
 * Flash LED while receiving or sending IR data. Does not check if enabled, this must be done by the caller.
 * Handles the USE_DEFAULT_FEEDBACK_LED_PIN / 0 value of FeedbackLEDPin and the macro FEEDBACK_LED_IS_ACTIVE_LOW.
 * If FeedbackLEDPin == USE_DEFAULT_FEEDBACK_LED_PIN and LED_BUILTIN is NOT defined no action is done
 */
#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
#endif
void setFeedbackLED(bool aSwitchLedOn) {
    if (aSwitchLedOn) {
        // Turn user defined pin LED on
        if (FeedbackLEDControl.FeedbackLEDPin == USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(LED_BUILTIN) // use fast macros here
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            digitalWriteFast(LED_BUILTIN, LOW); // For AVR, this generates a single cbi command
#  else
            digitalWriteFast(LED_BUILTIN, HIGH); // For AVR, this generates a single sbi command
#  endif
#endif

        } else {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            if (__builtin_constant_p(FeedbackLEDControl.FeedbackLEDPin) ) {
                digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, LOW);
            } else {
                digitalWrite(FeedbackLEDControl.FeedbackLEDPin, LOW);
            }
#else
            if (__builtin_constant_p(FeedbackLEDControl.FeedbackLEDPin)) {
                digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, HIGH);
            } else {
                digitalWrite(FeedbackLEDControl.FeedbackLEDPin, HIGH);
            }
#endif
        }

    } else {
        // Turn user defined pin LED off
        if (FeedbackLEDControl.FeedbackLEDPin == USE_DEFAULT_FEEDBACK_LED_PIN) {
#if defined(LED_BUILTIN)
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            digitalWriteFast(LED_BUILTIN, HIGH); // For AVR, this generates a single sbi command
#  else
            digitalWriteFast(LED_BUILTIN, LOW); // For AVR, this generates a single cbi command
#  endif
#endif
        } else {
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
            if (__builtin_constant_p(FeedbackLEDControl.FeedbackLEDPin) ) {
                digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, HIGH);
            } else {
                digitalWrite(FeedbackLEDControl.FeedbackLEDPin, HIGH);
            }
#else
            if (__builtin_constant_p(FeedbackLEDControl.FeedbackLEDPin)) {
                digitalWriteFast(FeedbackLEDControl.FeedbackLEDPin, LOW);
            } else {
                digitalWrite(FeedbackLEDControl.FeedbackLEDPin, LOW);
            }
#endif
        }
    }
}

/**
 * Old deprecated function name for setLEDFeedback() or enableLEDFeedback() / disableLEDFeedback()
 */
void IRrecv::blink13(uint8_t aEnableLEDFeedback) {
    setLEDFeedback(aEnableLEDFeedback);
}
/**
 * Old deprecated function name for setLEDFeedback()
 */
void setBlinkPin(uint8_t aBlinkPin) {
    setLEDFeedbackPin(aBlinkPin);
}

/** @}*/

#endif // _IR_FEEDBACK_LED_HPP
