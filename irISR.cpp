#include <avr/interrupt.h>

#include "IRremote.h"
#include "IRremoteInt.h"

//+=============================================================================
// Interrupt Service Routine - Fires every 50uS
// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50uS [microseconds, 0.000050 seconds]
// 'rawlen' counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a the first [SPACE] entry gets long:
//   Ready is set; State switches to IDLE; Timing of SPACE continues.
// As soon as first MARK arrives:
//   Gap width is recorded; Ready is cleared; New logging starts
//
ISR (TIMER_INTR_NAME)
{
	TIMER_RESET;

	// Read if IR Receiver -> SPACE [xmt LED off] or a MARK [xmt LED on]
	// digitalRead() is very slow. Optimisation is possible, but makes the code unportable
	uint8_t  irdata = (uint8_t)digitalRead(irparams.recvpin);

	irparams.timer++;  // One more 50uS tick
	if (irparams.rawlen >= RAWBUF)  irparams.rcvstate = STATE_OVERFLOW ;  // Buffer overflow

	switch(irparams.rcvstate) {
		case STATE_IDLE: // In the middle of a gap
			if (irdata == MARK) {
				if (irparams.timer < GAP_TICKS) {
					// Not big enough to be a gap.
					irparams.timer = 0;

				} else {
					// gap just ended, record duration and start recording transmission
					irparams.overflow                  = false;
					irparams.rawlen                    = 0;
					irparams.rawbuf[irparams.rawlen++] = irparams.timer;
					irparams.timer                     = 0;
					irparams.rcvstate                  = STATE_MARK;
				}
			}
			break;

		case STATE_MARK: // timing MARK
			if (irdata == SPACE) {   // MARK ended, record time
				irparams.rawbuf[irparams.rawlen++] = irparams.timer;
				irparams.timer                     = 0;
				irparams.rcvstate                  = STATE_SPACE;
			}
			break;

		case STATE_SPACE: // timing SPACE
			if (irdata == MARK) { // SPACE just ended, record it
				irparams.rawbuf[irparams.rawlen++] = irparams.timer;
				irparams.timer                     = 0;
				irparams.rcvstate                  = STATE_MARK;

			} else if (irparams.timer > GAP_TICKS) { // SPACE
					// big SPACE, indicates gap between codes
					// Mark current code as ready for processing
					// Switch to STOP
					// Don't reset timer; keep counting space width
					irparams.rcvstate = STATE_STOP;
			}
			break;

		case STATE_STOP: // waiting, measuring gap
		 	if (irdata == MARK)  irparams.timer = 0 ;  // reset gap timer
		 	break;

		case STATE_OVERFLOW:  // Flag up a read overflow
			irparams.overflow = true;
			irparams.rcvstate = STATE_STOP;
		 	break;
	}

	if (irparams.blinkflag) {
		if (irdata == MARK)  BLINKLED_ON() ;   // turn pin 13 LED on
		else                 BLINKLED_OFF() ;  // turn pin 13 LED off
	}
}

