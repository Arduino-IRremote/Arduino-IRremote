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

void IRsend::sendSpaceEnc(unsigned long data, int nbits, space_enc_data *spaceEncData)
{
  enableIROut(spaceEncData->frequency);
  mark(spaceEncData->headerMark);
  space(spaceEncData->headerSpace);
  data <<= (32 - nbits);
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
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);
      space(NEC_ONE_SPACE);
    } 
    else {
      mark(NEC_BIT_MARK);
      space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);
  space(0);
}

void IRsend::sendSony(unsigned long data, int nbits) {
  enableIROut(40);
  mark(SONY_HDR_MARK);
  space(SONY_HDR_SPACE);
  data = data << (32 - nbits);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SONY_ONE_MARK);
      space(SONY_HDR_SPACE);
    } 
    else {
      mark(SONY_ZERO_MARK);
      space(SONY_HDR_SPACE);
    }
    data <<= 1;
  }
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

/* Sharp and DISH support by Todd Treece

The Dish send function needs to be repeated 4 times and the Sharp function
has the necessary repeats built in. I know that it's not consistent,
but I don't have the time to update my code.

Here are the LIRC files that I found that seem to match the remote codes
from the oscilloscope:

Sharp LCD TV:
http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

DISH NETWORK (echostar 301):
http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

For the DISH codes, only send the last for characters of the hex.
i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
linked LIRC file.
*/

void IRsend::sendSharp(unsigned long data, int nbits) {
  unsigned long invertdata = data ^ SHARP_TOGGLE_MASK;
  enableIROut(38);
  for (int i = 0; i < nbits; i++) {
    if (data & 0x4000) {
      mark(SHARP_BIT_MARK);
      space(SHARP_ONE_SPACE);
    }
    else {
      mark(SHARP_BIT_MARK);
      space(SHARP_ZERO_SPACE);
    }
    data <<= 1;
  }
  
  mark(SHARP_BIT_MARK);
  space(SHARP_ZERO_SPACE);
  delay(46);
  for (int i = 0; i < nbits; i++) {
    if (invertdata & 0x4000) {
      mark(SHARP_BIT_MARK);
      space(SHARP_ONE_SPACE);
    }
    else {
      mark(SHARP_BIT_MARK);
      space(SHARP_ZERO_SPACE);
    }
    invertdata <<= 1;
  }
  mark(SHARP_BIT_MARK);
  space(SHARP_ZERO_SPACE);
  delay(46);
}

void IRsend::sendDISH(unsigned long data, int nbits)
{
  enableIROut(56);
  mark(DISH_HDR_MARK);
  space(DISH_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & DISH_TOP_BIT) {
      mark(DISH_BIT_MARK);
      space(DISH_ONE_SPACE);
    }
    else {
      mark(DISH_BIT_MARK);
      space(DISH_ZERO_SPACE);
    }
    data <<= 1;
  }
}
