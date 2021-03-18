/*
 * UnitTest.cpp
 *
 * Demonstrates sending IR codes in standard format with address and command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2021 Armin Joachimsmeyer
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

/*
 * Define macros for input and output pin etc.
 */
#include "PinDefinitionsAndMore.h"

//#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 240 bytes program space if IrSender.write is used
//#define SEND_PWM_BY_TIMER
//#define USE_NO_SEND_PWM
//#define IR_MEASURE_TIMING
#define MARK_EXCESS_MICROS    -40 // for my VS1838 to get Denon right
#define DISABLE_LED_FEEDBACK_FOR_RECEIVE // halves ISR duration

#include <IRremote.h>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN // if held low, print timing for each received data
#else
#define DEBUG_BUTTON_PIN   6
#endif

#define DELAY_AFTER_SEND 1000
#define DELAY_AFTER_LOOP 5000

void setup() {
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
    pinMode(IR_TIMING_TEST_PIN, OUTPUT);
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    /*
     * Start the receiver, enable feedback LED and (if not 3. parameter specified) take LED feedback pin from the internal boards definition
     */
    IrReceiver.begin(IR_RECEIVE_PIN);
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);

#if defined(USE_SOFT_SEND_PWM) && !defined(ESP32) // for esp32 we use PWM generation by hw_timer_t for each pin
    /*
     * Print internal signal generation info
     */
    IrSender.enableIROut(38);

    Serial.print(F("Send signal mark duration is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse correction is "));
    Serial.print((uint16_t) PULSE_CORRECTION_NANOS);
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
#endif
    // infos for receive
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
}

void checkReceive(uint16_t aSentAddress, uint16_t aSentCommand) {
    // wait until signal has received
    delay((RECORD_GAP_MICROS / 1000) + 1);

    if (IrReceiver.decode()) {
// Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);

        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            IrReceiver.decodedIRData.flags = false; // yes we have recognized the flag :-)
            Serial.println(F("Overflow detected"));
        } else if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        } else {
            /*
             * Check address
             */
            if (IrReceiver.decodedIRData.address != aSentAddress) {
                Serial.print(F("ERROR: Received address=0x"));
                Serial.print(IrReceiver.decodedIRData.address, HEX);
                Serial.print(F(" != sent address=0x"));
                Serial.println(aSentAddress, HEX);
            }
            /*
             * Check command
             */
            if (IrReceiver.decodedIRData.command != aSentCommand) {
                Serial.print(F("ERROR: Received command=0x"));
                Serial.print(IrReceiver.decodedIRData.command, HEX);
                Serial.print(F(" != sent command=0x"));
                Serial.println(aSentCommand, HEX);
            }
        }
        IrReceiver.resume();
    } else {
        Serial.println(F("No data received"));
    }
    Serial.println();
}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint16_t sAddress = 0xFFF1;
uint8_t sCommand = 0x76;
uint8_t sRepeats = 0;

