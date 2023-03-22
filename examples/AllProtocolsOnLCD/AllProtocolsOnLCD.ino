/*
 * AllProtocolsOnLCD.cpp
 *
 * Modified ReceiveDemo.cpp with additional 1602 LCD output.
 * If debug button is pressed (pin connected to ground) a long output is generated.
 *
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

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.

#if !defined(RAW_BUFFER_LENGTH)
#  if RAMEND <= 0x4FF || RAMSIZE < 0x4FF
#define RAW_BUFFER_LENGTH  180  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  elif RAMEND <= 0x8FF || RAMSIZE < 0x8FF
#define RAW_BUFFER_LENGTH  600  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  else
#define RAW_BUFFER_LENGTH  750  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  endif
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
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 142.
#define MARK_EXCESS_MICROS    20    // Adapt it to your IR receiver module. 20 is recommended for the cheap VS1838 modules.

//#define RECORD_GAP_MICROS 12000 // Default is 5000. Activate it for some LG air conditioner protocols.

//#define DEBUG // Activate this for lots of lovely debug output from the decoders.
//#define DECODE_NEC          // Includes Apple and Onkyo

#include <IRremote.hpp>

/*
 * Activate the type of LCD you use
 * Default is parallel LCD with 2 rows of 16 characters (1602).
 * Serial LCD has the disadvantage, that the first repeat is not detected,
 * because of the long lasting serial communication.
 */
//#define USE_NO_LCD
//#define USE_SERIAL_LCD
// Definitions for the 1602 LCD
#define LCD_COLUMNS 16
#define LCD_ROWS 2

#if defined(USE_SERIAL_LCD)
#include "LiquidCrystal_I2C.h" // Use an up to date library version, which has the init method
LiquidCrystal_I2C myLCD(0x27, LCD_COLUMNS, LCD_ROWS);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#elif !defined(USE_NO_LCD)
#define USE_PARALLEL_LCD
#include "LiquidCrystal.h"
//LiquidCrystal myLCD(4, 5, 6, 7, 8, 9);
LiquidCrystal myLCD(7, 8, 3, 4, 5, 6);
#endif

#if defined(USE_PARALLEL_LCD)
#define DEBUG_BUTTON_PIN            11 // If low, print timing for each received data set
#undef TONE_PIN
#define TONE_PIN                     9 // Pin 4 is used by LCD
#else
#define DEBUG_BUTTON_PIN             6
#endif
#define AUXILIARY_DEBUG_BUTTON_PIN  12 // Is set to low to enable using of a simple connector for enabling debug

#define MILLIS_BETWEEN_ATTENTION_BEEP   60000 // 60 sec
uint32_t sMillisOfLastReceivedIRFrame = 0;

#if defined(USE_SERIAL_LCD) || defined(USE_PARALLEL_LCD)
#define USE_LCD
#  if defined(__AVR__) && defined(ADCSRA) && defined(ADATE)
// For cyclically display of VCC
#include "ADCUtils.hpp"
#define MILLIS_BETWEEN_VOLTAGE_PRINT    5000
#define LCD_VOLTAGE_START_INDEX           11
uint32_t volatile sMillisOfLastVoltagePrint = 0;
bool ProtocolStringOverwritesVoltage = false;
#  endif
#define LCD_IR_COMMAND_START_INDEX         9

#endif // defined(USE_SERIAL_LCD) || defined(USE_PARALLEL_LCD)

void printIRResultOnLCD();
size_t printByteHexOnLCD(uint16_t aHexByteValue);
void printSpacesOnLCD(uint_fast8_t aNumberOfSpacesToPrint);

uint16_t sVCCMillivolt;

void setup() {
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#  if defined(AUXILIARY_DEBUG_BUTTON_PIN)
    pinMode(AUXILIARY_DEBUG_BUTTON_PIN, OUTPUT);
    digitalWrite(AUXILIARY_DEBUG_BUTTON_PIN, LOW); // To use a simple connector to enable debug
#  endif
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
// Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    tone(TONE_PIN, 2200);
    delay(200);
    noTone(TONE_PIN);

// In case the interrupt driver crashes on setup, give a clue
// to the user what's going on.
    Serial.println(F("Enabling IRin..."));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

#if defined(USE_SERIAL_LCD)
    Serial.println(
            F("With serial LCD connection, the first repeat is not detected, because of the long lasting serial communication!"));
#endif

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    Serial.println();
    Serial.print(F("If you connect debug pin "));
#  if defined(APPLICATION_PIN_STRING)
    Serial.print(APPLICATION_PIN_STRING);
#  else
    Serial.print(DEBUG_BUTTON_PIN);
#  endif
    Serial.print(F(" to ground or to pin "));
    Serial.print(AUXILIARY_DEBUG_BUTTON_PIN);
    Serial.println(F(", raw data is always printed"));

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
    myLCD.begin(LCD_COLUMNS, LCD_ROWS); // This also clears display
#endif

#if defined(USE_LCD)
    myLCD.setCursor(0, 0);
    myLCD.print(F("IRRemote  v" VERSION_IRREMOTE));
    myLCD.setCursor(0, 1);
    myLCD.print(F(__DATE__));
#endif

#if defined(USE_LCD) && defined(ADC_UTILS_ARE_AVAILABLE)
    sVCCMillivolt = getVCCVoltageMillivoltSimple();
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
        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);

        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
