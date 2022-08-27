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
 * Copyright (c) 2020-2022 Armin Joachimsmeyer
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
#ifndef _IR_PROTOCOL_H
#define _IR_PROTOCOL_H

/**
 * An enum consisting of all supported formats.
 * You do NOT need to remove entries from this list when disabling protocols!
 */
typedef enum {
    UNKNOWN = 0,
#if defined(SUPPORT_PULSE_WIDTH_DECODING) // The only known pulse width protocol is Sony
    PULSE_WIDTH,
#endif
    PULSE_DISTANCE,
    DENON,
    SHARP,
    JVC,
    LG,
    LG2,
    NEC,
    PANASONIC,
    KASEIKYO,
    KASEIKYO_DENON,
    KASEIKYO_SHARP,
    KASEIKYO_JVC,
    KASEIKYO_MITSUBISHI,
    RC5,
    RC6,
    SAMSUNG,
    SAMSUNG_LG,
    SONY,
    NEC2, /* NEC with full frame as repeat */
    ONKYO,
    APPLE,
#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    BOSEWAVE,
    LEGO_PF,
    MAGIQUEST,
    WHYNTER,
#endif
} decode_type_t;

struct PulsePauseWidthProtocolConstants {
    decode_type_t ProtocolIndex;
    uint_fast8_t FrequencyKHz;
    unsigned int HeaderMarkMicros;
    unsigned int HeaderSpaceMicros;
    unsigned int OneMarkMicros;
    unsigned int OneSpaceMicros;
    unsigned int ZeroMarkMicros;
    unsigned int ZeroSpaceMicros;
    bool isMSBFirst;
    bool hasStopBit;
    unsigned int RepeatPeriodMillis;
    void (*SpecialSendRepeatFunction)(); // using non member functions here saves up to 250 bytes for send demo
//    void (IRsend::*SpecialSendRepeatFunction)();
};

const __FlashStringHelper* getProtocolString(decode_type_t aProtocol);

#define PROTOCOL_IS_LSB_FIRST false
#define PROTOCOL_IS_MSB_FIRST true

/*
 * Carrier frequencies for various protocols
 */
#define SONY_KHZ        40
#define BOSEWAVE_KHZ    38
#define DENON_KHZ       38
#define JVC_KHZ         38
#define LG_KHZ          38
#define NEC_KHZ         38
#define SAMSUNG_KHZ     38
#define KASEIKYO_KHZ    37
#define RC5_RC6_KHZ     36

/*
 * Constants for some protocols
 */
#define PANASONIC_VENDOR_ID_CODE    0x2002
#define DENON_VENDOR_ID_CODE        0x3254
#define MITSUBISHI_VENDOR_ID_CODE   0xCB23
#define SHARP_VENDOR_ID_CODE        0x5AAA
#define JVC_VENDOR_ID_CODE          0x0103

#define SIRCS_12_PROTOCOL       12
#define SIRCS_15_PROTOCOL       15
#define SIRCS_20_PROTOCOL       20

#endif // _IR_PROTOCOL_H
