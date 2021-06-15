/*
 * ReceiveDemo.cpp
 *
 * Demonstrates receiving IR codes with the IRremote library.
 * If debug button is pressed (pin connected to ground) a long output is generated.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
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

#include <Arduino.h>

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols are active.
 * This must be done before the #include <IRremote.h>
 */
//#define DECODE_LG
//#define DECODE_NEC
// etc. see IRremote.h
//
//#define DISABLE_LED_FEEDBACK_FOR_RECEIVE // saves 108 bytes program space
#if FLASHEND <= 0x1FFF  // For 8k flash or less, like ATtiny85. Exclude exotic protocols.
#define EXCLUDE_EXOTIC_PROTOCOLS
#  if !defined(DIGISTUMPCORE) // ATTinyCore is bigger than Digispark core
#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program space.
#  endif
#endif
//#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program space.
//#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program space if all other protocols are active
//#define IR_MEASURE_TIMING

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules.
#define MARK_EXCESS_MICROS    20 // 20 is recommended for the cheap VS1838 modules

#define RECORD_GAP_MICROS 12000 // Activate it for some LG air conditioner protocols

/*
 * First define macros for input and output pin etc.
 */
#include "PinDefinitionsAndMore.h"

#include <IRremote.h>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN // if low, print timing for each received data set
#else
#define DEBUG_BUTTON_PIN   6
#endif

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
    pinMode(IR_TIMING_TEST_PIN, OUTPUT);
#endif
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program space of ATtiny85 etc.
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
// Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

// In case the interrupt driver crashes on setup, give a clue
// to the user what's going on.
    Serial.println(F("Enabling IRin..."));

    /*
     * Start the receiver, enable feedback LED and (if not 3. parameter specified) take LED feedback pin from the internal boards definition
     */
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals at pin "));
#if defined(ARDUINO_ARCH_STM32) || defined(ESP8266)
    Serial.println(IR_RECEIVE_PIN_STRING);
#else
    Serial.println(IR_RECEIVE_PIN);
#endif

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program space of ATtiny85 etc.
    Serial.print(F("Debug button pin is "));
    Serial.println(DEBUG_BUTTON_PIN);

    // infos for receive
    Serial.print(RECORD_GAP_MICROS);
    Serial.println(F(" us is the (minimum) gap, after which the start of a new IR packet is assumed"));
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
#endif

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
        Serial.println();
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            IrReceiver.decodedIRData.flags = false; // yes we have recognized the flag :-)
            Serial.println(F("Overflow detected"));
#  if !defined(ESP32) && !defined(ESP8266) && !defined(NRF5)
            /*
             * do double beep
             */
            IrReceiver.stop();
            tone(TONE_PIN, 1100, 10);
            delay(50);
#  endif

        } else {
            // Print a short summary of received data
            IrReceiver.printIRResultShort(&Serial);

            if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
                // We have an unknown protocol, print more info
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
        }

#  if !defined(ESP32) && !defined(ESP8266) && !defined(NRF5)
        if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
            /*
             * Play tone, wait and restore IR timer, if a valid protocol was received
             * Otherwise do not disturb the detection of the gap between transmissions. This will give
             * the next printIRResult* call a chance to report about changing the RECORD_GAP_MICROS value.
             */
            IrReceiver.stop();
            tone(TONE_PIN, 2200, 10);
            delay(8);
            IrReceiver.start(8000); // to compensate for 8 ms stop of receiver. This enables a correct gap measurement.
        }
#  endif
#else
        // Print a minimal summary of received data
        IrReceiver.printIRResultMinimal(&Serial);
#endif // FLASHEND

        /*
         * !!!Important!!! Enable receiving of the next value,
         * since receiving has stopped after the end of the current received data packet.
         */
        IrReceiver.resume();

        /*
         * Finally check the received data and perform actions according to the received address and commands
         */
        if (IrReceiver.decodedIRData.address == 0) {
            if (IrReceiver.decodedIRData.command == 0x10) {
                // do something
            } else if (IrReceiver.decodedIRData.command == 0x11) {
                // do something else
            }
        }
    }
}
