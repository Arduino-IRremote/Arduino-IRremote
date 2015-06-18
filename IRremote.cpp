//******************************************************************************
// IRremote
// Version 0.11 August, 2009
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
//
// Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
// Modified  by Mitra Ardron <mitra@mitra.biz>
// Added Sanyo and Mitsubishi controllers
// Modified Sony to spot the repeat codes that some Sony's send
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// LG added by Darryl Smith (based on the JVC protocol)
// Whynter A/C ARC-110WD added by Francesco Meschia
//******************************************************************************

#include "IRremote.h"
#include "IRremoteInt.h"

// Provides ISR
#include <avr/interrupt.h>

//------------------------------------------------------------------------------
// Debug directives
#ifdef DEBUG
#	define DBG_PRINTLN(s)  Serial.println(s);
#else
#	define DBG_PRINTLN(s)
#endif

//------------------------------------------------------------------------------
volatile irparams_t irparams;

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h
// Normally macros are used for efficiency
#ifdef DEBUG
int  MATCH (int measured,  int desired)
{
  Serial.print("Testing: ");
  Serial.print(TICKS_LOW(desired), DEC);
  Serial.print(" <= ");
  Serial.print(measured, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired), DEC);
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int  MATCH_MARK (int measured_ticks,  int desired_us)
{
  Serial.print("Testing mark ");
  Serial.print(measured_ticks * USECPERTICK, DEC);
  Serial.print(" vs ");
  Serial.print(desired_us, DEC);
  Serial.print(": ");
  Serial.print(TICKS_LOW(desired_us + MARK_EXCESS), DEC);
  Serial.print(" <= ");
  Serial.print(measured_ticks, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired_us + MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us + MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us + MARK_EXCESS);
}

int  MATCH_SPACE (int measured_ticks,  int desired_us)
{
  Serial.print("Testing space ");
  Serial.print(measured_ticks * USECPERTICK, DEC);
  Serial.print(" vs ");
  Serial.print(desired_us, DEC);
  Serial.print(": ");
  Serial.print(TICKS_LOW(desired_us - MARK_EXCESS), DEC);
  Serial.print(" <= ");
  Serial.print(measured_ticks, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired_us - MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us - MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us - MARK_EXCESS);
}

#else

int  MATCH (int measured,  int desired)
{
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int  MATCH_MARK (int measured_ticks,  int desired_us)
{
  return MATCH(measured_ticks, (desired_us + MARK_EXCESS));
}

int  MATCH_SPACE (int measured_ticks,  int desired_us)
{
  return MATCH(measured_ticks, (desired_us - MARK_EXCESS));
}

#endif

//+=============================================================================
// RRRR    AAA   W   W
// R   R  A   A  W   W
// RRRR   AAAAA  W W W
// R  R   A   A  W W W
// R   R  A   A   WWW
//
void  IRsend::sendRaw (unsigned int buf[],  int len,  int hz)
{
  // Set IR carrier frequency
  enableIROut(hz);

  for (int i = 0;  i < len;  i++) {
    if (i & 1)  space(buf[i]) ;
    else        mark (buf[i]) ;
  }

  space(0); // Just to be sure
}

//+=============================================================================
// N   N  EEEEE   CCCC
// NN  N  E      C
// N N N  EEE    C
// N  NN  E      C
// N   N  EEEEE   CCCC
//
#ifdef SEND_NEC
void  IRsend::sendNEC (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark(NEC_HDR_MARK);
	space(NEC_HDR_SPACE);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(NEC_BIT_MARK);
			space(NEC_ONE_SPACE);
		} else {
			mark(NEC_BIT_MARK);
			space(NEC_ZERO_SPACE);
		}
	}

	// Footer
	mark(NEC_BIT_MARK);
	space(0);  // Alwasy end with the LED off
}
#endif

