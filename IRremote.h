
//******************************************************************************
// IRremote
// Version 2.0.1 June, 2015
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
// Edited by Mitra to add new controller SANYO
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// LG added by Darryl Smith (based on the JVC protocol)
// Whynter A/C ARC-110WD added by Francesco Meschia
//******************************************************************************

#ifndef IRremote_h
#define IRremote_h

//------------------------------------------------------------------------------
// The ISR header contains several useful macros the user may wish to use
//
#include "IRremoteInt.h"

//------------------------------------------------------------------------------
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to 0) all the protocols you do not need/want!
//
#define DECODE_RC5           1
#define SEND_RC5             1

#define DECODE_RC6           1
#define SEND_RC6             1

#define DECODE_NEC           1
#define SEND_NEC             1

#define DECODE_SONY          1
#define SEND_SONY            1

#define DECODE_PANASONIC     1
#define SEND_PANASONIC       1

#define DECODE_JVC           1
#define SEND_JVC             1

#define DECODE_SAMSUNG       1
#define SEND_SAMSUNG         1

#define DECODE_WHYNTER       1
#define SEND_WHYNTER         1

#define DECODE_AIWA_RC_T501  1
#define SEND_AIWA_RC_T501    1

#define DECODE_LG            1
#define SEND_LG              1

#define DECODE_SANYO         1
#define SEND_SANYO           0 // NOT WRITTEN

#define DECODE_MITSUBISHI    1
#define SEND_MITSUBISHI      0 // NOT WRITTEN

#define DECODE_DISH          0 // NOT WRITTEN
#define SEND_DISH            1

#define DECODE_SHARP         0 // NOT WRITTEN
#define SEND_SHARP           1

#define DECODE_DENON         1
#define SEND_DENON           1

#define DECODE_PRONTO        0 // This function doe not logically make sense
#define SEND_PRONTO          1

#define DECODE_LEGO_PF       0 // NOT WRITTEN
#define SEND_LEGO_PF         1

//------------------------------------------------------------------------------
// When sending a Pronto code we request to send either the "once" code
//                                                   or the "repeat" code
// If the code requested does not exist we can request to fallback on the
// other code (the one we did not explicitly request)
//
// I would suggest that "fallback" will be the standard calling method
// The last paragraph on this page discusses the rationale of this idea:
//   http://www.remotecentral.com/features/irdisp2.htm
//
#define PRONTO_ONCE        false
#define PRONTO_REPEAT      true
#define PRONTO_FALLBACK    true
#define PRONTO_NOFALLBACK  false

//------------------------------------------------------------------------------
// An enumerated list of all supported formats
// You do NOT need to remove entries from this list when disabling protocols!
//
typedef
	enum {
		UNKNOWN      = -1,
		UNUSED       =  0,
		RC5,
		RC6,
		NEC,
		SONY,
		PANASONIC,
		JVC,
		SAMSUNG,
		WHYNTER,
		AIWA_RC_T501,
		LG,
		SANYO,
		MITSUBISHI,
		DISH,
		SHARP,
		DENON,
		PRONTO,
		LEGO_PF,
	}
decode_type_t;

//------------------------------------------------------------------------------
// Set DEBUG to 1 for lots of lovely debug output
//
#define DEBUG  0

//------------------------------------------------------------------------------
// Debug directives
//
#if DEBUG
#	define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
#	define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#	define DBG_PRINT(...)
#	define DBG_PRINTLN(...)
#endif

//------------------------------------------------------------------------------
// Mark & Space matching functions
//
int  MATCH       (int measured, int desired) ;
int  MATCH_MARK  (int measured_ticks, int desired_us) ;
int  MATCH_SPACE (int measured_ticks, int desired_us) ;

//------------------------------------------------------------------------------
// Results returned from the decoder
//
class decode_results
{
	public:
		decode_type_t          decode_type;  // UNKNOWN, NEC, SONY, RC5, ...
		unsigned int           address;      // Used by Panasonic & Sharp [16-bits]
		unsigned long          value;        // Decoded value [max 32-bits]
		int                    bits;         // Number of bits in decoded value
		volatile unsigned int  *rawbuf;      // Raw intervals in 50uS ticks
		int                    rawlen;       // Number of records in rawbuf
		int                    overflow;     // true iff IR raw code too long
};

