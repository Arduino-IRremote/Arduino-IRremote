/*
 * IRremote: IRrelay - demonstrates receiving IR codes with IRrecv
 * Toggles an output pin at each command received
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#else
int IR_RECEIVE_PIN = 11;
#endif
int RELAY_PIN = 4; // is labeled D2 on the Chinese SAMD21 M0-Mini clone

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
// The Chinese SAMD21 M0-Mini clone has no led connected, if you connect it, it is on pin 24 like on the original board.
// Attention! D2 and D4 are reversed on these boards
//#undef LED_BUILTIN
//#define LED_BUILTIN 25 // Or choose pin 25, it is the RX pin, but active low.
#endif

void dump(decode_results *results);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    irrecv.enableIRIn(); // Start the receiver

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

int on = 0;
unsigned long last = millis();

void loop() {
    if (irrecv.decode(&results)) {
        // If it's been at least 1/4 second since the last
        // IR received, toggle the relay
        if (millis() - last > 250) {
            on = !on;
            Serial.print(F("Switch relay "));
            if (on) {
                digitalWrite(RELAY_PIN, HIGH);
                digitalWrite(LED_BUILTIN, HIGH);
                Serial.println(F("on"));
            } else {
                digitalWrite(RELAY_PIN, LOW);
                digitalWrite(LED_BUILTIN, LOW);
                Serial.println(F("off"));
            }
            dump(&results);
        }
        last = millis();
        irrecv.resume(); // Receive the next value
    }
}


// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {
    int count = results->rawlen;
    if (results->decode_type == UNKNOWN) {
        Serial.println("Could not decode message");
    } else {
        if (results->decode_type == NEC) {
            Serial.print("Decoded NEC: ");
        } else if (results->decode_type == SONY) {
            Serial.print("Decoded SONY: ");
        } else if (results->decode_type == RC5) {
            Serial.print("Decoded RC5: ");
        } else if (results->decode_type == RC6) {
            Serial.print("Decoded RC6: ");
        }
        Serial.print(results->value, HEX);
        Serial.print(" (");
        Serial.print(results->bits, DEC);
        Serial.println(" bits)");
    }
    Serial.print("Raw (");
    Serial.print(count, DEC);
    Serial.print("): ");

    for (int i = 0; i < count; i++) {
        if ((i % 2) == 1) {
            Serial.print(results->rawbuf[i] * MICROS_PER_TICK, DEC);
        } else {
            Serial.print(-(int) results->rawbuf[i] * MICROS_PER_TICK, DEC);
        }
        Serial.print(" ");
    }
    Serial.println("");
}
