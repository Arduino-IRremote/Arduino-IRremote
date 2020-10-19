#include "IRremote.h"

//==============================================================================
//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//==============================================================================

#define PANASONIC_ADDRESS_BITS    16
#define PANASONIC_DATA_BITS       32
#define PANASONIC_BITS            (PANASONIC_ADDRESS_BITS + PANASONIC_DATA_BITS)
#define PANASONIC_HEADER_MARK   3502
#define PANASONIC_HEADER_SPACE  1750
#define PANASONIC_BIT_MARK       502
#define PANASONIC_ONE_SPACE     1244
#define PANASONIC_ZERO_SPACE     400

//+=============================================================================
#if SEND_PANASONIC
void IRsend::sendPanasonic(unsigned int address, unsigned long data) {
    // Set IR carrier frequency
    enableIROut(37); // 36.7kHz is the correct frequency

    // Header
    mark(PANASONIC_HEADER_MARK);
    space(PANASONIC_HEADER_SPACE);

    // Address
    sendPulseDistanceWidthData(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE, PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE, address,
    PANASONIC_ADDRESS_BITS);

    // Data
    sendPulseDistanceWidthData(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE, PANASONIC_BIT_MARK,
    PANASONIC_ZERO_SPACE, data, PANASONIC_DATA_BITS);

    // Footer
    mark(PANASONIC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_PANASONIC
bool IRrecv::decodePanasonic() {
    int offset = 1;

    if (results.rawlen < (2 * PANASONIC_BITS) + 2) {
        return false;
    }

    if (!MATCH_MARK(results.rawbuf[offset], PANASONIC_HEADER_MARK)) {
        return false;
    }
    offset++;
    if (!MATCH_MARK(results.rawbuf[offset], PANASONIC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    // decode address
    if (!decodePulseDistanceData(PANASONIC_ADDRESS_BITS, offset, PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE,
    PANASONIC_ZERO_SPACE)) {
        return false;
    }
    results.address = results.value;

    if (!decodePulseDistanceData(PANASONIC_DATA_BITS, offset + PANASONIC_ADDRESS_BITS, PANASONIC_BIT_MARK,
    PANASONIC_ONE_SPACE, PANASONIC_ZERO_SPACE)) {
        return false;
    }

    results.decode_type = PANASONIC;
    results.bits = PANASONIC_BITS;

    return true;
}

bool IRrecv::decodePanasonic(decode_results *aResults) {
    bool aReturnValue = decodePanasonic();
    *aResults = results;
    return aReturnValue;
}
#endif

