/*
 * IRremote: IRemulateDemo - demonstrates sending IR codes with IRemulate
 * An IR receiving module like another Arduino with IRrecv code should be attached EMULATE_PIN
 * Version 0.1 April, 2018
 * By Kristian Korsgaard
 */


#include <IRremote.h>

int EMULATE_PIN = 2;

IRemulate iremulate(EMULATE_PIN);

void setup()
{
}

void loop() {
	for (int i = 0; i < 3; i++) {
		iremulate.emulateNEC(0xabcd1234, 32);
		delay(40);
	}
	delay(5000); //5 second delay between each signal
}
