#include "IRremote.h"

//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================

#define LG_BITS 28

#define LG_HEADER_MARK  8400
#define LG_HEADER_SPACE 4200
#define LG_BIT_MARK      600
#define LG_ONE_SPACE    1600
#define LG_ZERO_SPACE    550

//+=============================================================================
#if DECODE_LG
bool IRrecv::decodeLG() {
    long data = 0;
    int offset = 1; // Skip first space

    // Check we have the right amount of data
    if (irparams.rawlen < (2 * LG_BITS) + 1)
        return false;

    // Initial mark/space
    if (!MATCH_MARK(results.rawbuf[offset], LG_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (!MATCH_SPACE(results.rawbuf[offset], LG_HEADER_SPACE)) {
        return false;
    }
    offset++;

    data = decodePulseDistanceData(LG_BITS, offset, LG_BIT_MARK, LG_ONE_SPACE, LG_ZERO_SPACE);
//    for (int i = 0; i < LG_BITS; i++) {
//        if (!MATCH_MARK(results.rawbuf[offset], LG_BIT_MARK)) {
//            return false;
//        }
//        offset++;
//
//        if (MATCH_SPACE(results.rawbuf[offset], LG_ONE_SPACE)) {
//            data = (data << 1) | 1;
//        } else if (MATCH_SPACE(results.rawbuf[offset], LG_ZERO_SPACE)) {
//            data = (data << 1) | 0;
//        } else {
//            return false;
//        }
//        offset++;
//    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset], LG_BIT_MARK)) {
        return false;
    }

    // Success
    results.bits = LG_BITS;
    results.value = data;
    results.decode_type = LG;
    return true;
}
bool IRrecv::decodeLG(decode_results *aResults) {
    bool aReturnValue = decodeLG();
    *aResults = results;
    return aReturnValue;
}
#endif

//+=============================================================================
#if SEND_LG
void IRsend::sendLG(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(LG_HEADER_MARK);
    space(LG_HEADER_SPACE);
//    mark(LG_BIT_MARK);

    // Data
    sendPulseDistanceWidthData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, data, nbits);
//    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
//        if (data & mask) {
//            space(LG_ONE_SPACE);
//            mark(LG_BIT_MARK);
//        } else {
//            space(LG_ZERO_SPACE);
//            mark(LG_BIT_MARK);
//        }
//    }

    mark(LG_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

