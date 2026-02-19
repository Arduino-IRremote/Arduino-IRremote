/*
 *  IRDispatcherDemo.cpp
 *
 *  Example how to use IRCommandDispatcher to receive IR commands and map them to different actions / functions by means of a mapping array.
 *
 *  Copyright (C) 2020-2026  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  IRMP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

/*
 * Choose the library to be used for IR receiving
 */
//#define USE_TINY_IR_RECEIVER // Recommended and default, but only for NEC protocol!!! If disabled and IRMP_INPUT_PIN is defined, the IRMP library is used for decoding
//#define USE_IRREMOTE_LIBRARY // The IRremote library is used for decoding
//#define USE_IRMP_LIBRARY     // The IRMP library is used for decoding
//#define NO_LED_FEEDBACK_CODE   // Activate this if you want to suppress LED feedback or if you do not have a LED. This saves 14 bytes code and 2 clock cycles per interrupt.
#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.

#if defined(USE_TINY_IR_RECEIVER) && defined(TONE_PIN)
#define DISPATCHER_BUZZER_FEEDBACK_PIN  TONE_PIN // The pin to be used for the optional 50 ms buzzer feedback before executing a command. Only available for TinyIR.
#endif

#if defined(USE_IRMP_LIBRARY)
//Enable protocols for IRMP
#define IRMP_SUPPORT_NEC_PROTOCOL         1 // this enables only one protocol
//#define IRMP_SUPPORT_KASEIKYO_PROTOCOL    1
//#define IRMP_ENABLE_PIN_CHANGE_INTERRUPT  // Enable interrupt functionality (not for all protocols) - requires around 376 additional bytes of program memory

void irmp_tone(uint8_t _pin, unsigned int frequency, unsigned long duration);
#endif // defined(USE_IRMP_LIBRARY)

bool doBlink = false;
uint16_t sBlinkDelay = 200;

/*
 * The functions which are called by the IR commands.
 * They must be declared before including DemoIRCommandMapping.h, where the mapping to IR keys is defined.
 */
void doPrintMenu();
void doLedOn();
void doLedOff();
void doIncreaseBlinkFrequency();
void doDecreaseBlinkFrequency();
void doStop();
void doResetBlinkFrequency();
void doLedBlinkStart();
void doLedBlink20times();
void doTone1800();
void doTone2200();

/*
 * Set definitions and include IRCommandDispatcher library after the declaration of all commands required for mapping
 */
#define INFO // to see some informative output of the IRCommandDispatcher library
#include "DemoIRCommandMapping.h" // must be included before IRCommandDispatcher.hpp to define IRMapping array, IR_ADDRESS etc.
#include "IRCommandDispatcher.hpp"

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
#if defined(ESP8266)
    Serial.println(); // to separate it from the internal boot output
#endif

    // Just to know which program is running on my Arduino
#if defined(USE_TINY_IR_RECEIVER)
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing TinyIRReceiver library version " VERSION_TINYIR));
#elif defined(USE_IRREMOTE_LIBRARY)
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing IRremote library version " VERSION_IRREMOTE));
#elif defined(USE_IRMP_LIBRARY)
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing IRMP library version " VERSION_IRMP));
#endif

#if !defined(ESP8266) && !defined(NRF5) && defined(TONE_PIN)
    // play feedback tone before IRDispatcher.init(), because it kills the IR timer settings, which are made by init()
    tone(TONE_PIN, 1000, 50);
    delay(50);
#endif

    IRDispatcher.init(); // This calls the init function of the chosen library
    IRDispatcher.printIRInfo(&Serial);

    doPrintMenu();
}

void loop() {

    IRDispatcher.checkAndRunSuspendedBlockingCommands();

    if (doBlink) {
        digitalWrite(LED_BUILTIN, HIGH);
        DELAY_AND_RETURN_IF_STOP(sBlinkDelay);
        digitalWrite(LED_BUILTIN, LOW);
        DELAY_AND_RETURN_IF_STOP(sBlinkDelay);
    }

    if (millis() - IRDispatcher.IRReceivedData.MillisOfLastCode > 120000) {
        //Short beep as remainder, if we did not receive any command in the last 2 minutes
        IRDispatcher.IRReceivedData.MillisOfLastCode += 120000;
        doTone1800();
    }

//    delay(10);
}

