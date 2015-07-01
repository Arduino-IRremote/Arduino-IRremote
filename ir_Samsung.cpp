#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//==============================================================================

#define SAMSUNG_BITS          32
#define SAMSUNG_HDR_MARK    5000
#define SAMSUNG_HDR_SPACE   5000
#define SAMSUNG_BIT_MARK     560
#define SAMSUNG_ONE_SPACE   1600
#define SAMSUNG_ZERO_SPACE   560
#define SAMSUNG_RPT_SPACE   2250

//+=============================================================================
#if SEND_SAMSUNG
void  IRsend::sendSAMSUNG (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark(SAMSUNG_HDR_MARK);
	space(SAMSUNG_HDR_SPACE);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ONE_SPACE);
		} else {
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ZERO_SPACE);
		}
	}

	// Footer
	mark(SAMSUNG_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// SAMSUNGs have a repeat only 4 items long
//
#if DECODE_SAMSUNG
bool  IRrecv::decodeSAMSUNG (decode_results *results)
{
	long  data   = 0;
	int   offset = 1;  // Skip first space

	// Initial mark
	if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_HDR_MARK))   return false ;
	offset++;

	// Check for repeat
	if (    (irparams.rawlen == 4)
	     && MATCH_SPACE(results->rawbuf[offset], SAMSUNG_RPT_SPACE)
	     && MATCH_MARK(results->rawbuf[offset+1], SAMSUNG_BIT_MARK)
	   ) {
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = SAMSUNG;
		return true;
	}
	if (irparams.rawlen < (2 * SAMSUNG_BITS) + 4)  return false ;

	// Initial space
	if (!MATCH_SPACE(results->rawbuf[offset++], SAMSUNG_HDR_SPACE))  return false ;

	for (int i = 0;  i < SAMSUNG_BITS;   i++) {
		if (!MATCH_MARK(results->rawbuf[offset++], SAMSUNG_BIT_MARK))  return false ;

		if      (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                                return false ;
		offset++;
	}

	// Success
	results->bits        = SAMSUNG_BITS;
	results->value       = data;
	results->decode_type = SAMSUNG;
	return true;
}
#endif

