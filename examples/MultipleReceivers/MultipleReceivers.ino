/*
 * MultipleReceivers.cpp
 *
 * Demonstrates receiving from multiple receivers at different pins using multiple IRreceiver instances
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2025 Armin Joachimsmeyer
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

#define IR_RECEIVE_PIN_OF_SECOND_RECEIVER   5

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols (except Bang&Olufsen) are active.
 * This must be done before the #include <IRremote.hpp>
 */
//#define DECODE_DENON        // Includes Sharp
//#define DECODE_JVC
//#define DECODE_KASEIKYO
//#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
//#define DECODE_LG
//#define DECODE_NEC          // Includes Apple and Onkyo. To enable all protocols , just comment/disable this line.
//#define DECODE_SAMSUNG
//#define DECODE_SONY
//#define DECODE_RC5
//#define DECODE_RC6
//#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
//#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER
//#define DECODE_FAST
//#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
//#define DECODE_HASH         // special decoder for all protocols
//#define DECODE_BEO          // This protocol must always be enabled manually, i.e. it is NOT enabled if no protocol is defined. It prevents decoding of SONY!
//#define DEBUG               // Activate this for lots of lovely debug output from the decoders.
//#define RAW_BUFFER_LENGTH  750 // For air condition remotes it may require up to 750. Default is 200.
#define SUPPORT_MULTIPLE_RECEIVER_INSTANCES
void UserIRReceiveTimerInterruptHandler(); // must also be before line #include <IRremote.hpp>

/*
 * This include defines the actual pin number for pins like IR_RECEIVE_PIN, IR_SEND_PIN for many different boards and architectures
 */
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

IRrecv MySecondIrReceiver(IR_RECEIVE_PIN_OF_SECOND_RECEIVER); // This sets the pin for the second instance

void handleSuccessfulDecoding(IRrecv *aIRReceiverInstance);

void setup() {
    Serial.begin(115200);

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // This sets the pin for the default / first instance and enables the global LED feedback

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN) " and pin " STR(IR_RECEIVE_PIN_OF_SECOND_RECEIVER)));
}

void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     *
     * E.g. command is in IrReceiver.decodedIRData.command
     * address is in command is in IrReceiver.decodedIRData.address
     * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
     */
    if (IrReceiver.decode()) {
        handleSuccessfulDecoding(&IrReceiver);
    }
    if (MySecondIrReceiver.decode()) {
        handleSuccessfulDecoding(&MySecondIrReceiver);
    }
}

void handleSuccessfulDecoding(IRrecv *aIRReceiverInstance) {

    Serial.print(F("Receiver at pin "));
    Serial.print(aIRReceiverInstance->irparams.IRReceivePin);
    Serial.print(F(": "));

    /*
     * Print a summary of received data
     */
    if (aIRReceiverInstance->decodedIRData.protocol == UNKNOWN) {
        Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        // We have an unknown protocol here, print extended info
        aIRReceiverInstance->printIRResultRawFormatted(&Serial, true);

        aIRReceiverInstance->resume(); // Do it here, to preserve raw data for printing with printIRResultRawFormatted()
    } else {
        aIRReceiverInstance->resume(); // Early enable receiving of the next IR frame

        aIRReceiverInstance->printIRResultShort(&Serial);
        aIRReceiverInstance->printIRSendUsage(&Serial);
    }
    Serial.println();

    /*
     * Finally, check the received data and perform actions according to the received command
     */
    if (aIRReceiverInstance->decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
        Serial.println(F("Repeat received. Here you can repeat the same action as before."));
    } else {
        if (aIRReceiverInstance->decodedIRData.command == 0x10) {
            Serial.println(F("Received command 0x10."));
            // do something
        } else if (aIRReceiverInstance->decodedIRData.command == 0x11) {
            Serial.println(F("Received command 0x11."));
            // do something else
        }
    }

}

void UserIRReceiveTimerInterruptHandler() {
    MySecondIrReceiver.ReceiveInterruptHandler();
}
