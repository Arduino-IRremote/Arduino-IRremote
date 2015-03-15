/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 */

#ifndef IRremote_h
#define IRremote_h

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
//#define DEBUG
// #define TEST

// Results returned from the decoder
class decode_results {
public:
  int decode_type; // NEC, SONY, RC5, UNKNOWN
  union { // This is used for decoding Panasonic and Sharp data
    unsigned int panasonicAddress;
    unsigned int sharpAddress;
  };
  unsigned long value; // Decoded value
  int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  int rawlen; // Number of records in rawbuf.
};

// Send types
#define IRsendNEC
#define IRsendSONY
#define IRsendRC5
#define IRsendRC6
#define IRsendDISH
#define IRsendSHARP
#define IRsendPANASONIC
#define IRsendJVC
#define IRsendSANYO
#define IRsendMITSUBISHI
#define IRsendSAMSUNG
#define IRsendRAW

// Values for decode_type
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define DISH 5
#define SHARP 6
#define PANASONIC 7
#define JVC 8
#define SANYO 9
#define MITSUBISHI 10
#define SAMSUNG 11
#define LG 12
#define WHYNTER 13
#define AIWA_RC_T501 14
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(int recvpin);
  void blink13(int blinkflag);
  int decode(decode_results *results);
  void enableIRIn();
  void resume();
private:
  // These are called by decode
  int getRClevel(decode_results *results, int *offset, int *used, int t1);
#ifdef NEC
  long decodeNEC(decode_results *results);
#endif
#ifdef SONY
  long decodeSony(decode_results *results);
#endif
#ifdef SANYO
  long decodeSanyo(decode_results *results);
#endif
#ifdef MITSUBISHI
  long decodeMitsubishi(decode_results *results);
#endif
#ifdef RC5
  long decodeRC5(decode_results *results);
#endif
#ifdef RC6
  long decodeRC6(decode_results *results);
#endif
#ifdef PANASONIC
  long decodePanasonic(decode_results *results);
#endif
#ifdef LG
  long decodeLG(decode_results *results);
#endif
#ifdef JVC
  long decodeJVC(decode_results *results);
#endif
#ifdef SAMSUNG
  long decodeSAMSUNG(decode_results *results);
#endif

#ifdef WHYNTER
  long decodeWhynter(decode_results *results);
#endif

#ifdef AIWA_RC_T501
  long decodeAiwaRCT501(decode_results *results);
#endif

  long decodeHash(decode_results *results);
  int compare(unsigned int oldval, unsigned int newval);

} ;

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
  void sendRaw(unsigned int buf[], int len, int hz);
  void sendRC5(unsigned long data, int nbits);
  void sendRC6(unsigned long data, int nbits);

#ifdef WHYNTER
  void sendWhynter(unsigned long data, int nbits);
#endif
#ifdef NEC 
  void sendNEC(unsigned long data, int nbits);
#endif
#ifdef SONY 
  void sendSony(unsigned long data, int nbits);
  // Neither Sanyo nor Mitsubishi send is implemented yet
  //  void sendSanyo(unsigned long data, int nbits);
  //  void sendMitsubishi(unsigned long data, int nbits);
#endif

#ifdef DISH 
  void sendDISH(unsigned long data, int nbits);
#endif
#ifdef SHARP
  void sendSharp(unsigned int address, unsigned int command);
  void sendSharpRaw(unsigned long data, int nbits);
#endif
#ifdef IRsendSHARP
  void sendSharp(unsigned long data, int nbits);
#endif
#ifdef PANASONIC
  void sendPanasonic(unsigned int address, unsigned long data);
#endif
#ifdef JVC
  void sendJVC(unsigned long data, int nbits, int repeat); // *Note instead of sending the REPEAT constant if you want the JVC repeat signal sent, send the original code value and change the repeat argument from 0 to 1. JVC protocol repeats by skipping the header NOT by sending a separate code value like NEC does.
  void sendAiwaRCT501(int code);
  // private:
#endif
#ifdef SAMSUNG 
  void sendSAMSUNG(unsigned long data, int nbits);
#endif
  void enableIROut(int khz);
  VIRTUAL void mark(int usec);
  VIRTUAL void space(int usec);
} ;

// Some useful constants

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 100 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif
