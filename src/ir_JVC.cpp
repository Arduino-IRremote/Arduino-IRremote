#include "IRremote.h"

//==============================================================================
//                             JJJJJ  V   V   CCCC
//                               J    V   V  C
//                               J     V V   C
//                             J J     V V   C
//                              J       V     CCCC
//==============================================================================

// https://www.sbprojects.net/knowledge/ir/jvc.php
// Here it is decoded MSB first!!! It is not corrected in order to be backwards compatible!

#define JVC_BITS             16
#define JVC_HEADER_MARK    8400
#define JVC_HEADER_SPACE   4200
#define JVC_BIT_MARK        600
#define JVC_ONE_SPACE      1600
#define JVC_ZERO_SPACE      550
#define JVC_REPEAT_SPACE  50000 // Not used.

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
    sendPulseDistanceWidthData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE, data, nbits, true);

// Footer
    mark(JVC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_JVC
bool IRrecv::decodeJVC() {
    int offset = 1; // Skip first space

    // Check for repeat
    if ((results.rawlen - 1 == 33) && MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)
            && MATCH_MARK(results.rawbuf[results.rawlen - 1], JVC_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        results.isRepeat = true;
        results.decode_type = JVC;
        return true;
    }

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], JVC_HEADER_MARK)) {
        return false;
    }
    offset++;

    // Check we have enough data - +3 for start bit mark and space + stop bit mark
    if (results.rawlen <= (2 * JVC_BITS) + 3) {
        return false;
    }

    // Initial space
    if (!MATCH_SPACE(results.rawbuf[offset], JVC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(JVC_BITS, offset, JVC_BIT_MARK, JVC_ONE_SPACE, JVC_ZERO_SPACE)) {
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * JVC_BITS)], JVC_BIT_MARK)) {
        DBG_PRINT("Stop bit verify failed");
        return false;
    }

    // Success
    results.bits = JVC_BITS;
    results.decode_type = JVC;

    return true;
}

bool IRrecv::decodeJVC(decode_results *aResults) {
    bool aReturnValue = decodeJVC();
    *aResults = results;
    return aReturnValue;
}
#endif