//+=============================================================================
// NECs have a repeat only 4 items long
//
#ifdef DECODE_NEC
long  IRrecv::decodeNEC (decode_results *results)
{
	long  data   = 0;  // We decode in to here; Start with nothing
	int   offset = 1;  // Index in to results; Skip first entry!?

	// Check header "mark"
	if (!MATCH_MARK(results->rawbuf[offset], NEC_HDR_MARK))  return false ;
	offset++;

	// Check for repeat
	if ( (irparams.rawlen == 4)
	    && MATCH_SPACE(results->rawbuf[offset  ], NEC_RPT_SPACE)
	    && MATCH_MARK (results->rawbuf[offset+1], NEC_BIT_MARK )
	   ) {
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = NEC;
		return true;
	}

	// Check we have enough data
	if (irparams.rawlen < (2 * NEC_BITS) + 4)  return false ;

	// Check header "space"
	if (!MATCH_SPACE(results->rawbuf[offset], NEC_HDR_SPACE))  return false ;
	offset++;

	// Build the data
	for (int i = 0;  i < NEC_BITS;  i++) {
		// Check data "mark"
		if (!MATCH_MARK(results->rawbuf[offset], NEC_BIT_MARK))  return false ;
		offset++;
        // Suppend this bit
		if      (MATCH_SPACE(results->rawbuf[offset], NEC_ONE_SPACE ))  data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], NEC_ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                            return false ;
		offset++;
	}

	// Success
	results->bits        = NEC_BITS;
	results->value       = data;
	results->decode_type = NEC;

	return true;
}
#endif

//+=============================================================================
// W   W  H   H  Y   Y N   N TTTTT EEEEE  RRRRR
// W   W  H   H   Y Y  NN  N   T   E      R   R
// W W W  HHHHH    Y   N N N   T   EEE    RRRR
// W W W  H   H    Y   N  NN   T   E      R  R
//  WWW   H   H    Y   N   N   T   EEEEE  R   R
//
#ifdef SEND_WHYNTER
void  IRsend::sendWhynter (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Start
	mark(WHYNTER_ZERO_MARK);
	space(WHYNTER_ZERO_SPACE);

	// Header
	mark(WHYNTER_HDR_MARK);
	space(WHYNTER_HDR_SPACE);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(WHYNTER_ONE_MARK);
			space(WHYNTER_ONE_SPACE);
		} else {
			mark(WHYNTER_ZERO_MARK);
			space(WHYNTER_ZERO_SPACE);
		}
	}

	// Footer
	mark(WHYNTER_ZERO_MARK);
	space(WHYNTER_ZERO_SPACE);  // Always end with the LED off
}
#endif

//+=============================================================================
//  SSSS   OOO   N   N  Y   Y
// S      O   O  NN  N   Y Y
//  SSS   O   O  N N N    Y
//     S  O   O  N  NN    Y
// SSSS    OOO   N   N    Y
//
#ifdef SEND_SONY
void  IRsend::sendSony (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(40);

	// Header
	mark(SONY_HDR_MARK);
	space(SONY_HDR_SPACE);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(SONY_ONE_MARK);
			space(SONY_HDR_SPACE);
		} else {
			mark(SONY_ZERO_MARK);
			space(SONY_HDR_SPACE);
    	}
  	}

	// We will have ended with LED off
}
#endif

//+=============================================================================
// RRRR    CCCC  55555
// R   R  C      5
// R   R  C      5555
// RRRR   C          5
// R  R   C      5   5
// R   R   CCCC   555
//
// Note: first bit must be a one (start bit)
//
#ifdef SEND_RC5
void  IRsend::sendRC5 (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(36);

	// Start
	mark(RC5_T1);
	space(RC5_T1);
	mark(RC5_T1);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			space(RC5_T1); // 1 is space, then mark
			mark(RC5_T1);
		} else {
			mark(RC5_T1);
			space(RC5_T1);
		}
	}

	space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// RRRR    CCCC    666
// R   R  C       6
// R   R  C      6 66
// RRRR   C      66  6
// R  R   C      6   6
// R   R   CCCC   666
//
// NB : Caller needs to take care of flipping the toggle bit
//
#ifdef SEND_RC6
void  IRsend::sendRC6 (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(36);

	// Header
	mark(RC6_HDR_MARK);
	space(RC6_HDR_SPACE);

	// Start bit
	mark(RC6_T1);
	space(RC6_T1);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		int  t = (i == 3) ? (RC6_T1 * 2) : (RC6_T1) ;
		if (data & mask) {
			mark(t);
			space(t);
		} else {
			space(t);
			mark(t);
		}
	}

	space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
