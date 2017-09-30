#include "IRremote.h"
#include "IRremoteInt.h"

// GREE IR CODE: S + 32 bits + 3 bits + C + 32 bits
// S: 9000 + 4500
// 32 bits: Data 1
// 3  bits: 0 1 0
// C: 600 + 20000
// 32 bits: Data 2
//
//==============================================================================
//                          GGGG  RRRR   EEEEE  EEEEE
//                         G      R   R  E      E
//                         G GG   RRRR   EEE    EEE
//                         G  G   R  R   E      E
//                          GGG   R   R  EEEEE  EEEEE
//==============================================================================

#define GREE_BITS          68
#define GREE_HDR_MARK    9000
#define GREE_HDR_SPACE   4500
#define GREE_BIT_MARK     600
#define GREE_ONE_SPACE   1600
#define GREE_MID_CONCAT 20000
#define GREE_ZERO_SPACE   600
#define GREE_RPT_SPACE   2250

//+=============================================================================
#if SEND_GREE
void  IRsend::sendGREE (unsigned long data1, unsigned long data2)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark(GREE_HDR_MARK);
	space(GREE_HDR_SPACE);

	int nbits = 32;
	// Data: 32 btis
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data1 & mask) {
			DBG_PRINT(F("1,"));
			mark(GREE_BIT_MARK);
			space(GREE_ONE_SPACE);
		} else {
			DBG_PRINT(F("0,"));
			mark(GREE_BIT_MARK);
			space(GREE_ZERO_SPACE);
		}
	}

	// Data: 0 1 0
	DBG_PRINTLN(F("0,1,0,"));
	mark(GREE_BIT_MARK);
	space(GREE_ZERO_SPACE);
	mark(GREE_BIT_MARK);
	space(GREE_ONE_SPACE);
	mark(GREE_BIT_MARK);
	space(GREE_ZERO_SPACE);
	
	mark(GREE_BIT_MARK);
	space(GREE_MID_CONCAT);

	// Data: 32 btis
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data2 & mask) {
			DBG_PRINT(F("1,"));
			mark(GREE_BIT_MARK);
			space(GREE_ONE_SPACE);
		} else {
			DBG_PRINT(F("0,"));
			mark(GREE_BIT_MARK);
			space(GREE_ZERO_SPACE);
		}
	}

	// Footer
	mark(GREE_BIT_MARK);
	space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// GREEs have a repeat only 4 items long
//
#if DECODE_GREE
bool  IRrecv::decodeGREE (decode_results *results)
{
	long  data   = 0;  // We decode in to here; Start with nothing
	long  addit_data   = 0;  // The data behind concat
	int   offset = 1;  // Index in to results; Skip first entry!?

	// Check header "mark"
	if (!MATCH_MARK(results->rawbuf[offset], GREE_HDR_MARK))  return false ;
	offset++;

	// Check for repeat. TBD: DO NOT test
	if ( (irparams.rawlen == 4)
	    && MATCH_SPACE(results->rawbuf[offset  ], GREE_RPT_SPACE)
	    && MATCH_MARK (results->rawbuf[offset+1], GREE_BIT_MARK )
	   ) {
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = GREE;
		return true;
	}

	// Check we have enough data
	if (irparams.rawlen < (2 * GREE_BITS) + 4)  return false ;

	// Check header "space"
	if (!MATCH_SPACE(results->rawbuf[offset], GREE_HDR_SPACE))  return false ;
	offset++;

	// Build the data
	for (int i = 0;  i < GREE_BITS;  i++) {
		// Check data "mark"
		if (!MATCH_MARK(results->rawbuf[offset], GREE_BIT_MARK))  return false ;
		offset++;
        // Suppend this bit
		// Ignore 32-34. 0, 1, 0
		// Ignore concat
		if      (MATCH_SPACE(results->rawbuf[offset], GREE_ONE_SPACE )) {
			if (i < 32) {
				data = (data << 1) | 1 ;
			} else if (i > 36){
				addit_data = (addit_data << 1) | 1 ;
			}
		}
		else if (MATCH_SPACE(results->rawbuf[offset], GREE_ZERO_SPACE)) {
			if (i < 32) {
				data = (data << 1) | 0 ;
			} else if (i > 36){
				addit_data = (addit_data << 1) | 0 ;
			}
		}
		else if (MATCH_MARK(results->rawbuf[offset], GREE_MID_CONCAT)) ;
		else                                                            return false ;
		offset++;
	}

	// Success
	results->bits        = GREE_BITS - 4;
	results->value       = data;
	results->addit_value = addit_data;
	results->decode_type = GREE;

	return true;
}
#endif
