/*
 * LegoPowerFunctionsSendDemo: LEGO Power Functions
 * Copyright (c) 2016 Philipp Henkel
 */

#include <IRremote.h>

IRsend irsend;

void setup() {
}

void loop() {
  // Send repeated command "channel 1, blue forward, red backward"
  irsend.sendLegoPowerFunctions(0x197);
  delay(2000);

  // Send single command "channel 1, blue forward, red backward"
  irsend.sendLegoPowerFunctions(0x197, false);
  delay(2000);
}
