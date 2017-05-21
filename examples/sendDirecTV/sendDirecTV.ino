/*
 * IRremote: sendDirecTV   -- shows how to control a DirecTV or SKY HDTV boxes.
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 1 May 21, 2017
 * Copyright 2017 Danilo Queiroz Barbosa
 * member of electronicdrops.com
 * https://github.com/electronicdrops
 */


#include <IRremote.h>
#include "DirecTVIRDevice.h"

IRsend irsend;

DirecTVBox directv(&irsend);

void setup()
{
}

void loop() {

  directv.btn_power();
  
  delay(5000); //5 second delay between each signal burst
}
