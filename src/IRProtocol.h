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
 * Copyright (c) 2020-2025 Armin Joachimsmeyer
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

/*
 * If activated, BANG_OLUFSEN, BOSEWAVE, MAGIQUEST, WHYNTER, FAST and LEGO_PF are excluded in decoding and in sending with IrSender.write
 */
//#define EXCLUDE_EXOTIC_PROTOCOLS

/*
 * Supported IR protocols
 * Each protocol you include costs memory and, during decode, costs time
 * Copy the lines with the protocols you need in your program before the  #include <IRremote.hpp> line
 * See also SimpleReceiver example
 */

#if !defined(NO_DECODER) // for sending raw only
#  if (!(defined(DECODE_DENON) || defined(DECODE_JVC) || defined(DECODE_KASEIKYO) \
|| defined(DECODE_PANASONIC) || defined(DECODE_LG) || defined(DECODE_NEC) || defined(DECODE_ONKYO) || defined(DECODE_SAMSUNG) \
|| defined(DECODE_SONY) || defined(DECODE_RC5) || defined(DECODE_RC6) \
|| defined(DECODE_DISTANCE_WIDTH) || defined(DECODE_HASH) || defined(DECODE_BOSEWAVE) \
|| defined(DECODE_LEGO_PF) || defined(DECODE_MAGIQUEST) || defined(DECODE_FAST) || defined(DECODE_WHYNTER)))
/*
 * If no protocol is explicitly enabled, we enable all protocols
 */
#define DECODE_DENON        // Includes Sharp
#define DECODE_JVC
#define DECODE_KASEIKYO
#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
#define DECODE_LG
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_SAMSUNG
#define DECODE_SONY
#define DECODE_RC5
#define DECODE_RC6

#    if !defined(EXCLUDE_EXOTIC_PROTOCOLS) // saves around 2000 bytes program memory
#define DECODE_BOSEWAVE
#define DECODE_LEGO_PF
#define DECODE_MAGIQUEST
#define DECODE_WHYNTER
#define DECODE_FAST
#    endif

#    if !defined(EXCLUDE_UNIVERSAL_PROTOCOLS)
#define DECODE_DISTANCE_WIDTH     // universal decoder for pulse distance width protocols - requires up to 750 bytes additional program memory
#define DECODE_HASH         // special decoder for all protocols - requires up to 250 bytes additional program memory
#    endif
#  endif
#endif // !defined(NO_DECODER)

//#define DECODE_BEO // Bang & Olufsen protocol always must be enabled explicitly. It prevents decoding of SONY!

#if defined(DECODE_NEC) && !(~(~DECODE_NEC + 0) == 0 && ~(~DECODE_NEC + 1) == 1)
#warning "The macros DECODE_XXX no longer require a value. Decoding is now switched by defining / non defining the macro."
#endif

/**
 * An enum consisting of all supported formats.
 * You do NOT need to remove entries from this list when disabling protocols!
 * !!!Must be the same order as ProtocolNames in IRReceive.hpp!!!
 */
typedef enum {
    UNKNOWN = 0,
    PULSE_WIDTH,
    PULSE_DISTANCE,
    APPLE,
    DENON,
    JVC,
    LG,
    LG2,
    NEC,
    NEC2, /* 10 NEC with full frame as repeat */
    ONKYO,
    PANASONIC,
    KASEIKYO,
    KASEIKYO_DENON,
    KASEIKYO_SHARP,
    KASEIKYO_JVC,
    KASEIKYO_MITSUBISHI,
    RC5,
    RC6,
    RC6A, /*31 bit +  3 fixed 0b110 mode bits*/
    SAMSUNG, /* 20*/
    SAMSUNGLG,
    SAMSUNG48,
    SHARP,
    SONY,
    /* Now the exotic protocols */
    BANG_OLUFSEN,
    BOSEWAVE,
    LEGO_PF,
    MAGIQUEST,
    WHYNTER, /* 30 */
    FAST,
    OTHER
} decode_type_t;
extern const char *const ProtocolNames[]; // The array of name strings for the decode_type_t enum

#define SIRCS_12_PROTOCOL       12
#define SIRCS_15_PROTOCOL       15
#define SIRCS_20_PROTOCOL       20

struct DistanceWidthTimingInfoStruct {
    uint16_t HeaderMarkMicros;
    uint16_t HeaderSpaceMicros;
    uint16_t OneMarkMicros;
    uint16_t OneSpaceMicros;
    uint16_t ZeroMarkMicros;
    uint16_t ZeroSpaceMicros;
};

