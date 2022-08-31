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
 * !!!Must be the same order as ProtocolNames in IRReceive.hpp!!!
 */
typedef enum {
    UNKNOWN = 0,
#if defined(SUPPORT_PULSE_WIDTH_DECODING) // The only known pulse width protocol is Sony
    PULSE_WIDTH,
#endif
    PULSE_DISTANCE,
    APPLE,
    DENON,
    JVC,
    LG,
    LG2,
    NEC,
    NEC2, /* NEC with full frame as repeat */
    ONKYO,
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
    SHARP,
    SONY,
    /* Now the exotic protocols */
    BOSEWAVE,
    LEGO_PF,
    MAGIQUEST,
    WHYNTER,

} decode_type_t;

const char string_Unknown[] PROGMEM = "UNKNOWN";
const char string_PulseWidth[] PROGMEM = "PulseWidth";
const char string_PulseDistance[] PROGMEM = "PulseDistance";
const char string_Apple[] PROGMEM = "Apple";
const char string_Denon[] PROGMEM = "Denon";
const char string_JVC[] PROGMEM = "JVC";
const char string_LG[] PROGMEM = "LG";
const char string_LG2[] PROGMEM = "LG2";
const char string_NEC[] PROGMEM = "NEC";
const char string_NEC2[] PROGMEM = "NEC2";
const char string_Onkyo[] PROGMEM = "Onkyo";
const char string_Panasonic[] PROGMEM = "Panasonic";
const char string_Kaseikyo[] PROGMEM = "Kaseikyo";
const char string_Kaseikyo_Denon[] PROGMEM = "Kaseikyo_Denon";
const char string_Kaseikyo_Sharp[] PROGMEM = "Kaseikyo_Sharp";
const char string_Kaseikyo_JVC[] PROGMEM = "Kaseikyo_JVC";
const char string_Kaseikyo_Mitsubishi[] PROGMEM = "Kaseikyo_Mitsubishi";
const char string_RC5[] PROGMEM = "RC5";
const char string_RC6[] PROGMEM = "RC6";
const char string_Samsung[] PROGMEM = "Samsung";
const char string_SamsungLG[] PROGMEM = "SamsungLG";
const char string_Sharp[] PROGMEM = "Sharp";
const char string_Sony[] PROGMEM = "Sony";
const char string_BoseWave[] PROGMEM = "BoseWave";
const char string_Lego[] PROGMEM = "Lego";
const char string_MagiQuest[] PROGMEM = "MagiQuest";
const char string_Whynter[] PROGMEM = "Whynter";

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

#endif // _IR_PROTOCOL_H
