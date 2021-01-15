/*
 * IRreceiveDemo.cpp
 *
 * Demonstrates receiving IR codes with IRrecv
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

#include <IRremote.h>

#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#elif defined(ARDUINO_AVR_PROMICRO)
int IR_RECEIVE_PIN = 10;
#else
int IR_RECEIVE_PIN = 11;
#endif
IRrecv IrReceiver(IR_RECEIVE_PIN);

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

    // In case the interrupt driver crashes on setup, give a clue
    // to the user what's going on.
    Serial.println("Enabling IRin");
    IrReceiver.enableIRIn();  // Start the receiver
    IrReceiver.blink13(true); // Enable feedback LED

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     */
    if (IrReceiver.decode()) {
        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
        Serial.println();

#if !defined(ESP32)
        /*
         * Play tone, wait and restore IR timer
         */
        tone(5, 2200, 10);
        delay(11);
        IrReceiver.enableIRIn();
#endif

        IrReceiver.resume(); // Enable receiving of the next value
        /*
         * Check the received data
         */
        if (IrReceiver.decodedIRData.command == 0x11) {
            // do something
        }
    } else if (IrReceiver.results.overflow) {
        IrReceiver.results.overflow = false;
        // no need to call resume, this is already done by decode()
        Serial.println(F("Overflow detected"));
    }
}
