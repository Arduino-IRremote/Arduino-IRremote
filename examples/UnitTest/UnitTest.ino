/*
 * UnitTest.cpp
 *
 * Demonstrates sending IR codes in standard format with address and command and
 * simultaneously receiving. Both values are checked for consistency.
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

#if !defined(RAW_BUFFER_LENGTH)
#  if RAMEND <= 0x4FF || RAMSIZE < 0x4FF
//#define RAW_BUFFER_LENGTH  180  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  elif RAMEND <= 0x8FF || RAMSIZE < 0x8FF
#define RAW_BUFFER_LENGTH  150  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  else
#define RAW_BUFFER_LENGTH  200  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#  endif
#endif

//#define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
//#define EXCLUDE_EXOTIC_PROTOCOLS  // Saves around 240 bytes program memory if IrSender.write is used
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition
#define NO_LED_FEEDBACK_CODE        // Saves 344 bytes program memory
// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
//#define USE_MSB_DECODING_FOR_DISTANCE_DECODER
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 142.
//#define MARK_EXCESS_MICROS    20    // Adapt it to your IR receiver module. 40 is taken for the cheap VS1838 module her, since we have high intensity.

//#define TRACE // For internal usage
//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#if FLASHEND >= 0x1FFF      // For 8k flash or more, like ATtiny85
#define DECODE_DENON        // Includes Sharp
#define DECODE_KASEIKYO
#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
#define DECODE_NEC          // Includes Apple and Onkyo
#endif

#if FLASHEND >= 0x3FFF      // For 16k flash or more, like ATtiny1604
#define DECODE_JVC
#define DECODE_RC5
#define DECODE_RC6

#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
#define DECODE_HASH         // special decoder for all protocols
#endif

#if FLASHEND >= 0x7FFF      // For 32k flash or more, like ATmega328
#define DECODE_SONY
#define DECODE_SAMSUNG
#define DECODE_LG

#define DECODE_BEO // It prevents decoding of SONY (default repeats), which we are not using here.
//#define ENABLE_BEO_WITHOUT_FRAME_GAP // For successful unit testing we must see the warning at ir_BangOlufsen.hpp:88:2
#if defined(DECODE_BEO)
#define RECORD_GAP_MICROS 16000 // always get the complete frame in the receive buffer
#define BEO_KHZ         38  // We send and receive Bang&Olufsen with 38 kHz here (instead of 455 kHz).
#endif

#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER
#define DECODE_FAST
#endif

#include <IRremote.hpp>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN // if held low, print timing for each received data
#else
#define DEBUG_BUTTON_PIN   6
#endif

#define DELAY_AFTER_SEND 1000
#define DELAY_AFTER_LOOP 5000

#if defined(SEND_PWM_BY_TIMER) && !defined(SEND_PWM_DOES_NOT_USE_RECEIVE_TIMER)
#error Unit test cannot run if SEND_PWM_BY_TIMER is enabled i.e. receive timer us also used by send
#endif

/*
 * For callback
 */
volatile bool sDataJustReceived = false;
void ReceiveCompleteCallbackHandler();

void setup() {
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    IrReceiver.registerReceiveCompleteCallback(ReceiveCompleteCallbackHandler);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
#if defined(IR_RECEIVE_PIN_STRING)
    Serial.println(F("at pin " IR_RECEIVE_PIN_STRING));
#else
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
#endif

#if defined(IR_SEND_PIN)
    IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
#  if defined(IR_SEND_PIN_STRING)
    Serial.println(F("at pin " IR_SEND_PIN_STRING));
# else
    Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));
#  endif
#else
    IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
    Serial.println(F("Send IR signals at pin 3"));
#endif

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
    Serial.print(F("If you connect debug pin "));
#  if defined(APPLICATION_PIN_STRING)
    Serial.print(APPLICATION_PIN_STRING);
#  else
    Serial.print(DEBUG_BUTTON_PIN);