/*
 * Definitions for member IRData.flags
 */
#define IRDATA_FLAGS_EMPTY              0x00
#define IRDATA_FLAGS_IS_REPEAT          0x01 ///< The gap between the preceding frame is as smaller than the maximum gap expected for a repeat. !!!We do not check for changed command or address, because it is almost not possible to press 2 different buttons on the remote within around 100 ms!!!
#define IRDATA_FLAGS_IS_AUTO_REPEAT     0x02 ///< The current repeat frame is a repeat, that is always sent after a regular frame and cannot be avoided. Only specified for protocols DENON, and LEGO.
#define IRDATA_FLAGS_PARITY_FAILED      0x04 ///< The current (autorepeat) frame violated parity check.
#define IRDATA_FLAGS_TOGGLE_BIT         0x08 ///< Is set if RC5 or RC6 toggle bit is set.
#define IRDATA_TOGGLE_BIT_MASK          0x08 ///< deprecated -is set if RC5 or RC6 toggle bit is set.
#define IRDATA_FLAGS_EXTRA_INFO         0x10 ///< There is extra info not contained in address and data (e.g. Kaseikyo unknown vendor ID, or in decodedRawDataArray).
#define IRDATA_FLAGS_IS_PROTOCOL_WITH_DIFFERENT_REPEAT 0x20 ///< Here we have a repeat of type NEC2 or SamsungLG
#define IRDATA_FLAGS_WAS_OVERFLOW       0x40 ///< irparams.rawlen is set to 0 in this case to avoid endless OverflowFlag.
#define IRDATA_FLAGS_IS_MSB_FIRST       0x80 ///< Value is mainly determined by the (known) protocol.
#define IRDATA_FLAGS_IS_LSB_FIRST       0x00
#define IRDATA_FLAGS_LSB_MSB_FIRST_MASK IRDATA_FLAGS_IS_MSB_FIRST

extern uint8_t sLastSendToggleValue; // Currently used by RC5 + RC6

struct PulseDistanceWidthProtocolConstants {
    decode_type_t ProtocolIndex;
    uint_fast8_t FrequencyKHz;
    DistanceWidthTimingInfoStruct DistanceWidthTimingInfo;
    uint8_t Flags;
    unsigned int RepeatPeriodMillis; // Time between start of two frames. Thus independent from frame length.
    void (*SpecialSendRepeatFunction)(); // using non member functions here saves up to 250 bytes for send demo
//    void (IRsend::*SpecialSendRepeatFunction)();
};
/*
 * Definitions for member PulseDistanceWidthProtocolConstants.Flags
 */
#define PROTOCOL_IS_PULSE_DISTANCE      0x00
#define PROTOCOL_IS_PULSE_DISTANCE_WIDTH 0x00 // can often successfully be decoded as pulse distance
#define PROTOCOL_IS_PULSE_WIDTH         0x10
#define PROTOCOL_IS_PULSE_WIDTH_MASK    PROTOCOL_IS_PULSE_WIDTH
#define SUPPRESS_STOP_BIT               0x20 // Stop bit is otherwise sent for all pulse distance protocols, i.e. aOneSpaceMicros != aZeroSpaceMicros.
#define PROTOCOL_IS_MSB_FIRST           IRDATA_FLAGS_IS_MSB_FIRST
#define PROTOCOL_IS_LSB_FIRST           IRDATA_FLAGS_IS_LSB_FIRST
#define PROTOCOL_IS_MSB_MASK            IRDATA_FLAGS_IS_MSB_FIRST

/*
 * Carrier frequencies for various protocols
 */
#if !defined(BEO_KHZ) // guard used for unit test, which sends and receive Bang&Olufsen with 38 kHz.
#define BEO_KHZ         455
#endif
#define SONY_KHZ        40
#define BOSEWAVE_KHZ    38
#define DENON_KHZ       38
#define JVC_KHZ         38
#define LG_KHZ          38
#define NEC_KHZ         38
#define SAMSUNG_KHZ     38
#define KASEIKYO_KHZ    37
#define RC5_RC6_KHZ     36

#if defined(__AVR__)
const __FlashStringHelper* getProtocolString(decode_type_t aProtocol);
#else
const char* getProtocolString(decode_type_t aProtocol);
#endif

/*
 * Convenience functions to convert MSB to LSB values
 */
uint8_t bitreverseOneByte(uint8_t aValue);
uint32_t bitreverse32Bit(uint32_t aInput);

#endif // _IR_PROTOCOL_H
