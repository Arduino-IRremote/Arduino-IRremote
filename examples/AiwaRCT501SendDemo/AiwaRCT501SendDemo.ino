/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include "IRremote.h"

#define POWER 0x7F80
#define AIWA_RC_T501

IRsend irsend;

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino Ready");
}

void loop() {
  if (Serial.read() != -1) {
    irsend.sendAiwaRCT501(POWER);
    delay(60); // Optional
  }
}
