/*
 * SendLGAirConditionerDemo.cpp
 *
 *  Sending LG air conditioner IR codes controlled by Serial input
 *  Based on he old IRremote source from https://github.com/chaeplin
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
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
#include <Arduino.h>

/*
 * LG2 has different header timing and a shorter bit time
 * Known LG remote controls, which uses LG2 protocol are:
 * AKB75215403
 * AKB74955603
 * AKB73757604:
 */
//#define USE_LG2_PROTOCOL // Try it if you do not have success with the default LG protocol
#define NUMBER_OF_COMMANDS_BETWEEN_PRINT_OF_MENU 5

#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif

#define INFO // Deactivate this to save program memory and suppress info output from the LG-AC driver.
//#define DEBUG // Activate this for more output from the LG-AC driver.

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#include "ATtinySerialOut.hpp" // Available as Arduino library "ATtinySerialOut"
#endif

#include "ac_LG.hpp"

#define SIZE_OF_RECEIVE_BUFFER 10
char sRequestString[SIZE_OF_RECEIVE_BUFFER];

Aircondition_LG MyLG_Aircondition;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif

// Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
    Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));

    /*
     * The IR library setup. That's all!
     */
    IrSender.begin(); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin

    Serial.println();
    MyLG_Aircondition.setType(LG_IS_WALL_TYPE);
    MyLG_Aircondition.printMenu(&Serial);

    delay(1000);

// test
//     MyLG_Aircondition.sendCommandAndParameter('j', 1);
//     delay(5000);
//     MyLG_Aircondition.sendCommandAndParameter('f', 3);
//     delay(5000);

}

void loop() {
    static uint8_t sShowmenuConter = 0;

    if (Serial.available()) {
        /*
         * Get parameters from serial
         */
        uint8_t tNumberOfBytesReceived = Serial.readBytesUntil('\n', sRequestString, SIZE_OF_RECEIVE_BUFFER - 1);
        // handle CR LF
        if (sRequestString[tNumberOfBytesReceived - 1] == '\r') {
            tNumberOfBytesReceived--;
        }
        sRequestString[tNumberOfBytesReceived] = '\0'; // terminate as string
        char tCommand = sRequestString[0];

        /*
         * Handle parameter numbers which can be greater 9
         */
        int tParameter = 0;
        if (tNumberOfBytesReceived >= 2) {
            tParameter = sRequestString[1] - '0';
            if (tCommand == LG_COMMAND_TEMPERATURE || tCommand == LG_COMMAND_SWING || tCommand == LG_COMMAND_SLEEP
                    || tCommand == LG_COMMAND_TIMER_ON || tCommand == LG_COMMAND_TIMER_OFF) {
                tParameter = atoi(&sRequestString[1]);
            }
        }

        /*
         * Print command to send
         */
        Serial.println();
        Serial.print(F("Command="));
        Serial.print(tCommand);
        if (tParameter != 0) {
            Serial.print(F(" Parameter="));
            Serial.print(tParameter);
        }
        Serial.println();

        if (!MyLG_Aircondition.sendCommandAndParameter(tCommand, tParameter)) {
            Serial.print(F("Error: unknown command or invalid parameter in \""));
            Serial.print(sRequestString);
            Serial.println('\"');
        }

        if (sShowmenuConter == 0) {
            MyLG_Aircondition.printMenu(&Serial);
            sShowmenuConter = NUMBER_OF_COMMANDS_BETWEEN_PRINT_OF_MENU;
        } else {
            sShowmenuConter--;
        }
    }
    delay(100);
}

