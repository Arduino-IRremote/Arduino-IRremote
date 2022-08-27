/*
 * AllProtocols.cpp
 *
 * Modified ReceiveDemo.cpp with additional LCD output.
 * If debug button is pressed (pin connected to ground) a long output is generated.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022 Armin Joachimsmeyer
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

#if RAMEND <= 0x4FF || (defined(RAMSIZE) && RAMSIZE < 0x4FF)
#define RAW_BUFFER_LENGTH  180  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#elif RAMEND <= 0x8FF || (defined(RAMSIZE) && RAMSIZE < 0x8FF)
#define RAW_BUFFER_LENGTH  600  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#else
#define RAW_BUFFER_LENGTH  750  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#endif
//#define NO_LED_FEEDBACK_CODE // saves 92 bytes program memory
#if FLASHEND <= 0x1FFF  // For 8k flash or less, like ATtiny85. Exclude exotic protocols.
#define EXCLUDE_EXOTIC_PROTOCOLS
#  if !defined(DIGISTUMPCORE) // ATTinyCore is bigger than Digispark core
#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
#  endif
#endif
//#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
//#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules.
//#define MARK_EXCESS_MICROS    20 // 20 is recommended for the cheap VS1838 modules

//#define RECORD_GAP_MICROS 12000 // Activate it for some LG air conditioner protocols

//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN // if low, print timing for each received data set
#else
#define DEBUG_BUTTON_PIN   6
#endif

/*
 * Activate the type of LCD you use
 * Default is serial LCD with 2 rows of 16 characters (1602).
 */
//#define USE_NO_LCD
//#define USE_PARALLEL_LCD
#if defined(USE_PARALLEL_LCD)
#include "LiquidCrystal.h"
#elif !defined(USE_NO_LCD)
#define USE_SERIAL_LCD
#include "LiquidCrystal_I2C.h" // Use an up to date library version, which has the init method
#endif

#if defined(USE_SERIAL_LCD) || defined(USE_PARALLEL_LCD)
#define USE_LCD
// definitions for a 1602 LCD
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#  if defined(__AVR__) && defined(ADCSRA) && defined(ADATE)
// For cyclically display of VCC
#include "ADCUtils.hpp"
#define MILLIS_BETWEEN_VOLTAGE_PRINT 5000
uint32_t volatile sMillisOfLastVoltagePrint;
#  endif

static uint8_t sLastProtocolIndex;
#endif

#if defined(USE_SERIAL_LCD)
LiquidCrystal_I2C myLCD(0x27, LCD_COLUMNS, LCD_ROWS);  // set the LCD address to 0x27 for a 20 chars and 2 line display
#endif
#if defined(USE_PARALLEL_LCD)
LiquidCrystal myLCD(4, 5, 6, 7, 8, 9);
#endif

void printIRResultOnLCD();

void setup() {
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
// Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

// In case the interrupt driver crashes on setup, give a clue
// to the user what's going on.
    Serial.println(F("Enabling IRin..."));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols (&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    Serial.print(F("Debug button pin is "));
    Serial.println(DEBUG_BUTTON_PIN);

    // infos for receive
    Serial.print(RECORD_GAP_MICROS);
    Serial.println(F(" us is the (minimum) gap, after which the start of a new IR packet is assumed"));
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
#endif

#if defined(USE_LCD) && defined(__AVR__) && defined(ADCSRA) && defined(ADATE)
    getVCCVoltageMillivoltSimple(); // to initialize ADC mux and reference
#endif

#if defined(USE_SERIAL_LCD)
    myLCD.init();
    myLCD.clear();
    myLCD.backlight();
#endif
#if defined(USE_PARALLEL_LCD)
    myLCD.begin(LCD_COLUMNS, LCD_ROWS);
#endif

#if defined(USE_LCD)
    myLCD.print(F("IRRemote  v" VERSION_IRREMOTE));
    myLCD.setCursor(0, 1);
    myLCD.print(F(__DATE__));
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
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            Serial.println(F("Overflow detected"));
            Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
            // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library

        } else {
            // Print a short summary of received data
            IrReceiver.printIRResultShort(&Serial);

            if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
                // We have an unknown protocol, print more info
                IrReceiver.printIRResultRawFormatted(&Serial, false);
            }
#if defined(USE_LCD)
            printIRResultOnLCD();
#endif
        }

        /*
         * !!!Important!!! Enable receiving of the next value,
         * since receiving has stopped after the end of the current received data packet.
         */
        IrReceiver.resume();
    } // if (IrReceiver.decode())

#if defined(USE_LCD) && defined(__AVR__) && defined(ADCSRA) && defined(ADATE)
    /*
     * Periodically print VCC
     */
    if (millis() - sMillisOfLastVoltagePrint > MILLIS_BETWEEN_VOLTAGE_PRINT) {
        sMillisOfLastVoltagePrint = millis();
        uint16_t tVCC = getVCCVoltageMillivoltSimple();

        myLCD.setCursor(10, 0);
        myLCD.print(' ');
        myLCD.print(tVCC / 1000);
        myLCD.print('.');
        myLCD.print(((tVCC + 5) / 10) % 100);
        myLCD.print('V');

        sLastProtocolIndex = 0; // enable new printing of protocol
    }
#endif

}

/*
 * LCD output for 1602 LCDs
 * 40 - 55 Milliseconds per initial output
 * The expander runs at 100 kHz :-(
 * 8 milliseconds for 8 bit; 10 ms for 16 bit code output
 * 3 milliseconds for repeat output
 *
 */
void printIRResultOnLCD() {
#if defined(USE_LCD)
    static uint16_t sLastProtocolAddress;
    static uint16_t sLastCommand;
    static uint8_t sLastCommandPrintPosition;

    bool tDisplayWasCleared = false;

    /*
     * Print only if protocol or address has changed
     */
    if (sLastProtocolIndex != IrReceiver.decodedIRData.protocol || sLastProtocolAddress != IrReceiver.decodedIRData.address) {
        sLastProtocolIndex = IrReceiver.decodedIRData.protocol;
        sLastProtocolAddress = IrReceiver.decodedIRData.address;

        myLCD.clear(); // includes a delay of 2 ms
        tDisplayWasCleared = true;

        /*
         * Show protocol name
         */
        myLCD.setCursor(0, 0);
        const __FlashStringHelper *tProtocolStringPtr = getProtocolString(IrReceiver.decodedIRData.protocol);
        myLCD.print(tProtocolStringPtr);

        /*
         * Show address
         */
        myLCD.setCursor(0, 1);
        myLCD.print(F("A=0x"));
        myLCD.print(IrReceiver.decodedIRData.address, HEX);

    } else {
        /*
         * Show or clear repetition flag
         */
        myLCD.setCursor(15, 1);
        if (IrReceiver.decodedIRData.flags & (IRDATA_FLAGS_IS_REPEAT)) {
            myLCD.print('R');
            return; // Since it is a repetition, printed data has not changed
        } else {
            myLCD.print(' ');
        }
    }

    /*
     * Command prefix
     */
    uint16_t tCommand = IrReceiver.decodedIRData.command;

    // check if prefix position must change
    if (tDisplayWasCleared || (sLastCommand > 0x100 && tCommand < 0x100) || (sLastCommand < 0x100 && tCommand > 0x100)) {
        sLastCommand = tCommand;
        /*
         * Print prefix of command
         */
        myLCD.setCursor(9, 1);

        /*
         * Print prefix for 8/16 bit commands
         */
        if (tCommand >= 0x100) {
            myLCD.print(F("0x"));
            sLastCommandPrintPosition = 11;
        } else {
            myLCD.print(F("C=0x"));
            sLastCommandPrintPosition = 13;
        }
    }

    /*
     * Command data
     */
    myLCD.setCursor(sLastCommandPrintPosition, 1);
    if (IrReceiver.decodedIRData.command < 0x10) {
        // leading 0
        myLCD.print('0');
    }
    myLCD.print(tCommand, HEX);

#endif // defined(USE_LCD)
}

