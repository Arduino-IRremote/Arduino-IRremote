/*
 * IRremote: IRsendDemoAC - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * Modified to use AC 48bit protocol by Marcelo Buregio
 */

#include <IRremote.h>

IRsend irsend;

void setup()
{
}

void loop() {
  irsend.sendAC(0xB24D, 0x9F6000FF); // Turn AC On
  delay(5000);
  irsend.sendAC(0xB24D, 0x7B84E01F); // Turn AC Off
  delay(5000);
}
