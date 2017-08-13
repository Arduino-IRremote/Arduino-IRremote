#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//==============================================================================

#define PANASONIC_BITS          48
#define PANASONIC_HDR_MARK    3502
#define PANASONIC_HDR_SPACE   1750
#define PANASONIC_BIT_MARK     502
#define PANASONIC_ONE_SPACE   1244
#define PANASONIC_ZERO_SPACE   400

//+=============================================================================
#if SEND_PANASONIC
void  IRsend::sendPanasonic (unsigned int address,  unsigned long data)
{
	// Set IR carrier frequency
	enableIROut(35);

	// Header
	mark(PANASONIC_HDR_MARK);
	space(PANASONIC_HDR_SPACE);

	// Address
	for (unsigned long  mask = 1UL << (16 - 1);  mask;  mask >>= 1) {
		mark(PANASONIC_BIT_MARK);
		if (address & mask)  space(PANASONIC_ONE_SPACE) ;
		else                 space(PANASONIC_ZERO_SPACE) ;
    }

	// Data
	for (unsigned long  mask = 1UL << (32 - 1);  mask;  mask >>= 1) {
        mark(PANASONIC_BIT_MARK);
        if (data & mask)  space(PANASONIC_ONE_SPACE) ;
        else              space(PANASONIC_ZERO_SPACE) ;
    }

	// Footer
    mark(PANASONIC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_PANASONIC
bool  IRrecv::decodePanasonic (decode_results *results)
{
    unsigned long long  data   = 0;
    int                 offset = 1;

    if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_HDR_MARK ))  return false ;
    if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_HDR_SPACE))  return false ;

    // decode address
    for (int i = 0;  i < PANASONIC_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_BIT_MARK))  return false ;

        if      (MATCH_SPACE(results->rawbuf[offset],PANASONIC_ONE_SPACE ))  data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset],PANASONIC_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                                 return false ;
        offset++;
    }

    results->value       = (unsigned long)data;
    results->address     = (unsigned int)(data >> 32);
    results->decode_type = PANASONIC;
    results->bits        = PANASONIC_BITS;

    return true;
}
#endif

