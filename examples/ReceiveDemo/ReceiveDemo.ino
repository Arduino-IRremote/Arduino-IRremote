/*
 * ReceiveDemo.cpp
 *
 * Demonstrates receiving IR codes with the IRremote library and the use of the Arduino tone() function with this library.
 * Long press of one IR button (receiving of multiple repeats for one command) is detected.
 * If debug button is pressed (pin connected to ground) a long output is generated, which may disturb detecting of repeats.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2024 Armin Joachimsmeyer
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

//#define LOCAL_DEBUG // If defined, print timing for each received data set (the same as if DEBUG_BUTTON_PIN was connected to low)

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
//#define DECODE_ONKYO        // Decodes only Onkyo and not NEC or Apple
//#define DECODE_NEC          // Includes Apple and Onkyo
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
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
// !!! Enabling B&O disables detection of Sony, because the repeat gap for SONY is smaller than the B&O frame gap :-( !!!
//#define DECODE_BEO // Bang & Olufsen protocol always must be enabled explicitly. It has an IR transmit frequency of 455 kHz! It prevents decoding of SONY!
#endif
#if defined(DECODE_BEO)
#define RECORD_GAP_MICROS 16000 // always get the complete frame in the receive buffer, but this prevents decoding of SONY!
#endif
// etc. see IRremote.hpp
//

#if !defined(RAW_BUFFER_LENGTH)
// For air condition remotes it requires 750. Default is 200.
#  if !((defined(RAMEND) && RAMEND <= 0x4FF) || (defined(RAMSIZE) && RAMSIZE < 0x4FF))
#define RAW_BUFFER_LENGTH  750
#  endif
#endif

//#define NO_LED_FEEDBACK_CODE // saves 92 bytes program memory
//#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
//#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active
//#define IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK // saves 32 bytes program memory

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 142.
//#define MARK_EXCESS_MICROS    20    // Adapt it to your IR receiver module. 40 is taken for the cheap VS1838 module her, since we have high intensity.

//#define RECORD_GAP_MICROS 12000 // Default is 8000. Activate it for some LG air conditioner protocols

//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN // if low, print timing for each received data set
#else
#define DEBUG_BUTTON_PIN   6
#endif

void generateTone();
void handleOverflow();
bool detectLongPress(uint16_t aLongPressDurationMillis);

void setup() {
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#endif

    Serial.begin(115200);
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
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
    printActiveIRProtocols(&Serial);
#if defined(IR_RECEIVE_PIN_STRING)
    Serial.println(F("at pin " IR_RECEIVE_PIN_STRING));
#else
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
#endif

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    Serial.println();
    if (digitalRead(DEBUG_BUTTON_PIN) != LOW) {
        Serial.print(F("If you connect debug pin "));
#  if defined(APPLICATION_PIN_STRING)
        Serial.print(APPLICATION_PIN_STRING);
#  else
        Serial.print(DEBUG_BUTTON_PIN);
#  endif
        Serial.print(F(" to ground, "));
    }
    Serial.println(F("raw data is always printed"));

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
#if FLASHEND < 0x3FFF  //
        // For less than 16k flash, only print a minimal summary of received data
        IrReceiver.printIRResultMinimal(&Serial);
#else

        /*
         *
         */
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            handleOverflow();
        } else {
            /*
             * No overflow here.
             * Stop receiver, generate a single beep, print short info and send usage and start receiver again
             *****************************************************************************************************/

            if ((IrReceiver.decodedIRData.protocol != SONY) && (IrReceiver.decodedIRData.protocol != PULSE_WIDTH)
                    && (IrReceiver.decodedIRData.protocol != PULSE_DISTANCE) && (IrReceiver.decodedIRData.protocol != UNKNOWN)
                    && digitalRead(DEBUG_BUTTON_PIN) != LOW) {
                /*
                 * For SONY the tone prevents the detection of a repeat after the 15 ms SONY gap.
                 * In debug mode and for unknown protocols, we need the time for extended output.
                 * Skipping tone will get exact gap time between transmissions and not running into repeat frames while wait for tone to end.
                 * This in turn enables the next CheckForRecordGapsMicros() call a chance to eventually propose a change of the current RECORD_GAP_MICROS value.
                 */
                generateTone();
            }

            /*
             * Print info
             */
            if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
                // We have debug enabled or an unknown protocol, print extended info
                if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                    Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
                }
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
            if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
                /*
                 * The info output for a successful receive
                 */
                IrReceiver.printIRResultShort(&Serial);
                IrReceiver.printIRSendUsage(&Serial);
            }
        }