void loop() {
    /*
     * Print values
     */
    Serial.println();
    Serial.print(F("address=0x"));
    Serial.print(sAddress, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(" repeats="));
    Serial.println(sRepeats);
    Serial.println();
    Serial.println();

    Serial.println(F("Send NEC with 8 bit address"));
    Serial.flush();
    IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
    checkReceive(sAddress & 0xFF, sCommand);
    delay(DELAY_AFTER_SEND); // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal

    Serial.println(F("Send NEC with 16 bit address"));
    Serial.flush();
    IrSender.sendNEC(sAddress, sCommand, sRepeats);
    checkReceive(sAddress, sCommand);
    delay(DELAY_AFTER_SEND);

    if (sRepeats == 0) {
        /*
         * Send constant values only once in this demo
         */
        Serial.println(F("Sending NEC Pronto data with 8 bit address 0x80 and command 0x45 and no repeats"));
        Serial.flush();
        IrSender.sendPronto(F("0000 006D 0022 0000 015E 00AB " /* Pronto header + start bit */
                "0017 0015 0017 0015 0017 0017 0015 0017 0017 0015 0017 0015 0017 0015 0017 003F " /* Lower address byte */
                "0017 003F 0017 003E 0017 003F 0015 003F 0017 003E 0017 003F 0017 003E 0017 0015 " /* Upper address byte (inverted at 8 bit mode) */
                "0017 003E 0017 0015 0017 003F 0017 0015 0017 0015 0017 0015 0017 003F 0017 0015 " /* command byte */
                "0019 0013 0019 003C 0017 0015 0017 003F 0017 003E 0017 003F 0017 0015 0017 003E " /* inverted command byte */
                "0017 0806"), 0); //stop bit, no repeat possible, because of missing repeat pattern
        checkReceive(0x80, 0x45);
        delay(DELAY_AFTER_SEND);
        /*
         * With sendNECRaw() you can send even "forbidden" codes with parity errors
         */
        Serial.println(
                F(
                        "Send NEC with 16 bit address 0x0102 and command 0x34 with NECRaw(0xCC340102) which results in a parity error, since 34 == ~CB and not C0"));
        Serial.flush();
        IrSender.sendNECRaw(0xC0340102, sRepeats);
        checkReceive(0x0102, 0x34);
        delay(DELAY_AFTER_SEND);

        /*
         * With Send sendNECMSB() you can send your old 32 bit codes.
         * To convert one into the other, you must reverse the byte positions and then reverse all positions of each byte.
         * Example:
         * 0xCB340102 byte reverse -> 0x020134CB bit reverse-> 40802CD3
         */
        Serial.println(F("Send NEC with 16 bit address 0x0102 and command 0x34 with old 32 bit format MSB first"));
        Serial.flush();
        IrSender.sendNECMSB(0x40802CD3, 32, false);
        checkReceive(0x0102, 0x34);
        delay(DELAY_AFTER_SEND);
    }

    Serial.println(F("Send Apple"));
    Serial.flush();
    IrSender.sendApple(sAddress & 0xFF, sCommand, sRepeats);
    checkReceive(sAddress & 0xFF, sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Panasonic"));
    Serial.flush();
    IrSender.sendPanasonic(sAddress & 0xFFF, sCommand, sRepeats);
    checkReceive(sAddress & 0xFFF, sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Kaseikyo with 0x4711 as Vendor ID"));
    Serial.flush();
    IrSender.sendKaseikyo(sAddress & 0xFFF, sCommand, sRepeats, 0x4711);
    checkReceive(sAddress & 0xFFF, sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Denon"));
    Serial.flush();
    IrSender.sendDenon(sAddress & 0x1F, sCommand, sRepeats);
    checkReceive(sAddress & 0x1F, sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Denon/Sharp variant"));
    Serial.flush();
    IrSender.sendSharp(sAddress & 0x1F, sCommand, sRepeats);
    checkReceive(sAddress & 0x1F, sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 5 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0x1F, sCommand & 0x7F, sRepeats);
    checkReceive(sAddress & 0x1F, sCommand & 0x7F);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 8 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0xFF, sCommand, sRepeats, SIRCS_15_PROTOCOL);
    checkReceive(sAddress & 0xFF, sCommand & 0x7F);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 13 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0x1FFF, sCommand & 0x7F, sRepeats, SIRCS_20_PROTOCOL);
    checkReceive(sAddress & 0x1FFF, sCommand & 0x7F);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC5"));
    Serial.flush();
    IrSender.sendRC5(sAddress & 0x1F, sCommand & 0x3F, sRepeats, true);  // 5 address, 6 command bits
    checkReceive(sAddress & 0x1F, sCommand & 0x3F);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC5X with 7.th MSB of command set"));
    Serial.flush();
    IrSender.sendRC5(sAddress & 0x1F, (sCommand & 0x3F) + 0x40, sRepeats, true);  // 5 address, 7 command bits
    checkReceive(sAddress & 0x1F, (sCommand & 0x3F) + 0x40);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC6"));
    // RC6 check does not work stable without the flush
    Serial.flush();
    IrSender.sendRC6(sAddress & 0xFF, sCommand, sRepeats, true);
    checkReceive(sAddress & 0xFF, sCommand);
    delay(DELAY_AFTER_SEND);

    /*
     * Next example how to use the IrSender.write function
     */
    IRData IRSendData;
    // prepare data
    IRSendData.address = sAddress;
    IRSendData.command = sCommand;
    IRSendData.flags = IRDATA_FLAGS_EMPTY;

    IRSendData.protocol = SAMSUNG;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(IRSendData.address, IRSendData.command);
    delay(DELAY_AFTER_SEND);

    IRSendData.protocol = JVC; // switch protocol
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(IRSendData.address & 0xFF, IRSendData.command);
    delay(DELAY_AFTER_SEND);

    IRSendData.protocol = LG;
    IRSendData.command = sCommand << 8 | sCommand; // LG supports 16 bit command
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(IRSendData.address & 0xFF, IRSendData.command);
    delay(DELAY_AFTER_SEND);

    IRSendData.protocol = BOSEWAVE;
    Serial.println(F("Send Bosewave with no address and 8 command bits"));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(0, IRSendData.command & 0xFF);
    delay(DELAY_AFTER_SEND);

    /*
     * LEGO is difficult to receive because of its short marks and spaces
     */
//    Serial.println(F("Send Lego with 2 channel and with 4 command bits"));
//    Serial.flush();
//    IrSender.sendLegoPowerFunctions(sAddress, sCommand, LEGO_MODE_COMBO, true);
//    checkReceive(sAddress, sCommand); // never has success for Lego protocol :-(
//    delay(DELAY_AFTER_SEND);
    /*
     * Force buffer overflow
     */
    Serial.println(F("Force buffer overflow by sending 100 marks and spaces"));
    for (unsigned int i = 0; i < RAW_BUFFER_LENGTH; ++i) {
        IrSender.mark(400);
        IrSender.space(400);
    }
    checkReceive(sAddress, sCommand);
    delay(DELAY_AFTER_SEND);

    /*
     * Increment values
     * Also increment address just for demonstration, which normally makes no sense
     */
    sAddress += 0x0101;
    sCommand += 0x11;

    delay(DELAY_AFTER_LOOP); // additional delay at the end of each loop
}