// P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
// PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
// P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
// P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//
#ifdef SEND_PANASONIC
void  IRsend::sendPanasonic (unsigned int address,  unsigned long data)
{
	// Set IR carrier frequency
	enableIROut(35);

	// Header
	mark(PANASONIC_HDR_MARK);
	space(PANASONIC_HDR_SPACE);

	// Address
	for (unsigned long  mask = 1 << (16 - 1);  mask;  mask >>= 1) {
		mark(PANASONIC_BIT_MARK);
		if (address & mask)  space(PANASONIC_ONE_SPACE) ;
		else                 space(PANASONIC_ZERO_SPACE) ;
    }

	// Data
	for (unsigned long  mask = 1 << (32 - 1);  mask;  mask >>= 1) {
        mark(PANASONIC_BIT_MARK);
        if (data & mask)  space(PANASONIC_ONE_SPACE) ;
        else              space(PANASONIC_ZERO_SPACE) ;
    }

	// Footer
    mark(PANASONIC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// JJJJJ  V   V   CCCC
//   J    V   V  C
//   J     V V   C
// J J     V V   C
//  J       V     CCCC
//
#ifdef SEND_JVC
void  IRsend::sendJVC (unsigned long data,  int nbits, int repeat)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Only send the Header if this is NOT a repeat command
	if (!repeat){
		mark(JVC_HDR_MARK);
		space(JVC_HDR_SPACE);
	}

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(JVC_BIT_MARK);
			space(JVC_ONE_SPACE);
		} else {
			mark(JVC_BIT_MARK);
			space(JVC_ZERO_SPACE);
		}
	}

	// Footer
    mark(JVC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//  SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
// S      A   A  M M M  S      U   U  NN  N  G
//  SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//     S  A   A  M   M      S  U   U  N  NN  G   G
// SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//
#ifdef SEND_SAMSUNG
void  IRsend::sendSAMSUNG (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark(SAMSUNG_HDR_MARK);
	space(SAMSUNG_HDR_SPACE);

	// Data
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ONE_SPACE);
		} else {
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ZERO_SPACE);
		}
	}

	// Footer
	mark(SAMSUNG_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// Sends an IR mark for the specified number of microseconds.
// The mark output is modulated at the PWM frequency.
//
void  IRsend::mark (int time)
{
  TIMER_ENABLE_PWM; // Enable pin 3 PWM output
  if (time > 0) delayMicroseconds(time);
}

//+=============================================================================
// Leave pin off for time (given in microseconds)
// Sends an IR space for the specified number of microseconds.
// A space is no output, so the PWM output is disabled.
//
void  IRsend::space (int time)
{
  TIMER_DISABLE_PWM; // Disable pin 3 PWM output
  if (time > 0) delayMicroseconds(time);
}

//+=============================================================================
// Enables IR output.  The khz value controls the modulation frequency in kilohertz.
// The IR output will be on pin 3 (OC2B).
// This routine is designed for 36-40KHz; if you use it for other values, it's up to you
// to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
// TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
// controlling the duty cycle.
// There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
// To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
// A few hours staring at the ATmega documentation and this will all make sense.
// See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
//
void  IRsend::enableIROut (int khz)
{
  // Disable the Timer2 Interrupt (which is used for receiving IR)
  TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt

  pinMode(TIMER_PWM_PIN, OUTPUT);
  digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low

  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  TIMER_CONFIG_KHZ(khz);
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
  // setup pulse clock timer interrupt
  //Prescale /8 (16M/8 = 0.5 microseconds per tick)
  // Therefore, the timer interval can range from 0.5 to 128 microseconds
  // depending on the reset value (255 to 0)
  TIMER_CONFIG_NORMAL();

  //Timer2 Overflow Interrupt Enable
  TIMER_ENABLE_INTR;

  TIMER_RESET;

  sei();  // enable interrupts

  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;

  // set pin modes
  pinMode(irparams.recvpin, INPUT);
}

//+=============================================================================
// enable/disable blinking of pin 13 on IR processing
//
void  IRrecv::blink13 (int blinkflag)
{
  irparams.blinkflag = blinkflag;
  if (blinkflag)  pinMode(BLINKLED, OUTPUT) ;
}

//+=============================================================================
// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
//
ISR (TIMER_INTR_NAME)
{
  TIMER_RESET;

  uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);

  irparams.timer++; // One more 50us tick
  if (irparams.rawlen >= RAWBUF)  irparams.rcvstate = STATE_STOP ;  // Buffer overflow

  switch(irparams.rcvstate) {
    case STATE_IDLE: // In the middle of a gap
      if (irdata == MARK) {
        if (irparams.timer < GAP_TICKS) {
          // Not big enough to be a gap.
          irparams.timer = 0;
        }
        else {
          // gap just ended, record duration and start recording transmission
          irparams.rawlen = 0;
          irparams.rawbuf[irparams.rawlen++] = irparams.timer;
          irparams.timer = 0;
          irparams.rcvstate = STATE_MARK;
        }
      }
      break;

    case STATE_MARK: // timing MARK
      if (irdata == SPACE) {   // MARK ended, record time
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_SPACE;
      }
      break;

    case STATE_SPACE: // timing SPACE
      if (irdata == MARK) { // SPACE just ended, record it
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      }
      else { // SPACE
        if (irparams.timer > GAP_TICKS) {
          // big SPACE, indicates gap between codes
          // Mark current code as ready for processing
          // Switch to STOP
          // Don't reset timer; keep counting space width
          irparams.rcvstate = STATE_STOP;
        }
      }
      break;

    case STATE_STOP: // waiting, measuring gap
      if (irdata == MARK)  irparams.timer = 0 ;  // reset gap timer
      break;
  }

  if (irparams.blinkflag) {
    if (irdata == MARK)  BLINKLED_ON() ;   // turn pin 13 LED on
    else                 BLINKLED_OFF() ;  // turn pin 13 LED off
    }
  }
}

