#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                            COMMODORE AMIGA CD-TV
//==============================================================================

//==============================================================================
//                           CCCC  DDDD   TTTTT V       V
//                          C      D   D    T    V     V
//                          C      D   D    T     V   V
//                          C      D   D    T      V V
//                           CCCC  DDDD     T       V
//==============================================================================

//==============================================================================
//                                 MEASUREMENTS
//==============================================================================

/*Encoding: UNKNOWN
	Code : 72A03D6B(32 bits)
	Timing[51] :
	+8900, -4450 + 400, -1200 + 350, -400 + 400, -400
	+ 400, -400 + 400, -1200 + 400, -400 + 350, -450
	+ 350, -400 + 400, -400 + 400, -350 + 450, -400
	+ 350, -450 + 350, -400 + 400, -1200 + 400, -1200
	+ 350, -1200 + 400, -450 + 350, -1200 + 400, -1200
	+ 350, -1200 + 400, -1200 + 350, -1250 + 350, -1200
	+ 400, -1200 + 350
	unsigned int rawData[51] = { 8900,4450, 400,1200, 350,400, 400,400, 400,400, 400,1200, 400,400, 350,450, 350,400, 400,400, 400,350, 450,400, 350,450, 350,400, 400,1200, 400,1200, 350,1200, 400,450, 350,1200, 400,1200, 350,1200, 400,1200, 350,1250, 350,1200, 400,1200, 350 }; // UNKNOWN 72A03D6B
*/

// NOTE: The array dump above begins at index = 1; therefore offset = 1 in the code below

//==============================================================================

#define CDTV_BITS          24
// timing intervals in usec
#define CDTV_HDR_MARK    8850			// start burst
#define CDTV_HDR_SPACE   4450			// pause after start
#define CDTV_BIT_MARK     350			// pulse
#define CDTV_ONE_SPACE   1250			// receive a '1'
#define CDTV_ZERO_SPACE   450			// receive a '0'
#define CDTV_RPT_SPACE   2250			// repeat signal
// message sizes measured in raw pulses
#define CDTV_RAW_REPEAT_LENGTH	   4    // CDTV_HDR_MARK + CDTV_HDR_SPACE + CDTV_BIT_MARK
#define CDTV_RAW_SIGNAL_LENGTH	   52   // CDTV_HDR_MARK + CDTV_HDR_SPACE + CDTV_BITS * (CDTV_BIT_MARK + CDTV_ZERO_SPACE | CDTV_ONE_SPACE)

//+=============================================================================
#if SEND_CDTV
void  IRsend::sendCDTV(unsigned long data, int nbits) {

	// set IR carrier frequency
	enableIROut(40);

	// send header
	mark(CDTV_HDR_MARK);
	space(CDTV_HDR_SPACE);

	// send data
	for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
		if (data & mask) {
			mark(CDTV_BIT_MARK);
			space(CDTV_ONE_SPACE);
		} else {
			mark(CDTV_BIT_MARK);
			space(CDTV_ZERO_SPACE);
		}
	}

	// send footer
	mark(CDTV_BIT_MARK);
	// always end with the LED off
	space(0);

}
#endif

//+=============================================================================
// CDTV have a repeat signal that is 4-bits long [#FFFFFF]
//
#if DECODE_CDTV
bool  IRrecv::decodeCDTV(decode_results *results) {

	unsigned long  data   = 0;    // we decode into this field; start with empty placeholder
	int            offset = 1;	  // skip first entry since it has not a lot of meaning

	// check if header mark is within range
	if (!MATCH_MARK(results->rawbuf[offset], CDTV_HDR_MARK)) {
		return false;
	}
	offset++;	// -> offset[2]

	// check for 4-bit repeat signal
	if ((irparams.rawlen == CDTV_RAW_REPEAT_LENGTH)
		&& MATCH_SPACE(results->rawbuf[offset], CDTV_RPT_SPACE)
		&& MATCH_MARK(results->rawbuf[offset+1], CDTV_BIT_MARK)) {
		results->bits = 4;
		results->value = 0xFFFFFF;
		results->decode_type = CDTV;
		return true;
	}

	// check on minimal raw length for decoding the 24-bit signal correctly
	if (irparams.rawlen < CDTV_RAW_SIGNAL_LENGTH) {
		return false;
	}

	// check header space
	if (!MATCH_SPACE(results->rawbuf[offset], CDTV_HDR_SPACE)) {
		return false;
	}
	offset++;	// -> offset[3]

	// build the data
	for (int i = 0; i < CDTV_BITS; i++) {
		// check if data mark is available; otherwise break
		if (!MATCH_MARK(results->rawbuf[offset], CDTV_BIT_MARK)) {
			return false;
		}
		offset++;    // -> offset[4->28]
		// append this bit
		if (MATCH_SPACE(results->rawbuf[offset], CDTV_ONE_SPACE)) {
			data = (data << 1) | 1;
		} else if (MATCH_SPACE(results->rawbuf[offset], CDTV_ZERO_SPACE)) {
			data = (data << 1) | 0;
		} else {
			return false;
		}
		offset++;
	}

	// validate checksum by comparing lower 12-bits with inverted higher 12-bits
	unsigned long lo = data & 0xFFF;		// extract lower 12-bit
	unsigned long hi = data >> 12;			// extract higher 12-bit

	// success if (low XOR high == 0xFFF)
	if (lo^hi == 0xFFF) {
		results->bits = CDTV_BITS;
		results->value = data;
		results->decode_type = CDTV;
		return true;
	}

	return false;

}
#endif
