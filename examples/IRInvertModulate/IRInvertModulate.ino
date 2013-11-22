/*
 * IRremote: IRInvertModulate - demonstrates the ability enable/disable
 * IR signal modulation and inversion.
 * An IR LED must be connected to Arduino PWM pin 3.
 * To view the results, attach an Oscilloscope or Signal Analyser across the
 * legs of the IR LED.
 * Version 0.1 November, 2013
 * Copyright 2013 Aaron Snoswell
 * http://elucidatedbinary.com
 */

#include <IRremote.h>

IRsend irsend;

void setup()
{
  Serial.begin(9600);
  Serial.println("Welcome, visitor");
  Serial.println("Press 'm' to toggle IR modulation");
  Serial.println("Press 'i' to toggle IR inversion");
}

bool modulate = true;
bool invert = false;

void loop() {
  if (!Serial.available()) {
    // Send some random data
    irsend.sendNEC(0xa90, 12);
  } else {
    char c;
    do {
      c = Serial.read();
    } while(Serial.available());

    if(c == 'm') {
      modulate = !modulate;
      if(modulate) Serial.println("Enabling Modulation");
      else Serial.println("Disabling Modulation");
      irsend.enableIRModulation(modulate);
    } else if(c == 'i') {
      invert = !invert;
      if(invert) Serial.println("Enabling Invert");
      else Serial.println("Disabling Invert");
      irsend.enableIRInvert(invert);
    } else {
      Serial.println("Unknown Command");
    }
  }

  delay(300);
}