//+=============================================================================
void  IRrecv::resume ( )
{
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
}

//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int  IRrecv::decode (decode_results *results)
{
  results->rawbuf = irparams.rawbuf;
  results->rawlen = irparams.rawlen;

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

#ifdef AIWA_RC_T501
  DBG_PRINTLN("Attempting Aiwa RC-T501 decode");
  if (decodeAiwaRCT501(results))  return true ;
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
#ifdef DECODE_SONY
long  IRrecv::decodeSony (decode_results *results)
{
  long data = 0;
  if (irparams.rawlen < 2 * SONY_BITS + 2)  return false ;
  int offset = 0; // Dont skip first space, check its size

  // Some Sony's deliver repeats fast after first
  // unfortunately can't spot difference from of repeat from two fast clicks
  if (results->rawbuf[offset] < SONY_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;

#   ifdef DECODE_SANYO
      results->decode_type = SANYO;
#   else
      results->decode_type = UNKNOWN;
#   endif

    return true;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SONY_HDR_MARK))  return false ;
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset], SONY_HDR_SPACE))  break ;
    offset++;
    if      (MATCH_MARK(results->rawbuf[offset], SONY_ONE_MARK))   data = (data << 1) | 1 ;
    else if (MATCH_MARK(results->rawbuf[offset], SONY_ZERO_MARK))  data <<= 1 ;
    else                                                           return false ;
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return false;
  }
  results->value       = data;
  results->decode_type = SONY;
  return true;
}
#endif

//+=============================================================================
#ifdef DECODE_WHYNTER
long  IRrecv::decodeWhynter (decode_results *results)
{
  long data = 0;

  if (irparams.rawlen < 2 * WHYNTER_BITS + 6)  return false ;

  int offset = 1; // skip initial space

  // sequence begins with a bit mark and a zero space
  if (!MATCH_MARK(results->rawbuf[offset], WHYNTER_BIT_MARK))  return false ;
  offset++;
  if (!MATCH_SPACE(results->rawbuf[offset], WHYNTER_ZERO_SPACE))  return false ;
  offset++;

  // header mark and space
  if (!MATCH_MARK(results->rawbuf[offset], WHYNTER_HDR_MARK))  return false ;
  offset++;
  if (!MATCH_SPACE(results->rawbuf[offset], WHYNTER_HDR_SPACE))  return false ;
  offset++;

  // data bits
  for (int i = 0;  i < WHYNTER_BITS;  i++) {
    if (!MATCH_MARK(results->rawbuf[offset], WHYNTER_BIT_MARK))  return false ;
    offset++;

    if      (MATCH_SPACE(results->rawbuf[offset], WHYNTER_ONE_SPACE))  data = (data << 1) | 1 ;
    else if (MATCH_SPACE(results->rawbuf[offset],WHYNTER_ZERO_SPACE))  data <<= 1 ;
    else                                                               return false ;
    offset++;
  }

  // trailing mark
  if (!MATCH_MARK(results->rawbuf[offset], WHYNTER_BIT_MARK))  return false ;

  // Success
  results->bits = WHYNTER_BITS;
  results->value = data;
  results->decode_type = WHYNTER;
  return true;
}
#endif

