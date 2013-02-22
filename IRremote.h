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
 */

#ifndef IRremote_h
#define IRremote_h
#include <inttypes.h>

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define DEBUG
// #define TEST

// MagiQuest packet is both Wand ID and magnitude of swish and flick
union magiquest {
  uint64_t llword;
  uint8_t    byte[8];
//  uint16_t   word[4];
//  uint32_t  lword[2];
  struct {
    uint16_t magnitude;
    uint32_t wand_id;
    uint8_t  padding;
    uint8_t  scrap;
  } cmd ;
} ;

// Results returned from the decoder
class decode_results {
public:
  int16_t decode_type; // NEC, SONY, RC5, UNKNOWN
  uint16_t panasonicAddress; // This is only used for decoding Panasonic data
  uint16_t magiquestMagnitude; // This is only used for MagiQuest
  int32_t value; // Decoded value //mpf need to make unsigned.
  int16_t bits; // Number of bits in decoded value
  volatile uint16_t *rawbuf; // Raw intervals in .5 us ticks
  int16_t rawlen; // Number of records in rawbuf.
};

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
#define MAGIQUEST 11
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(int recvpin);
  void blink13(int blinkflag);
  int16_t decode(decode_results *results);
  void enableIRIn();
  void resume();
private:
  // These are called by decode
  int16_t getRClevel(decode_results *results, int16_t *offset, int16_t *used, int16_t t1);
  int32_t  decodeNEC(decode_results *results);
  int32_t  decodeSony(decode_results *results);
  int32_t  decodeSanyo(decode_results *results);
  int32_t  decodeMitsubishi(decode_results *results);
  int32_t  decodeRC5(decode_results *results);
  int32_t  decodeRC6(decode_results *results);
  int32_t  decodePanasonic(decode_results *results);
  int32_t  decodeJVC(decode_results *results);
  int32_t  decodeMagiQuest(decode_results *results);
  int32_t decodeHash(decode_results *results);
  int16_t compare(uint16_t oldval, uint16_t newval);

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
  void sendNEC(int32_t data, int16_t nbits);
  void sendSony(int32_t data, int16_t nbits);
  // Neither Sanyo nor Mitsubishi send is implemented yet
  //  void sendSanyo(int32_t data, int16_t nbits);
  //  void sendMitsubishi(int32_t data, int16_t nbits);
  void sendRaw(uint16_t buf[], int16_t len, int16_t hz);
  void sendRC5(int32_t data, int16_t nbits);
  void sendRC6(int32_t data, int16_t nbits);
  void sendDISH(int32_t data, int16_t nbits);
  void sendSharp(int32_t data, int16_t nbits);
  void sendPanasonic(uint16_t address, int32_t data);
  void sendJVC(int32_t data, int16_t nbits, int16_t repeat); // *Note instead of sending the REPEAT constant if you want the JVC repeat signal sent, send the original code value and change the repeat argument from 0 to 1. JVC protocol repeats by skipping the header NOT by sending a separate code value like NEC does.
  void sendMagiQuest(uint32_t wand_id, uint16_t magitude);
  // private:
  void enableIROut(int16_t khz);
  VIRTUAL void mark(int16_t usec);
  VIRTUAL void space(int16_t usec);
}
;

// Some useful constants

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 112 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif
