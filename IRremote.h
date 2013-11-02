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
 *
 * RCMM protocol added by Matthias Neeracher.
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
//#define TEST

// MagiQuest packet is both Wand ID and magnitude of swish and flick
union magiquest {
  uint64_t llword;
  uint8_t    byte[8];
//  uint16_t   word[4];
  uint32_t  lword[2];
  struct {
    uint16_t magnitude;
    uint32_t wand_id;
    uint8_t  padding;
    uint8_t  scrap;
  } cmd ;
} ;

union helicopter {
  uint32_t dword;
  struct
  {
     uint8_t Throttle  : 7;    //  0..6   0 - 127
     uint8_t Channel   : 1;    //  7      A=0, B=1
     uint8_t Pitch     : 7;    //  8..14  0(forward) - 63(center) 127(back)
     uint8_t Pspacer   : 1;    // 15      na
     uint8_t Yaw       : 7;    // 16..22  127(left) - 63(center) - 0(right)
  } symaR3;
  struct
  {
     uint8_t Trim      : 7;    //  0..6  127(left) - 63(center) - 0(right)
     uint8_t Tspacer   : 1;    //  7     na
     uint8_t Throttle  : 7;    //  8..14 0 - 127
     uint8_t Channel   : 1;    // 15     A=0, B=1
     uint8_t Pitch     : 7;    // 16..22 0(forward) - 63(center) 127(back)
     uint8_t Pspacer   : 1;    // 23     na
     uint8_t Yaw       : 7;    // 24..30 127(left) - 63(center) - 0(right)
  } symaR5;
  struct
  {
     uint8_t cksum     : 3;    // 0..2
     uint8_t Rbutton   : 1;    // 3      0-normally off, 1-depressed
     uint8_t Lbutton   : 1;    // 4      0-normally off, 1-depressed
     uint8_t Turbo     : 1;    // 5      1-off, 0-on
     uint8_t Channel   : 2;    // 6,7    A=1, B=2, C=0
     uint8_t Trim      : 6;    // 8..13  (left)63 - 31(center) - 0(right)
     uint8_t Yaw       : 5;    // 14..18 31(left) - 15(center) - 0(right)
     uint8_t Pitch     : 6;    // 19..24 0(foward) - 31(center) - 63(back)
     uint8_t Throttle  : 7;    // 25..31 0 - 127
  } uSeries;
  struct
  {
     uint8_t Trim      : 4;    //  0..3  15(left) -  8(center) - 0(right)
     uint8_t Trim_dir  : 1;    //  4     0= , 1=
     uint8_t Yaw_dir   : 1;    //  5
     uint8_t Fire      : 1;    //  6
     uint8_t Yaw       : 4;    //  7..10 15(left) - 8(center) - 0(right)
     uint8_t Pitch     : 4;    // 11..14 1(back) - 8(center) 15(forward)
     uint8_t Throttle  : 6;    // 15..20 0 - 63
     uint8_t Channel   : 2;    // 21..22 ?A=0, B=1
  } fastlane;
};

// Results returned from the decoder
class decode_results {
public:
  int16_t decode_type; // NEC, SONY, RC5, UNKNOWN
  uint16_t panasonicAddress; // This is only used for decoding Panasonic data
  uint16_t magiquestMagnitude; // This is only used for MagiQuest
  int32_t value; // Decoded value //mpf need to make unsigned.
  union helicopter helicopter;
  unsigned int parity;
  int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  int rawlen; // Number of records in rawbuf.
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
#define SYMA_R3 12
#define SYMA_R5 13
#define USERIES 14
#define FASTLANE 15
#define RCMM 16
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
  long decodeNEC(decode_results *results);
  long decodeSony(decode_results *results);
  long decodeSanyo(decode_results *results);
  long decodeMitsubishi(decode_results *results);
  long decodeRC5(decode_results *results);
  long decodeRC6(decode_results *results);
  long decodePanasonic(decode_results *results);
  long decodeJVC(decode_results *results);
  long decodeRCMM(decode_results *results);
  long decodeHash(decode_results *results);
  long  decodeSyma(decode_results *results);
  long  decodeUseries(decode_results *results);
  long  decodeFastLane(decode_results *results);
  long  decodeMagiQuest(decode_results *results);
  int compare(unsigned int oldval, unsigned int newval);

}
;

uint8_t UseriesChecksum(uint32_t val);

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
  void sendNEC(unsigned long data, int nbits);
  void sendSony(unsigned long data, int nbits);
  // Neither Sanyo nor Mitsubishi send is implemented yet
  //  void sendSanyo(unsigned long data, int nbits);
  //  void sendMitsubishi(unsigned long data, int nbits);
  void sendRaw(unsigned int buf[], int len, int hz);
  void sendRC5(unsigned long data, int nbits);
  void sendRC6(unsigned long data, int nbits);
  void sendDISH(unsigned long data, int nbits);
  void sendSharp(unsigned long data, int nbits);
  void sendPanasonic(unsigned int address, unsigned long data);
  void sendJVC(unsigned long data, int nbits, int repeat); // *Note instead of sending the REPEAT constant if you want the JVC repeat signal sent, send the original code value and change the repeat argument from 0 to 1. JVC protocol repeats by skipping the header NOT by sending a separate code value like NEC does.
  void sendMagiQuest(unsigned long wand_id, unsigned magitude);
  void sendSymaR5(unsigned long data);
  void sendSymaR3(unsigned long data);
  void sendUseries(unsigned long data);
  void sendFastLane(unsigned long data);
  void sendRCMM(unsigned long data, int nbits);
  // private:
  void enableIROut(int khz);
  VIRTUAL void mark(int usec);
  VIRTUAL void space(int usec);
  void on(unsigned int freq);
  void off(void);
}
;

// Some useful constants

#define USECPERTICK 25  // microseconds per clock interrupt tick
#define RAWBUF 112 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 50
#endif