//+=============================================================================
// I think this is a Sanyo decoder - serial = SA 8650B
// Looks like Sony except for timings, 48 chars of data and time/space different
//
#ifdef DECODE_SANYO
long  IRrecv::decodeSanyo (decode_results *results)
{
  long data = 0;
  if (irparams.rawlen < 2 * SANYO_BITS + 2)  return false ;
  int offset = 0; // Skip first space
  // Initial space

#if 0
  // Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
#endif

  if (results->rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return true;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SANYO_HDR_MARK))  return false ;
  offset++;

  // Skip Second Mark
  if (!MATCH_MARK(results->rawbuf[offset], SANYO_HDR_MARK))  return false ;
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset], SANYO_HDR_SPACE))  break ;
    offset++;
    if      (MATCH_MARK(results->rawbuf[offset], SANYO_ONE_MARK))   data = (data << 1) | 1 ;
    else if (MATCH_MARK(results->rawbuf[offset], SANYO_ZERO_MARK))  data <<= 1 ;
    else                                                            return false ;
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return false;
  }

  results->value       = data;
  results->decode_type = SANYO;
  return true;
}
#endif

//+=============================================================================
// Looks like Sony except for timings, 48 chars of data and time/space different
//
#ifdef DECODE_MITSUBISHI
long  IRrecv::decodeMitsubishi (decode_results *results)
{
  // Serial.print("?!? decoding Mitsubishi:");Serial.print(irparams.rawlen); Serial.print(" want "); Serial.println( 2 * MITSUBISHI_BITS + 2);
  long data = 0;
  if (irparams.rawlen < 2 * MITSUBISHI_BITS + 2)  return false ;
  int offset = 0; // Skip first space
  // Initial space

#if 0
  // Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
#endif

#if 0
  // Not seeing double keys from Mitsubishi
  if (results->rawbuf[offset] < MITSUBISHI_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = MITSUBISHI;
    return true;
  }
#endif

  offset++;

  // Typical
  // 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7

  // Initial Space
  if (!MATCH_MARK(results->rawbuf[offset], MITSUBISHI_HDR_SPACE))  return false ;
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if      (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ONE_MARK))   data = (data << 1) | 1 ;
    else if (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ZERO_MARK))  data <<= 1 ;
    else                                                                 return false ;
    offset++;

    if (!MATCH_SPACE(results->rawbuf[offset], MITSUBISHI_HDR_SPACE))  break ;
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < MITSUBISHI_BITS) {
    results->bits = 0;
    return false;
  }

  results->value       = data;
  results->decode_type = MITSUBISHI;
  return true;
}
#endif

//+=============================================================================
// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
//
int  IRrecv::getRClevel (decode_results *results,  int *offset,  int *used,  int t1)
{
  if (*offset >= results->rawlen)  return SPACE ;  // After end of recorded buffer, assume SPACE.
  int width = results->rawbuf[*offset];
  int val = ((*offset) % 2) ? MARK : SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  int avail;
  if      (MATCH(width,   t1 + correction))  avail = 1 ;
  else if (MATCH(width, 2*t1 + correction))  avail = 2 ;
  else if (MATCH(width, 3*t1 + correction))  avail = 3 ;
  else                                       return -1 ;

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }

  DBG_PRINTLN( (val == MARK) ? "MARK" : "SPACE" );
  return val;
}