#if defined(USE_LCD)
            myLCD.setCursor(0, 0);
            myLCD.print(F("Overflow   "));
#endif

            // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library

        } else {
            // play tone
            auto tStartMillis = millis();
            IrReceiver.stop();
            tone(TONE_PIN, 2200);

            if ((IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW)
#if defined(USE_LCD) && defined(ADC_UTILS_ARE_AVAILABLE)
                    && sVCCMillivolt > 4222
#endif
            ) {
                // Print more info, but only if we are connected to USB, i.e. VCC is > 4222 mV, because this may take to long to detect some fast repeats
                IrReceiver.printIRSendUsage(&Serial);
                IrReceiver.printIRResultRawFormatted(&Serial, false); // print ticks, this is faster :-)
            }

            // Guarantee at least 5 millis for tone. decode starts 5 millis (RECORD_GAP_MICROS) after end of frame
            // so here we are 10 millis after end of frame. Sony20 has only a 12 ms repeat gap.
            while ((millis() - tStartMillis) < 5)
                ;
            noTone(TONE_PIN);

            // Restore IR timer. millis() - tStartMillis to compensate for stop of receiver. This enables a correct gap measurement.
            IrReceiver.startWithTicksToAdd((millis() - tStartMillis) * (MICROS_IN_ONE_MILLI / MICROS_PER_TICK));

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

    /*
     * Check for attention every 10 minute, after the current measurement was finished
     */
    if (millis() - sMillisOfLastReceivedIRFrame >= MILLIS_BETWEEN_ATTENTION_BEEP) {
        sMillisOfLastReceivedIRFrame = millis();
        IrReceiver.stop();
        tone(TONE_PIN, 2200);
        delay(50);
        noTone(TONE_PIN);
        IrReceiver.startWithTicksToAdd(50 * (MICROS_IN_ONE_MILLI / MICROS_PER_TICK));
    }

#if defined(USE_LCD) && defined(ADC_UTILS_ARE_AVAILABLE)
    //Periodically print VCC
    if (!ProtocolStringOverwritesVoltage && millis() - sMillisOfLastVoltagePrint > MILLIS_BETWEEN_VOLTAGE_PRINT) {
        /*
         * Periodically print VCC
         */
        sMillisOfLastVoltagePrint = millis();
        sVCCMillivolt = getVCCVoltageMillivoltSimple();
        char tVoltageString[5];
        dtostrf(sVCCMillivolt / 1000.0, 4, 2, tVoltageString);
        myLCD.setCursor(LCD_VOLTAGE_START_INDEX - 1, 0);
        myLCD.print(' ');
        myLCD.print(tVoltageString);
        myLCD.print('V');
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
    static uint16_t sLastProtocolIndex = 4711;
    static uint16_t sLastProtocolAddress = 4711;
    static uint16_t sLastCommand = 0;
    static uint8_t sLastCommandPrintPosition;

    /*
     * Print only if protocol has changed
     */
    if (sLastProtocolIndex != IrReceiver.decodedIRData.protocol) {
        sLastProtocolIndex = IrReceiver.decodedIRData.protocol;
        /*
         * Show protocol name and handle overwrite over voltage display
         */
        myLCD.setCursor(0, 0);
        uint_fast8_t tProtocolStringLength = myLCD.print(getProtocolString(IrReceiver.decodedIRData.protocol));
#  if defined(__AVR__) && defined(ADCSRA) && defined(ADATE)
        if (tProtocolStringLength > LCD_VOLTAGE_START_INDEX) {
            // we overwrite the voltage -> clear rest of line and inhibit new printing of voltage
            ProtocolStringOverwritesVoltage = true;
            if (tProtocolStringLength < LCD_COLUMNS) {
                printSpacesOnLCD(LCD_COLUMNS - tProtocolStringLength);
            }
        } else {
            // Trigger printing of VCC in main loop
            sMillisOfLastVoltagePrint = 0;
            ProtocolStringOverwritesVoltage = false;
            printSpacesOnLCD(LCD_VOLTAGE_START_INDEX - tProtocolStringLength);
        }
#  else
        printSpacesOnLCD(LCD_COLUMNS - tProtocolStringLength);
#  endif
    }

    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        /*
         * Print number of bits received and microseconds of signal
         */
        myLCD.setCursor(0, 1);
        uint8_t tNumberOfBits = (IrReceiver.decodedIRData.rawDataPtr->rawlen + 1) / 2;
        if (tNumberOfBits < 10) {
            myLCD.print(' '); // padding space
        }
        myLCD.print(tNumberOfBits);
        myLCD.print(F(" bit "));
        uint_fast8_t tDurationStringLength = myLCD.print(IrReceiver.getTotalDurationOfRawData());
        myLCD.print(F(" \xE4s")); // \xE4 is micro symbol
        printSpacesOnLCD(7 - tDurationStringLength);
        sLastProtocolAddress = 4711;
        sLastCommand = 44711;

    } else {
        /*
         * Print only if address has changed
         */
        if (sLastProtocolAddress != IrReceiver.decodedIRData.address) {
            sLastProtocolAddress = IrReceiver.decodedIRData.address;

            myLCD.setCursor(0, 1);
            /*
             * Show address
             */
#  if defined(DECODE_DISTANCE_WIDTH)
            if (IrReceiver.decodedIRData.protocol == PULSE_DISTANCE || IrReceiver.decodedIRData.protocol == PULSE_WIDTH) {
                myLCD.print(F("[0]=0x"));
                uint_fast8_t tAddressStringLength = myLCD.print(IrReceiver.decodedIRData.decodedRawDataArray[0], HEX);
                printSpacesOnLCD(LCD_COLUMNS - tAddressStringLength);
                sLastCommand = 0; // to trigger restoration of "C=" string
                return; // no command here
            } else {
#  endif
                myLCD.print(F("A="));
                uint_fast8_t tAddressStringLength = printByteHexOnLCD(IrReceiver.decodedIRData.address);
                printSpacesOnLCD((LCD_IR_COMMAND_START_INDEX - 2) - tAddressStringLength);
#  if defined(DECODE_DISTANCE_WIDTH)
            }
#  endif
        }

        /*
         * Print command always
         */
        uint16_t tCommand = IrReceiver.decodedIRData.command;

// Check if prefix position must change
        if (sLastCommand == 0 || (sLastCommand >= 0x100 && tCommand < 0x100) || (sLastCommand < 0x100 && tCommand >= 0x100)) {
            sLastCommand = tCommand;
            /*
             * Print prefix for 8/16 bit commands
             */
            if (tCommand >= 0x100) {
                // Do not print "C=" here to have 2 additional characters for command
                sLastCommandPrintPosition = 9;
            } else {
                myLCD.setCursor(LCD_IR_COMMAND_START_INDEX, 1);
                myLCD.print(F("C="));
                sLastCommandPrintPosition = 11;
            }
        }

        /*
         * Command data
         */
        myLCD.setCursor(sLastCommandPrintPosition, 1);
        printByteHexOnLCD(tCommand);

        /*
         * Show or clear repetition flag
         */
        if (IrReceiver.decodedIRData.flags & (IRDATA_FLAGS_IS_REPEAT)) {
            myLCD.print('R');
            return; // Since it is a repetition, printed data has not changed
        } else {
            myLCD.print(' ');
        }
    } // IrReceiver.decodedIRData.protocol == UNKNOWN
#endif // defined(USE_LCD)
}

#if defined(USE_LCD)
size_t printByteHexOnLCD(uint16_t aHexByteValue) {
    myLCD.print(F("0x"));
    size_t tPrintSize = 2;
    if (aHexByteValue < 0x10 || (aHexByteValue > 0x100 && aHexByteValue < 0x1000)) {
        myLCD.print('0'); // leading 0
        tPrintSize++;
    }
    return myLCD.print(aHexByteValue, HEX) + tPrintSize;
}

void printSpacesOnLCD(uint_fast8_t aNumberOfSpacesToPrint) {
    for (uint_fast8_t i = 0; i < aNumberOfSpacesToPrint; ++i) {
        myLCD.print(' ');
    }
}
#endif
