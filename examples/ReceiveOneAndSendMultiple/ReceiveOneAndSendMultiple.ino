/*
 *  ReceiveOneAndSendMultiple.cpp
 *
 *  Serves as a IR remote macro expander
 *  Receives Samsung32 protocol and on receiving a specified input frame,
 *  it sends multiple Samsung32 frames with appropriate delays in between.
 *  This serves as a Netflix-key emulation for my old Samsung H5273 TV.
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

// Digispark ATMEL ATTINY85
// Piezo speaker must have a 270 Ohm resistor in series for USB programming and running at the Samsung TV.
// IR LED has a 270 Ohm resistor in series.
//                                                    +-\/-+
//                                   !RESET (5) PB5  1|    |8  Vcc
// USB+ 3.6V Z-Diode, 1.5kOhm to VCC  Piezo (3) PB3  2|    |7  PB2 (2) TX Debug output
// USB- 3.6V Z-Diode              IR Output (4) PB4  3|    |6  PB1 (1) Feedback LED
//                                              GND  4|    |5  PB0 (0) IR Input
//                                                    +----+
#include <Arduino.h>

// select only Samsung protocol for sending and receiving
#define DECODE_SAMSUNG
#define ADDRESS_OF_SAMSUNG_REMOTE   0x0707 // The value you see as address in printIRResultShort()
/*
 * Define macros for input and output pin etc.
 */
#include "PinDefinitionsAndMore.h"

#include <IRremote.hpp>

void sendSamsungSmartHubMacro(bool aDoSelect);
void IRSendWithDelay(uint8_t aCommand, uint16_t aDelayMillis);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // tone before IR setup, since it kills the IR timer settings
    tone(TONE_PIN, 2200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(400);
    digitalWrite(LED_BUILTIN, LOW);
    noTone(TONE_PIN);

    /*
     * Start the receiver, enable feedback LED and take LED feedback pin from the internal boards definition
     */
    IrReceiver.begin(IR_RECEIVE_PIN);
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.print(F("at pin "));
#if defined(ARDUINO_ARCH_STM32) || defined(ESP8266)
    Serial.println(IR_RECEIVE_PIN_STRING);
#else
    Serial.println(IR_RECEIVE_PIN);
#endif
    Serial.print(F("Ready to send IR signals at pin "));
#if defined(ARDUINO_ARCH_STM32) || defined(ESP8266)
    Serial.println(IR_SEND_PIN_STRING);
#else
    Serial.println(IR_SEND_PIN);
#endif

}

void loop() {
    /*
     * Check if new data available and get them
     */
    if (IrReceiver.decode()) {
        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);
        Serial.println();

        /*
         * Here data is available -> evaluate IR command
         */
        switch (IrReceiver.decodedIRData.command) {
        case 0x47: // The play key on the bottom of my Samsung remote
            Serial.println(F("Play key detected, open Netflix"));
            sendSamsungSmartHubMacro(true);
            break;

        case 0x4A: // The pause key on the bottom of my Samsung remote
            Serial.println(F("Pause key detected, open SmartHub"));
            sendSamsungSmartHubMacro(false);
            break;

        default:
            break;
        }

        /*
         * !!!Important!!! Enable receiving of the next value,
         * since receiving has stopped after the end of the current received data packet.
         */
        IrReceiver.resume(); // Enable receiving of the next value
    }
}

void IRSendWithDelay(uint8_t aCommand, uint16_t aDelayMillis) {
    IrSender.sendSamsung(ADDRESS_OF_SAMSUNG_REMOTE, aCommand, 1); // send with one repeat
    Serial.print(F("Send Samsung command 0x"));
    Serial.println(aCommand);
    delay(aDelayMillis);
}

bool sMacroWasCalledBefore = false;
#define INITIAL_WAIT_TIME_APPS_READY_MILLIS 70000 // Time to let the TV load all software before Netflix can be started without an error
#define INITIAL_WAIT_TIME_SMARTHUB_READY_MILLIS 20000 // Time to let the TV load all software before SmartHub manu can be displayed

/*
 * This macro calls the last SmartHub application you selected manually
 *
 * @param aDoSelect - if true select the current app (needs longer initial wait time) else show smarthub menu
 *
 */
void sendSamsungSmartHubMacro(bool aDoSelect) {
    uint32_t tWaitTimeAfterBoot;
    if (aDoSelect) {
        tWaitTimeAfterBoot = INITIAL_WAIT_TIME_APPS_READY_MILLIS;
    } else {
        tWaitTimeAfterBoot = INITIAL_WAIT_TIME_SMARTHUB_READY_MILLIS;
    }

#    if !defined(ESP32)
    IrReceiver.stop(); // ESP32 uses another timer for tone()
#    endif
    if (millis() < tWaitTimeAfterBoot) {
        // division by 1000 and printing requires much (8%) program space
        Serial.print(F("It is "));
        Serial.print(millis() / 1000);
        Serial.print(F(" seconds after boot, Samsung H5273 TV requires "));
        Serial.print(tWaitTimeAfterBoot / 1000);
        Serial.println(F(" seconds after boot to be ready for the command"));

        tone(TONE_PIN, 2200);
        delay(100);
        noTone(TONE_PIN);
        delay(100);
        tone(TONE_PIN, 2200);
        delay(100);
        noTone(TONE_PIN);

        if (millis() < tWaitTimeAfterBoot) {
            Serial.print(F("Now do a blocking wait for "));
            Serial.print(tWaitTimeAfterBoot - millis());
            Serial.println(F(" milliseconds"));
            delay(tWaitTimeAfterBoot - millis());
        }
    }

    // Do beep feedback for special key to be received
    tone(TONE_PIN, 2200);
    delay(200);
    noTone(TONE_PIN);
#    if !defined(ESP32)
    IrReceiver.start(200000); // to compensate for 200 ms stop of receiver. This enables a correct gap measurement.
#    endif

    Serial.println(F("Wait for \"not supported\" to disappear"));
    delay(2000);

    Serial.println(F("Start sending of Samsung IR macro"));

    IRSendWithDelay(0x1A, 2000); // Menu and wait for the Menu to pop up

    Serial.println(F("Wait for the menu to pop up"));
    if (!sMacroWasCalledBefore) {
        delay(2000); // wait additional time for the Menu load
    }

    for (uint_fast8_t i = 0; i < 4; ++i) {
        IRSendWithDelay(0x61, 250); // Down arrow. For my Samsung, the high byte of the command is the inverse of the low byte
    }

    IRSendWithDelay(0x62, 400); // Right arrow
    for (uint_fast8_t i = 0; i < 2; ++i) {
        IRSendWithDelay(0x61, 250); // Down arrow
    }

    delay(250);
    IRSendWithDelay(0x68, 1); // Enter for SmartHub

    if (aDoSelect) {
        Serial.println(F("Wait for SmartHub to show up, before entering current application"));
        delay(10000); // Wait not longer than 12 seconds, because smarthub menu then disappears
        IRSendWithDelay(0x68, 1); // Enter for last application (e.g. Netflix or Amazon)
    }

    sMacroWasCalledBefore = true;
    Serial.println(F("Done"));

}
