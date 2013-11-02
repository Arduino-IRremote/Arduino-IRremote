/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

IRsend irsend;

void setup()
{
  Serial.begin(115200);
}

void loop() {
  if (Serial.read() != -1) {
      delay(500);
      irsend.sendMagiQuest(0x19D1DD01, 0xB6);
      delay(3000);
      irsend.sendMagiQuest(0x18C04B01, 0xEEEE);
      delay(3000);
      irsend.sendMagiQuest(0x4FAB881, 0xB6);
  }
}
