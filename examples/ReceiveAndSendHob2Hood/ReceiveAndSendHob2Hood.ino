/*
 * ReceiveAndSendHob2Hood.cpp
 *
 * Demonstrates receiving and sending of IR codes for AEG / Elektrolux Hob2Hood protocol
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

#define DECODE_HASH                 // Only decoder, which works for Hob2Hood. protocol is UNKNOWN and only raw data is set.

//#define NO_LED_FEEDBACK_CODE      // saves 92 bytes program memory
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

// IR commands from AEG hob2hood device
#define NUMBER_OF_HOB_TO_HOOD_COMMANDS      7
#define HOB_TO_HOOD_HASH_CODE_FAN_1         0xE3C01BE2
#define HOB_TO_HOOD_HASH_CODE_FAN_2         0xD051C301
#define HOB_TO_HOOD_HASH_CODE_FAN_3         0xC22FFFD7
#define HOB_TO_HOOD_HASH_CODE_FAN_4         0xB9121B29
#define HOB_TO_HOOD_HASH_CODE_FAN_OFF       0x55303A3
#define HOB_TO_HOOD_HASH_CODE_LIGHT_ON      0xE208293C
#define HOB_TO_HOOD_HASH_CODE_LIGHT_OFF     0x24ACF947

// based on https://pastebin.com/N6kG7Wu5
#define HOB_TO_HOOD_UNIT_MICROS     725
#define H2H_1   HOB_TO_HOOD_UNIT_MICROS
#define H2H_2   (HOB_TO_HOOD_UNIT_MICROS*2) // 1450
#define H2H_3   (HOB_TO_HOOD_UNIT_MICROS*3) // 2175
#define H2H_4   (HOB_TO_HOOD_UNIT_MICROS*4) // 2900
#define H2H_5   (HOB_TO_HOOD_UNIT_MICROS*5) // 3625

// First entry is the length of the raw command
const uint16_t Fan1[] PROGMEM { 15, H2H_2, H2H_2, H2H_1, H2H_2, H2H_3, H2H_2, H2H_1, H2H_2, H2H_1, H2H_1, H2H_1, H2H_2, H2H_1,
H2H_3, H2H_1 };
const uint16_t Fan2[] PROGMEM { 9, H2H_2, H2H_2, H2H_1, H2H_4, H2H_1, H2H_3, H2H_5, H2H_3, H2H_3 };
const uint16_t Fan3[] PROGMEM { 9, H2H_1, H2H_3, H2H_4, H2H_4, H2H_3, H2H_1, H2H_1, H2H_3, H2H_3 };
const uint16_t Fan4[] PROGMEM { 13, H2H_2, H2H_3, H2H_2, H2H_1, H2H_2, H2H_3, H2H_2, H2H_2, H2H_1, H2H_3, H2H_1, H2H_1, H2H_2 };
const uint16_t FanOff[] PROGMEM { 15, H2H_1, H2H_2, H2H_1, H2H_2, H2H_3, H2H_2, H2H_1, H2H_2, H2H_2, H2H_3, H2H_1, H2H_2, H2H_1,
H2H_1, H2H_1 };
const uint16_t LightOn[] PROGMEM { 17, H2H_1, H2H_2, H2H_1, H2H_1, H2H_2, H2H_1, H2H_1, H2H_2, H2H_1, H2H_1, H2H_2, H2H_4, H2H_1,
H2H_1, H2H_1, H2H_1, H2H_2 };
const uint16_t LightOff[] PROGMEM { 17, H2H_1, H2H_2, H2H_1, H2H_1, H2H_1, H2H_1, H2H_1, H2H_3, H2H_1, H2H_1, H2H_1, H2H_2, H2H_1,
H2H_2, H2H_1, H2H_1, H2H_1 };
const uint16_t *const Hob2HoodSendCommands[NUMBER_OF_HOB_TO_HOOD_COMMANDS] = { Fan1, Fan2, Fan3, Fan4, FanOff, LightOn, LightOff }; // Constant array in RAM

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive Hob2Hood IR signals at pin " STR(IR_RECEIVE_PIN)));
    IrSender.begin(); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
    Serial.println(F("Send Hob2Hood IR signals at pin " STR(IR_SEND_PIN)));
}

/*
 * Receive and send Hob2Hood protocol
 */
void loop() {
    static long sLastMillisOfSend = 0;
    static uint8_t sSendCommandIndex = 0;

    if (IrReceiver.decode()) {
        IrReceiver.resume(); // Early enable receiving of the next IR frame
        IrReceiver.printIRResultShort(&Serial);

        /*
         * Finally, check the received data and perform actions according to the received command
         */
        switch (IrReceiver.decodedIRData.decodedRawData) {
        case HOB_TO_HOOD_HASH_CODE_FAN_OFF:
            Serial.print(F("FAN off"));
            break;
        case HOB_TO_HOOD_HASH_CODE_FAN_1:
            Serial.print(F("FAN 1"));
            break;
        case HOB_TO_HOOD_HASH_CODE_FAN_2:
            Serial.print(F("FAN 2"));
            break;
        default:
            Serial.print(F("unknown Hob2Hood IR command"));
            break;
        }
    }

    /*
     * Send next command every 5 seconds
     */
    if (millis() - sLastMillisOfSend > 2000) {
        sLastMillisOfSend = millis();

#if defined(__AVR__)
        uint16_t tLengthOfRawCommand = pgm_read_word(Hob2HoodSendCommands[sSendCommandIndex]); // length is the 1. word in array
#else
        uint16_t tLengthOfRawCommand = *Hob2HoodSendCommands[sSendCommandIndex]; // length is the 1. word in array
#endif
        const uint16_t *tAddressOfRawCommandSequence = Hob2HoodSendCommands[sSendCommandIndex] + 1; // Raw sequence starts at the 2. word of array
        Serial.print(F("Send Hob2Hood command index="));
        Serial.println(sSendCommandIndex);
        IrSender.sendRaw_P(tAddressOfRawCommandSequence, tLengthOfRawCommand, 38);

        // Prepare for next command
        sSendCommandIndex++;
        if (sSendCommandIndex >= NUMBER_OF_HOB_TO_HOOD_COMMANDS) {
            sSendCommandIndex = 0;
        }
    }
}
