/*
Created by Gaspar de Elias for handling an A/C unit
*/

#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              TCL112AC
//
//
//==============================================================================

#define BITS          112  // The number of bits in the command

#define HDR_MARK    3000  // The length of the Header:Mark
#define HDR_SPACE   1650  // The lenght of the Header:Space

#define BIT_MARK    525  // The length of a Bit:Mark
#define ONE_SPACE   450  // The length of a Bit:Space for 1's
#define ZERO_SPACE  1200  // The length of a Bit:Space for 0's

#define OTHER       1234  // Other things you may need to define

//+=============================================================================
//
#if SEND_TCL112AC
void  IRsend::sendTcl112ac (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark (HDR_MARK);
	space(HDR_SPACE);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark (BIT_MARK);
			space(ONE_SPACE);
		} else {
			mark (BIT_MARK);
			space(ZERO_SPACE);
		}
	}

	// Footer
	mark(BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_TCL112AC
bool  IRrecv::decodeTcl112ac (decode_results *results)
{
	unsigned long  data   = 0;  // Somewhere to build our code
	int            offset = 1;  // Skip the Gap reading

	DBG_PRINTLN("Checking right amount of data: ");
	DBG_PRINTLN(irparams.rawlen);

	// Check we have the right amount of data
	if (irparams.rawlen != 1 + 2 + (2 * BITS) + 1)  return false ;

	// Check initial Mark+Space match
	if (!MATCH_MARK (results->rawbuf[offset++], HDR_MARK ))  return false ;
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))  return false ;

	// Read the bits in
	for (int i = 0;  i < BITS;  i++) {
		// Each bit looks like: MARK + SPACE_1 -> 1
		//                 or : MARK + SPACE_0 -> 0
		if (!MATCH_MARK(results->rawbuf[offset++], BIT_MARK))  return false ;

		// IR data is big-endian, so we shuffle it in from the right:
		if      (MATCH_SPACE(results->rawbuf[offset], ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                        return false ;
		offset++;
	}

	// Success
	results->bits        = BITS;
	results->value       = data;
	results->decode_type = TCL112AC;
	return true;
}
#endif
