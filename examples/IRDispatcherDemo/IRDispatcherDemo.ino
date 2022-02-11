/*
 *  IRDispatcherDemo.cpp
 *
 *  Receives NEC IR commands and maps them to different actions by means of a mapping array.
 *
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/ukw100/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  IRMP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

/*
 * Choose the library to be used for IR receiving
 */
#define USE_TINY_IR_RECEIVER // Recommended, but only for NEC protocol!!! If disabled and IRMP_INPUT_PIN is defined, the IRMP library is used for decoding
//#define TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT // costs 112 bytes program space + 4 bytes RAM

#include "PinDefinitionsAndMore.h"
// Some kind of auto detect library if USE_TINY_IR_RECEIVER is deactivated
#if !defined(USE_TINY_IR_RECEIVER)
#  if defined(IR_RECEIVE_PIN)
#define USE_TINY_IR_RECEIVER
#  elif !defined(USE_IRMP_LIBRARY) && defined(IRMP_INPUT_PIN)
#define USE_IRMP_LIBRARY
#  else
#error No IR library selected
#  endif
#endif

#define IR_INPUT_PIN    2

//#define NO_LED_FEEDBACK_CODE // You can set it here, before the include of IRCommandDispatcher below

#if defined(USE_TINY_IR_RECEIVER) && !defined(IR_INPUT_PIN)
  #if defined(IR_RECEIVE_PIN)
#define IR_INPUT_PIN   IR_RECEIVE_PIN   // The pin where the IR input signal is expected. The pin must be capable of generating a pin change interrupt.
  #endif
  #if defined(IRMP_INPUT_PIN)
#define IR_INPUT_PIN   IRMP_INPUT_PIN   // The pin where the IR input signal is expected. The pin must be capable of generating a pin change interrupt.
  #endif

#elif defined(USE_IRMP_LIBRARY)
#define IRMP_USE_COMPLETE_CALLBACK       1 // Enable callback functionality is required if IRMP library is used

#if defined(ALTERNATIVE_IR_FEEDBACK_LED_PIN)
#define FEEDBACK_LED_PIN    ALTERNATIVE_IR_FEEDBACK_LED_PIN
#endif

//#define IRMP_ENABLE_PIN_CHANGE_INTERRUPT  // Enable interrupt functionality (not for all protocols) - requires around 376 additional bytes of program space

#define IRMP_PROTOCOL_NAMES 1               // Enable protocol number mapping to protocol strings - requires some program space. Must before #include <irmp*>

#define IRMP_SUPPORT_NEC_PROTOCOL         1 // this enables only one protocol
//#define IRMP_SUPPORT_KASEIKYO_PROTOCOL    1

#  ifdef ALTERNATIVE_IR_FEEDBACK_LED_PIN
#define IRMP_FEEDBACK_LED_PIN   ALTERNATIVE_IR_FEEDBACK_LED_PIN
#  endif
/*
 * After setting the definitions we can include the code and compile it.
 */
#include <irmp.hpp>
void handleReceivedIRData();
void irmp_tone(uint8_t _pin, unsigned int frequency, unsigned long duration);
#endif // #if defined(USE_IRMP_LIBRARY)

bool doBlink = false;
uint16_t sBlinkDelay = 200;

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
 * Set definitions and include IRCommandDispatcher library after the declaration of all commands to map
 */
#define INFO // to see some informative output
#include "IRCommandDispatcher.h" // Only for required declarations, the library itself is included below after the definitions of the commands
#include "IRCommandMapping.h" // must be included before IRCommandDispatcher.hpp to define IR_ADDRESS and IRMapping and string "unknown".
#include "IRCommandDispatcher.hpp"

/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
#if defined(ESP8266)
    Serial.println(); // to separate it from the internal boot output
#endif

    // Just to know which program is running on my Arduino
#if defined(USE_TINY_IR_RECEIVER)
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing TinyIRReceiver"));
#elif defined(USE_IRREMOTE_LIBRARY)
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing IRremote library version " VERSION_IRREMOTE));
#elif defined(USE_IRMP_LIBRARY)
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing IRMP library version " VERSION_IRMP));
#endif

#if !defined(ESP8266) && !defined(NRF5)
    // play feedback tone before setup, since it kills the IR timer settings
    tone(TONE_PIN, 1000);
    delay(50);
    noTone(TONE_PIN);
#endif

    IRDispatcher.init(); // This just calls irmp_init()
#if defined(USE_TINY_IR_RECEIVER)
    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_INPUT_PIN)));
#else
    irmp_register_complete_callback_function(&handleReceivedIRData); // fixed function in IRCommandDispatcher.hpp

    Serial.print(F("Ready to receive IR signals of protocols: "));
    irmp_print_active_protocols(&Serial);
#  if defined(ARDUINO_ARCH_STM32)
    Serial.println(F("at pin " IRMP_INPUT_PIN_STRING));
#  else
    Serial.println(F("at pin " STR(IRMP_INPUT_PIN)));
#  endif

#  ifdef ALTERNATIVE_IR_FEEDBACK_LED_PIN
    irmp_irsnd_LEDFeedback(true); // Enable receive signal feedback at ALTERNATIVE_IR_FEEDBACK_LED_PIN
    Serial.println(F("IR feedback pin is " STR(ALTERNATIVE_IR_FEEDBACK_LED_PIN)));
#  endif
#endif

    Serial.print(F("Listening to commands of IR remote of type "));
    Serial.println(IR_REMOTE_NAME);
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

    if (millis() - IRDispatcher.IRReceivedData.MillisOfLastCode > 120000)
    {
        //Short beep as remainder, if we did not receive any command in the last 2 minutes
        IRDispatcher.IRReceivedData.MillisOfLastCode += 120000;
        doTone1800();
    }

//    delay(10);
}

void doPrintMenu(){
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
 * This is a blocking function and checks periodically for stop
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
#if defined(USE_IRMP_LIBRARY) && !defined(IRMP_ENABLE_PIN_CHANGE_INTERRUPT)
    irmp_tone(TONE_PIN, 1800, 200);
#else
#  if !defined(ESP8266) && !defined(NRF5) // tone() stops timer 1 for ESP8266
    tone(TONE_PIN, 1800, 200);
#  endif
#endif
}

void doTone2200() {
#if defined(USE_IRMP_LIBRARY) && !defined(IRMP_ENABLE_PIN_CHANGE_INTERRUPT)
    // use IRMP compatible function for tone()
    irmp_tone(TONE_PIN, 2200, 50);
#else
#  if !defined(ESP8266) && !defined(NRF5) // tone() stops timer 1 for ESP8266
    tone(TONE_PIN, 2200, 50);
#  endif
#endif
}

#if defined(USE_IRMP_LIBRARY)
/*
 * Convenience IRMP compatible wrapper function for Arduino tone() if IRMP_ENABLE_PIN_CHANGE_INTERRUPT is NOT activated
 * It currently disables the receiving of repeats
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