/*
 * Menu for simple China Keyes or Keyes clone IR controls with number pad and direction control pad
 */
void doPrintMenu() {
    Serial.println();
    Serial.println(F("Press 1 for tone 1800 Hz"));
    Serial.println(F("Press 2 for tone 2200 Hz"));
    Serial.println(F("Press 3 for this Menu"));
    Serial.println(F("Press 0 for LED blink 20 times"));
    Serial.println(F("Press UP for LED on"));
    Serial.println(F("Press DOWN for LED off"));
    Serial.println(F("Press OK for LED blink start"));
    Serial.println(F("Press RIGHT for LED increase blink frequency"));
    Serial.println(F("Press LEFT for LED decrease blink frequency"));
    Serial.println(F("Press STAR for reset blink frequency"));
    Serial.println(F("Press HASH for stop"));
    Serial.println();
}
/*
 * Here the actions that are matched to IR keys
 */
void doLedOn() {
    digitalWrite(LED_BUILTIN, HIGH);
    doBlink = false;
}
void doLedOff() {
    digitalWrite(LED_BUILTIN, LOW);
    doBlink = false;
}
void doIncreaseBlinkFrequency() {
    doBlink = true;
    if (sBlinkDelay > 5) {
        sBlinkDelay -= sBlinkDelay / 4;
    }
}
void doDecreaseBlinkFrequency() {
    doBlink = true;
    sBlinkDelay += sBlinkDelay / 4;
}
void doStop() {
    doBlink = false;
}
void doResetBlinkFrequency() {
    sBlinkDelay = 200;
    digitalWrite(LED_BUILTIN, LOW);
}
void doLedBlinkStart() {
    doBlink = true;
}
/*
 * This is a blocking function which checks for stop
 */
void doLedBlink20times() {
    for (int i = 0; i < 20; ++i) {
        digitalWrite(LED_BUILTIN, HIGH);
        DELAY_AND_RETURN_IF_STOP(200);
        digitalWrite(LED_BUILTIN, LOW);
        DELAY_AND_RETURN_IF_STOP(200);
    }
}

void doTone1800() {
#if defined(USE_IRMP_LIBRARY) && !defined(IRMP_ENABLE_PIN_CHANGE_INTERRUPT) && defined(TONE_PIN)
    irmp_tone(TONE_PIN, 1800, 200);
#elif !defined(ESP8266) && !defined(NRF5) && defined(TONE_PIN) // tone() stops timer 1 for ESP8266
    tone(TONE_PIN, 1800, 200);
#endif
}

void doTone2200() {
#if defined(USE_IRMP_LIBRARY) && !defined(IRMP_ENABLE_PIN_CHANGE_INTERRUPT) && defined(TONE_PIN)
    // use IRMP compatible function for tone()
    irmp_tone(TONE_PIN, 2200, 50);
#elif !defined(ESP8266) && !defined(NRF5) && defined(TONE_PIN) // tone() stops timer 1 for ESP8266
    tone(TONE_PIN, 2200, 50);
#endif
}

#if defined(USE_IRMP_LIBRARY)
/*
 * Convenience IRMP compatible wrapper function for Arduino tone() if IRMP_ENABLE_PIN_CHANGE_INTERRUPT is NOT activated
 * It currently disables the receiving of repeats
 * It is not part of the library because it statically allocates the tone interrupt vector 7.
 */
void irmp_tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
#  if defined(__AVR__) && !defined(IRMP_ENABLE_PIN_CHANGE_INTERRUPT)
    storeIRTimer();
    tone(_pin, frequency, 0);
    if (duration == 0) {
        duration = 100;
    }
    delay(duration);
    noTone(_pin);
    restoreIRTimer();
#elif defined(ESP8266)
    // tone() stops timer 1
    (void)  _pin;
    (void)  frequency;
    (void)  duration;
#else
    tone(_pin, frequency, duration);
#endif
}
#endif // #if defined(USE_IRMP_LIBRARY)
