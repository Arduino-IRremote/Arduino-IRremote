/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#ifndef IRremote_h
#define IRremote_h

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define DEBUG
// #define TEST

// Information on a generic space encoding code.
// This is a code that varies the mark width (or the space
// width) to distingish bits.
class space_enc_data {
public:
  int headerMark; // Mark time for header in us
  int headerSpace; // Space time for header in us
  int mark0; // Mark time in us for 0 bit
  int space0; // Space time in us for 0 bit
  int mark1; // Mark time in us for 1 bit
  int space1; // Space time in us for 1 bit
  int trailer; // Trailer mark time in us or 0
  int frequency; // Not used by IRrecv
};

// Results returned from the decoder

class decode_results {
public:
  int decode_type; // NEC, SONY, RC5, UNKNOWN
  unsigned long long value; // Decoded value
  int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  int rawlen; // Number of records in rawbuf.
  class space_enc_data spaceEncData;
};

// Values for decode_type
#define NEC_REPEAT 1
#define RC5 3
#define RC6 4
#define SPACE_ENC 7 // Generic space encoding
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(int recvpin);
  int decode(decode_results *results);
  void enableIRIn();
  void pause();
  void resume(); // deprecated
private:
  // These are called by decode
  int getRClevel(decode_results *results, int *offset, int *used, int t1);
  long decodeSpaceEnc(decode_results *results);
  long decodeNecRepeat(decode_results *results);
  long decodeRC5(decode_results *results);
  long decodeRC6(decode_results *results);
  long decodeHash(decode_results *results);
  int compare(unsigned int oldval, unsigned int newval);
} 
;

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

class IRsend
{
public:
  IRsend() {}
  void sendSpaceEnc(unsigned long long data, int nbits, space_enc_data *spaceEncData);
  void sendNEC(unsigned long data, int nbits); // deprecated
  void sendSony(unsigned long data, int nbits); // deprecated
  void sendRaw(unsigned int buf[], int len, int hz);
  void sendRC5(unsigned long data, int nbits);
  void sendRC6(unsigned long data, int nbits);
  // private:
  void enableIROut(int khz);
  VIRTUAL void mark(int usec);
  VIRTUAL void space(int usec);
}
;

// Some useful constants

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 76 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif
