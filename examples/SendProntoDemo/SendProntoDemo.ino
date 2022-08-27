/*
 * SendProntoDemo.cpp
 *
 *  Example for sending pronto codes with the IRremote library.
 *  The code used here, sends NEC protocol data.
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

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

#define NUMBER_OF_REPEATS 3U

// The first number, here 0000, denotes the type of the signal. 0000 denotes a raw IR signal with modulation.
// The second number, here 006C, denotes a frequency code. 006C corresponds to 1000000/(0x006c * 0.241246) = 38381 Hertz.
// The third and the forth number denote the number of pairs (= half the number of durations) in the start- and the repeat sequence respectively.
const char yamahaVolDown[] PROGMEM
= "0000 006C 0022 0002 015B 00AD " /* Pronto header + start bit */
        "0016 0016 0016 0041 0016 0016 0016 0041 0016 0041 0016 0041 0016 0041 0016 0016 " /* Lower address byte */
        "0016 0041 0016 0016 0016 0041 0016 0016 0016 0016 0016 0016 0016 0016 0016 0041 " /* Upper address byte (inverted at 8 bit mode) */
        "0016 0041 0016 0041 0016 0016 0016 0041 0016 0041 0016 0016 0016 0016 0016 0016 " /* command byte */
        "0016 0016 0016 0016 0016 0041 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 05F7 " /* inverted command byte + stop bit */
        "015B 0057 0016 0E6C"; /* NEC repeat pattern*/

IRsend irsend;

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ;

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

#if defined(IR_SEND_PIN)
    IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
#else
    IrSender.begin(3, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin
#endif

    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

void loop() {

#if defined(__AVR__)
    Serial.println(F("Sending NEC from PROGMEM: address 0x85, data 0x1B"));
    irsend.sendPronto_P(yamahaVolDown, NUMBER_OF_REPEATS);
#else
    Serial.println(F("Sending from normal memory"));
    irsend.sendPronto(yamahaVolDown, NUMBER_OF_REPEATS);
#endif

    delay(2000);
    Serial.println(F("Sending the NEC from PROGMEM using the F()-form: address 0x5, data 0x1A"));
    irsend.sendPronto(F("0000 006C 0022 0002 015B 00AD " /* Pronto header + start bit */
            "0016 0016 0016 0041 0016 0016 0016 0041 0016 0041 0016 0041 0016 0041 0016 0041 " /* Lower address byte */
            "0016 0041 0016 0016 0016 0041 0016 0016 0016 0016 0016 0016 0016 0016 0016 0016 " /* Upper address byte (inverted at 8 bit mode) */
            "0016 0016 0016 0041 0016 0016 0016 0041 0016 0041 0016 0016 0016 0016 0016 0016 " /* command byte */
            "0016 0041 0016 0016 0016 0041 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 05F7 " /* inverted command byte + stop bit */
            "015B 0057 0016 0E6C"), /* NEC repeat pattern*/
    NUMBER_OF_REPEATS);
    delay(2000);

    // send Nec code acquired by IRreceiveDump.cpp
    Serial.println(F("Sending NEC from RAM: address 0xFF00, data 0x15"));
    // 006D -> 38029 Hz
    irsend.sendPronto("0000 006D 0022 0000 015C 00AB " /* Pronto header + start bit */
            "0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 " /* Lower address byte */
            "0017 003F 0017 003E 0017 003F 0017 003E 0017 003F 0015 003F 0017 003F 0015 003F " /* Upper address byte (inverted at 8 bit mode) */
            "0017 003E 0017 0015 0017 003F 0017 0015 0017 003E 0017 0015 0017 0017 0015 0017 " /* command byte */
            "0017 0015 0017 003E 0017 0015 0017 003F 0015 0017 0017 003E 0017 003F 0015 003F 0017 0806" /* inverted command byte + stop bit */
    , 0); // No repeat possible, because of missing repeat pattern

    delay(5000);
}
