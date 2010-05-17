/*
 * IRremote: IRrelay - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;
int RELAY_PIN = 4;

IRrecv irrecv(RECV_PIN);
decode_results results;


void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(13, OUTPUT);
    Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

int on = 0;
unsigned long last = millis();

void loop() {
  if (irrecv.decode(&results)) {
    // If it's been at least 1/4 second since the last
    // IR received, toggle the relay
    if (millis() - last > 250) {
      on = !on;
      digitalWrite(RELAY_PIN, on ? HIGH : LOW);
      digitalWrite(13, on ? HIGH : LOW);
    }
    last = millis();      
    irrecv.resume(); // Receive the next value
  }
}
