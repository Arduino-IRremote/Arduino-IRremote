#include "IRremote.h"
#include "IRremoteInt.h"

//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
//
int  IRrecv::decode (decode_results *results)
{
	results->rawbuf   = irparams.rawbuf;
	results->rawlen   = irparams.rawlen;

	results->overflow = irparams.overflow;

	if (irparams.rcvstate != STATE_STOP)  return false ;

#ifdef DECODE_NEC
	DBG_PRINTLN("Attempting NEC decode");
	if (decodeNEC(results))  return true ;
#endif

#ifdef DECODE_SONY
	DBG_PRINTLN("Attempting Sony decode");
	if (decodeSony(results))  return true ;
#endif

#ifdef DECODE_SANYO
	DBG_PRINTLN("Attempting Sanyo decode");
	if (decodeSanyo(results))  return true ;
#endif

#ifdef DECODE_MITSUBISHI
	DBG_PRINTLN("Attempting Mitsubishi decode");
	if (decodeMitsubishi(results))  return true ;
#endif

#ifdef DECODE_RC5
	DBG_PRINTLN("Attempting RC5 decode");
	if (decodeRC5(results))  return true ;
#endif

#ifdef DECODE_RC6
	DBG_PRINTLN("Attempting RC6 decode");
	if (decodeRC6(results))  return true ;
#endif

#ifdef DECODE_PANASONIC
	DBG_PRINTLN("Attempting Panasonic decode");
	if (decodePanasonic(results))  return true ;
#endif

#ifdef DECODE_LG
	DBG_PRINTLN("Attempting LG decode");
	if (decodeLG(results))  return true ;
#endif

#ifdef DECODE_JVC
	DBG_PRINTLN("Attempting JVC decode");
	if (decodeJVC(results))  return true ;
#endif

#ifdef DECODE_SAMSUNG
	DBG_PRINTLN("Attempting SAMSUNG decode");
	if (decodeSAMSUNG(results))  return true ;
#endif

#ifdef DECODE_WHYNTER
	DBG_PRINTLN("Attempting Whynter decode");
	if (decodeWhynter(results))  return true ;
#endif

#ifdef DECODE_AIWA_RC_T501
	DBG_PRINTLN("Attempting Aiwa RC-T501 decode");
	if (decodeAiwaRCT501(results))  return true ;
#endif

#ifdef DECODE_DENON
	DBG_PRINTLN("Attempting Denon decode");
	if (decodeDenon(results))  return true ;
#endif

	// decodeHash returns a hash on any input.
	// Thus, it needs to be last in the list.
	// If you add any decodes, add them before this.
	if (decodeHash(results))  return true ;

	// Throw away and start over
	resume();
	return false;
}

//+=============================================================================
IRrecv::IRrecv (int recvpin)
{
	irparams.recvpin = recvpin;
	irparams.blinkflag = 0;
}

//+=============================================================================
// initialization
//
void  IRrecv::enableIRIn ( )
{
	cli();
	// Setup pulse clock timer interrupt
	// Prescale /8 (16M/8 = 0.5 microseconds per tick)
	// Therefore, the timer interval can range from 0.5 to 128 microseconds
	// Depending on the reset value (255 to 0)
	TIMER_CONFIG_NORMAL();

	// Timer2 Overflow Interrupt Enable
	TIMER_ENABLE_INTR;

	TIMER_RESET;

	sei();  // enable interrupts

	// Initialize state machine variables
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;

	// Set pin modes
	pinMode(irparams.recvpin, INPUT);
}

//+=============================================================================
// Enable/disable blinking of pin 13 on IR processing
//
void  IRrecv::blink13 (int blinkflag)
{
	irparams.blinkflag = blinkflag;
	if (blinkflag)  pinMode(BLINKLED, OUTPUT) ;
}

//+=============================================================================
// Restart the ISR state machine
//
void  IRrecv::resume ( )
{
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;
}

//+=============================================================================
// hashdecode - decode an arbitrary IR code.
// Instead of decoding using a standard encoding scheme
// (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
//
// The algorithm: look at the sequence of MARK signals, and see if each one
// is shorter (0), the same length (1), or longer (2) than the previous.
// Do the same with the SPACE signals.  Hash the resulting sequence of 0's,
// 1's, and 2's to a 32-bit value.  This will give a unique value for each
// different code (probably), for most code systems.
//
// http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
//
// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
//
int  IRrecv::compare (unsigned int oldval,  unsigned int newval)
{
	if      (newval < oldval * .8)  return 0 ;
	else if (oldval < newval * .8)  return 2 ;
	else                            return 1 ;
}

//+=============================================================================
// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
// Converts the raw code values into a 32-bit hash code.
// Hopefully this code is unique for each button.
// This isn't a "real" decoding, just an arbitrary value.
//
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

long  IRrecv::decodeHash (decode_results *results)
{
	long  hash = FNV_BASIS_32;

	// Require at least 6 samples to prevent triggering on noise
	if (results->rawlen < 6)  return false ;

	for (int i = 1;  (i + 2) < results->rawlen;  i++) {
		int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
		// Add value into the hash
		hash = (hash * FNV_PRIME_32) ^ value;
	}

	results->value       = hash;
	results->bits        = 32;
	results->decode_type = UNKNOWN;

	return true;
}
