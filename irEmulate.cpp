#include "IRremote.h"
#include "IRremoteInt.h"

//+=============================================================================
IRemulate::IRemulate (int emulatepin, bool solesource)
: emulatepin_(emulatepin), solesource_(solesource)
{
}

//+=============================================================================
void  IRemulate::emulateRaw (const unsigned int buf[],  unsigned int len)
{
	// Manage outputs
	enableIROut();

	for (unsigned int i = 0;  i < len;  i++) {
		if (i & 1)  space(buf[i]) ;
		else        mark (buf[i]) ;
	}

	disableIROut();
}

//+=============================================================================
// Sends an IR mark for the specified number of microseconds.
//
void  IRemulate::mark (unsigned int time)
{
	digitalWrite(emulatepin_, LOW); // Disable emulation pin output
	if (time > 0) custom_delay_usec(time);
}

//+=============================================================================
// Leave pin on for time (given in microseconds)
// Sends an IR space for the specified number of microseconds.
//
void  IRemulate::space (unsigned int time)
{
	digitalWrite(emulatepin_, HIGH); // Enable emulation pin output
	if (time > 0) custom_delay_usec(time);
}





//+=============================================================================
// Enables emulation output.
//
void  IRemulate::enableIROut ()
{
	// Disable the Timer2 Interrupt (which is used for receiving IR)
	TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt

	pinMode(emulatepin_, OUTPUT);
	digitalWrite(emulatepin_, HIGH); // When not sending, we want it high
}

//+=============================================================================
// Disables emulation output.
//
void  IRemulate::disableIROut ()
{
	// Enable the Timer2 Interrupt (which is used for receiving IR)
	TIMER_ENABLE_INTR; //Timer2 Overflow Interrupt

	if (solesource_) {
		space(0);  // Always end with the Emulator on
	} else {
		pinMode(emulatepin_, INPUT); // Enable high impedance on pin to allow actual receiver
	}
}

//+=============================================================================
// Custom delay function that circumvents Arduino's delayMicroseconds limit

void IRemulate::custom_delay_usec(unsigned long uSecs) {
  if (uSecs > 4) {
    unsigned long start = micros();
    unsigned long endMicros = start + uSecs - 4;
    if (endMicros < start) { // Check if overflow
      while ( micros() > start ) {} // wait until overflow
    }
    while ( micros() < endMicros ) {} // normal wait
  }
  //else {
  //  __asm__("nop\n\t"); // must have or compiler optimizes out
  //}
}
