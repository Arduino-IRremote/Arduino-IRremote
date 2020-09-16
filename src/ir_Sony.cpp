#include "IRremote.h"

//==============================================================================
//                           SSSS   OOO   N   N  Y   Y
//                          S      O   O  NN  N   Y Y
//                           SSS   O   O  N N N    Y
//                              S  O   O  N  NN    Y
//                          SSSS    OOO   N   N    Y
//==============================================================================

#define SONY_BITS                   12
#define SONY_HEADER_MARK          2400
#define SONY_HEADER_SPACE          600
#define SONY_ONE_MARK             1200
#define SONY_ZERO_MARK             600
#define SONY_RPT_LENGTH          45000
#define SONY_DOUBLE_SPACE_USECS    500  // usually see 713 - not using ticks as get number wrap around

//+=============================================================================
#if SEND_SONY
void IRsend::sendSony(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(40);

    // Header
    mark(SONY_HEADER_MARK);
    space(SONY_HEADER_SPACE);

    sendPulseDistanceWidthData(SONY_ONE_MARK, SONY_HEADER_SPACE, SONY_ZERO_MARK, SONY_HEADER_SPACE, data, nbits);
//    // Data - Pulse width coding
//    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
//        if (data & mask) {
//            mark(SONY_ONE_MARK);
//            space(SONY_HEADER_SPACE);
//        } else {
//            mark(SONY_ZERO_MARK);
//            space(SONY_HEADER_SPACE);
//        }
//    }

    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_SONY
bool IRrecv::decodeSony() {
    long data = 0;
    unsigned int offset = 0;  // Dont skip first space, check its size

    if (results.rawlen < (2 * SONY_BITS) + 2) {
        return false;
    }

    // Some Sony's deliver repeats fast after first
    // unfortunately can't spot difference from of repeat from two fast clicks
    if (results.rawbuf[offset] * MICROS_PER_TICK < SONY_DOUBLE_SPACE_USECS) {
        DBG_PRINTLN("IR Gap found");
        results.bits = 0;
        results.value = REPEAT;
        results.isRepeat = true;
        results.decode_type = UNKNOWN;
        return true;
    }
    offset++;

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], SONY_HEADER_MARK)) {
        return false;
    }
    offset++;

    while (offset + 1 < results.rawlen) {
        if (!MATCH_SPACE(results.rawbuf[offset], SONY_HEADER_SPACE)) {
            break;
        }
        offset++;

        if (MATCH_MARK(results.rawbuf[offset], SONY_ONE_MARK)) {
            data = (data << 1) | 1;
        } else if (MATCH_MARK(results.rawbuf[offset], SONY_ZERO_MARK)) {
            data = (data << 1) | 0;
        } else {
            return false;
        }
        offset++;
    }

    // Success
    results.bits = (offset - 1) / 2;
    if (results.bits < 12) {
        results.bits = 0;
        return false;
    }
    results.value = data;
    results.decode_type = SONY;
    return true;
}
bool IRrecv::decodeSony(decode_results *aResults) {
    bool aReturnValue = decodeSony();
    *aResults = results;
    return aReturnValue;
}
#endif

