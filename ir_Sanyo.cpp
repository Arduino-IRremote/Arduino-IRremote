#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                      SSSS   AAA   N   N  Y   Y   OOO
//                     S      A   A  NN  N   Y Y   O   O
//                      SSS   AAAAA  N N N    Y    O   O
//                         S  A   A  N  NN    Y    O   O
//                     SSSS   A   A  N   N    Y     OOO
//==============================================================================

// I think this is a Sanyo decoder:  Serial = SA 8650B
// Looks like Sony except for timings, 48 chars of data and time/space different

#define SANYO_BITS                   12
#define SANYO_HDR_MARK	           3500  // seen range 3500
#define SANYO_HDR_SPACE	            950  // seen 950
#define SANYO_ONE_MARK	           2400  // seen 2400
#define SANYO_ZERO_MARK             700  // seen 700
#define SANYO_DOUBLE_SPACE_USECS    800  // usually ssee 713 - not using ticks as get number wrapround
#define SANYO_RPT_LENGTH          45000

//+=============================================================================
#if DECODE_SANYO
bool  IRrecv::decodeSanyo (decode_results *results)
{
	long  data   = 0;
	int   offset = 0;  // Skip first space  <-- CHECK THIS!

	if (irparams.rawlen < (2 * SANYO_BITS) + 2)  return false ;

#if 0
	// Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
	Serial.print("IR Gap: ");
	Serial.println( results->rawbuf[offset]);
	Serial.println( "test against:");
	Serial.println(results->rawbuf[offset]);
#endif

	// Initial space
	if (results->rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
		//Serial.print("IR Gap found: ");
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = SANYO;
		return true;
	}
	offset++;

	// Initial mark
	if (!MATCH_MARK(results->rawbuf[offset++], SANYO_HDR_MARK))  return false ;

	// Skip Second Mark
	if (!MATCH_MARK(results->rawbuf[offset++], SANYO_HDR_MARK))  return false ;

	while (offset + 1 < irparams.rawlen) {
		if (!MATCH_SPACE(results->rawbuf[offset++], SANYO_HDR_SPACE))  break ;

		if      (MATCH_MARK(results->rawbuf[offset], SANYO_ONE_MARK))   data = (data << 1) | 1 ;
		else if (MATCH_MARK(results->rawbuf[offset], SANYO_ZERO_MARK))  data = (data << 1) | 0 ;
		else                                                            return false ;
		offset++;
	}

	// Success
	results->bits = (offset - 1) / 2;
	if (results->bits < 12) {
		results->bits = 0;
		return false;
	}

	results->value       = data;
	results->decode_type = SANYO;
	return true;
}
#endif
