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
#ifdef DECODE_LG
long  IRrecv::decodeLG (decode_results *results)
{
    long  data   = 0;
    int   offset = 1; // Skip first space

    // Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], LG_HDR_MARK))  return false ;
    offset++;
    if (irparams.rawlen < 2 * LG_BITS + 1 )  return false ;
    // Initial space
    if (!MATCH_SPACE(results->rawbuf[offset], LG_HDR_SPACE))  return false ;
    offset++;
    for (int i = 0;  i < LG_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK))  return false ;
        offset++;
        if      (MATCH_SPACE(results->rawbuf[offset], LG_ONE_SPACE))   data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset], LG_ZERO_SPACE))  data <<= 1 ;
        else                                                           return false ;
        offset++;
    }

    // Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK))   return false ;

    // Success
    results->bits = LG_BITS;
    results->value = data;
    results->decode_type = LG;
    return true;
}
#endif

