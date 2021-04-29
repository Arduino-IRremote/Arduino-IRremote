#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for DEBUG_PRINT

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//               W   W  H   H  Y   Y N   N TTTTT EEEEE  RRRRR
//               W   W  H   H   Y Y  NN  N   T   E      R   R
//               W W W  HHHHH    Y   N N N   T   EEE    RRRR
//               W W W  H   H    Y   N  NN   T   E      R  R
//                WWW   H   H    Y   N   N   T   EEEEE  R   R
//==============================================================================
// Whynter A/C ARC-110WD added by Francesco Meschia
// see https://docs.google.com/spreadsheets/d/1dsr4Jh-nzC6xvSKGpLlPBF0NRwvlpyw-ozg8eZU813w/edit#gid=0

#define WHYNTER_BITS            32
#define WHYNTER_HEADER_MARK   2850
#define WHYNTER_HEADER_SPACE  2850
#define WHYNTER_BIT_MARK       750
#define WHYNTER_ONE_SPACE     2150
#define WHYNTER_ZERO_SPACE     750

//+=============================================================================
void IRsend::sendWhynter(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Start
    mark(WHYNTER_BIT_MARK);
    space(WHYNTER_ZERO_SPACE);

    // Header
    mark(WHYNTER_HEADER_MARK);
    space(WHYNTER_HEADER_SPACE);

    // Data + stop bit
    sendPulseDistanceWidthData(WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_BIT_MARK, WHYNTER_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST,
    SEND_STOP_BIT);
}

//+=============================================================================
bool IRrecv::decodeWhynter() {

    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawDataPtr->rawlen != (2 * WHYNTER_BITS) + 4) {
        return false;
    }

    // Sequence begins with a bit mark and a zero space
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], WHYNTER_BIT_MARK)
            || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], WHYNTER_HEADER_SPACE)) {
        DEBUG_PRINT(F("Whynter: "));
        DEBUG_PRINTLN(F("Header mark or space length is wrong"));
        return false;
    }

    if (!decodePulseDistanceData(WHYNTER_BITS, 3, WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        return false;
    }

    // trailing mark / stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * WHYNTER_BITS)], WHYNTER_BIT_MARK)) {
        DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.numberOfBits = WHYNTER_BITS;
    decodedIRData.protocol = WHYNTER;
    return true;
}

/** @}*/
