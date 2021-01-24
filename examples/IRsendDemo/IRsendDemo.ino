/*
 * IRsendDemo.cpp
 *
 * Demonstrates sending IR codes in standard format with address and command
 *
 * An IR LED must be connected to Arduino PWM pin 3 (IR_SEND_PIN).
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

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(2000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);

    IrSender.begin(true); // Enable feedback LED,
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
    IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
    delay(2000); // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal

    Serial.println(F("Send NEC with 16 bit address"));
    IrSender.sendNEC(sAddress, sCommand, sRepeats);
    delay(2000);

    if (sRepeats == 0) {
        /*
         * Send constant values only once in this demo
         */
        Serial.println(F("Sending NEC Pronto data with 8 bit address 0x80 and command 0x45 and no repeats"));
        IrSender.sendPronto(F("0000 006D 0022 0000 015E 00AB " /* Pronto header + start bit */
                "0017 0015 0017 0015 0017 0017 0015 0017 0017 0015 0017 0015 0017 0015 0017 003F " /* Lower address byte */
                "0017 003F 0017 003E 0017 003F 0015 003F 0017 003E 0017 003F 0017 003E 0017 0015 " /* Upper address byte (inverted at 8 bit mode) */
                "0017 003E 0017 0015 0017 003F 0017 0015 0017 0015 0017 0015 0017 003F 0017 0015 " /* command byte */
                "0019 0013 0019 003C 0017 0015 0017 003F 0017 003E 0017 003F 0017 0015 0017 003E " /* inverted command byte */
                "0017 0806"), 0); //stop bit, no repeat possible, because of missing repeat pattern
        delay(2000);

        Serial.println(F("Send NECRaw 0xCC340102 with 16 bit address 0x102 and command 0x34 which results in a parity error, since CC != ~34"));
        IrSender.sendNECRaw(0xCC340102, sRepeats);
        delay(2000);
    }

    Serial.println(F("Send Panasonic"));
    IrSender.sendPanasonic(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Kaseikyo with 0x4711 as Vendor ID"));
    IrSender.sendKaseikyo(sAddress, sCommand, sRepeats, 0x4711);
    delay(2000);

    Serial.println(F("Send Denon"));
    IrSender.sendDenon(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Denon/Sharp variant"));
    IrSender.sendSharp(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Sony/SIRCS with 7 command and 5 address bits"));
    IrSender.sendSony(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send Sony/SIRCS with 7 command and 13 address bits"));
    IrSender.sendSony(sAddress, sCommand, sRepeats, SIRCS_20_PROTOCOL);
    delay(2000);

    Serial.println(F("Send RC5"));
    IrSender.sendRC5(sAddress, sCommand, sRepeats, true);
    delay(2000);

    Serial.println(F("Send RC5X with command + 0x40"));
    IrSender.sendRC5(sAddress, sCommand + 0x40, sRepeats, true);
    delay(2000);

    Serial.println(F("Send RC6"));
    IrSender.sendRC6(sAddress, sCommand, sRepeats, true);
    delay(2000);

    Serial.println(F("Send Samsung"));
    IrSender.sendSamsung(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send JVC"));
    IrSender.sendJVC(sAddress, sCommand, sRepeats);
    delay(2000);

    Serial.println(F("Send LG"));
    IrSender.sendLG((uint8_t) sAddress, sCommand, sRepeats);
    delay(2000);

    if (sRepeats == 0) {
        /*
         * Send constant values only once in this demo
         * This value is the valid LG turn off command which violates the parity rule
         * All other commands obey this rule.
         * see: https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/examples/LGACSendDemo
         */
        Serial.println(F("Send LGRaw 0x88C0051 with \"parity error\" since C+0+0+5 != 1"));
        IrSender.sendLGRaw(0x88C0051, sRepeats);
        delay(2000);
    }

#if !defined(__AVR_ATtiny85__) // code does not fit in program space of ATtiny85
    Serial.println(F("Send Bosewave with 8 command bits"));
    IrSender.sendBoseWave(sCommand, sRepeats);
    delay(2000);

    /*
     * LEGO is difficult to receive because of its short marks and spaces
     */
    Serial.println(F("Send Lego with 2 channel and with 4 command bits"));
    IrSender.sendLegoPowerFunctions(sAddress, sCommand, LEGO_MODE_COMBO, true);
    delay(2000);

    /*
     * Force buffer overflow
     */
    Serial.println(F("Force buffer overflow by sending 100 marks and spaces"));
    for (unsigned int i = 0; i < RAW_BUFFER_LENGTH; ++i) {
        IrSender.mark(400);
        IrSender.space(400);
    }
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
#endif // !defined(__AVR_ATtiny85__)

    delay(4000); // additional delay at the end of each loop
}