#  endif
    Serial.println(F(" to ground, raw data is always printed"));

    // For esp32 we use PWM generation by ledcWrite() for each pin.
#  if !defined(SEND_PWM_BY_TIMER)
    /*
     * Print internal software PWM generation info
     */
    IrSender.enableIROut(38); // Call it with 38 kHz to initialize the values printed below
    Serial.print(F("Send signal mark duration for 38kHz is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse narrowing correction is "));
    Serial.print(IrSender.getPulseCorrectionNanos());
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
#  endif

    // infos for receive
    Serial.print(RECORD_GAP_MICROS);
    Serial.println(F(" us is the (minimum) gap, after which the start of a new IR packet is assumed"));
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
#endif
    delay(DELAY_AFTER_SEND);

}

void checkReceivedRawData(IRRawDataType aRawData) {
    // wait until signal has received
    while (!sDataJustReceived) {
    };
    sDataJustReceived = false;

    if (IrReceiver.decode()) {
// Print a short summary of received data
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
#else
        IrReceiver.printIRResultMinimal(&Serial);
#endif
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
#endif
        if (IrReceiver.decodedIRData.protocol == PULSE_DISTANCE || IrReceiver.decodedIRData.protocol == PULSE_WIDTH) {
            if (IrReceiver.decodedIRData.decodedRawData != aRawData) {
                Serial.print(F("ERROR: Received data=0x"));
#if (__INT_WIDTH__ < 32)
                Serial.print(IrReceiver.decodedIRData.decodedRawData, HEX);
#else
                PrintULL::print(&Serial, IrReceiver.decodedIRData.decodedRawData, HEX);
#endif
                Serial.print(F(" != sent data=0x"));
#if (__INT_WIDTH__ < 32)
                Serial.print(aRawData, HEX);
#else
                PrintULL::print(&Serial, aRawData, HEX);
#endif
                Serial.println();
            }
        }
        IrReceiver.resume();
    } else {
        Serial.println(F("No data received"));
    }
    Serial.println();
}

#if defined(DECODE_DISTANCE_WIDTH)
void checkReceivedArray(IRRawDataType *aRawDataArrayPointer, uint8_t aArraySize) {
    // wait until signal has received
    while (!sDataJustReceived) {
    };
    sDataJustReceived = false;

    if (IrReceiver.decode()) {
// Print a short summary of received data
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
#else
        IrReceiver.printIRResultMinimal(&Serial);
#endif
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
#endif

        if (IrReceiver.decodedIRData.protocol == PULSE_DISTANCE || IrReceiver.decodedIRData.protocol == PULSE_WIDTH) {
            for (uint_fast8_t i = 0; i < aArraySize; ++i) {
                if (IrReceiver.decodedIRData.decodedRawDataArray[i] != *aRawDataArrayPointer) {
                    Serial.print(F("ERROR: Received data=0x"));
#  if (__INT_WIDTH__ < 32)
                    Serial.print(IrReceiver.decodedIRData.decodedRawDataArray[i], HEX);
#  else
                    PrintULL::print(&Serial, IrReceiver.decodedIRData.decodedRawDataArray[i], HEX);
#  endif
                    Serial.print(F(" != sent data=0x"));
                    Serial.println(*aRawDataArrayPointer, HEX);
                }
                aRawDataArrayPointer++;
            }
        }
        IrReceiver.resume();
    } else {
        Serial.println(F("No data received"));
    }
    Serial.println();
}
#endif

/*
 * Test callback function
 * Has the same functionality as available()
 */
void ReceiveCompleteCallbackHandler() {
    sDataJustReceived = true;
}

