/*
 * IRreceiveDemo.cpp
 *
 * Demonstrates receiving IR codes with IRrecv
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols are active.
 * This must be done before the #include <IRremote.h>
 */
//#define DECODE_LG           1
//#define DECODE_NEC          1
// etc. see IRremote.h

//#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 2000 bytes program space if all protocols are active

#define MARK_EXCESS_MICROS    20 // recommended for the cheap VS1838 modules

#include <IRremote.h>

#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#elif defined(ARDUINO_AVR_PROMICRO)
int IR_RECEIVE_PIN = 10;
#else
int IR_RECEIVE_PIN = 11;
#endif

#define BUZZER_PIN          5
#define DEBUG_BUTTON_PIN    6 // if held low, print timing for each received data

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(2000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // In case the interrupt driver crashes on setup, give a clue
    // to the user what's going on.
    Serial.println("Enabling IRin");
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     */
    if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            IrReceiver.decodedIRData.flags = false; // yes we have recognized the flag :-)
            // no need to call resume, this is already done by decode()
            Serial.println(F("Overflow detected"));
#if !defined(ESP32)
            /*
             * do double beep
             */
            IrReceiver.stop();
            tone(BUZZER_PIN, 1100, 10);
            delay(50);
#endif

        } else {
            // Print a short summary of received data
            IrReceiver.printIRResultShort(&Serial);
            if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
                // We have an unknown protocol, print more info
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
        }
        Serial.println();

#if !defined(ESP32)
        /*
         * Play tone, wait and restore IR timer
         */
        IrReceiver.stop();
        tone(BUZZER_PIN, 2200, 10);
        delay(11);
        IrReceiver.start();
#endif

        IrReceiver.resume(); // Enable receiving of the next value

        /*
         * Finally check the received data and perform actions according to the received address and command
         */
        if (IrReceiver.decodedIRData.address == 0) {
            if (IrReceiver.decodedIRData.command == 0x10) {
                // do something
            } else if (IrReceiver.decodedIRData.command == 0x11) {
                // do something else
            }
        }
    }
}
