//-*- mode: C; c-basic-offset: 8; tab-width: 8; indent-tabs-mode: t; -*-
#include "IRremote.h"

//==============================================================================
//            SSSS  H   H   AAA   RRRR   PPPP      AAA   L      TTTTT
//           S      H   H  A   A  R   R  P   P    A   A  L        T
//            SSS   HHHHH  AAAAA  RRRR   PPPP     AAAAA  L        T
//               S  H   H  A   A  R  R   P        A   A  L        T
//           SSSS   H   H  A   A  R   R  P        A   A  LLLLL    T
//==============================================================================

// This is an alternative protocol to that in ir_Sharp.cpp. It was tested with
// the original Sharp GA538WJSA remote control. LIRC file with codes for this
// remote control: http://lirc.sourceforge.net/remotes/sharp/GA538WJSA
//
// Author: Sergiy Kolesnikov
//

#define SHARP_ALT_RAWLEN                32
#define SHARP_ALT_ADDRESS_BITS           5
#define SHARP_ALT_COMMAND_BITS           8
#define SHARP_ALT_BIT_MARK             150
#define SHARP_ALT_SEND_BIT_MARK        300
#define SHARP_ALT_ONE_SPACE           1750
#define SHARP_ALT_ZERO_SPACE           700
#define SHARP_ALT_REPEAT_SPACE       50000
#define SHARP_ALT_SEND_REPEAT_SPACE  44000
#define SHARP_ALT_TOGGLE_MASK        0x3FF
#define SHARP_ALT_SEND_INVERT_MASK  0x7FE0

//+=============================================================================
#if SEND_SHARP_ALT
void IRsend::sendSharpAltRaw(unsigned int data, int nbits) {
    enableIROut(38);

    for (int n = 0; n < 3; n++) {
        // From LSB to MSB
        sendPulseDistanceWidthData(SHARP_ALT_SEND_BIT_MARK, SHARP_ALT_ONE_SPACE, SHARP_ALT_SEND_BIT_MARK, SHARP_ALT_ZERO_SPACE,
                data, nbits, false);
//        unsigned long mask =  1UL;
//        for (int i = 0; i < nbits; i++) {
//            if (data & mask) {
//                mark(SHARP_ALT_SEND_BIT_MARK);
//                space(SHARP_ALT_ONE_SPACE);
//            } else {
//                mark(SHARP_ALT_SEND_BIT_MARK);
//                space(SHARP_ALT_ZERO_SPACE);
//            }
//            mask <<= 1;
//        }
        mark(SHARP_ALT_BIT_MARK);
        space(SHARP_ALT_SEND_REPEAT_SPACE);
        data = data ^ SHARP_ALT_SEND_INVERT_MASK;
    }
}

void IRsend::sendSharpAlt(uint8_t address, uint8_t command) {
    // 1 = The expansion and the check bits (01).
    unsigned int data = (1 << SHARP_ALT_COMMAND_BITS) | command;
    data = (data << SHARP_ALT_ADDRESS_BITS) | address;

    // (+2) is for the expansion and the check bits 0b01.
    sendSharpAltRaw(data, SHARP_ALT_ADDRESS_BITS + SHARP_ALT_COMMAND_BITS + 2);
}
#endif

//+=============================================================================
#if DECODE_SHARP_ALT
bool IRrecv::decodeSharpAlt() {
    static boolean is_first_repeat = true;

    // Check we have enough data.
    if (results.rawlen < (SHARP_ALT_RAWLEN))
        return false;

    // Check stop mark.
    if (!MATCH_MARK(results.rawbuf[SHARP_ALT_RAWLEN - 1], SHARP_ALT_BIT_MARK))
        return false;

    // Check the "check bit." If this bit is not 0 than it is an inverted
    // frame, which we ignore.
    if (!MATCH_SPACE(results.rawbuf[SHARP_ALT_RAWLEN - 2], SHARP_ALT_ZERO_SPACE))
        return false;

    // Check for repeat.
    long initial_space = ((long) results.rawbuf[0]) * MICROS_PER_TICK;
    if (initial_space <= SHARP_ALT_REPEAT_SPACE) {
        if (!is_first_repeat) {
            results.bits = 0;
            results.value = REPEAT;
            results.isRepeat = true;
            results.decode_type = SHARP;
            return true;
        } else {
            // Ignore the first repeat that always comes after the
            // inverted frame (even if the button was pressed only once).
            is_first_repeat = false;
            return false;
        }
    }

    // Decode bits. SHARP_ALT_RAWLEN-6 because index starts with 0 (-1) and we
    // omit the timings for the stop mark (-1), the check bit (-2), and the
    // expansion bit (-2).
    uint16_t bits = 0;
    for (uint8_t i = SHARP_ALT_RAWLEN - 6; i > 1; i -= 2) {
        if (MATCH_SPACE(results.rawbuf[i], SHARP_ALT_ONE_SPACE)) {
            bits = (bits << 1) | 1;
        } else if (MATCH_SPACE(results.rawbuf[i], SHARP_ALT_ZERO_SPACE)) {
            bits = (bits << 1) | 0;
        } else {
            return false;
        }
    }

    results.bits = SHARP_ALT_ADDRESS_BITS + SHARP_ALT_COMMAND_BITS;
    results.address = (bits & (1 << (SHARP_ALT_ADDRESS_BITS))) - 1;
    results.value = bits >> SHARP_ALT_ADDRESS_BITS; // command
    results.decode_type = SHARP_ALT;
    is_first_repeat = true;
    return true;
}
bool IRrecv::decodeSharpAlt(decode_results *aResults) {
    bool aReturnValue = decodeSharpAlt();
    *aResults = results;
    return aReturnValue;
}
#endif