void checkReceive(uint16_t aSentAddress, uint16_t aSentCommand) {
    // wait until signal has received
    while (!sDataJustReceived) {
    };
    sDataJustReceived = false;

    if (IrReceiver.decode()) {
// Print a short summary of received data
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
#else
        IrReceiver.printIRResultMinimal(&Serial);
#endif

        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
            // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library
        }

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
        if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
#endif
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            Serial.println(F("ERROR: Unknown protocol"));
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
uint16_t s16BitCommand = 0x9876;
#define sRepeats  0 // no unit test for repeats

void loop() {
    /*
     * Print values
     */
    Serial.println();
    Serial.print(F("address=0x"));
    Serial.print(sAddress, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(sCommand, HEX);
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

    Serial.println(F("Send NEC2 with 16 bit address"));
    Serial.flush();
    IrSender.sendNEC2(sAddress, sCommand, sRepeats);
    checkReceive(sAddress, sCommand);
    delay(DELAY_AFTER_SEND);

#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.

    if (sAddress == 0xFFF1) {
#  if FLASHEND >= 0x7FFF  // For 32k flash or more, like UNO. Code does not fit in program memory of ATtiny1604 etc.
        /*
         * Send constant values only once in this demo
         */
        Serial.println(F("Send NEC Pronto data with 8 bit address 0x80 and command 0x45 and no repeats"));
        Serial.flush();
        IrSender.sendPronto(F("0000 006D 0022 0000 015E 00AB " /* Pronto header + start bit */
                "0017 0015 0017 0015 0017 0017 0015 0017 0017 0015 0017 0015 0017 0015 0017 003F " /* Lower address byte */
                "0017 003F 0017 003E 0017 003F 0015 003F 0017 003E 0017 003F 0017 003E 0017 0015 " /* Upper address byte (inverted at 8 bit mode) */
                "0017 003E 0017 0015 0017 003F 0017 0015 0017 0015 0017 0015 0017 003F 0017 0015 " /* command byte */
                "0019 0013 0019 003C 0017 0015 0017 003F 0017 003E 0017 003F 0017 0015 0017 003E " /* inverted command byte */
                "0017 0806"), 0); //stop bit, no repeat possible, because of missing repeat pattern
        checkReceive(0x80, 0x45);
        delay(DELAY_AFTER_SEND);

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
        checkReceive(0xFB04 & 0xFF, 0x08);
        delay(DELAY_AFTER_SEND);

        /*
         * With sendNECRaw() you can send 32 bit codes directly, i.e. without parity etc.
         */
        Serial.println(F("Send ONKYO with 16 bit address 0x0102 and 16 bit command 0x0304 with NECRaw(0x03040102)"));
        Serial.flush();
        IrSender.sendNECRaw(0x03040102, sRepeats);
        checkReceive(0x0102, 0x304);
        delay(DELAY_AFTER_SEND);

        /*
         * With Send sendNECMSB() you can send your old 32 bit codes.
         * To convert one into the other, you must reverse the byte positions and then reverse all positions of each byte.
         * Example:
         * 0xCB340102 byte reverse -> 0x020134CB bit reverse-> 40802CD3
         */
        Serial.println(F("Send ONKYO with 16 bit address 0x0102 and command 0x34 with old 32 bit format MSB first (0x40802CD3)"));
        Serial.flush();
        IrSender.sendNECMSB(0x40802CD3, 32, false);
        checkReceive(0x0102, 0x34);
        delay(DELAY_AFTER_SEND);
#  endif

#  if defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)
        Serial.println(F("Send Panasonic 0xB, 0x10 as 48 bit generic PulseDistance using ProtocolConstants"));
        Serial.flush();
#    if __INT_WIDTH__ < 32
        IRRawDataType tRawData[] = { 0xB02002, 0xA010, 0x0 }; // LSB of tRawData[0] is sent first
        IrSender.sendPulseDistanceWidthFromArray(&KaseikyoProtocolConstants, &tRawData[0], 48, NO_REPEATS); // Panasonic is a Kaseikyo variant
        checkReceive(0x0B, 0x10);
#    else
        IrSender.sendPulseDistanceWidth(&KaseikyoProtocolConstants, 0xA010B02002, 48, NO_REPEATS); // Panasonic is a Kaseikyo variant
        checkReceivedRawData(0xA010B02002);
#    endif
        delay(DELAY_AFTER_SEND);

        /*
         * Send 2 Panasonic 48 bit codes as generic Pulse Distance data, once with LSB and once with MSB first
         */
        Serial.println(F("Send Panasonic 0xB, 0x10 as generic 48 bit PulseDistance"));
        Serial.println(F(" LSB first"));
        Serial.flush();
#    if __INT_WIDTH__ < 32
        IrSender.sendPulseDistanceWidthFromArray(38, 3450, 1700, 450, 1250, 450, 400, &tRawData[0], 48, PROTOCOL_IS_LSB_FIRST, 0,
        NO_REPEATS);
        checkReceive(0x0B, 0x10);
#    else
        IrSender.sendPulseDistanceWidth(38, 3450, 1700, 450, 1250, 450, 400, 0xA010B02002, 48, PROTOCOL_IS_LSB_FIRST,
        0, NO_REPEATS);
        checkReceivedRawData(0xA010B02002);
#    endif
        delay(DELAY_AFTER_SEND);

        // The same with MSB first. Use bit reversed raw data of LSB first part
        Serial.println(F(" MSB first"));
#    if __INT_WIDTH__ < 32
        tRawData[0] = 0x40040D00;  // MSB of tRawData[0] is sent first
        tRawData[1] = 0x805;
        IrSender.sendPulseDistanceWidthFromArray(38, 3450, 1700, 450, 1250, 450, 400, &tRawData[0], 48, PROTOCOL_IS_MSB_FIRST, 0,
        NO_REPEATS);
        checkReceive(0x0B, 0x10);
#    else
        IrSender.sendPulseDistanceWidth(38, 3450, 1700, 450, 1250, 450, 400, 0x40040D000805, 48, PROTOCOL_IS_MSB_FIRST, 0, NO_REPEATS);
        checkReceivedRawData(0x40040D000805);
#    endif

        delay(DELAY_AFTER_SEND);
#  endif // defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)

#  if defined(DECODE_DISTANCE_WIDTH)
#    if defined(USE_MSB_DECODING_FOR_DISTANCE_DECODER)
        Serial.println(F("Send generic 52 bit PulseDistance 0x43D8613C and 0x3BC3B MSB first"));
        Serial.flush();
#      if __INT_WIDTH__ < 32
        tRawData[0] = 0x43D8613C;  // MSB of tRawData[0] is sent first
        tRawData[1] = 0x3BC3B;
        IrSender.sendPulseDistanceWidthFromArray(38, 8900, 4450, 550, 1700, 550, 600, &tRawData[0], 52, PROTOCOL_IS_MSB_FIRST, 0,
        NO_REPEATS);
        checkReceivedArray(tRawData, 2);
#      else
        IrSender.sendPulseDistanceWidth(38, 8900, 4450, 550, 1700, 550, 600, 0x43D8613CBC3B, 52, PROTOCOL_IS_MSB_FIRST, 0, NO_REPEATS);
        checkReceivedRawData(0x43D8613CBC3B);
#      endif
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send generic 52 bit PulseDistanceWidth 0x43D8613C and 0x3BC3B MSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does not require a stop bit
#      if __INT_WIDTH__ < 32
        IrSender.sendPulseDistanceWidthFromArray(38, 300, 600, 600, 300, 300, 600, &tRawData[0], 52, PROTOCOL_IS_MSB_FIRST, 0, 0);
        checkReceivedArray(tRawData, 2);
#      else
        IrSender.sendPulseDistanceWidth(38, 300, 600, 600, 300, 300, 600, 0x123456789ABC, 52, PROTOCOL_IS_MSB_FIRST, 0, 0);
        checkReceivedRawData(0x123456789ABC);
#      endif
        delay(DELAY_AFTER_SEND);
        Serial.println(F("Send generic 32 bit PulseWidth 0x43D8613C MSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does not require a stop bit
        IrSender.sendPulseDistanceWidth(38, 1000, 500, 600, 300, 300, 300, 0x43D8613C, 32, PROTOCOL_IS_MSB_FIRST, 0, 0);
        checkReceivedRawData(0x43D8613C);
        delay(DELAY_AFTER_SEND);

#    else // defined(USE_MSB_DECODING_FOR_DISTANCE_DECODER)
        Serial.println(F("Send generic 72 bit PulseDistance 0x5A AFEDCBA9 87654321 LSB first"));
        Serial.flush();
#      if __INT_WIDTH__ < 32
        tRawData[0] = 0x87654321;  // LSB of tRawData[0] is sent first
        tRawData[1] = 0xAFEDCBA9;
        tRawData[2] = 0x5A;
        IrSender.sendPulseDistanceWidthFromArray(38, 8900, 4450, 550, 1700, 550, 600, &tRawData[0], 72, PROTOCOL_IS_LSB_FIRST, 0,
        NO_REPEATS);
        checkReceivedArray(tRawData, 3);
#      else
        IRRawDataType tRawData[] = { 0xAFEDCBA987654321, 0x5A }; // LSB of tRawData[0] is sent first
        IrSender.sendPulseDistanceWidthFromArray(38, 8900, 4450, 550, 1700, 550, 600, &tRawData[0], 72, PROTOCOL_IS_LSB_FIRST, 0, NO_REPEATS);
        checkReceivedArray(tRawData, 2);
#      endif
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send generic 52 bit PulseDistanceWidth 0xDCBA9 87654321 LSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does not require a stop bit
#      if __INT_WIDTH__ < 32
        tRawData[1] = 0xDCBA9;
        IrSender.sendPulseDistanceWidthFromArray(38, 300, 600, 600, 300, 300, 600, &tRawData[0], 52, PROTOCOL_IS_LSB_FIRST, 0, 0);
        checkReceivedArray(tRawData, 2);
#      else
        IrSender.sendPulseDistanceWidth(38, 300, 600, 600, 300, 300, 600, 0xDCBA987654321, 52, PROTOCOL_IS_LSB_FIRST, 0, 0);
        checkReceivedRawData(0xDCBA987654321);
#      endif
        delay(DELAY_AFTER_SEND);

        Serial.println(F("Send generic 32 bit PulseWidth 0x87654321 LSB first"));
        Serial.flush();
        // Real PulseDistanceWidth (constant bit length) does not require a stop bit
        IrSender.sendPulseDistanceWidth(38, 1000, 500, 600, 300, 300, 300, 0x87654321, 32, PROTOCOL_IS_LSB_FIRST, 0, 0);
        checkReceivedRawData(0x87654321);
        delay(DELAY_AFTER_SEND);
#    endif // defined(USE_MSB_DECODING_FOR_DISTANCE_DECODER)
#  endif // defined(DECODE_DISTANCE_WIDTH)

#  if defined(DECODE_MAGIQUEST)
        Serial.println(F("Send MagiQuest 0x6BCDFF00, 0x176 as generic 55 bit PulseDistanceWidth MSB first"));
        Serial.flush();
#    if __INT_WIDTH__ < 32
        tRawData[0] = 0x01AF37FC; // We have 1 header (start) bit and 7 start bits and 31 address bits for MagiQuest, so 0x6BCDFF00 is shifted 2 left
        tRawData[1] = 0x017619; // We send only 23 instead of 24 bite here! 19 is the checksum
        IrSender.sendPulseDistanceWidthFromArray(38, 287, 864, 576, 576, 287, 864, &tRawData[0], 55, PROTOCOL_IS_MSB_FIRST, 0, 0);
#    else
        // 0xD79BFE00 is 0x6BCDFF00 is shifted 1 left
        IrSender.sendPulseDistanceWidth(38, 287, 864, 576, 576, 287, 864, 0xD79BFE017619, 55, PROTOCOL_IS_MSB_FIRST, 0, 0);
#    endif
        checkReceive(0xFF00, 0x176);
        if (IrReceiver.decodedIRData.decodedRawData != 0x6BCDFF00) {
            Serial.print(F("ERROR: Received address=0x"));
#if (__INT_WIDTH__ < 32)
            Serial.print(IrReceiver.decodedIRData.decodedRawData, HEX);
#else
            PrintULL::print(&Serial, IrReceiver.decodedIRData.decodedRawData, HEX);
#endif
            Serial.println(F(" != sent address=0x6BCDFF00"));
            Serial.println();
        }
        delay(DELAY_AFTER_SEND);
#  endif // defined(DECODE_MAGIQUEST)

    }
#endif // if FLASHEND >= 0x3FFF

    Serial.println(F("Send Onkyo (NEC with 16 bit command)"));
    Serial.flush();
    IrSender.sendOnkyo(sAddress, (sCommand + 1) << 8 | sCommand, sRepeats);
    checkReceive(sAddress, (sCommand + 1) << 8 | sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Apple"));
    Serial.flush();
    IrSender.sendApple(sAddress & 0xFF, sCommand, sRepeats);
    checkReceive(sAddress & 0xFF, sCommand);
    delay(DELAY_AFTER_SEND);

#if defined(DECODE_PANASONIC) || defined(DECODE_KASEIKYO)
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

    Serial.println(F("Send Kaseikyo_Denon variant"));
    Serial.flush();
    IrSender.sendKaseikyo_Denon(sAddress & 0xFFF, sCommand, sRepeats);
    checkReceive(sAddress & 0xFFF, sCommand);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_DENON)
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
#endif

#if defined(DECODE_SONY)
    Serial.println(F("Send Sony/SIRCS with 7 command and 5 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0x1F, sCommand, sRepeats);
    checkReceive(sAddress & 0x1F, sCommand & 0x7F);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 8 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0xFF, sCommand, sRepeats, SIRCS_15_PROTOCOL);
    checkReceive(sAddress & 0xFF, sCommand & 0x7F);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Sony/SIRCS with 7 command and 13 address bits"));
    Serial.flush();
    IrSender.sendSony(sAddress & 0x1FFF, sCommand, sRepeats, SIRCS_20_PROTOCOL);
    checkReceive(sAddress & 0x1FFF, sCommand & 0x7F);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_SAMSUNG)
    Serial.println(F("Send Samsung 8 bit command"));
    Serial.flush();
    IrSender.sendSamsung(sAddress, sCommand, sRepeats);
    checkReceive(sAddress, sCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Samsung 16 bit command"));
    Serial.flush();
    IrSender.sendSamsung(sAddress, s16BitCommand, sRepeats);
    checkReceive(sAddress, s16BitCommand);
    delay(DELAY_AFTER_SEND);

    Serial.println(F("Send Samsung48 16 bit command"));
    Serial.flush();
    IrSender.sendSamsung48(sAddress, s16BitCommand, sRepeats);
    checkReceive(sAddress, s16BitCommand);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_RC5)
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
#endif

#if defined(DECODE_RC6)
    Serial.println(F("Send RC6"));
    // RC6 check does not work stable without the flush
    Serial.flush();
    IrSender.sendRC6(sAddress & 0xFF, sCommand, sRepeats, true);
    checkReceive(sAddress & 0xFF, sCommand);
    delay(DELAY_AFTER_SEND);
#endif

    /*
     * Next example how to use the IrSender.write function
     */
    IRData IRSendData;
    // prepare data
    IRSendData.address = sAddress;
    IRSendData.command = sCommand;
    IRSendData.flags = IRDATA_FLAGS_EMPTY;

#if defined(DECODE_JVC)
    IRSendData.protocol = JVC;  // switch protocol
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(IRSendData.address & 0xFF, IRSendData.command);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_LG) || defined(DECODE_MAGIQUEST)
    IRSendData.command = s16BitCommand; // LG support more than 8 bit command
#endif

#if defined(DECODE_SAMSUNG)
    IRSendData.protocol = SAMSUNG;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(IRSendData.address, IRSendData.command);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_LG)
    IRSendData.protocol = LG;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(IRSendData.address & 0xFF, IRSendData.command);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_MAGIQUEST)
    Serial.println(F("Send MagiQuest"));
    Serial.flush();
    IrSender.sendMagiQuest(0x6BCD0000 | (uint32_t) sAddress, s16BitCommand); // we have 31 bit address
    checkReceive(sAddress, s16BitCommand & 0x1FF); // we have 9 bit command
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_BEO)
    Serial.println(F("Send Bang&Olufsen"));
    Serial.flush();
    IrSender.sendBangOlufsen(sAddress & 0x0FF, sCommand, sRepeats);
#  if defined(ENABLE_BEO_WITHOUT_FRAME_GAP)
    delay((RECORD_GAP_MICROS / 1000) + 1);
    IrReceiver.printIRResultRawFormatted(&Serial, true);
    uint8_t tOriginalRawlen = IrReceiver.decodedIRData.rawDataPtr->rawlen;
    IrReceiver.decodedIRData.rawDataPtr->rawlen = 6;
    // decode first part of frame
    IrReceiver.decode();
    IrReceiver.printIRResultShort(&Serial);

    // Remove trailing 6 entries for next decode
    IrReceiver.decodedIRData.rawDataPtr->rawlen = tOriginalRawlen - 6;
    for (uint_fast8_t i = 0; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; ++i) {
        IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i + 6];
    }
