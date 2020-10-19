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
    int offset = 1; // Skip first space

    // Check we have the right amount of data  +3 for start bit mark and space + stop bit mark
    if (irparams.rawlen <= (2 * LG_BITS) + 3)
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

    if (!decodePulseDistanceData(LG_BITS, offset, LG_BIT_MARK, LG_ONE_SPACE, LG_ZERO_SPACE)) {
        return false;
    }
    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * LG_BITS)], LG_BIT_MARK)) {
        DBG_PRINT("Stop bit verify failed");
        return false;
    }

    // Success
    results.bits = LG_BITS;
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

    mark(LG_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

