/*
 * MultipleSendPins.cpp
 *
 *  Demonstrates sending IR codes toggling between 2 different send pins.
 *  Based on SimpleSender.
 *
 *  Copyright (C) 2025  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  MIT License
 */
#include <Arduino.h>

#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition

#include <IRremote.hpp> // include the library

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
    Serial.print(F("Send IR signals alternating at pin 3 and 4"));

    /*
     * The IR library setup. That's all!
     */
    IrSender.begin(3); // Start with pin3 as send pin and enable feedback LED at default feedback LED pin
    disableLEDFeedback(); // Disable feedback LED at default feedback LED pin
}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

void loop() {
    /*
     * Print current send values
     */
    Serial.println();
    Serial.print(F("Send now: address=0x00, command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(", repeats="));
    Serial.print(sRepeats);
    Serial.println();

    Serial.println(F("Send standard NEC with 8 bit address"));
    Serial.flush();

    // Receiver output for the first loop must be: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendNEC(0x00, sCommand, sRepeats);

    /*
     * If you want to send a raw HEX value directly like e.g. 0xCB340102 you must use sendNECRaw()
     */
//    Serial.println(F("Send 32 bit LSB 0xCB340102 with NECRaw()"));
//    IrSender.sendNECRaw(0xCB340102, sRepeats);
    /*
     * If you want to send an "old" MSB HEX value used by IRremote versions before 3.0 like e.g. 0x40802CD3 you must use sendNECMSB()
     */
//    Serial.println(F("Send old 32 bit MSB 0x40802CD3 with sendNECMSB()"));
//    IrSender.sendNECMSB(0x40802CD3, 32, sRepeats);
    /*
     * Increment send values
     */
    sCommand += 0x11;
    sRepeats++;
    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }

    /*
     * Toggle between send pin 3 and 4
     */
    if (IrSender.sendPin == 3) {
        IrSender.setSendPin(4);
    } else {
        IrSender.setSendPin(3);
    }
    delay(1000);  // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal
}
