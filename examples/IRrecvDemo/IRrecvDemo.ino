/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int IR_RECEIVE_PIN = 11;

IRrecv irrecv(IR_RECEIVE_PIN);

decode_results results;

void setup() {
    Serial.begin(9600);
#if defined(__AVR_ATmega32U4__)
  while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    // In case the interrupt driver crashes on setup, give a clue
    // to the user what's going on.
    Serial.println("Enabling IRin");
    irrecv.enableIRIn(); // Start the receiver
    Serial.println("Enabled IRin");
}

void loop() {
    if (irrecv.decode(&results)) {
        Serial.println(results.value, HEX);
        irrecv.resume(); // Receive the next value
    }
    delay(100);
}
