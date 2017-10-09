#include "IRremote.h"
#include "IRremoteInt.h"

//====================================================================
//                         RRRR    CCC   AAA
//                         R   R  C     A   A
//                         RRRR   C     AAAAA
//                         R  R   C     A   A
//                         R   R   CCC  A   A
//====================================================================

// Based on protocol description from
//   http://www.sbprojects.net/knowledge/ir/rca.php
//
// RCA is very similar to NEC, so ir_NEC.cpp was used as a template for this
// implementation.

#define RCA_BITS          12
#define RCA_HDR_MARK    4000
#define RCA_HDR_SPACE   4000
#define RCA_BIT_MARK     500
#define RCA_ONE_SPACE   2000
#define RCA_ZERO_SPACE  1000

//+=============================================================================
#if SEND_RCA
void  IRsend::sendRCA (unsigned long data,  int nbits)
{
    // Set IR carrier frequency
    enableIROut(56);

    // Header
    mark(RCA_HDR_MARK);
    space(RCA_HDR_SPACE);

    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
        if (data & mask) {
            mark(RCA_BIT_MARK);
            space(RCA_ONE_SPACE);
        } else {
            mark(RCA_BIT_MARK);
            space(RCA_ZERO_SPACE);
        }
    }
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
        if (data & mask) {
            mark(RCA_BIT_MARK);
            space(RCA_ZERO_SPACE);
        } else {
            mark(RCA_BIT_MARK);
            space(RCA_ONE_SPACE);
        }
    }

    // Footer
    mark(RCA_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_RCA
bool  IRrecv::decodeRCA (decode_results *results)
{
    long  data   = 0;  // We decode in to here; Start with nothing
    int   offset = 1;  // Index in to results; Skip first entry!?

    // Check header "mark"
    if (!MATCH_MARK(results->rawbuf[offset], RCA_HDR_MARK))  return false ;
    offset++;

    // Check we have enough data
    if (irparams.rawlen < (2 * RCA_BITS) + 4)  return false ;

    // Check header "space"
    if (!MATCH_SPACE(results->rawbuf[offset], RCA_HDR_SPACE))  return false ;
    offset++;

    // Build the data
    for (int i = 0;  i < RCA_BITS;  i++) {
        // Check data "mark"
        if (!MATCH_MARK(results->rawbuf[offset], RCA_BIT_MARK))  return false ;
        offset++;
        // Suppend this bit
        if      (MATCH_SPACE(results->rawbuf[offset], RCA_ONE_SPACE ))  data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset], RCA_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                            return false ;
        offset++;
    }

    // RCA transmits the address and command twice, but with the bits
    // inverted the second time. Verify that the two transmissions match.
    for (int i = 0;  i < RCA_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset], RCA_BIT_MARK))  return false ;
        offset++;

        if (MATCH_SPACE(results->rawbuf[offset], RCA_ONE_SPACE )) {
            if (data & (1 << (11-i))) {
                return false;
            }
        }
        else if (MATCH_SPACE(results->rawbuf[offset], RCA_ZERO_SPACE)) {
            if (!(data & (1 << (11-i)))) {
                return false;
            }
        }
        else {
            return false ;
        }
        offset++;
    }

    // Success
    results->bits        = RCA_BITS;
    results->value       = data;
    results->decode_type = RCA;

    return true;
}
#endif
