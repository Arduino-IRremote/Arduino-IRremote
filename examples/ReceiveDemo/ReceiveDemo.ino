/*
 * ReceiveDemo.cpp
 *
 * Demonstrates receiving IR codes with the IRremote library and the use of the Arduino tone() function with this library.
 * Long press of one IR button (receiving of multiple repeats for one command) is detected.
 * If debug button is pressed (pin connected to ground) a long output is generated.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2023 Armin Joachimsmeyer
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
#  if RAMEND <= 0x4FF || RAMSIZE < 0x4FF
#define RAW_BUFFER_LENGTH  130  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active
#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
#  elif RAMEND <= 0x8FF || RAMSIZE < 0x8FF
#define RAW_BUFFER_LENGTH  600  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  else
#define RAW_BUFFER_LENGTH  750  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  endif
#endif

//#define NO_LED_FEEDBACK_CODE // saves 92 bytes program memory
//#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
//#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 142.
//#define MARK_EXCESS_MICROS    20    // Adapt it to your IR receiver module. 40 is taken for the cheap VS1838 module her, since we have high intensity.

//#define RECORD_GAP_MICROS 12000 // Default is 5000. Activate it for some LG air conditioner protocols

//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN // if low, print timing for each received data set
#else
#define DEBUG_BUTTON_PIN   6
#endif

bool detectLongPress(uint16_t aLongPressDurationMillis);

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
    printActiveIRProtocols(&Serial);
#if defined(IR_RECEIVE_PIN_STRING)
    Serial.println(F("at pin " IR_RECEIVE_PIN_STRING));
#else
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
#endif

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
    Serial.println();
    Serial.print(F("If you connect debug pin "));
#  if defined(APPLICATION_PIN_STRING)
    Serial.print(APPLICATION_PIN_STRING);
#  else
    Serial.print(DEBUG_BUTTON_PIN);
#  endif
    Serial.println(F(" to ground, raw data is always printed"));

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
     *
     * At 115200 baud, printing takes 40 ms for NEC protocol and 10 ms for NEC repeat
     */
    if (IrReceiver.decode()) {
        Serial.println();
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            Serial.println(F("Overflow detected"));
            Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
            // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library
#  if !defined(ESP8266) && !defined(NRF5)
            /*
             * do double beep
             */
#    if !defined(ESP32)
            IrReceiver.stop(); // ESP32 uses another timer for tone()
#    endif
            tone(TONE_PIN, 1100, 10);
            delay(50);
            tone(TONE_PIN, 1100, 10);
            delay(50);
#    if !defined(ESP32)
            IrReceiver.start(100000); // to compensate for 100 ms stop of receiver. This enables a correct gap measurement.
#    endif
#  endif

        } else {
            auto tStartMillis = millis();
#    if !defined(ESP32)
            IrReceiver.stop(); // ESP32 uses another timer for tone()
#    endif
            tone(TONE_PIN, 2200);

            // No overflow, print a short summary of received data
#  if defined(LOCAL_DEBUG)
            IrReceiver.printIRResultShort(&Serial, true);
#  else
            IrReceiver.printIRResultShort(&Serial, true, digitalRead(DEBUG_BUTTON_PIN) == LOW);
#  endif
            // Guarantee at least 5 millis for tone. decode starts 5 millis (RECORD_GAP_MICROS) after end of frame
            // so here we are 10 millis after end of frame. Sony20 has only a 12 ms repeat gap.
            while ((millis() - tStartMillis) < 5)
                ;
            noTone(TONE_PIN);

#    if !defined(ESP32)
            // Restore IR timer. millis() - tStartMillis to compensate for stop of receiver. This enables a correct gap measurement.
            IrReceiver.startWithTicksToAdd((millis() - tStartMillis) * (MICROS_IN_ONE_MILLI / MICROS_PER_TICK));
#    endif

            IrReceiver.printIRSendUsage(&Serial);
#  if defined(LOCAL_DEBUG)
            IrReceiver.printIRResultRawFormatted(&Serial, true);
#  else
            if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
                // We have an unknown protocol, print more info
                if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                    Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
                }
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
#  endif
        }

        // tone on esp8266 works once, then it disables the successful IrReceiver.start() / timerConfigForReceive().
#  if !defined(ESP8266) && !defined(NRF5) && !defined(LOCAL_DEBUG)
        if ((IrReceiver.decodedIRData.protocol != SONY) && (IrReceiver.decodedIRData.protocol != PULSE_WIDTH)
                && (IrReceiver.decodedIRData.protocol != PULSE_DISTANCE) && (IrReceiver.decodedIRData.protocol != UNKNOWN)
                && digitalRead(DEBUG_BUTTON_PIN) != LOW) {
            /*
             * If no debug mode or a valid protocol was received, play tone, wait and restore IR timer.
             * For SONY the tone prevents the detection of a repeat
             * Otherwise do not play a tone to get exact gap time between transmissions and not running into repeat frames while wait for tone to end.
             * This will give the next CheckForRecordGapsMicros() call a chance to eventually propose a change of the current RECORD_GAP_MICROS value.
             */
#    if !defined(ESP32)
            IrReceiver.stop(); // ESP32 uses another timer for tone()
#    endif
            tone(TONE_PIN, 2200, 8);
#    if !defined(ESP32)
            delay(8);
            IrReceiver.start(8000); // Restore IR timer. 8000 to compensate for 8 ms stop of receiver. This enables a correct gap measurement.
#    endif
        }
#  endif
#else // #if FLASHEND >= 0x3FFF
        // Print a minimal summary of received data
        IrReceiver.printIRResultMinimal(&Serial);
#endif // #if FLASHEND >= 0x3FFF

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

        // Check if the command was repeated for more than 1000 ms
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

unsigned long sMillisOfFirstReceive;
bool sLongPressJustDetected;
/**
 * @return true once after the repeated command was received for longer than aLongPressDurationMillis milliseconds, false otherwise.
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

