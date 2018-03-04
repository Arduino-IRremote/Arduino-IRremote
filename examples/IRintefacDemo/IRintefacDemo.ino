/*
 * IRremote: IRinterfacDemo - demonstrates sending IR codes with IRsend using IRinterfaceAC
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * Modified to use AC 48bit protocol by Marcelo Buregio
 */

#include <IRremote.h>
#include <IRinterfaceAC.h>

IRinterfaceAC irintf;

void setup()
{
}

void loop() {
  irintf.turnOn(); // Turn AC On
  delay(5000);
  irintf.turnOff(); // Turn AC Off
  delay(5000);
}
