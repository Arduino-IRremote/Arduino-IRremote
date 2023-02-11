/*
 *  TinyReceiver.cpp
 *
 *  Small memory footprint and no timer usage!
 *
 *  Receives IR protocol data of NEC protocol using pin change interrupts.
 *  On complete received IR command the function handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, uint8_t aFlags)
 *  is called in Interrupt context but with interrupts being enabled to enable use of delay() etc.
 *  !!!!!!!!!!!!!!!!!!!!!!
 *  Functions called in interrupt context should be running as short as possible,
 *  so if you require longer action, save the data (address + command) and handle it in the main loop.
 *  !!!!!!!!!!!!!!!!!!!!!
 *
 *  FAST protocol is proprietary and a JVC protocol without address and with a shorter header.
 *  FAST takes 21 ms for sending and can be sent at a 50 ms period. It still supports parity.
 *  FAST Protocol characteristics:
 *  - Bit timing is like JVC
 *  - The header is shorter, 4000 vs. 12500
 *  - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *      leading to a fixed protocol length of (7 + (16 * 2) + 1) * 526 = 40 * 560 = 21040 microseconds or 21 ms.
 *  - Repeats are sent as complete frames but in a 50 ms period.
 *
 *
 *  Copyright (C) 2020-2023  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  TinyIRReceiver is free software: you can redistribute it and/or modify
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
 * Set sensible receive pin for different CPU's
 */
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
/*
 * This program runs on 1 MHz CPU clock :-) and is requires only 1550 bytes program memory using ATTiny core
 */
#include "ATtinySerialOut.hpp" // TX is at pin 2 - Available as Arduino library "ATtinySerialOut" - Saves up to 700 bytes program memory and 70 bytes RAM for ATtinyCore
#  if defined(ARDUINO_AVR_DIGISPARKPRO)
#define IR_RECEIVE_PIN    9 // PA3 - on Digispark board labeled as pin 9
#  else
#define IR_RECEIVE_PIN    0 // PCINT0
#  endif
#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
#define IR_RECEIVE_PIN    10
#elif (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
#define IR_RECEIVE_PIN    21 // INT0
#elif defined(ESP8266)
#define IR_RECEIVE_PIN    14 // D5
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define IR_RECEIVE_PIN    8
#elif defined(ESP32)
#define IR_RECEIVE_PIN    15
#elif defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_MBED_NANO)
#define IR_RECEIVE_PIN    3   // GPIO15 Use pin 3 since pin 2|GPIO25 is connected to LED on Pi pico
#elif defined(ARDUINO_ARCH_RP2040) // Pi Pico with arduino-pico core https://github.com/earlephilhower/arduino-pico
#define IR_RECEIVE_PIN    15  // to be compatible with the Arduino Nano RP2040 Connect (pin3)
#else
#define IR_RECEIVE_PIN    2   // INT0
//#define NO_LED_FEEDBACK_CODE   // Activate this if you want to suppress LED feedback or if you do not have a LED. This saves 14 bytes code and 2 clock cycles per interrupt.
#endif

//#define DEBUG // to see if attachInterrupt is used
//#define TRACE // to see the state of the ISR state machine

/*
 * Second: include the code and compile it.
 */
//#define USE_FAST_PROTOCOL // Use short protocol. No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command
#include "TinyIRReceiver.hpp"

/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

volatile struct TinyIRReceiverCallbackDataStruct sCallbackData;

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
#if defined(ESP8266) || defined(ESP32)
    Serial.println();
#endif
    Serial.println(F("START " __FILE__ " from " __DATE__));
    // Enables the interrupt generation on change of IR input signal
    if (!initPCIInterruptForTinyReceiver()) {
        Serial.println(F("No interrupt available for pin " STR(IR_RECEIVE_PIN))); // optimized out by the compiler, if not required :-)
    }
#if defined(USE_FAST_PROTOCOL)
    Serial.println(F("Ready to receive Fast IR signals at pin " STR(IR_RECEIVE_PIN)));
#else
    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RECEIVE_PIN)));
#endif
}

void loop() {
    if (sCallbackData.justWritten) {
        sCallbackData.justWritten = false;
#if defined(USE_FAST_PROTOCOL)
        Serial.print(F("Command=0x"));
#else
        Serial.print(F("Address=0x"));
        Serial.print(sCallbackData.Address, HEX);
        Serial.print(F(" Command=0x"));
#endif
        Serial.print(sCallbackData.Command, HEX);
        if (sCallbackData.Flags == IRDATA_FLAGS_IS_REPEAT) {
            Serial.print(F(" Repeat"));
        }
        if (sCallbackData.Flags == IRDATA_FLAGS_PARITY_FAILED) {
            Serial.print(F(" Parity failed"));
        }
        Serial.println();
    }
    /*
     * Put your code here
     */
}

/*
 * This is the function is called if a complete command was received
 * It runs in an ISR context with interrupts enabled, so functions like delay() etc. should work here
 */
#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif

#if defined(USE_FAST_PROTOCOL)
void handleReceivedTinyIRData(uint8_t aCommand, uint8_t aFlags)
#else
void handleReceivedTinyIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags)
#endif
        {
#if defined(ARDUINO_ARCH_MBED) || defined(ESP32)
    // Copy data for main loop, this is the recommended way for handling a callback :-)
#  if !defined(USE_FAST_PROTOCOL)
    sCallbackData.Address = aAddress;
#  endif
    sCallbackData.Command = aCommand;
    sCallbackData.Flags = aFlags;
    sCallbackData.justWritten = true;
#else
    /*
     * Printing is not allowed in ISR context for any kind of RTOS
     * For Mbed we get a kernel panic and "Error Message: Semaphore: 0x0, Not allowed in ISR context" for Serial.print()
     * for ESP32 we get a "Guru Meditation Error: Core  1 panic'ed" (we also have an RTOS running!)
     */
    // Print only very short output, since we are in an interrupt context and do not want to miss the next interrupts of the repeats coming soon
#  if defined(USE_FAST_PROTOCOL)
    printTinyReceiverResultMinimal(&Serial, aCommand, aFlags);
#  else
    printTinyReceiverResultMinimal(&Serial, aAddress, aCommand, aFlags);
#  endif
#endif
}
