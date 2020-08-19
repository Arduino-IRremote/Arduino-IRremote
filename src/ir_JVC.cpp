#include "IRremote.h"

//==============================================================================
//                             JJJJJ  V   V   CCCC
//                               J    V   V  C
//                               J     V V   C
//                             J J     V V   C
//                              J       V     CCCC
//==============================================================================

#define JVC_BITS             16
#define JVC_HEADER_MARK    8400
#define JVC_HEADER_SPACE   4200
#define JVC_BIT_MARK        600
#define JVC_ONE_SPACE      1600
#define JVC_ZERO_SPACE      550
#define JVC_REPEAT_SPACE  50000

//+=============================================================================
// JVC does NOT repeat by sending a separate code (like NEC does).
// The JVC protocol repeats by skipping the header.
// To send a JVC repeat signal, send the original code value
//   and set 'repeat' to true
//
// JVC commands sometimes need to be sent two or three times with 40 to 60 ms pause in between.
//
#if SEND_JVC
void IRsend::sendJVC(unsigned long data, int nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut(38);

    // Only send the Header if this is NOT a repeat command
    if (!repeat) {
        mark(JVC_HEADER_MARK);
        space(JVC_HEADER_SPACE);
    }

    // Data
    sendPulseDistanceWidthData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE, data, nbits);
//    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
//        if (data & mask) {
//            mark(JVC_BIT_MARK);
//            space(JVC_ONE_SPACE);
//        } else {
//            mark(JVC_BIT_MARK);
//            space(JVC_ZERO_SPACE);
//        }
//    }

// Footer
    mark(JVC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_JVC
bool IRrecv::decodeJVC() {
    long data = 0;
    int offset = 1; // Skip first space

    // Check for repeat
    if ((results.rawlen - 1 == 33) && MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)
            && MATCH_MARK(results.rawbuf[results.rawlen - 1], JVC_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        results.decode_type = JVC;
        return true;
    }

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], JVC_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (results.rawlen < (2 * JVC_BITS) + 1) {
        return false;
    }

    // Initial space
    if (!MATCH_SPACE(results.rawbuf[offset], JVC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    data = decodePulseDistanceData(JVC_BITS, offset, JVC_BIT_MARK, JVC_ONE_SPACE, JVC_ZERO_SPACE);
//    for (int i = 0; i < JVC_BITS; i++) {
//        // Check for constant length mark
//        if (!MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)) {
//            return false;
//        }
//        offset++;
//
//        // Check for variable length space indicating a 0 or 1
//        if (MATCH_SPACE(results.rawbuf[offset], JVC_ONE_SPACE)) {
//            data = (data << 1) | 1;
//        } else if (MATCH_SPACE(results.rawbuf[offset], JVC_ZERO_SPACE)) {
//            data = (data << 1) | 0;
//        } else {
//            return false;
//        }
//        offset++;
//    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)) {
        return false;
    }

    // Success
    results.bits = JVC_BITS;
    results.value = data;
    results.decode_type = JVC;

    return true;
}
bool IRrecv::decodeJVC(decode_results *aResults) {
    bool aReturnValue = decodeJVC();
    *aResults = results;
    return aReturnValue;
}
#endif