#endif // #if FLASHEND >= 0x3FFF

        /*
         * !!!Important!!! Enable receiving of the next value, because receiving
         * has stopped after the end of the current received data packet.
         * Do it here, to preserve raw data for printing with printIRResultRawFormatted()
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

        // Check if repeats of the IR command was sent for more than 1000 ms
        if (detectLongPress(1000)) {
            Serial.print(F("Command 0x"));
            Serial.print(IrReceiver.decodedIRData.command, HEX);
            Serial.println(F(" was repeated for more than 2 seconds"));
        }
    } // if (IrReceiver.decode())

    /*
     * Your code here
     * For all users of the FastLed library, use this code for strip.show() to improve receiving performance (which is still not 100%):
     * if (IrReceiver.isIdle()) {
     *     strip.show();
     * }
     */

}

/*
 * Stop receiver, generate a single beep and start receiver again
 */
void generateTone() {
#if !defined(ESP8266) && !defined(NRF5) // tone on esp8266 works only once, then it disables IrReceiver.restartTimer() / timerConfigForReceive().
#  if defined(ESP32) // ESP32 uses another timer for tone()
    tone(TONE_PIN, 2200, 8);
#  else
    IrReceiver.stopTimer(); // ESP32 uses another timer for tone(), maybe other platforms (not tested yet) too.
    tone(TONE_PIN, 2200, 8);
    delay(8);
    IrReceiver.restartTimer(8000); // Restart IR timer. 8000 to compensate for 8 ms stop of receiver. This enables a correct gap measurement.
#  endif
#endif
}

void handleOverflow() {
    Serial.println(F("Overflow detected"));
    Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
    // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library

#if !defined(ESP8266) && !defined(NRF5) // tone on esp8266 works once, then it disables IrReceiver.restartTimer() / timerConfigForReceive().
    /*
     * Stop timer, generate a double beep and start timer again
     */
#  if defined(ESP32) // ESP32 uses another timer for tone()
    tone(TONE_PIN, 1100, 10);
    delay(50);
    tone(TONE_PIN, 1100, 10);
#  else
    IrReceiver.stopTimer();
    tone(TONE_PIN, 1100, 10);
    delay(50);
    tone(TONE_PIN, 1100, 10);
    delay(50);
    IrReceiver.restartTimer(100000); // to compensate for 100 ms stop of receiver. This enables a correct gap measurement.
#  endif
#endif
}

unsigned long sMillisOfFirstReceive;
bool sLongPressJustDetected;
/**
 * True once we received the consecutive repeats for more than aLongPressDurationMillis milliseconds.
 * The first frame, which is no repeat, is NOT counted for the duration!
 * @return true once after the repeated IR command was received for longer than aLongPressDurationMillis milliseconds, false otherwise.
 */
bool detectLongPress(uint16_t aLongPressDurationMillis) {
    if (!sLongPressJustDetected && (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
        /*
         * Here the repeat flag is set (which implies, that command is the same as the previous one)
         */
        if (millis() - aLongPressDurationMillis > sMillisOfFirstReceive) {
            sLongPressJustDetected = true; // Long press here
        }
    } else {
        // No repeat here
        sMillisOfFirstReceive = millis();
        sLongPressJustDetected = false;
    }
    return sLongPressJustDetected; // No long press here
}

