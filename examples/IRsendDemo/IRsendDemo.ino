/*
 * IRsendDemo.cpp
 *
 *  Demonstrates sending IR codes in standard format with address and command
 *
 * An IR LED must be connected to Arduino PWM pin 3 (IR_SEND_PIN).
 * To receive IR signals in compatible format, you must comment out the line
 * #define USE_STANDARD_DECODE in IRremote.h.
 *
 *
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 */

#include <IRremote.h>

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
    Serial.println(F("START " __FILE__ " from " __DATE__));
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

    Serial.println(F("Send Kaseikyo with 0x7411 as Vendor ID"));
    IrSender.sendKaseikyoStandard(sAddress, sCommand, 0x7411, sRepeats);
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

    Serial.println(F("Send Sony/SIRCS with with 7 command and 13 address bits"));
    IrSender.sendSonyStandard(sAddress, sCommand, true, sRepeats);
    delay(2000);

    /*
     * Increment values
     */
    sAddress += 0x0101;
    sCommand += 0x11;
    sRepeats++;
    // clip repeats at 3
    if (sRepeats > 3) {
        sRepeats = 3;
    }

    delay(5000); //  second additional delay between each values
}
