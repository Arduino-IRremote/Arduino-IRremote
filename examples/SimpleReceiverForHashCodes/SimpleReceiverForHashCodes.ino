/*
 * SimpleReceiverForHashCodes.cpp
 *
 * Demonstrates receiving hash codes of unknown protocols with IRremote
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2024 Armin Joachimsmeyer
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

/*
 * Specify which protocol(s) should be used for decoding.
 * This must be done before the #include <IRremote.hpp>
 */
#define DECODE_HASH             // special decoder for all protocols
#define RAW_BUFFER_LENGTH  1000 // Especially useful for unknown and probably long protocols
//#define DEBUG                 // Activate this for lots of lovely debug output from the decoders.

/*
 * This include defines the actual pin number for pins like IR_RECEIVE_PIN, IR_SEND_PIN for many different boards and architectures
 */
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.println(F("Ready to receive unknown IR signals at pin " STR(IR_RECEIVE_PIN) " and decode it with hash decoder."));
}

void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded hash result is in IrReceiver.decodedIRData.decodedRawData
     */
    if (IrReceiver.available()) {
        IrReceiver.initDecodedIRData(); // is required, if we do not call decode();
        IrReceiver.decodeHash();
        IrReceiver.resume(); // Early enable receiving of the next IR frame
        /*
         * Print a summary and then timing of received data
         */
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRResultRawFormatted(&Serial, true);

        Serial.println();

        /*
         * Finally, check the received data and perform actions according to the received command
         */
        auto tDecodedRawData = IrReceiver.decodedIRData.decodedRawData; // uint32_t on 8 and 16 bit CPUs and uint64_t on 32 and 64 bit CPUs
        if (tDecodedRawData == 0x4F7BE2FB) {
            // do something
        } else if (tDecodedRawData == 0x97483BFB) {
            // do something else
        }
    }
}
