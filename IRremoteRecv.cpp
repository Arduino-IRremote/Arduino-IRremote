/*
 * IRremote - receiving code
 * Copyright 2009-2010 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#include "IRremote.h"
#include "IRremoteInt.h"

#define RC5_T1		889
#define RC6_HDR_MARK	2666
#define RC6_HDR_SPACE	889
#define RC6_T1		444

volatile irparams_t *irparamsList = NULL;

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h
// Normally macros are used for efficiency
#ifdef DEBUG
int MATCH(int measured, int desired) {
  Serial.print("Testing: ");
  Serial.print(TICKS_LOW(desired), DEC);
  Serial.print(" <= ");
  Serial.print(measured, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired), DEC);
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int MATCH_MARK(int measured_ticks, int desired_us) {
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

int MATCH_SPACE(int measured_ticks, int desired_us) {
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
#endif

static void interrupt_handler(); // forward definition

IRrecv::IRrecv(int recvpin)
{
  // Add this irparams to the list
  irparams.next = irparamsList;
  irparamsList = &irparams;
  irparams.recvpin = recvpin;
  IRremoteRegisterHandler(&interrupt_handler);
}

// initialization
void IRrecv::enableIRIn() {
  IRremoteEnableIRinput();

  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;

  // set pin modes
  pinMode(irparams.recvpin, INPUT);
}

// interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
static void interrupt_handler() {
  for (volatile irparams_t *irparams = irparamsList; irparams; irparams = irparams->next) {
    uint8_t irdata = (uint8_t)digitalRead(irparams->recvpin);

    irparams->timer++; // One more 50us tick
    if (irparams->rawlen >= RAWBUF) {
      // Buffer overflow
      irparams->rcvstate = STATE_STOP;
    }
    switch(irparams->rcvstate) {
    case STATE_IDLE: // In the middle of a gap
      if (irdata == MARK) {
	if (irparams->timer < GAP_TICKS) {
	  // Not big enough to be a gap.
	  irparams->timer = 0;
	} 
	else {
	  // gap just ended, record duration and start recording transmission
	  irparams->rawlen = 0;
	  irparams->rawbuf[irparams->rawlen++] = irparams->timer;
	  irparams->timer = 0;
	  irparams->rcvstate = STATE_MARK;
	}
      }
      break;
    case STATE_MARK: // timing MARK
      if (irdata == SPACE) {   // MARK ended, record time
	irparams->rawbuf[irparams->rawlen++] = irparams->timer;
	irparams->timer = 0;
	irparams->rcvstate = STATE_SPACE;
      }
      break;
    case STATE_SPACE: // timing SPACE
      if (irdata == MARK) { // SPACE just ended, record it
	irparams->rawbuf[irparams->rawlen++] = irparams->timer;
	irparams->timer = 0;
	irparams->rcvstate = STATE_MARK;
      } 
      else { // SPACE
	if (irparams->timer > GAP_TICKS) {
	  // big SPACE, indicates gap between codes
	  // Mark current code as ready for processing
	  // Switch to STOP
	  // Don't reset timer; keep counting space width
	  irparams->rcvstate = STATE_STOP;
	} 
      }
      break;
    case STATE_STOP: // waiting, measuring gap
      if (irdata == MARK) { // reset gap timer
	irparams->timer = 0;
      }
      break;
    }
  }
}

void IRrecv::resume() {
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
}

void IRrecv::pause() {
  irparams.rcvstate = STATE_STOP;
  irparams.rawlen = 0;
}

// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv::decode(decode_results *results) {
  results->rawbuf = irparams.rawbuf;
  results->rawlen = irparams.rawlen;
  if (irparams.rcvstate != STATE_STOP) {
    return ERR;
  }
#ifdef DEBUG
  Serial.println("Attempting SPACE_ENC decode");
#endif
  if (decodeSpaceEnc(results)) {
    // Don't resume until decoding is done because we don't
    // want the data to change in the middle of decoding.
    resume();
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting NEC decode");
#endif
  if (decodeNecRepeat(results)) {
    resume();
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting RC5 decode");
#endif  
  if (decodeRC5(results)) {
    resume();
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting RC6 decode");
#endif 
  if (decodeRC6(results)) {
    resume();
    return DECODED;
  }
  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  if (decodeHash(results)) {
    resume();
    return DECODED;
  }
  // Throw away and start over
  resume();
  return ERR;
}

// Decoding a generic space encoded signal is a bit tricky.
// We assume one of two cases:
// a) a 0 is a mark and a short space, and a 1 is a mark and a long space (spaceVaries), or
// b) a 0 is a short mark and a space, and a 1 is a long mark and a space (markVaries)
// The NEC code is an example of varying space, and the Sony code is an example of varying mark.
//
// We assume that if the space varies, there is a trailing mark (so you can tell when the last space ends),
// but if the mark varies then the last mark is part of the last bit, but the last space is very long.
//
// To decode, the first step is to find the shortest and longest marks and spaces (excluding headers and trailers). 
// Then see if the marks or spaces vary.  (If both, probably RC5/6 so quit.)
// Loop through all the mark/space pairs to see if it is a 1 or a 0.
// Finally, fill in the results.
//
// The code is somewhat long and confusing because it is generic and handles both cases.
//
long IRrecv::decodeSpaceEnc(decode_results *results) {
  if (irparams.rawlen < 10) {
    // Don't have a reasonable number of bits to decode.
    return ERR;
  }
  unsigned int minMark = 999999;
  unsigned int maxMark = 0;
  unsigned int minSpace = 999999;
  unsigned int maxSpace = 0;
  // Compute the minimum and maximum mark and space durations, ignoring
  // header and trailer.
  // start with entry 3, skipping first space and two header elements
  // skip the last space and mark in case they are a trailer
  for (int i = 3; i < irparams.rawlen-2; i += 2) {
    if (results->rawbuf[i] < minMark) {
      minMark = results->rawbuf[i];
    } else if (results->rawbuf[i] > maxMark) {
      maxMark = results->rawbuf[i];
    }
    if (results->rawbuf[i+1] < minSpace) {
      minSpace = results->rawbuf[i+1];
    } else if (results->rawbuf[i+1] > maxSpace) {
      maxSpace = results->rawbuf[i+1];
    }
  }
  maxMark *= USECPERTICK;
  maxSpace *= USECPERTICK;
  // The second argument is us, the first is ticks, so need to multiply the second
  // markVaries is true if there are two different mark values
  // spaceVaries is true if there are two different space values
  int markVaries = !MATCH(minMark, maxMark);
  int spaceVaries = !MATCH(minSpace, maxSpace);
  minMark *= USECPERTICK;
  minSpace *= USECPERTICK;
#ifdef DEBUG
  Serial.print("min mark: ");
  Serial.println(minMark, DEC);
  Serial.print("max mark: ");
  Serial.println(maxMark, DEC);
  Serial.print("min space: ");
  Serial.println(minSpace, DEC);
  Serial.print("max space: ");
  Serial.println(maxSpace, DEC);
#endif
  // Only one of these can vary for SPACE_ENC
  if (markVaries == spaceVaries) {
    return ERR;
  }
  // Subtract 4 entries: space, 2 for header, 1 for trailer
  int nbits = (irparams.rawlen-4) / 2; 
  // Clean up the non-varying value by averaging the min and max
  // They will probably be slightly different due to random fluctuations
  // so the average is probably best to use.
  if (markVaries) {
    minSpace = (minSpace + maxSpace) / 2;
    maxSpace = minSpace;
    nbits += 1; // Last mark is a bit, not a trailer 
    results->spaceEncData.trailer = 0;
  } else {
    minMark = (minMark + maxMark) / 2;
    maxMark = minMark;
    // If space varies, need a trailer to delimit the last space
    results->spaceEncData.trailer = results->rawbuf[irparams.rawlen-1] * USECPERTICK;
  }
#ifdef DEBUG
  Serial.print("nbits: ");
  Serial.println(nbits);
  Serial.print("markVaries: ");
  Serial.println(markVaries);
  Serial.print("spaceVaries: ");
  Serial.println(spaceVaries);
  Serial.print("rawlen: ");
  Serial.println(irparams.rawlen);
#endif

  // Now loop through the data and determine the bit values.

  unsigned long long data = 0;
  int offset = 3; // Offset into rawbuf; skip the header
  if (markVaries) {
    // The decode loop where the mark width determines the bit value
    for (int i=0; i < nbits-1; i++) {
      data <<= 1;
      // Check the mark and determine the bit
      unsigned int markVal = results->rawbuf[offset++];
      if (MATCH(markVal, minMark)) {
        // 0 bit
      } else if (MATCH(markVal, maxMark)) {
        data |= 1; // 1 bit
      } else {
        // The mark is no good.
        return ERR;
      }
      // Check that the space is okay
      unsigned int spaceVal = results->rawbuf[offset++];
      if (!MATCH(spaceVal, minSpace)) {
        // The space is no good
  	return ERR;
      }
    }
    // Process the last bit specially because it's just a mark without a space
    // (because the transmission has to end with a mark).
    // If it makes sense as a bit, treat it as a bit, otherwise treat it as a
    // trailer.
    unsigned int markVal = results->rawbuf[offset++];
    if (MATCH(markVal, minMark)) {
      // 0 bit
      data <<= 1;
    } else if (MATCH(markVal, maxMark)) {
      data <<= 1;
      data |= 1; // 1 bit
    } else {
      // Guess the last mark was just a trailer after all
      nbits--;
      results->spaceEncData.trailer = results->rawbuf[irparams.rawlen-1] * USECPERTICK;
    }
  } else {

    // The decode loop where the space width determines the bit value
    for (int i=0; i < nbits; i++) {
      data <<= 1; // Shift the data over for the next bit
      // Check that the mark is okay
      unsigned int markVal = results->rawbuf[offset++];
      if (!MATCH(markVal, minMark)) {
	return ERR;
      }
      // Check the space and determine the bit
      unsigned int spaceVal = results->rawbuf[offset++];
      if (MATCH(spaceVal, minSpace)) {
        // 0 bit
      } else if (MATCH(spaceVal, maxSpace)) {
        data |= 1; // 1 bit
      } else {
        return ERR;
      }
    }
  }

  // Finally, save the results

  results->spaceEncData.headerMark = results->rawbuf[1] * USECPERTICK;
  results->spaceEncData.headerSpace = results->rawbuf[2] * USECPERTICK;
  results->spaceEncData.mark0 = minMark;
  results->spaceEncData.space0 = minSpace;
  results->spaceEncData.mark1 = maxMark;
  results->spaceEncData.space1 = maxSpace;
  results->spaceEncData.frequency = 0; // Don't know
  results->bits = nbits;
  results->value = data;
  results->decode_type = SPACE_ENC;
#ifdef DEBUG
  Serial.print("headerMark: ");
  Serial.println(results->spaceEncData.headerMark, DEC);
  Serial.print("headerSpace: ");
  Serial.println(results->spaceEncData.headerSpace, DEC);
  Serial.print("mark0: ");
  Serial.println(results->spaceEncData.mark0, DEC);
  Serial.print("space0: ");
  Serial.println(results->spaceEncData.space0, DEC);
  Serial.print("mark1: ");
  Serial.println(results->spaceEncData.mark1, DEC);
  Serial.print("space1: ");
  Serial.println(results->spaceEncData.space1, DEC);
  Serial.print("trailer: ");
  Serial.println(results->spaceEncData.trailer, DEC);
#endif
  return SPACE_ENC;
}

// Just handle the repeat code; decodeSpaceEnc can handle the rest
long IRrecv::decodeNecRepeat(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], NEC_HDR_MARK)) {
    return ERR;
  }
  offset++;
  // Check for repeat
  if (irparams.rawlen == 4 &&
    MATCH_SPACE(results->rawbuf[offset], NEC_RPT_SPACE) &&
    MATCH_MARK(results->rawbuf[offset+1], NEC_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = NEC_REPEAT;
    return DECODED;
  } else {
    return ERR;
  }
}

// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int IRrecv::getRClevel(decode_results *results, int *offset, int *used, int t1) {
  if (*offset >= results->rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  int width = results->rawbuf[*offset];
  int val = ((*offset) % 2) ? MARK : SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  int avail;
  if (MATCH(width, t1 + correction)) {
    avail = 1;
  } 
  else if (MATCH(width, 2*t1 + correction)) {
    avail = 2;
  } 
  else if (MATCH(width, 3*t1 + correction)) {
    avail = 3;
  } 
  else {
    return -1;
  }

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
#ifdef DEBUG
  if (val == MARK) {
    Serial.println("MARK");
  } 
  else {
    Serial.println("SPACE");
  }
#endif
  return val;   
}

long IRrecv::decodeRC5(decode_results *results) {
  if (irparams.rawlen < MIN_RC5_SAMPLES + 2) {
    return ERR;
  }
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  // Get start bits
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != SPACE) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  int nbits;
  for (nbits = 0; offset < irparams.rawlen; nbits++) {
    int levelA = getRClevel(results, &offset, &used, RC5_T1); 
    int levelB = getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      // 1 bit
      data = (data << 1) | 1;
    } 
    else if (levelA == MARK && levelB == SPACE) {
      // zero bit
      data <<= 1;
    } 
    else {
      return ERR;
    } 
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC5;
  return DECODED;
}

long IRrecv::decodeRC6(decode_results *results) {
  if (results->rawlen < MIN_RC6_SAMPLES) {
    return ERR;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], RC6_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (!MATCH_SPACE(results->rawbuf[offset], RC6_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  long data = 0;
  int used = 0;
  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK) return ERR;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return ERR;
  int nbits;
  for (nbits = 0; offset < results->rawlen; nbits++) {
    int levelA, levelB; // Next two levels
    levelA = getRClevel(results, &offset, &used, RC6_T1); 
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    } 
    levelB = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    } 
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    } 
    else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    } 
    else {
      return ERR; // Error
    } 
  }
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC6;
  return DECODED;
}

/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int IRrecv::compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } 
  else if (oldval < newval * .8) {
    return 2;
  } 
  else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
long IRrecv::decodeHash(decode_results *results) {
  // Require at least 10 samples to prevent triggering on noise
  if (results->rawlen < 10) {
    return ERR;
  }
  long hash = FNV_BASIS_32;
  for (int i = 1; i+2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value = hash;
  results->bits = 32;
  results->decode_type = HASH;
  return DECODED;
}

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h
// Normally macros are used for efficiency
#ifdef DEBUG
int MATCH(int measured, int desired) {
  Serial.print("Testing: ");
  Serial.print(TICKS_LOW(desired), DEC);
  Serial.print(" <= ");
  Serial.print(measured, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired), DEC);
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int MATCH_MARK(int measured_ticks, int desired_us) {
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

int MATCH_SPACE(int measured_ticks, int desired_us) {
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
#endif
