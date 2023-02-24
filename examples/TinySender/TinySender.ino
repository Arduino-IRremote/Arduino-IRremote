/*
 * TinySender.cpp
 *
 *  Example for sending FAST or NEC protocol using TinyIR.
 *
 *  NEC protocol codes are sent in standard format with 8bit address and 8 bit command as in SimpleSender example.
 *  Saves 780 bytes program memory and 26 bytes RAM compared to SimpleSender, which does the same, but uses the IRRemote library (and is therefore much more flexible).
 *
 * The FAST protocol is a proprietary modified JVC protocol without address, with parity and with a shorter header.
 *  FAST Protocol characteristics:
 * - Bit timing is like NEC or JVC
 * - The header is shorter, 3156 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *     leading to a fixed protocol length of (6 + (16 * 3) + 1) * 526 = 55 * 526 = 28930 microseconds or 29 ms.
 * - Repeats are sent as complete frames but in a 50 ms period / with a 21 ms distance.
 *
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Armin Joachimsmeyer
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
#include <Arduino.h>

//#define USE_FAST_PROTOCOL     // Use FAST protocol. No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command
//#define DISABLE_PARITY_CHECKS // Disable parity checks. Saves 48 bytes of program memory.
//#define USE_ONKYO_PROTOCOL    // Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value.
//#define USE_FAST_PROTOCOL     // Use FAST protocol (no address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command) instead of NEC.
//#define ENABLE_NEC2_REPEATS   // Instead of sending / receiving the NEC special repeat code, send / receive the original frame for repeat.

#include "TinyIRSender.hpp"

#define IR_SEND_PIN         3

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_TINYIR));
    Serial.print(F("Send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

/*
 * Set up the data to be sent.
 * The compiler is intelligent and removes the code for 16 bit address handling if we call it with an uint8_t address :-).
 * Using an uint16_t address or data requires additional 28 bytes program memory for NEC and 56 bytes program memory for FAST.
 */
uint8_t sAddress = 0x02;
//uint16_t sAddress = 0x02;
uint8_t sCommand = 0x34;
//uint16_t sCommand = 0x34;
uint8_t sRepeats = 0;

void loop() {
    /*
     * Print current send values
     */
    Serial.println();
    Serial.print(F("Send now:"));
#if defined(USE_FAST_PROTOCOL)
    Serial.print(F(" address=0x"));
    Serial.print(sAddress, HEX);
#endif
    Serial.print(F(" command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(" repeats="));
    Serial.print(sRepeats);
    Serial.println();

#if defined(USE_FAST_PROTOCOL)
    Serial.println(F("Send FAST with 8 bit address"));
    Serial.flush();
    sendFAST(IR_SEND_PIN, sCommand, sRepeats);
#elif defined(USE_ONKYO_PROTOCOL)
    Serial.println(F("Send ONKYO with 16 bit address and command"));
    Serial.flush();
    sendONKYO(IR_SEND_PIN, sAddress, sCommand, sRepeats);
#else
    Serial.println(F("Send NEC with 8 bit address"));
    Serial.flush();
    sendNEC(IR_SEND_PIN, sAddress, sCommand, sRepeats);
#endif
    /*
     * Increment send values
     * Also increment address just for demonstration, which normally makes no sense
     */
    sAddress += 0x0101;
    sCommand += 0x11;
    sRepeats++;
    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }

    delay(1000);  // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal
}