//+=============================================================================
#ifdef DECODE_RC5
long  IRrecv::decodeRC5 (decode_results *results)
{
  if (irparams.rawlen < MIN_RC5_SAMPLES + 2)  return false ;
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  // Get start bits
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK)   return false ;
  if (getRClevel(results, &offset, &used, RC5_T1) != SPACE)  return false ;
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK)   return false ;
  int nbits;
  for (nbits = 0;  offset < irparams.rawlen;  nbits++) {
    int levelA = getRClevel(results, &offset, &used, RC5_T1);
    int levelB = getRClevel(results, &offset, &used, RC5_T1);

    if      (levelA == SPACE && levelB == MARK)  data = (data << 1) | 1 ;  // 1 bit
    else if (levelA == MARK && levelB == SPACE)  data <<= 1 ;              // zero bit
    else                                         return false ;
  }

  // Success
  results->bits        = nbits;
  results->value       = data;
  results->decode_type = RC5;
  return true;
}
#endif

//+=============================================================================
#ifdef DECODE_RC6
long  IRrecv::decodeRC6 (decode_results *results)
{
  if (results->rawlen < MIN_RC6_SAMPLES)  return false ;
  int offset = 1; // Skip first space

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], RC6_HDR_MARK))  return false ;
  offset++;

  if (!MATCH_SPACE(results->rawbuf[offset], RC6_HDR_SPACE))  return false ;
  offset++;

  long  data = 0;
  int   used = 0;

  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK)   return false ;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE)  return false ;
  int nbits;
  for (nbits = 0;  offset < results->rawlen;  nbits++) {
    int levelA, levelB; // Next two levels
    levelA = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != getRClevel(results, &offset, &used, RC6_T1)) return false;
    }
    levelB = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != getRClevel(results, &offset, &used, RC6_T1)) return false;
    }
    if      (levelA == MARK && levelB == SPACE)  data = (data << 1) | 1 ; // 1-bit (reversed compared to RC5)
    else if (levelA == SPACE && levelB == MARK)  data <<= 1 ;             // zero bit
    else                                         return false ;             // Error
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC6;
  return true;
}
#endif

//+=============================================================================
#ifdef DECODE_PANASONIC
long  IRrecv::decodePanasonic (decode_results *results)
{
    unsigned long long data = 0;
    int offset = 1;

    if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_MARK))  return false ;
    offset++;
    if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_SPACE))  return false ;
    offset++;

    // decode address
    for (int i = 0;  i < PANASONIC_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_BIT_MARK))  return false ;

        if      (MATCH_SPACE(results->rawbuf[offset],PANASONIC_ONE_SPACE))   data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset],PANASONIC_ZERO_SPACE))  data <<= 1 ;
        else                                                                 return false ;
        offset++;
    }

    results->value            = (unsigned long)data;
    results->panasonicAddress = (unsigned int)(data >> 32);
    results->decode_type      = PANASONIC;
    results->bits             = PANASONIC_BITS;

    return true;
}
#endif

//+=============================================================================
#ifdef DECODE_LG
long  IRrecv::decodeLG (decode_results *results)
{
    long  data   = 0;
    int   offset = 1; // Skip first space

    // Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], LG_HDR_MARK))  return false ;
    offset++;
    if (irparams.rawlen < 2 * LG_BITS + 1 )  return false ;
    // Initial space
    if (!MATCH_SPACE(results->rawbuf[offset], LG_HDR_SPACE))  return false ;
    offset++;
    for (int i = 0;  i < LG_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK))  return false ;
        offset++;
        if      (MATCH_SPACE(results->rawbuf[offset], LG_ONE_SPACE))   data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset], LG_ZERO_SPACE))  data <<= 1 ;
        else                                                           return false ;
        offset++;
    }

    // Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK))   return false ;

    // Success
    results->bits = LG_BITS;
    results->value = data;
    results->decode_type = LG;
    return true;
}
#endif

