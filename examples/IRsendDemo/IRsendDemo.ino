/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;
IRrecv irrecv (RECV_PIN);

void setup()
{
  Serial.begin(115200);
}

void loop() {
  if (irrecv.isIdle())
  Serial.println("Idling");
}
