/*
 * IRremote: IRemulateRawDemo - demonstrates sending IR codes with emulateRaw
 * An IR receiving module like another Arduino with IRrecv code should be attached EMULATE_PIN
 * Version 0.1 April, 2018
 * By Kristian Korsgaard
 *
 */


#include <IRremote.h>

int EMULATE_PIN = 2;

IRemulate iremulate(EMULATE_PIN);

void setup()
{

}

void loop() {
  unsigned int irSignal[] = {9000, 4500, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 39416, 9000, 2210, 560}; //AnalysIR Batch Export (IRremote) - RAW

  iremulate.emulateRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0])); //Note the approach used to automatically calculate the size of the array.

  delay(5000); //In this example, the signal will be repeated every 5 seconds, approximately.
}