//+=============================================================================
#ifdef DECODE_JVC
long  IRrecv::decodeJVC (decode_results *results)
{
    long  data   = 0;
    int   offset = 1; // Skip first space

    // Check for repeat
    if (irparams.rawlen - 1 == 33 &&
        MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK) &&
        MATCH_MARK(results->rawbuf[irparams.rawlen-1], JVC_BIT_MARK)) {
        results->bits = 0;
        results->value = REPEAT;
        results->decode_type = JVC;
        return true;
    }

    // Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], JVC_HDR_MARK))  return false ;
    offset++;

    if (irparams.rawlen < 2 * JVC_BITS + 1 )  return false ;

    // Initial space
    if (!MATCH_SPACE(results->rawbuf[offset], JVC_HDR_SPACE))  return false ;
    offset++;

    for (int i = 0;  i < JVC_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK))  return false ;
        offset++;
        if      (MATCH_SPACE(results->rawbuf[offset], JVC_ONE_SPACE))   data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset], JVC_ZERO_SPACE))  data <<= 1 ;
        else                                                            return false ;
        offset++;
    }

    // Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK))  return false ;

    // Success
    results->bits        = JVC_BITS;
    results->value       = data;
    results->decode_type = JVC;

    return true;
}
#endif

//+=============================================================================
// SAMSUNGs have a repeat only 4 items long
//
#ifdef DECODE_SAMSUNG
long  IRrecv::decodeSAMSUNG (decode_results *results)
{
  long  data   = 0;
  int   offset = 1; // Skip first space

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_HDR_MARK))   return false ;
  offset++;

  // Check for repeat
  if (irparams.rawlen == 4 &&
    MATCH_SPACE(results->rawbuf[offset], SAMSUNG_RPT_SPACE) &&
    MATCH_MARK(results->rawbuf[offset+1], SAMSUNG_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SAMSUNG;
    return true;
  }
  if (irparams.rawlen < 2 * SAMSUNG_BITS + 4)  return false ;

  // Initial space
  if (!MATCH_SPACE(results->rawbuf[offset], SAMSUNG_HDR_SPACE))  return false ;
  offset++;

  for (int i = 0;  i < SAMSUNG_BITS;   i++) {
    if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_BIT_MARK))  return false ;

    offset++;

    if      (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ONE_SPACE))   data = (data << 1) | 1 ;
    else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ZERO_SPACE))  data <<= 1 ;
    else                                                                return false ;
    offset++;
  }

  // Success
  results->bits        = SAMSUNG_BITS;
  results->value       = data;
  results->decode_type = SAMSUNG;
  return true;
}
#endif

//+=============================================================================
// Aiwa system
// Remote control RC-T501
// Lirc file http://lirc.sourceforge.net/remotes/aiwa/RC-T501
//
#ifdef DECODE_AIWA_RC_T501
long  IRrecv::decodeAiwaRCT501 (decode_results *results)
{
  int  data   = 0;
  int  offset = 1; // skip first garbage read

  // Check SIZE
  if (irparams.rawlen < 2 * (AIWA_RC_T501_SUM_BITS) + 4)  return false ;

  // Check HDR
  if (!MATCH_MARK(results->rawbuf[offset], AIWA_RC_T501_HDR_MARK))  return false ;
  offset++;

  // Check HDR space
  if (!MATCH_SPACE(results->rawbuf[offset], AIWA_RC_T501_HDR_SPACE))  return false ;
  offset++;

  offset += 26; // skip pre-data - optional
  while(offset < irparams.rawlen - 4) {
    if (MATCH_MARK(results->rawbuf[offset], AIWA_RC_T501_BIT_MARK))  offset++ ;
    else                                                             return false ;

    // ONE & ZERO
    if      (MATCH_SPACE(results->rawbuf[offset], AIWA_RC_T501_ONE_SPACE))   data = (data << 1) | 1 ;
    else if (MATCH_SPACE(results->rawbuf[offset], AIWA_RC_T501_ZERO_SPACE))  data <<= 1 ;
    else                                                                     break ;  // End of one & zero detected
    offset++;
  }

  results->bits = (offset - 1) / 2;
  if (results->bits < 42)  return false ;
  results->value       = data;
  results->decode_type = AIWA_RC_T501;
  return true;
}
#endif

//+=============================================================================
// hashdecode - decode an arbitrary IR code.
// Instead of decoding using a standard encoding scheme
// (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
//
// The algorithm: look at the sequence of MARK signals, and see if each one
// is shorter (0), the same length (1), or longer (2) than the previous.
// Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
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
  // Require at least 6 samples to prevent triggering on noise
  if (results->rawlen < 6)  return false ;
  long hash = FNV_BASIS_32;
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

