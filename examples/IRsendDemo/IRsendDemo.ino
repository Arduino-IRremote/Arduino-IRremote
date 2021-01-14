/*
 * IRsendDemo.cpp
 *
 *  Demonstrates sending IR codes in standard format with address and command
 *
 * An IR LED must be connected to Arduino PWM pin 3 (IR_SEND_PIN).
 * To receive IR signals in compatible format, you must activate the line #define USE_STANDARD_DECODE in IRremote.h.
 *
 *
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 */

#include <IRremote.h>
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#include "ATtinySerialOut.h"
#endif

IRsend IrSender;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
    Serial.println(F("Send with standard protocol encoders"));
}

// Some protocols have 5, some 8 and some 16 bit Address
uint16_t sAddress = 0x0102;
uint8_t sCommand = 0x34;
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
    Serial.print(sRepeats);
    Serial.println();

    Serial.println(F("Send NEC with 8 bit address"));
    IrSender.sendNECStandard(sAddress, sCommand, false, sRepeats);
    delay(2000);

    Serial.println(F("Send NEC with 16 bit address"));
    IrSender.sendNECStandard(sAddress, sCommand, true, sRepeats);
    delay(2000);

    Serial.println(F("Send Panasonic"));
    IrSender.sendPanasonicStandard(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Kaseikyo with 0x4711 as Vendor ID"));
    IrSender.sendKaseikyoStandard(sAddress, sCommand, 0x4711, sRepeats);
    delay(2000);

    Serial.println(F("Send Denon"));
    IrSender.sendDenonStandard(sAddress, sCommand, false, sRepeats);
    delay(2000);

    Serial.println(F("Send Denon/Sharp variant"));
    IrSender.sendSharpStandard(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Sony/SIRCS with 7 command and 5 address bits"));
    IrSender.sendSonyStandard(sAddress, sCommand, false, sRepeats);
    delay(2000);

    Serial.println(F("Send Sony/SIRCS with 7 command and 13 address bits"));
    IrSender.sendSonyStandard(sAddress, sCommand, true, sRepeats);
    delay(2000);

    Serial.println(F("Send RC5"));
    IrSender.sendRC5Standard(sAddress, sCommand, true, sRepeats);
    delay(2000);

    Serial.println(F("Send RC5X with command + 0x40"));
    IrSender.sendRC5Standard(sAddress, sCommand + 0x40, true, sRepeats);
    delay(2000);

    Serial.println(F("Send RC6"));
    IrSender.sendRC6Standard(sAddress, sCommand, true, sRepeats);
    delay(2000);

    Serial.println(F("Send Samsung"));
    IrSender.sendSamsungStandard(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send JVC"));
    IrSender.sendJVCStandard(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send LG"));
    IrSender.sendLGStandard(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Bosewave with 8 command bits"));
    IrSender.sendBoseWaveStandard(sCommand, sRepeats);
    delay(2000);

    /*
     * !!LEGO is difficult to receive because of its short marks and spaces!!!
     */
    Serial.println(F("Send Lego with 2 channel and with 4 command bits"));
    IrSender.sendLegoPowerFunctions(sAddress, LEGO_MODE_COMBO, sCommand, true);
    delay(2000);

    /*
     * Increment values
     */
    sAddress += 0x0101;
    sCommand += 0x11;
    sRepeats++;
    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }

    delay(5000); //  second additional delay between each values
}
