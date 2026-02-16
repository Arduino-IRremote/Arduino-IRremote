/*
 * CallbackDemo.cpp
 *
 * Demonstrates receiving NEC IR codes with IRrecv using callback function
 * Based on SimpleReceiver example
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2026 Armin Joachimsmeyer
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
#define DECODE_NEC          // Includes Apple and Onkyo
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

#include <Arduino.h>

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

/*
 * Callback specific variables and functions
 */
volatile bool sIRDataJustReceived = false;
void ReceiveCompleteCallbackHandler();

void setup() {
    Serial.begin(115200);

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    /*
     * Tell the ISR to call this function, when a complete frame has been received
     */
    IrReceiver.registerReceiveCompleteCallback(ReceiveCompleteCallbackHandler);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
    Serial.print(F("Using callback function for processing received data"));

}

void loop() {
    /*
     * Print in loop (interrupts are enabled here) if received data is available.
     */
    if (sIRDataJustReceived) {
        sIRDataJustReceived = false;
        /*
         * Print info of the received data
         */
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
            /*
             * We have an unknown protocol here, print more info.
             * This will print incorrect timing values, if we are late (not in this simple example :-))
             * and the the first mark of the next (repeat) data was yet received.
             * Therefore we first test for this condition.
             */
            if (!IrReceiver.isIdle()) {
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
        } else {
            IrReceiver.printIRResultShort(&Serial);
            IrReceiver.printIRSendUsage(&Serial);
        }
        Serial.println();
    }
}

/*
 * Callback function
 * Here we know, that data is available.
 * This function is executed in an ISR (Interrupt Service Routine) context.
 * This means that interrupts are blocked here, so delay(), millis() and Serial prints of data longer than the print buffer size etc. will block forever.
 * This is because they require their internal interrupt routines to run in order to return.
 * Therefore it is best to make this callback function short and fast!
 * A dirty hack is to enable interrupts again by calling sei() (enable interrupt again), but you should know what you are doing,
 *
 * A good practice -but somewhat more complex- is to perform small and time critical actions in the callback function,
 * copy the relevant data to local variables and signal the receiving to main loop, which in turn processes this data.
 * This is similar to simply using "if (IrReceiver.decode())", but has the advantage,
 * that it does not block the receiving of the next IR frame, until the main loop reaches this statement.
 */
#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
# endif
void ReceiveCompleteCallbackHandler() {
    /*
     * Fill IrReceiver.decodedIRData
     */
    IrReceiver.decode();

    /*
     * Enable receiving of the next frame.
     */
    IrReceiver.resume();

    /*
     * Check the received data and perform actions according to the received command
     * Decoded result is in the IrReceiver.decodedIRData structure.
     *
     * E.g. command is in IrReceiver.decodedIRData.command,
     * address is in command is in IrReceiver.decodedIRData.address
     * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
     */
    if (IrReceiver.decodedIRData.command == 0x10) {
        // do something SHORT here
    } else if (IrReceiver.decodedIRData.command == 0x11) {
        // do something SHORT here too
    }

    /*
     * Set flag to trigger printing of results in main loop,
     * since printing should not be done in a callback function
     * running in ISR (Interrupt Service Routine) context where interrupts are disabled.
     */
    sIRDataJustReceived = true;
}
