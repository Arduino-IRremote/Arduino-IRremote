/*
 * IRremote: IRrelay - demonstrates receiving IR codes with IRrecv
 * Toggles an output pin at each command received
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Initially coded 2009 Ken Shirriff http://www.righto.com
 */

#include <IRremote.h>

#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#else
int IR_RECEIVE_PIN = 11;
#endif
int RELAY_PIN = 4; // is labeled D2 on the Chinese SAMD21 M0-Mini clone

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
// The Chinese SAMD21 M0-Mini clone has no led connected, if you connect it, it is on pin 24 like on the original board.
// Attention! D2 and D4 are reversed on these boards
//#undef LED_BUILTIN
//#define LED_BUILTIN 25 // Or choose pin 25, it is the RX pin, but active low.
#endif

void dump();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(2000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

int on = 0;
unsigned long last = millis();

void loop() {
    if (IrReceiver.decode()) {
        // If it's been at least 1/4 second since the last
        // IR received, toggle the relay
        if (millis() - last > 250) {
            on = !on;
            Serial.print(F("Switch relay "));
            if (on) {
                digitalWrite(RELAY_PIN, HIGH);
                Serial.println(F("on"));
            } else {
                digitalWrite(RELAY_PIN, LOW);
                Serial.println(F("off"));
            }

            IrReceiver.printIRResultShort(&Serial);
            Serial.println();
            if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                // We have an unknown protocol, print more info
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
        }
        last = millis();
        IrReceiver.resume(); // Enable receiving of the next value
    }
}
