/*
 * SendDemo.cpp
 *
 * Demonstrates sending IR codes in standard format with address and command
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

#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.

#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
//#define EXCLUDE_EXOTIC_PROTOCOLS  // Saves around 240 bytes program memory if IrSender.write is used
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition
//#define USE_ACTIVE_HIGH_OUTPUT_FOR_SEND_PIN // Simulate an active high receiver signal instead of an active low signal.
//#define USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN  // Use or simulate open drain output mode at send pin. Attention, active state of open drain is LOW, so connect the send LED between positive supply and send pin!
//#define NO_LED_FEEDBACK_CODE      // Saves 566 bytes program memory

//#undef IR_SEND_PIN // enable this, if you need to set send pin programmatically using uint8_t tSendPin below
#include <IRremote.hpp>

#define DELAY_AFTER_SEND 2000
#define DELAY_AFTER_LOOP 5000

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

#if defined(IR_SEND_PIN)
    IrSender.begin(); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
//    disableLEDFeedback(); // Disable feedback LED at default feedback LED pin
#  if defined(IR_SEND_PIN_STRING)
    Serial.println(F("Send IR signals at pin " IR_SEND_PIN_STRING));
#  else
    Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));
#  endif
#else
    // Here the macro IR_SEND_PIN is not defined or undefined above with #undef IR_SEND_PIN
    uint8_t tSendPin = 3;
    IrSender.begin(tSendPin, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
    // You can change send pin later with IrSender.setSendPin();

    Serial.print(F("Send IR signals at pin "));
    Serial.println(tSendPin);
#endif

#if !defined(SEND_PWM_BY_TIMER)
    /*
     * Print internal software PWM signal generation info
     */
    IrSender.enableIROut(38); // Call it with 38 kHz just to initialize the values printed below
    Serial.print(F("Send signal mark duration is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse narrowing correction is "));
    Serial.print(IrSender.getPulseCorrectionNanos());
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
#endif

#if defined(LED_BUILTIN) && !defined(NO_LED_FEEDBACK_CODE)
#  if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
    Serial.print(F("Active low "));
#  endif
    Serial.print(F("FeedbackLED at pin "));
    Serial.println(LED_BUILTIN); // Works also for ESP32: static const uint8_t LED_BUILTIN = 8; #define LED_BUILTIN LED_BUILTIN
