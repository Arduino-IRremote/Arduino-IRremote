/*
 *  MinimalReceiver.cpp
 *
 *  Small memory footprint and no timer usage!
 *
 *  Receives IR protocol data of NEC protocol using pin change interrupts.
 *  On complete received IR command the function handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, bool isRepetition)
 *  is called in Interrupt context but with interrupts being enabled to enable use of delay() etc.
 *  !!!!!!!!!!!!!!!!!!!!!!
 *  Functions called in interrupt context should be running as short as possible,
 *  so if you require longer action, save the data (address + command) and handle it in the main loop.
 *  !!!!!!!!!!!!!!!!!!!!!
 *
 *
 *  Copyright (C) 2020-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/ukw100/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  MinimalReceiver is free software: you can redistribute it and/or modify
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
 * Set sensible receive pin for different CPU's
 */
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#include "ATtinySerialOut.hpp" // Available as Arduino library "ATtinySerialOut"
#  if defined(ARDUINO_AVR_DIGISPARKPRO)
#define IR_INPUT_PIN    9 // PA3 - on Digispark board labeled as pin 9
#  else
#define IR_INPUT_PIN    0 // PCINT0
#  endif
#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
#define IR_INPUT_PIN    10
#elif defined(ESP8266)
#define IR_INPUT_PIN    14 // D5
#elif defined(ESP32)
#define IR_INPUT_PIN    15
#elif defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_MBED_NANO)
#define IR_INPUT_PIN    3   // GPIO15 Use pin 3 since pin 2|GPIO25 is connected to LED on Pi pico
#elif defined(ARDUINO_ARCH_RP2040) // Pi Pico with arduino-pico core https://github.com/earlephilhower/arduino-pico
#define IR_INPUT_PIN    15  // to be compatible with the Arduino Nano RP2040 Connect (pin3)
#else
#define IR_INPUT_PIN    2
//#define NO_LED_FEEDBACK_CODE // activating saves 14 bytes program space
#endif

//#define DEBUG // to see if attachInterrupt is used
//#define TRACE // to see the state of the ISR state machine

/*
 * Second: include the code and compile it.
 */
#include "TinyIRReceiver.hpp"

/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

volatile struct TinyIRReceiverCallbackDataStruct sCallbackData;

void setup()
{
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
#if defined(ESP8266)
    Serial.println();
#endif
    Serial.println(F("START " __FILE__ " from " __DATE__));
    initPCIInterruptForTinyReceiver();
    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_INPUT_PIN)));
}

void loop()
{
    if (sCallbackData.justWritten)
    {
        sCallbackData.justWritten = false;
        Serial.print(F("Address=0x"));
        Serial.print(sCallbackData.Address, HEX);
        Serial.print(F(" Command=0x"));
        Serial.print(sCallbackData.Command, HEX);
        if (sCallbackData.isRepeat)
        {
            Serial.print(F(" Repeat"));
        }
        Serial.println();
    }
    /*
     * Put your code here
     */
}

/*
 * This is the function is called if a complete command was received
 * It runs in an ISR context with interrupts enabled, so functions like delay() etc. are working here
 */
#if defined(ESP8266)
void ICACHE_RAM_ATTR handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat)
#elif defined(ESP32)
void IRAM_ATTR handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat)
#else
void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat)
#endif
{

#if defined(ARDUINO_ARCH_MBED) || defined(ESP32)
    // Copy data for main loop, this is the recommended way for handling a callback :-)
    sCallbackData.Address = aAddress;
    sCallbackData.Command = aCommand;
    sCallbackData.isRepeat = isRepeat;
    sCallbackData.justWritten = true;
#else
    /*
     * This is not allowed in ISR context for any kind of RTOS
     * For Mbed we get a kernel panic and "Error Message: Semaphore: 0x0, Not allowed in ISR context" for Serial.print()
     * for ESP32 we get a "Guru Meditation Error: Core  1 panic'ed" (we also have an RTOS running!)
     */
    // Print only very short output, since we are in an interrupt context and do not want to miss the next interrupts of the repeats coming soon
    Serial.print(F("A=0x"));
    Serial.print(aAddress, HEX);
    Serial.print(F(" C=0x"));
    Serial.print(aCommand, HEX);
    Serial.print(F(" R="));
    Serial.print(isRepeat);
    Serial.println();
#endif
}
