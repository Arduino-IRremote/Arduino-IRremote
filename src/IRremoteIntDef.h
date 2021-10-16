/**
 * @file IRremoteIntDef.h
 * @brief Contains all declarations required for the internal functions.
 *
 * This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2015-2021 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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
#ifndef IRremoteIntDef_h
#define IRremoteIntDef_h

#include <Arduino.h>

/*
 * The length of the buffer where the IR timing data is stored before decoding
 * 100 is sufficient for most standard protocols, but air conditioners often send a longer protocol data stream
 */
#if !defined(RAW_BUFFER_LENGTH)
#define RAW_BUFFER_LENGTH  100  ///< Maximum length of raw duration buffer. Must be even. 100 supports up to 48 bit codings inclusive 1 start and 1 stop bit.
//#define RAW_BUFFER_LENGTH  750  // Value for air condition remotes.
#endif
#if RAW_BUFFER_LENGTH % 2 == 1
#error RAW_BUFFER_LENGTH must be even, since the array consists of space / mark pairs.
#endif

#define MARK   1
#define SPACE  0

//#define DEBUG // Activate this for lots of lovely debug output from the IRremote core and all protocol decoders.
//#define TRACE // Activate this for more debug output.

/**
 * For better readability of code
 */
#define DISABLE_LED_FEEDBACK false
#define ENABLE_LED_FEEDBACK true
#define USE_DEFAULT_FEEDBACK_LED_PIN 0

#include "IRProtocol.h"

/****************************************************
 * Declarations for the receiver Interrupt Service Routine
 ****************************************************/
// ISR State-Machine : Receiver States
#define IR_REC_STATE_IDLE      0
#define IR_REC_STATE_MARK      1
#define IR_REC_STATE_SPACE     2
#define IR_REC_STATE_STOP      3 // set to IR_REC_STATE_IDLE only by resume()

/**
 * This struct contains the data and control used for receiver static functions and the ISR (interrupt service routine)
 * Only StateForISR needs to be volatile. All the other fields are not written by ISR after data available and before start/resume.
 */
struct irparams_struct {
    // The fields are ordered to reduce memory over caused by struct-padding
    volatile uint8_t StateForISR;   ///< State Machine state
    uint8_t IRReceivePin;           ///< Pin connected to IR data from detector
#if defined(__AVR__)
    volatile uint8_t *IRReceivePinPortInputRegister;
    uint8_t IRReceivePinMask;
#endif
    uint16_t TickCounterForISR;     ///< Counts 50uS ticks. The value is copied into the rawbuf array on every transition.

    bool OverflowFlag;              ///< Raw buffer OverflowFlag occurred
#if RAW_BUFFER_LENGTH <= 254        // saves around 75 bytes program space and speeds up ISR
    uint8_t rawlen;                 ///< counter of entries in rawbuf
#else
    unsigned int rawlen;            ///< counter of entries in rawbuf
#endif
    uint16_t rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space, first entry is the length of the gap between previous and current command
};

/*
 * Info directives
 * Can be disabled to save program space
 */
#ifdef INFO
#  define INFO_PRINT(...)    Serial.print(__VA_ARGS__)
#  define INFO_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
/**
 * If INFO, print the arguments, otherwise do nothing.
 */
#  define INFO_PRINT(...) void()
/**
 * If INFO, print the arguments as a line, otherwise do nothing.
 */
#  define INFO_PRINTLN(...) void()
#endif

/*
 * Debug directives
 */
#ifdef DEBUG
#  define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
#  define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
/**
 * If DEBUG, print the arguments, otherwise do nothing.
 */
#  define DEBUG_PRINT(...) void()
/**
 * If DEBUG, print the arguments as a line, otherwise do nothing.
 */
#  define DEBUG_PRINTLN(...) void()
#endif

#ifdef TRACE
#  define TRACE_PRINT(...)    Serial.print(__VA_ARGS__)
#  define TRACE_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#  define TRACE_PRINT(...) void()
#  define TRACE_PRINTLN(...) void()
#endif

#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
#define COMPILER_HAS_PRAGMA_MESSAGE
#endif

#endif // IRremoteIntDef_h