//+=============================================================================
// Sharp and DISH support by Todd Treece ( http://unionbridge.org/design/ircommand )
//
// The Dish send function needs to be repeated 4 times, and the Sharp function
// has the necessary repeat built in because of the need to invert the signal.
//
// Sharp protocol documentation:
// http://www.sbprojects.com/knowledge/ir/sharp.htm
//
// Here are the LIRC files that I found that seem to match the remote codes
// from the oscilloscope:
//
// Sharp LCD TV:
// http://lirc.sourceforge.net/remotes/sharp/GA538WJSA
//
// DISH NETWORK (echostar 301):
// http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx
//
// For the DISH codes, only send the last for characters of the hex.
// i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
// linked LIRC file.
//
#ifdef SEND_SHARP
void  IRsend::sendSharp (unsigned long data,  int nbits)
{
  unsigned long  invertdata = data ^ SHARP_TOGGLE_MASK;
  enableIROut(38);

  // Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
  // much more reliable. That's the exact behaviour of CD-S6470 remote control.
  for (int n = 0;  n < 3;  n++) {
	for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
      if (data & mask) {
        mark(SHARP_BIT_MARK);
        space(SHARP_ONE_SPACE);
      } else {
        mark(SHARP_BIT_MARK);
        space(SHARP_ZERO_SPACE);
      }
    }

    mark(SHARP_BIT_MARK);
    space(SHARP_ZERO_SPACE);
    delay(40);

    data = data ^ SHARP_TOGGLE_MASK;
  }
}

//+=============================================================================
// Sharp send compatible with data obtained through decodeSharp
//
void  IRsend::sendSharp (unsigned int address,  unsigned int command)
{
  sendSharpRaw((address << 10) | (command << 2) | 2, 15);
}

#endif

//+=============================================================================
#ifdef SEND_DISH
void  IRsend::sendDISH (unsigned long data,  int nbits)
{
  // Set IR carrier frequency
  enableIROut(56);

  mark(DISH_HDR_MARK);
  space(DISH_HDR_SPACE);

  for (unsigned long  mask = 1 << (nbits - 1);  mask;  mask >>= 1) {
    if (data & mark) {
      mark(DISH_BIT_MARK);
      space(DISH_ONE_SPACE);
    } else {
      mark(DISH_BIT_MARK);
      space(DISH_ZERO_SPACE);
    }
  }
}
#endif

//+=============================================================================
// Aiwa system
// Remote control RC-T501
// Lirc file http://lirc.sourceforge.net/remotes/aiwa/RC-T501
//
#ifdef SEND_AIWA_RC_T501
void  IRsend::sendAiwaRCT501 (int code)
{
  // PRE-DATA, 26 bits, 0x227EEC0
  unsigned long  pre = 0x227EEC0;
  int            mask;

  // Set IR carrier frequency
  enableIROut(AIWA_RC_T501_HZ);

  // HDR mark + HDR space
  mark(AIWA_RC_T501_HDR_MARK);
  space(AIWA_RC_T501_HDR_SPACE);

  // Send pre-data
  for (unsigned long  mask = 1 << (26 - 1);  mask;  mask >>= 1) {
    mark(AIWA_RC_T501_BIT_MARK);
    if (pre & mask)  space(AIWA_RC_T501_ONE_SPACE) ;
    else             space(AIWA_RC_T501_ZERO_SPACE) ;
  }

//-v- THIS CODE LOOKS LIKE IT MIGHT BE WRONG - CHECK!
//    it only send 15bits and ignores the top bit
//    then uses TOPBIT which is bit-31 to check the bit code
//    I suspect TOPBIT should be changed to 0x00008000
  // Skip firts code bit
  code <<= 1;
  // Send code
  for (i = 0;  i < 15;  i++) {
    mark(AIWA_RC_T501_BIT_MARK);
    if (code & TOPBIT)  space(AIWA_RC_T501_ONE_SPACE) ;
    else                space(AIWA_RC_T501_ZERO_SPACE) ;
    code <<= 1;
  }
//-^- THIS CODE LOOKS LIKE IT MIGHT BE WRONG - CHECK!

  // POST-DATA, 1 bit, 0x0
  mark(AIWA_RC_T501_BIT_MARK);
  space(AIWA_RC_T501_ZERO_SPACE);

  mark(AIWA_RC_T501_BIT_MARK);
  space(0);
}
#endif
