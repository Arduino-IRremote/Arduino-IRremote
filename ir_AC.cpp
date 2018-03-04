#include "IRremote.h"
#include "IRremoteInt.h"
/*
 * ir_AC.cpp
 * Version 1.0 September, 2018
 * by Marcelo Buregio
 *
 * This is an add-on for IRremote library for use in Air Conditioner that uses 48-bit protocol.
 * Tested successfully on Midea, Electrolux and Hitachi (some).
 *
 * This add-on needs to be used with IRremote 2.x version
 * This version can be downloaded on https://github.com/marceloburegio/Arduino-IRremote
 *
 * For more details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 */
//==============================================================================
//                                 AAA    CCCC
//                                A   A  C
//                                AAAAA  C
//                                A   A  C
//                                A   A   CCCC
//==============================================================================

// Times definition for AC IR Controller
#define AC_HDR_MARK 4400
#define AC_HDR_SPACE 4400
#define AC_BIT_MARK 550
#define AC_ONE_SPACE 1600
#define AC_ZERO_SPACE 500
#define AC_RPT_SPACE 5200

// Length of AC Code
#define AC_BITS 48
#define TOPBIT 0x80000000

//+=============================================================================
#if SEND_AC
void  IRsend::sendAC (unsigned int address,  unsigned long data)
{
	// Set IR carrier frequency
	enableIROut(38);
	
	// Sending 2x times...
	for (uint8_t x=0;x<2;x++)
	{
		unsigned int address1 = address;
		unsigned long data1 = data;
		
		// Header
		mark(AC_HDR_MARK);
		space(AC_HDR_SPACE);
		
		// Address
		for (unsigned long  mask = 1UL << (16 - 1);  mask;  mask >>= 1) {
			mark(AC_BIT_MARK);
			if (address & mask)  space(AC_ONE_SPACE) ;
			else                 space(AC_ZERO_SPACE) ;
		}

		// Data
		for (unsigned long  mask = 1UL << (32 - 1);  mask;  mask >>= 1) {
			mark(AC_BIT_MARK);
			if (data & mask)  space(AC_ONE_SPACE) ;
			else              space(AC_ZERO_SPACE) ;
		}
		
		// Footer
		mark(AC_BIT_MARK);
		space(AC_RPT_SPACE);
	}
}
#endif

//+=============================================================================
#if DECODE_AC

// Decode Air Conditioner with 48bit (Midea, Komeco, Electrolux, Hitachi)
bool  IRrecv::decodeAC (decode_results *results)
{
	unsigned long long data = 0;
	uint8_t offset = 1;
	
	if (results->rawlen < AC_BITS * 2) return false; // Samsung is almost the same protocol, but is 32-bit
	if (!MATCH_MARK(results->rawbuf[offset++], AC_HDR_MARK)) return false;
	if (!MATCH_MARK(results->rawbuf[offset++], AC_HDR_SPACE)) return false;
	
	// Reading the first bit mark.
	int bitmark = results->rawbuf[offset]*USECPERTICK;
	for (uint8_t i = 0; i < AC_BITS; i++) {
		if (!MATCH_MARK(results->rawbuf[offset++], bitmark)) return false;
		
		if      (MATCH_SPACE(results->rawbuf[offset], AC_ONE_SPACE))  data = (data << 1) | 1;
		else if (MATCH_SPACE(results->rawbuf[offset], AC_ZERO_SPACE)) data <<= 1;
		else return false;
		offset++;
		
		// Updating the bit mark. It's almost the same on the current transmission.
		bitmark = results->rawbuf[offset]*USECPERTICK;
	}
	results->value       = (unsigned long)data;
	results->address     = (unsigned int)(data >> 32);
	results->decode_type = AC;
	results->bits        = AC_BITS;
	return true;
}
#endif
