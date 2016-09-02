/*
 * IRremote: IRrecvNew - demonstrates receiving IR codes with IRrecv
 * using standard patterns: begin(), available() and read()
 * 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 2.1 2016-09-02 by DoDi
 */

#include <IRremote.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

void setup()
{
  Serial.begin(9600);
  irrecv.begin(true); //blinking
}

void loop() {
  if (irrecv.available(irNEC)) {
      Serial.println(irrecv.read(), HEX);
  }
}
