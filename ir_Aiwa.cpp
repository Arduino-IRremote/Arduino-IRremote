#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                         AAA   IIIII  W   W   AAA
//                        A   A    I    W   W  A   A
//                        AAAAA    I    W W W  AAAAA
//                        A   A    I    W W W  A   A
//                        A   A  IIIII   WWW   A   A
//==============================================================================

// Baszed off the RC-T501 RCU
// Lirc file http://lirc.sourceforge.net/remotes/aiwa/RC-T501

#define AIWA_RC_T501_HZ            38
#define AIWA_RC_T501_BITS          15
#define AIWA_RC_T501_PRE_BITS      26
#define AIWA_RC_T501_POST_BITS      1
#define AIWA_RC_T501_SUM_BITS    (AIWA_RC_T501_PRE_BITS + AIWA_RC_T501_BITS + AIWA_RC_T501_POST_BITS)
#define AIWA_RC_T501_HDR_MARK    8800
#define AIWA_RC_T501_HDR_SPACE   4500
#define AIWA_RC_T501_BIT_MARK     500
#define AIWA_RC_T501_ONE_SPACE    600
#define AIWA_RC_T501_ZERO_SPACE  1700

//+=============================================================================
#if SEND_AIWA_RC_T501
void  IRsend::sendAiwaRCT501 (int code)
{
	unsigned long  pre = 0x0227EEC0;  // 26-bits
	int            mask;

	// Set IR carrier frequency
	enableIROut(AIWA_RC_T501_HZ);

	// Header
	mark(AIWA_RC_T501_HDR_MARK);
	space(AIWA_RC_T501_HDR_SPACE);

	// Send "pre" data
	for (unsigned long  mask = 1UL << (26 - 1);  mask;  mask >>= 1) {
		mark(AIWA_RC_T501_BIT_MARK);
		if (pre & mask)  space(AIWA_RC_T501_ONE_SPACE) ;
		else             space(AIWA_RC_T501_ZERO_SPACE) ;
	}

//-v- THIS CODE LOOKS LIKE IT MIGHT BE WRONG - CHECK!
//    it only send 15bits and ignores the top bit
//    then uses TOPBIT which is 0x80000000 to check the bit code
//    I suspect TOPBIT should be changed to 0x00008000

	// Skip first code bit
	code <<= 1;
	// Send code
	for (int  i = 0;  i < 15;  i++) {
		mark(AIWA_RC_T501_BIT_MARK);
		if (code & 0x80000000)  space(AIWA_RC_T501_ONE_SPACE) ;
		else                    space(AIWA_RC_T501_ZERO_SPACE) ;
		code <<= 1;
	}

//-^- THIS CODE LOOKS LIKE IT MIGHT BE WRONG - CHECK!

	// POST-DATA, 1 bit, 0x0
	mark(AIWA_RC_T501_BIT_MARK);
	space(AIWA_RC_T501_ZERO_SPACE);

	mark(AIWA_RC_T501_BIT_MARK);
	space(0);
}
#endif

//+=============================================================================
#if DECODE_AIWA_RC_T501
bool  IRrecv::decodeAiwaRCT501 (decode_results *results)
{
	int  data   = 0;
	int  offset = 1;

	// Check SIZE
	if (irparams.rawlen < 2 * (AIWA_RC_T501_SUM_BITS) + 4)  return false ;

	// Check HDR Mark/Space
	if (!MATCH_MARK (results->rawbuf[offset++], AIWA_RC_T501_HDR_MARK ))  return false ;
	if (!MATCH_SPACE(results->rawbuf[offset++], AIWA_RC_T501_HDR_SPACE))  return false ;

	offset += 26;  // skip pre-data - optional
	while(offset < irparams.rawlen - 4) {
		if (MATCH_MARK(results->rawbuf[offset], AIWA_RC_T501_BIT_MARK))  offset++ ;
		else                                                             return false ;

		// ONE & ZERO
		if      (MATCH_SPACE(results->rawbuf[offset], AIWA_RC_T501_ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], AIWA_RC_T501_ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                                     break ;  // End of one & zero detected
		offset++;
	}

	results->bits = (offset - 1) / 2;
	if (results->bits < 42)  return false ;

	results->value       = data;
	results->decode_type = AIWA_RC_T501;
	return true;
}
#endif
