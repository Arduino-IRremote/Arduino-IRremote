#include "IRremote.h"

//==============================================================================
//               W   W  H   H  Y   Y N   N TTTTT EEEEE  RRRRR
//               W   W  H   H   Y Y  NN  N   T   E      R   R
//               W W W  HHHHH    Y   N N N   T   EEE    RRRR
//               W W W  H   H    Y   N  NN   T   E      R  R
//                WWW   H   H    Y   N   N   T   EEEEE  R   R
//==============================================================================

#define WHYNTER_BITS            32
#define WHYNTER_HEADER_MARK   2850
#define WHYNTER_HEADER_SPACE  2850
#define WHYNTER_BIT_MARK       750
#define WHYNTER_ONE_SPACE     2150
#define WHYNTER_ZERO_SPACE     750

//+=============================================================================
#if SEND_WHYNTER
void IRsend::sendWhynter(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Start
    mark(WHYNTER_BIT_MARK);
    space(WHYNTER_ZERO_SPACE);

    // Header
    mark(WHYNTER_HEADER_MARK);
    space(WHYNTER_HEADER_SPACE);

    // Data
    sendPulseDistanceWidthData(WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_BIT_MARK, WHYNTER_ZERO_SPACE, data, nbits);
//    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
//        if (data & mask) {
//            mark(WHYNTER_ONE_MARK);
//            space(WHYNTER_ONE_SPACE);
//        } else {
//            mark(WHYNTER_ZERO_MARK);
//            space(WHYNTER_ZERO_SPACE);
//        }
//    }

// Footer
    mark(WHYNTER_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_WHYNTER
bool IRrecv::decodeWhynter() {
    int offset = 1;  // skip initial space

    // Check we have the right amount of data +5 for (start bit + header) mark and space + stop bit mark
    if (results.rawlen <= (2 * WHYNTER_BITS) + 5) {
        return false;
    }

    // Sequence begins with a bit mark and a zero space
    if (!MATCH_MARK(results.rawbuf[offset], WHYNTER_BIT_MARK)) {
        return false;
    }
    offset++;

    if (!MATCH_SPACE(results.rawbuf[offset], WHYNTER_ZERO_SPACE)) {
        return false;
    }
    offset++;

    // header mark and space
    if (!MATCH_MARK(results.rawbuf[offset], WHYNTER_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (!MATCH_SPACE(results.rawbuf[offset], WHYNTER_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(WHYNTER_BITS, offset, WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_ZERO_SPACE)) {
        return false;
    }

    // trailing mark / stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * WHYNTER_BITS)], WHYNTER_BIT_MARK)) {
        DBG_PRINT("Stop bit verify failed");
        return false;
    }

    // Success
    results.bits = WHYNTER_BITS;
    results.decode_type = WHYNTER;
    return true;
}

bool IRrecv::decodeWhynter(decode_results *aResults) {
    bool aReturnValue = decodeWhynter();
    *aResults = results;
    return aReturnValue;
}
#endif

