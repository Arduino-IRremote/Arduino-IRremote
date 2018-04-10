#include "IRremote.h"
#include "IRremoteInt.h"

//+=============================================================================
IRemulate::IRemulate (int emulatepin)
: emulatepin_(emulatepin)
{
}

//+=============================================================================
void  IRemulate::emulateRaw (const unsigned int buf[],  unsigned int len)
{
	// Set IR carrier frequency
	enableIROut();

	for (unsigned int i = 0;  i < len;  i++) {
		if (i & 1)  space(buf[i]) ;
		else        mark (buf[i]) ;
	}

	space(0);  // Always end with the Emulator on
}

//+=============================================================================
// Sends an IR mark for the specified number of microseconds.
// The mark output is modulated at the PWM frequency.
//
void  IRemulate::mark (unsigned int time)
{
	digitalWrite(emulatepin_, LOW); // Disable emulation pin output
	if (time > 0) custom_delay_usec(time);
}

//+=============================================================================
// Leave pin off for time (given in microseconds)
// Sends an IR space for the specified number of microseconds.
// A space is no output, so the PWM output is disabled.
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
