/*
 * IRremote
 * Copyright 2009-2010 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#include "IRremote.h"
#include "IRremoteInt.h"

class space_enc_data necEnc = {9000 /* headerMark */ , 4500 /* headerSpace */,
	560 /* mark0 */, 560 /* space0 */, 560 /* mark1 */, 1600 /* space1 */,
	560 /* trailer */, 38 /* frequency */};

class space_enc_data sonyEnc = {2400 /* headerMark */ , 600 /* headerSpace */,
	600 /* mark0 */, 600 /* space0 */, 1200 /* mark1 */, 600 /* space1 */,
	0 /* trailer */, 40 /* frequency */};

class space_enc_data sharpEnc = {2400 /* headerMark */ , 600 /* headerSpace */,
	600 /* mark0 */, 600 /* space0 */, 1200 /* mark1 */, 600 /* space1 */,
	0 /* trailer */, 40 /* frequency */};

#define RC5_T1		889
#define RC5_RPT_LENGTH	46000

#define RC6_HDR_MARK	2666
#define RC6_HDR_SPACE	889
#define RC6_T1		444
#define RC6_RPT_LENGTH	46000

// Send a generic space encoded code
// The timings and frequency are in spaceEncData
void IRsend::sendSpaceEnc(unsigned long long data, int nbits, space_enc_data *spaceEncData)
{
  enableIROut(spaceEncData->frequency);
  mark(spaceEncData->headerMark);
  space(spaceEncData->headerSpace);
  data <<= (64 - nbits);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(spaceEncData->mark1);
      space(spaceEncData->space1);
    } 
    else {
      mark(spaceEncData->mark0);
      space(spaceEncData->space0);
    }
    data <<= 1;
  }
  if (spaceEncData->trailer > 0) {
    mark(spaceEncData->trailer);
    space(0);
  }
}

void IRsend::sendNEC(unsigned long data, int nbits)
{
  sendSpaceEnc(data, nbits, &necEnc);
}

void IRsend::sendSony(unsigned long data, int nbits) {
  sendSpaceEnc(data, nbits, &sonyEnc);
}

void IRsend::sendRaw(unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } 
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

// Note: first bit must be a one (start bit)
void IRsend::sendRC5(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC5_T1); // First start bit
  space(RC5_T1); // Second start bit
  mark(RC5_T1); // Second start bit
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      space(RC5_T1); // 1 is space, then mark
      mark(RC5_T1);
    } 
    else {
      mark(RC5_T1);
      space(RC5_T1);
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}

// Caller needs to take care of flipping the toggle bit
void IRsend::sendRC6(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC6_HDR_MARK);
  space(RC6_HDR_SPACE);
  mark(RC6_T1); // start bit
  space(RC6_T1);
  int t;
  for (int i = 0; i < nbits; i++) {
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    } 
    else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      mark(t);
      space(t);
    } 
    else {
      space(t);
      mark(t);
    }

    data <<= 1;
  }
  space(0); // Turn off at end
}

void IRsend::mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  IRremoteEnablePwm();
  delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void IRsend::space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  IRremoteDisablePwm();
  delayMicroseconds(time);
}

void IRsend::enableIROut(int khz) {
  IRremoteEnableIRoutput(khz);
}