#  endif
    checkReceive(sAddress & 0x0FF, sCommand);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_BOSEWAVE)
    IRSendData.protocol = BOSEWAVE;
    Serial.println(F("Send Bosewave with no address and 8 command bits"));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(0, IRSendData.command & 0xFF);
    delay(DELAY_AFTER_SEND);
#endif

#if defined(DECODE_FAST)
    IRSendData.protocol = FAST;
    Serial.print(F("Send "));
    Serial.println(getProtocolString(IRSendData.protocol));
    Serial.flush();
    IrSender.write(&IRSendData, sRepeats);
    checkReceive(0, IRSendData.command & 0xFF);
    delay(DELAY_AFTER_SEND);
#endif

    /*
     * LEGO is skipped, since it is difficult to receive because of its short marks and spaces
     */
//    Serial.println(F("Send Lego with 2 channel and with 4 command bits"));
//    Serial.flush();
//    IrSender.sendLegoPowerFunctions(sAddress, sCommand, LEGO_MODE_COMBO, true);
//    checkReceive(sAddress, sCommand); // never has success for Lego protocol :-(
//    delay(DELAY_AFTER_SEND);
    /*
     * Force buffer overflow
     */
    Serial.println(F("Force buffer overflow by sending 280 marks and spaces"));
    for (unsigned int i = 0; i < 140; ++i) {
        // 400 + 400 should be received as 8/8 and sometimes as 9/7 or 7/9 if compensation by MARK_EXCESS_MICROS is optimal.
        // 210 + 540 = 750 should be received as 5/10 or 4/11 if compensation by MARK_EXCESS_MICROS is optimal.
        IrSender.mark(210);         // 8 pulses at 38 kHz
        IrSender.space(540);        // to fill up to 750 us
    }
    checkReceive(sAddress, sCommand);
    delay(DELAY_AFTER_SEND);

    /*
     * Increment values
     * Also increment address just for demonstration, which normally makes no sense
     */
    sAddress += 0x0101;
    sCommand += 0x11;
    s16BitCommand += 0x1111;

    delay(DELAY_AFTER_LOOP); // additional delay at the end of each loop
}

