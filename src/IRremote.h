/**
 * @file IRremote.h
 * @brief Public API to the library.
 *
 *
 * !!! All the macro values defined here can be overwritten with values     !!!
 * !!! the user defines in its source code BEFORE the #include <IRremote.h> !!!
 *
 * This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
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
#ifndef IRremote_h
#define IRremote_h

/*
 * If activated, BOSEWAVE, MAGIQUEST,WHYNTER and LEGO_PF are excluded in decoding and in sending with IrSender.write
 */
//#define EXCLUDE_EXOTIC_PROTOCOLS
/****************************************************
 *                     PROTOCOLS
 ****************************************************/

//------------------------------------------------------------------------------
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to 0) all the protocols you do not need/want!
//
#if(!(DECODE_DENON || DECODE_SHARP || DECODE_JVC || DECODE_KASEIKYO || DECODE_PANASONIC || DECODE_LG || DECODE_NEC \
|| DECODE_SAMSUNG || DECODE_SONY || DECODE_RC5 || DECODE_RC6 || DECODE_HASH || DECODE_BOSEWAVE || DECODE_LEGO_PF || DECODE_MAGIQUEST || DECODE_WHYNTER))
#define DECODE_DENON        1
#define DECODE_SHARP        1 // the same as DECODE_DENON
#define DECODE_JVC          1
#define DECODE_KASEIKYO     1
#define DECODE_PANASONIC    1 // the same as DECODE_KASEIKYO
#define DECODE_LG           1
#define DECODE_NEC          1
#define DECODE_SAMSUNG      1
#define DECODE_SONY         1
#define DECODE_RC5          1
#define DECODE_RC6          1

#if !defined(EXCLUDE_EXOTIC_PROTOCOLS) // saves around 2000 bytes program space
#define DECODE_BOSEWAVE     1
#define DECODE_LEGO_PF      1
#define DECODE_MAGIQUEST    1
#define DECODE_WHYNTER      1
#endif

#define DECODE_HASH         1 // special decoder for all protocols
#endif

/**
 * MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
 * to compensate for the signal forming of different IR receiver modules
 * For Vishay TSOP*, marks tend to be too long and spaces tend to be too short.
 * If you set MARK_EXCESS to approx. 50us then the TSOP4838 works best.
 * At 100us it also worked, but not as well.
 * Set MARK_EXCESS to 100us and the VS1838 doesn't work at all.
 *  Observed values:
 *  Delta of each signal type is around 50 up to 100 and at low signals up to 200. TSOP is better, especially at low IR signal level.
 *  VS1838      Mark Excess -50 to +50 us
 *  TSOP31238   Mark Excess 0 to +50
 */
#if !defined(MARK_EXCESS_MICROS)
//#define MARK_EXCESS_MICROS    50
#define MARK_EXCESS_MICROS    20 // 20 is recommended for the cheap VS1838 modules
#endif

//------------------------------------------------------------------------------
#include "IRremoteInt.h"

/*
 * Include the sources here to enable compilation with macro values set by user program.
 */
#include "irReceive.cpp.h"
#include "irSend.cpp.h"
#include "IRremote.cpp.h"

#endif // IRremote_h

#pragma once

