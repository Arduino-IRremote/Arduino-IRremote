/**
 * @file IRProtocol.h
 * @brief Common declarations for receiving and sending.
 *
 * This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2021 Armin Joachimsmeyer
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
#ifndef IR_PROTOCOL_H
#define IR_PROTOCOL_H

/**
 * An enum consisting of all supported formats.
 * You do NOT need to remove entries from this list when disabling protocols!
 */
typedef enum {
    UNKNOWN = 0,
    PULSE_DISTANCE,
    PULSE_WIDTH,
    DENON,
    DISH,
    JVC,
    LG,
    LG2,
    NEC,
    PANASONIC,
    KASEIKYO,
    KASEIKYO_JVC,
    KASEIKYO_DENON,
    KASEIKYO_SHARP,
    KASEIKYO_MITSUBISHI,
    RC5,
    RC6,
    SAMSUNG,
    SHARP,
    SONY,
    ONKYO,
    APPLE,
    BOSEWAVE,
    LEGO_PF,
    MAGIQUEST,
    WHYNTER,
} decode_type_t;

const __FlashStringHelper* getProtocolString(decode_type_t aProtocol);

#define PROTOCOL_IS_LSB_FIRST false
#define PROTOCOL_IS_MSB_FIRST true

/*
 * Constants for some protocols
 */
#define PANASONIC_VENDOR_ID_CODE    0x2002
#define SHARP_VENDOR_ID_CODE        0x5AAA
#define DENON_VENDOR_ID_CODE        0x3254
#define MITSUBISHI_VENDOR_ID_CODE   0xCB23
#define JVC_VENDOR_ID_CODE          0x0103

#define SIRCS_12_PROTOCOL       12
#define SIRCS_15_PROTOCOL       15
#define SIRCS_20_PROTOCOL       20

#define LEGO_MODE_EXTENDED  0
#define LEGO_MODE_COMBO     1
#define LEGO_MODE_SINGLE    0x4 // here the 2 LSB have meanings like Output A / Output B

#endif // IR_PROTOCOL_H

#pragma once
