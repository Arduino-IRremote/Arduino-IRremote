#include "IRremote.h"

//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================
// MSB first, timing is like NEC but 28 data bits
#define LG_ADDRESS_BITS          8
#define LG_COMMAND_BITS         16
#define LG_PARITY_BITS           4
#define LG_BITS                 (LG_ADDRESS_BITS + LG_COMMAND_BITS + LG_PARITY_BITS) // 28

#define LG_UNIT                 560

#define LG_HEADER_MARK          (16 * LG_UNIT) // 9000
#define LG_HEADER_SPACE         (8 * LG_UNIT)  // 4500

#define LG_BIT_MARK             LG_UNIT
#define LG_ONE_SPACE            (3 * LG_UNIT)  // 1690
#define LG_ZERO_SPACE           LG_UNIT

//+=============================================================================
bool IRrecv::decodeLG() {
    unsigned int offset = 1; // Skip first space

    // Check we have enough data (60) - +4 for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != (2 * LG_BITS) + 4) {
        return false;
    }

    // Initial mark/space
    if (!MATCH_MARK(results.rawbuf[offset], LG_HEADER_MARK)) {
        return false;
    }
    offset++;

    if (!MATCH_SPACE(results.rawbuf[offset], LG_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(LG_BITS, offset, LG_BIT_MARK, LG_ONE_SPACE, LG_ZERO_SPACE, true)) {
        return false;
    }
    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * LG_BITS)], LG_BIT_MARK)) {
        DBG_PRINT("Stop bit verify failed");
        return false;
    }

    // Success
    // no parity check yet :-(
    decodedIRData.address = results.value >> (LG_COMMAND_BITS + LG_PARITY_BITS);
    decodedIRData.command = (results.value >> LG_COMMAND_BITS) & 0xFFFF;

    decodedIRData.numberOfBits = LG_BITS;
    decodedIRData.protocol = LG;
    return true;
}

bool IRrecv::decodeLG(decode_results *aResults) {
    bool aReturnValue = decodeLG();
    *aResults = results;
    return aReturnValue;
}

//+=============================================================================
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