#endif

}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint16_t sAddress = 0x0102;
uint8_t sCommand = 0x34;
uint16_t s16BitCommand = 0x5634;
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
    Serial.flush();

    Serial.println(F("Send NEC with 8 bit address"));
    Serial.flush();
    IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND); // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal

    Serial.println(F("Send NEC with 16 bit address"));
    Serial.flush();
    IrSender.sendNEC(sAddress, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send NEC2 with 16 bit address"));
    Serial.flush();
    IrSender.sendNEC2(sAddress, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    if (sRepeats == 0) {
#if FLASHEND >= 0x3FFF && ((defined(RAMEND) && RAMEND > 0x6FF) || (defined(RAMSIZE) && RAMSIZE > 0x6FF)) // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
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
        delay(DELAY_AFTER_SEND);

        /*
         * !!! The next data occupies 136 bytes RAM !!!
         */
        Serial.println(
                F("Send NEC sendRaw data with 8 bit address=0xFB04 and command 0x08 and exact timing (16 bit array format)"));
        Serial.flush();
        const uint16_t irSignal[] = { 9000, 4500/*Start bit*/, 560, 560, 560, 560, 560, 1690, 560,
                560/*0010 0x4 of 16 bit address LSB first*/, 560, 560, 560, 560, 560, 560, 560, 560/*0000*/, 560, 1690, 560, 1690,
                560, 560, 560, 1690/*1101 0xB*/, 560, 1690, 560, 1690, 560, 1690, 560, 1690/*1111*/, 560, 560, 560, 560, 560, 560,
                560, 1690/*0001 0x08 of command LSB first*/, 560, 560, 560, 560, 560, 560, 560, 560/*0000 0x00*/, 560, 1690, 560,
                1690, 560, 1690, 560, 560/*1110 Inverted 8 of command*/, 560, 1690, 560, 1690, 560, 1690, 560,
                1690/*1111 inverted 0 of command*/, 560 /*stop bit*/}; // Using exact NEC timing
        IrSender.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
        delay(DELAY_AFTER_SEND);

        /*
         * With sendNECRaw() you can send 32 bit combined codes
         */
        Serial.println(F("Send ONKYO with 16 bit address 0x0102 and 16 bit command 0x0304 with NECRaw(0x03040102)"));
        Serial.flush();
        IrSender.sendNECRaw(0x03040102, sRepeats);
        delay(DELAY_AFTER_SEND);

        /*
         * With Send sendNECMSB() you can send your old 32 bit codes.
         * To convert one into the other, you must reverse the byte positions and then reverse all positions of each byte.
         * Use bitreverse32Bit().
         * Example:
         * 0xCB340102 byte reverse -> 0x020134CB bit reverse-> 40802CD3
         */
        Serial.println(F("Send ONKYO with 16 bit address 0x0102 and command 0x34 with old 32 bit format MSB first (0x40802CD3)"));
        Serial.flush();
        IrSender.sendNECMSB(0x40802CD3, 32, false);
        delay(DELAY_AFTER_SEND);
#endif

        Serial.println(F("Send Panasonic 0xB, 0x10 as 48 bit PulseDistance using ProtocolConstants"));
        Serial.flush();
#if __INT_WIDTH__ < 32
        IRRawDataType tRawData[] = { 0xB02002, 0xA010 }; // LSB of tRawData[0] is sent first
        IrSender.sendPulseDistanceWidthFromArray(&KaseikyoProtocolConstants, &tRawData[0], 48, NO_REPEATS); // Panasonic is a Kaseikyo variant
#else
        IrSender.sendPulseDistanceWidth(&KaseikyoProtocolConstants, 0xA010B02002, 48, NO_REPEATS); // Panasonic is a Kaseikyo variant
#endif

        delay(DELAY_AFTER_SEND);

        /*
         * Send 2 Panasonic 48 bit codes as Pulse Distance data, once with LSB and once with MSB first
         */
        Serial.println(F("Send Panasonic 0xB, 0x10 as 48 bit PulseDistance"));
        Serial.println(F(" LSB first"));
        Serial.flush();
#if __INT_WIDTH__ < 32
        IrSender.sendPulseDistanceWidthFromArray(38, 3450, 1700, 450, 1250, 450, 400, &tRawData[0], 48,
        PROTOCOL_IS_LSB_FIRST, 0, NO_REPEATS);
#else
        IrSender.sendPulseDistanceWidth(38, 3450, 1700, 450, 1250, 450, 400, 0xA010B02002, 48, PROTOCOL_IS_LSB_FIRST,
         0, NO_REPEATS);
#endif
        delay(DELAY_AFTER_SEND);

        // The same with MSB first. Use bit reversed raw data of LSB first part
        Serial.println(F(" MSB first"));
#if __INT_WIDTH__ < 32
        tRawData[0] = 0x40040D00;  // MSB of tRawData[0] is sent first
        tRawData[1] = 0x805;
        IrSender.sendPulseDistanceWidthFromArray(38, 3450, 1700, 450, 1250, 450, 400, &tRawData[0], 48,
        PROTOCOL_IS_MSB_FIRST, 0, NO_REPEATS);
#else
        IrSender.sendPulseDistanceWidth(38, 3450, 1700, 450, 1250, 450, 400, 0x40040D000805, 48, PROTOCOL_IS_MSB_FIRST,
         0, NO_REPEATS);
#endif
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send 72 bit PulseDistance 0x5A AFEDCBA9 87654321 LSB first"));
        Serial.flush();
#      if __INT_WIDTH__ < 32
        tRawData[0] = 0x87654321;  // LSB of tRawData[0] is sent first
        tRawData[1] = 0xAFEDCBA9;
        tRawData[2] = 0x5A;
        IrSender.sendPulseDistanceWidthFromArray(38, 8900, 4450, 550, 1700, 550, 600, &tRawData[0], 72, PROTOCOL_IS_LSB_FIRST, 0,
        NO_REPEATS);
#      else
        IRRawDataType tRawData[] = { 0xAFEDCBA987654321, 0x5A }; // LSB of tRawData[0] is sent first
        IrSender.sendPulseDistanceWidthFromArray(38, 8900, 4450, 550, 1700, 550, 600, &tRawData[0], 72, PROTOCOL_IS_LSB_FIRST, 0, NO_REPEATS);
#      endif
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send 52 bit PulseDistanceWidth 0xDCBA9 87654321 LSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does not require a stop bit
#if __INT_WIDTH__ < 32
        IrSender.sendPulseDistanceWidthFromArray(38, 300, 600, 600, 300, 300, 600, &tRawData[0], 52,
        PROTOCOL_IS_LSB_FIRST, 0, 0);
#else
        IrSender.sendPulseDistanceWidth(38, 300, 600, 600, 300, 300, 600, 0xDCBA987654321, 52, PROTOCOL_IS_LSB_FIRST,
        0, 0);
#endif
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send ASCII 7 bit PulseDistanceWidth LSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does theoretically not require a stop bit, but we know the stop bit from serial transmission
        IrSender.sendPulseDistanceWidth(38, 6000, 500, 500, 1500, 1500, 500, sCommand, 7, PROTOCOL_IS_LSB_FIRST, 0, 0);
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send Sony12 as PulseWidth LSB first"));
        Serial.flush();
        uint32_t tData = (uint32_t) sAddress << 7 | (sCommand & 0x7F);
        IrSender.sendPulseDistanceWidth(38, 2400, 600, 1200, 600, 600, 600, tData, SIRCS_12_PROTOCOL, PROTOCOL_IS_LSB_FIRST, 0, 0);
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send 32 bit PulseWidth 0x87654321 LSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does not require a stop bit
        IrSender.sendPulseDistanceWidth(38, 1000, 500, 600, 300, 300, 300, 0x87654321, 32, PROTOCOL_IS_LSB_FIRST, 0, 0);
        delay(DELAY_AFTER_SEND);
    }

    Serial.println(F("Send Onkyo (NEC with 16 bit command)"));
    Serial.flush();
    IrSender.sendOnkyo(sAddress, s16BitCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Apple"));
    Serial.flush();
    IrSender.sendApple(sAddress & 0xFF, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Panasonic"));
    Serial.flush();
    IrSender.sendPanasonic(sAddress & 0xFFF, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Kaseikyo with 0x4711 as Vendor ID"));
    Serial.flush();
    IrSender.sendKaseikyo(sAddress & 0xFFF, sCommand, sRepeats, 0x4711);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Kaseikyo_Denon variant"));
    Serial.flush();
    IrSender.sendKaseikyo_Denon(sAddress & 0xFFF, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Denon"));
    Serial.flush();
    IrSender.sendDenon(sAddress & 0x1F, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Denon/Sharp variant"));
    Serial.flush();
    IrSender.sendSharp(sAddress & 0x1F, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 5 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0x1F, sCommand & 0x7F, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 8 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0xFF, sCommand, sRepeats, SIRCS_15_PROTOCOL);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 13 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0x1FFF, sCommand & 0x7F, sRepeats, SIRCS_20_PROTOCOL);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Samsung 8 bit command"));
    Serial.flush();
    IrSender.sendSamsung(sAddress, sCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Samsung 16 bit command"));
    Serial.flush();
    IrSender.sendSamsung(sAddress, s16BitCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Samsung48 16 bit command"));
    Serial.flush();
    IrSender.sendSamsung48(sAddress, s16BitCommand, sRepeats);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC5"));
    Serial.flush();
    IrSender.sendRC5(sAddress & 0x1F, sCommand & 0x3F, sRepeats, true); // 5 address, 6 command bits
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC5X with 7.th MSB of command set"));
    Serial.flush();
    IrSender.sendRC5(sAddress & 0x1F, (sCommand & 0x3F) + 0x40, sRepeats, true); // 5 address, 7 command bits
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC6"));
    Serial.flush();
    IrSender.sendRC6(sAddress, sCommand, sRepeats, true);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send RC6A with 14 bit 0x2711 as extra"));
    Serial.flush();
    IrSender.sendRC6A(sAddress & 0xFF, sCommand, sRepeats, 0x2711, true);
    delay(DELAY_AFTER_SEND);

#if FLASHEND >= 0x3FFF && ((defined(RAMEND) && RAMEND > 0x4FF) || (defined(RAMSIZE) && RAMSIZE > 0x4FF)) // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.

    Serial.println(F("Send MagiQuest"));
    Serial.flush();
    IrSender.sendMagiQuest(0x6BCD0000 | (uint32_t) sAddress, s16BitCommand); // we have 31 bit address
    delay(DELAY_AFTER_SEND);

    // Bang&Olufsen must be sent with 455 kHz
//    Serial.println(F("Send Bang&Olufsen"));
//    Serial.flush();
//    IrSender.sendBangOlufsen(sAddress, sCommand, sRepeats);
//    delay(DELAY_AFTER_SEND);

    /*
     * Next example how to use the IrSender.write function
     */
    IRData IRSendData;
    // prepare data
    IRSendData.address = sAddress;
    IRSendData.command = sCommand;
    IRSendData.flags = IRDATA_FLAGS_EMPTY;

    Serial.println(F("Send next protocols with IrSender.write"));
    Serial.flush();

    IRSendData.protocol = JVC; // switch protocol
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    delay(DELAY_AFTER_SEND);

    IRSendData.command = s16BitCommand;  // LG support more than 8 bit command

    IRSendData.protocol = SAMSUNG;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    delay(DELAY_AFTER_SEND);

    IRSendData.protocol = LG;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    delay(DELAY_AFTER_SEND);

    IRSendData.protocol = BOSEWAVE;
    Serial.println(F("Send Bosewave with no address and 8 command bits"));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    delay(DELAY_AFTER_SEND);

    IRSendData.protocol = FAST;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    delay(DELAY_AFTER_SEND);

    /*
     * LEGO is difficult to receive because of its short marks and spaces
     */
    Serial.println(F("Send Lego with 2 channel and with 4 command bits"));
    Serial.flush();
    IrSender.sendLegoPowerFunctions(sAddress, sCommand, LEGO_MODE_COMBO, true);
    delay(DELAY_AFTER_SEND);

#endif // FLASHEND
    /*
     * Force buffer overflow
     */
    Serial.println(F("Force buffer overflow by sending 700 marks and spaces"));
    for (unsigned int i = 0; i < 350; ++i) {
        // 400 + 400 should be received as 8/8 and sometimes as 9/7 or 7/9 if compensation by MARK_EXCESS_MICROS is optimal.
        // 210 + 540 = 750 should be received as 5/10 or 4/11 if compensation by MARK_EXCESS_MICROS is optimal.
        IrSender.mark(210); // 8 pulses at 38 kHz
        IrSender.space(540); // to fill up to 750 us
    }
    delay(DELAY_AFTER_SEND);

    /*
     * Increment values
     * Also increment address just for demonstration, which normally makes no sense
     */
    sAddress += 0x0101;
    sCommand += 0x11;
    s16BitCommand += 0x1111;
    sRepeats++;
    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }

    delay(DELAY_AFTER_LOOP); // additional delay at the end of each loop
}

