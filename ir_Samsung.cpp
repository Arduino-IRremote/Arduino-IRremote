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
/*
 * Address separator. It seems to appear between the address bits and the data bits.
*/
#define SAMSUNG_36_BITS          36
#define SAMSUNG_36_MIDDLE_SPACE	 4600 // 4400
#define SAMSUNG_36_END_SPACE	 100 // 68

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
	for (unsigned long  mask = 1UL << (nbits - 1);  mask; mask >>= 1) {
		
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

void  IRsend::sendSAMSUNG36 (unsigned long long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark(SAMSUNG_HDR_MARK);
	space(SAMSUNG_HDR_SPACE);

	int posCount = 0;
	// Data
	for (unsigned long long  mask = 1ULL << (nbits - 1);  mask; /*mask >>= 1*/) {
		
		if(posCount == 16) {
			// Send 16 mark
			mark(SAMSUNG_BIT_MARK);
			space(4400);
		}
		else if(posCount == 28) {
			space(68);
		}
		if (data & mask) {
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ONE_SPACE);
			mask >>= 1;
		} else {
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ZERO_SPACE);
			mask >>= 1;
		}
		
		posCount++;
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

bool  IRrecv::decodeSAMSUNG36 (decode_results *results)
{
	long  data   = 0;
	long  data1 = 0;
	
	int   offset = 1;  // Skip first space

	// Initial mark
	if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_HDR_MARK))   return false ;
	offset++;

	if (irparams.rawlen < (2 * SAMSUNG_36_BITS) + 4)  return false ;

	// Initial space
	if (!MATCH_SPACE(results->rawbuf[offset++], SAMSUNG_HDR_SPACE))  return false ;
	
	DBG_PRINTLN("About to look at data");
	for (int i = 0;  i < SAMSUNG_36_BITS;   i++) {
		if (!MATCH_MARK(results->rawbuf[offset++], SAMSUNG_BIT_MARK))  return false ;

		if(i == 32) {
			data1 = data;
			data = 0;
		}
		
		if      (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ZERO_SPACE))  data = (data << 1) | 0 ;
		else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_36_MIDDLE_SPACE)) {
			// 16 bit mark. Not a data byte
			i--;
		}
		else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_36_END_SPACE)) {
			// 28 bit mark. Not a data byte
			i--;
		}
		else                                                                return false ;
		offset++;
	}


	// Success
	results->bits        = SAMSUNG_36_BITS;
	results->value       = data1;
	results->valueOver   = data;
	results->decode_type = SAMSUNG;
	return true;
}
#endif

