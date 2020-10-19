/*
 * ir_NEC.cpp
 *
 *  Contains functions for receiving and sending NEC IR Protocol in "raw" and standard format with 16 bit Address  8bit Data
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

#include "IRremote.h"

//==============================================================================
//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC
//==============================================================================

#define NEC_BITS             32
#define NEC_HEADER_MARK    9000
#define NEC_HEADER_SPACE   4500
#define NEC_BIT_MARK        560
#define NEC_ONE_SPACE      1690
#define NEC_ZERO_SPACE      560
#define NEC_REPEAT_SPACE   2250

//+=============================================================================
#if SEND_NEC || SEND_NEC_STANDARD
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendNECRepeat() {
    enableIROut(38);
    mark(NEC_HEADER_MARK);
    space(NEC_REPEAT_SPACE);
    mark(NEC_BIT_MARK);
    space(0); // Always end with the LED off
}
#endif

//+=============================================================================
#if SEND_NEC
/*
 * Repeat commands should be sent in a 110 ms raster.
 * https://www.sbprojects.net/knowledge/ir/nec.php
 */
void IRsend::sendNEC(uint32_t data, uint8_t nbits, bool repeat) {
    // Set IR carrier frequency
    enableIROut(38);

    if (data == REPEAT || repeat) {
        sendNECRepeat();
        return;
    }

    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);
    // Data
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, data, nbits);

    // Stop bit
    mark(NEC_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if SEND_NEC_STANDARD
/*
 * Repeat commands should be sent in a 110 ms raster.
 * https://www.sbprojects.net/knowledge/ir/nec.php
 */
void IRsend::sendNECStandard(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(38);

    unsigned long tStartMillis = millis();
    // Header
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE);
    // Address 16 bit LSB first
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, aAddress, 16, false);

    // send 8 command bits and then 8 inverted command bits LSB first
    uint16_t tCommand = ((~aCommand) << 8) | aCommand;
    // Command 16 bit LSB first
    sendPulseDistanceWidthData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE, tCommand, 16, false);
    mark(NEC_BIT_MARK); // Stop bit
    space(0); // Always end with the LED off

    for (uint8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        delay((tStartMillis + 110) - millis());
        tStartMillis = millis();
        // send repeat
        sendNECRepeat();
    }
}
#endif
//+=============================================================================
// NECs have a repeat only 4 items long
//
#if DECODE_NEC
bool IRrecv::decodeNEC() {
    int offset = 1;  // Index in to results; Skip first space.

// Check header "mark"
    if (!MATCH_MARK(results.rawbuf[offset], NEC_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat
    if ((results.rawlen == 4) && MATCH_SPACE(results.rawbuf[offset], NEC_REPEAT_SPACE)
            && MATCH_MARK(results.rawbuf[offset + 1], NEC_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        results.isRepeat = true;
        results.decode_type = NEC;
        return true;
    }

// Check we have enough data - +3 for start bit mark and space + stop bit mark
    if (results.rawlen <= (2 * NEC_BITS) + 3) {
        return false;
    }

// Check header "space"
    if (!MATCH_SPACE(results.rawbuf[offset], NEC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    if (!decodePulseDistanceData(NEC_BITS, offset, NEC_BIT_MARK, NEC_ONE_SPACE, NEC_ZERO_SPACE)) {
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * NEC_BITS)], NEC_BIT_MARK)) {
        DBG_PRINT("Stop bit verify failed");
        return false;
    }

// Success
    results.bits = NEC_BITS;
    results.decode_type = NEC;

    return true;
}

bool IRrecv::decodeNEC(decode_results *aResults) {
    bool aReturnValue = decodeNEC();
    *aResults = results;
    return aReturnValue;
}
#endif

//+=============================================================================
// NECs have a repeat only 4 items long
//
#if DECODE_NEC_STANDARD
bool IRrecv::decodeNECStandard() {
    long data = 0;  // We decode in to here; Start with nothing
    int offset = 1;  // Index in to results; Skip first space.

    // Check header "mark"
    if (!MATCH_MARK(results.rawbuf[offset], NEC_HEADER_MARK)) {
        return false;
    }
    offset++;

    // Check for repeat
    if ((results.rawlen == 4) && MATCH_SPACE(results.rawbuf[offset], NEC_REPEAT_SPACE)
            && MATCH_MARK(results.rawbuf[offset + 1], NEC_BIT_MARK)) {
        results.isRepeat = true;
        results.bits = 0;
        return true;
    }

    // Check we have enough data - +3 for start bit mark and space + stop bit mark
    if (results.rawlen <= (2 * NEC_BITS) + 3) {
        return false;
    }
    // Check header "space"
    if (!MATCH_SPACE(results.rawbuf[offset], NEC_HEADER_SPACE)) {
        return false;
    }
    offset++;

    data = decodePulseDistanceData(NEC_BITS, offset, NEC_BIT_MARK, NEC_ONE_SPACE, NEC_ZERO_SPACE, false);

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[offset + (2 * NEC_BITS)], NEC_BIT_MARK)) {
        DBG_PRINT("Stop bit verify failed");
       return false;
    }

    // Success
    uint16_t tCommand = data >> 16;
    uint8_t tCommandNotInverted = tCommand & 0xFF;
    uint8_t tCommandInverted = tCommand >> 8;
    // plausi check for command
    if ((tCommandNotInverted ^ tCommandInverted) != 0xFF) {
        return false;
    }

    results.isRepeat = false;
    results.value = tCommandNotInverted;
    results.bits = NEC_BITS;
    results.address = data & 0xFFFF; // first 16 bit
    results.decode_type = NEC_STANDARD;

    return true;
}
#endif

