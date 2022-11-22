/*
 * ControlRelay.cpp
 *
 * Toggles an output pin at each command received
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Initially coded 2009 Ken Shirriff http://www.righto.com
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2009-2021 Ken Shirriff, Armin Joachimsmeyer
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

#if FLASHEND <= 0x1FFF || (RAMEND <= 0x4FF || RAMSIZE < 0x4FF)  // For 8k flash or 512 bytes RAM or less, like ATtiny85, ATtiny167
#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
#define EXCLUDE_EXOTIC_PROTOCOLS
#endif

#include <IRremote.hpp>

#if defined(APPLICATION_PIN)
#define RELAY_PIN   APPLICATION_PIN
#else
#define RELAY_PIN   5
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
}

int on = 0;
unsigned long last = millis();

void loop() {
    if (IrReceiver.decode()) {
        // If it's been at least 1/4 second since the last
        // IR received, toggle the relay
        if (millis() - last > 250) {
            on = !on;
            Serial.print(F("Switch relay "));
            if (on) {
                digitalWrite(RELAY_PIN, HIGH);
                Serial.println(F("on"));
            } else {
                digitalWrite(RELAY_PIN, LOW);
                Serial.println(F("off"));
            }

#if FLASHEND >= 0x3FFF      // For 16k flash or more, like ATtiny1604
            IrReceiver.printIRResultShort(&Serial);
            IrReceiver.printIRSendUsage(&Serial);
            Serial.println();
            if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                // We have an unknown protocol, print more info
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
#else
            // Print a minimal summary of received data
            IrReceiver.printIRResultMinimal(&Serial);
            Serial.println();
#endif // FLASHEND
        }
        last = millis();
        IrReceiver.resume(); // Enable receiving of the next value
    }
}