//------------------------------------------------------------------------------
// Decoded value for NEC when a repeat code is received
//
#define REPEAT 0xFFFFFFFF

//------------------------------------------------------------------------------
// Main class for receiving IR
//
class IRrecv
{
	public:
		IRrecv (int recvpin) ;
		IRrecv (int recvpin, int blinkpin);

		void  blink13    (int blinkflag) ;
		int   decode     (decode_results *results) ;
		void  enableIRIn ( ) ;
		bool  isIdle     ( ) ;
		void  resume     ( ) ;

	private:
		long  decodeHash (decode_results *results) ;
		int   compare    (unsigned int oldval, unsigned int newval) ;

		//......................................................................
#		if (DECODE_RC5 || DECODE_RC6)
			// This helper function is shared by RC5 and RC6
			int  getRClevel (decode_results *results,  int *offset,  int *used,  int t1) ;
#		endif
#		if DECODE_RC5
			bool  decodeRC5        (decode_results *results) ;
#		endif
#		if DECODE_RC6
			bool  decodeRC6        (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_NEC
			bool  decodeNEC        (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_SONY
			bool  decodeSony       (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_PANASONIC
			bool  decodePanasonic  (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_JVC
			bool  decodeJVC        (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_SAMSUNG
			bool  decodeSAMSUNG    (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_WHYNTER
			bool  decodeWhynter    (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_AIWA_RC_T501
			bool  decodeAiwaRCT501 (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_LG
			bool  decodeLG         (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_SANYO
			bool  decodeSanyo      (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_MITSUBISHI
			bool  decodeMitsubishi (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_DISH
			bool  decodeDish (decode_results *results) ; // NOT WRITTEN
#		endif
		//......................................................................
#		if DECODE_SHARP
			bool  decodeSharp (decode_results *results) ; // NOT WRITTEN
#		endif
		//......................................................................
#		if DECODE_DENON
			bool  decodeDenon (decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_LEGO_PF
			bool  decodeLegoPowerFunctions (decode_results *results) ;
#		endif
} ;

//------------------------------------------------------------------------------
// Main class for sending IR
//
class IRsend
{
	public:
		IRsend () { }

		void  custom_delay_usec (unsigned long uSecs);
		void  enableIROut 		(int khz) ;
		void  mark        		(unsigned int usec) ;
		void  space       		(unsigned int usec) ;
		void  sendRaw     		(const unsigned int buf[],  unsigned int len,  unsigned int hz) ;

		//......................................................................
#		if SEND_RC5
			void  sendRC5        (unsigned long data,  int nbits) ;
#		endif
#		if SEND_RC6
			void  sendRC6        (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_NEC
			void  sendNEC        (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_SONY
			void  sendSony       (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_PANASONIC
			void  sendPanasonic  (unsigned int address,  unsigned long data) ;
#		endif
		//......................................................................
#		if SEND_JVC
			// JVC does NOT repeat by sending a separate code (like NEC does).
			// The JVC protocol repeats by skipping the header.
			// To send a JVC repeat signal, send the original code value
			//   and set 'repeat' to true
			void  sendJVC        (unsigned long data,  int nbits,  bool repeat) ;
#		endif
		//......................................................................
#		if SEND_SAMSUNG
			void  sendSAMSUNG    (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_WHYNTER
			void  sendWhynter    (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_AIWA_RC_T501
			void  sendAiwaRCT501 (int code) ;
#		endif
		//......................................................................
#		if SEND_LG
			void  sendLG         (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_SANYO
			void  sendSanyo      ( ) ; // NOT WRITTEN
#		endif
		//......................................................................
#		if SEND_MISUBISHI
			void  sendMitsubishi ( ) ; // NOT WRITTEN
#		endif
		//......................................................................
#		if SEND_DISH
			void  sendDISH       (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_SHARP
			void  sendSharpRaw   (unsigned long data,  int nbits) ;
			void  sendSharp      (unsigned int address,  unsigned int command) ;
#		endif
		//......................................................................
#		if SEND_DENON
			void  sendDenon      (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_PRONTO
			void  sendPronto     (char* code,  bool repeat,  bool fallback) ;
#		endif
//......................................................................
#		if SEND_LEGO_PF
			void  sendLegoPowerFunctions (uint16_t data, bool repeat = true) ;
#		endif
} ;

#endif
