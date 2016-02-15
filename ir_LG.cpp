#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================

#define LG_BITS 28

#define LG_HDR_MARK 8000
#define LG_HDR_SPACE 4000
#define LG_BIT_MARK 600
#define LG_ONE_SPACE 1600
#define LG_ZERO_SPACE 550
#define LG_RPT_LENGTH 60000

//+=============================================================================
#if DECODE_LG
bool  IRrecv::decodeLG (decode_results *results)
{
    long  data   = 0;
    int   offset = 1; // Skip first space

	// Check we have the right amount of data
    if (irparams.rawlen < (2 * LG_BITS) + 1 )  return false ;

    // Initial mark/space
    if (!MATCH_MARK(results->rawbuf[offset++], LG_HDR_MARK))  return false ;
    if (!MATCH_SPACE(results->rawbuf[offset++], LG_HDR_SPACE))  return false ;

    for (int i = 0;  i < LG_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], LG_BIT_MARK))  return false ;

        if      (MATCH_SPACE(results->rawbuf[offset], LG_ONE_SPACE))   data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset], LG_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                           return false ;
        offset++;
    }

    // Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK))   return false ;

    // Success
    results->bits        = LG_BITS;
    results->value       = data;
    results->decode_type = LG;
    return true;
}
#endif

//+=============================================================================
#if SEND_LG
void  IRsend::sendLG (unsigned long data,  int nbits)
{
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(LG_HDR_MARK);
    space(LG_HDR_SPACE);
    mark(LG_BIT_MARK);

    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
        if (data & mask) {
            space(LG_ONE_SPACE);
            mark(LG_BIT_MARK);
        } else {
            space(LG_ZERO_SPACE);
            mark(LG_BIT_MARK);
        }
    }
    space(0);  // Always end with the LED off
}
#endif

