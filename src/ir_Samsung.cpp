#include "IRremote.h"

//==============================================================================
//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//==============================================================================

#define SAMSUNG_BITS            32
#define SAMSUNG_HEADER_MARK   4500
#define SAMSUNG_HEADER_SPACE  4500
#define SAMSUNG_BIT_MARK       560
#define SAMSUNG_ONE_SPACE     1600
#define SAMSUNG_ZERO_SPACE     560
#define SAMSUNG_REPEAT_SPACE  2250

//+=============================================================================
#if SEND_SAMSUNG
void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Data
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, data, nbits);

    // Footer
    mark(SAMSUNG_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// SAMSUNGs have a repeat only 4 items long
//
#if DECODE_SAMSUNG
bool IRrecv::decodeSAMSUNG() {
    int offset = 1;  // Skip first space

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], SAMSUNG_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat
    if ((irparams.rawlen == 4) && MATCH_SPACE(results.rawbuf[offset], SAMSUNG_REPEAT_SPACE)
            && MATCH_MARK(results.rawbuf[offset + 1], SAMSUNG_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        results.isRepeat = true;
        results.decode_type = SAMSUNG;
        return true;
    }
    if (irparams.rawlen < (2 * SAMSUNG_BITS) + 4) {
        return false;
    }

// Initial space
    if (!MATCH_SPACE(results.rawbuf[offset], SAMSUNG_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(SAMSUNG_BITS, offset, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE)) {
        return false;
    }

// Success
    results.bits = SAMSUNG_BITS;
    results.decode_type = SAMSUNG;
    return true;
}

bool IRrecv::decodeSAMSUNG(decode_results *aResults) {
    bool aReturnValue = decodeSAMSUNG();
    *aResults = results;
    return aReturnValue;
}
#endif

