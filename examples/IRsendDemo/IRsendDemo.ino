/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

IRsend irsend;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

void loop() {
    unsigned long tData = 0xa90;
    for (int i = 0; i < 3; i++) {
        irsend.sendSony(tData, 12);
        Serial.print(F("sendSony(0x"));
        Serial.print(tData,HEX);
        Serial.println(F(", 12)"));
//        irsend.sendJVC(0xC5B8, 16,0); // hex value, 16 bits, no repeat
//        delayMicroseconds(50); // see http://www.sbprojects.com/knowledge/ir/jvc.php for information
//        irsend.sendJVC(0xC5B8, 16,1); // hex value, 16 bits, repeat
//        Serial.println(F("sendJVC(9xC5B8, 16)"));

        delay(40);
    }

    tData++;
    delay(5000); //5 second delay between each signal burst
}
