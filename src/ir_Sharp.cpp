#include "IRremote.h"

//==============================================================================
//                       SSSS  H   H   AAA   RRRR   PPPP
//                      S      H   H  A   A  R   R  P   P
//                       SSS   HHHHH  AAAAA  RRRR   PPPP
//                          S  H   H  A   A  R  R   P
//                      SSSS   H   H  A   A  R   R  P
//==============================================================================

// Sharp and DISH support by Todd Treece: http://unionbridge.org/design/ircommand
//
// The send function has the necessary repeat built in because of the need to
// invert the signal.
//
// Sharp protocol documentation:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
//
// Here is the LIRC file I found that seems to match the remote codes from the
// oscilloscope:
//   Sharp LCD TV:
//   http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

#define SHARP_BITS             15
#define SHARP_ONE_SPACE      1805
//#define SHARP_ONE_SPACE      1850

#define SHARP_ADDR_BITS         5
#define SHARP_DATA_BITS         8
#define SHARP_BIT_MARK_SEND   250
#define SHARP_BIT_MARK_RECV   150

#define SHARP_ZERO_SPACE      795
#define SHARP_GAP          600000 // Not used.
#define SHARP_REPEAT_SPACE   3000

#define SHARP_TOGGLE_MASK   0x3FF

//+=============================================================================
#if SEND_SHARP
void IRsend::sendSharpRaw(unsigned long data, int nbits) {
    enableIROut(38);

    // Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
    // much more reliable. That's the exact behavior of CD-S6470 remote control.
    for (int n = 0; n < 3; n++) {
        sendPulseDistanceWidthData(SHARP_BIT_MARK_SEND, SHARP_ONE_SPACE, SHARP_BIT_MARK_SEND, SHARP_ZERO_SPACE, data, nbits);
//        for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
//            if (data & mask) {
//                mark (SHARP_BIT_MARK_SEND);
//                space(SHARP_ONE_SPACE);
//            } else {
//                mark (SHARP_BIT_MARK_SEND);
//                space(SHARP_ZERO_SPACE);
//            }
//        }

        mark(SHARP_BIT_MARK_SEND);
        space(SHARP_ZERO_SPACE);
        delay(40);

        data = data ^ SHARP_TOGGLE_MASK;
    }

    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// Sharp send compatible with data obtained through decodeSharp()
//                                                  ^^^^^^^^^^^^^ FUNCTION MISSING!
//
#if SEND_SHARP
void IRsend::sendSharp(unsigned int address, unsigned int command) {
    sendSharpRaw((address << 10) | (command << 2) | 2, SHARP_BITS);
    /*
     * Use this code instead of the line above to be code compatible to the decoded values from decodeSharp
     */
//    //Change address to big-endian (five bits swap place)
//    address = (address & 0x10) >> 4 | (address & 0x01) << 4 | (address & 0x08) >> 2 | (address & 0x02) << 2 | (address & 0x04) ;
//    //Change command to big-endian (eight bit swap place)
//    command = (command & 0xF0) >> 4 | (command & 0x0F) << 4;
//    command = (command & 0xCC) >> 2 | (command & 0x33) << 2;
//    command = (command & 0xAA) >> 1 | (command & 0x55) << 1;
//    sendSharpRaw((address << 10) | (command << 2) | 0, SHARP_BITS);
}

#endif // SEND_SHARP

//+=============================================================================
// Sharp decode function written based on Sharp protocol documentation:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
// Tesded on a DENON AVR-1804 reciever

#if DECODE_SHARP
bool IRrecv::decodeSharp() {
    unsigned long lastData = 0;  // to store last data
    int offset = 1;  //skip long space
    int loops = 1; //number of bursts

    // Check we have the right amount of data
    // Either one burst or three where second is inverted
    // The setting #define _GAP 5000 in IRremoteInt.h will give one burst and possibly three calls to this function
    if (irparams.rawlen == (SHARP_BITS + 1) * 2)
        loops = 1;
    else if (irparams.rawlen == (SHARP_BITS + 1) * 2 * 3)
        loops = 3;
    else
        return false;

    // Check the first mark to see if it fits the SHARP_BIT_MARK_RECV length
    if (!MATCH_MARK(results.rawbuf[offset], SHARP_BIT_MARK_RECV))
        return false;
    //check the first pause and see if it fits the SHARP_ONE_SPACE or SHARP_ZERO_SPACE length
    if (!(MATCH_SPACE(results.rawbuf[offset + 1], SHARP_ONE_SPACE) || MATCH_SPACE(results.rawbuf[offset + 1], SHARP_ZERO_SPACE)))
        return false;

    // Read the bits in
    for (int j = 0; j < loops; j++) {
        if (!decodePulseDistanceData(SHARP_ADDR_BITS, offset, SHARP_BIT_MARK_SEND, SHARP_ONE_SPACE, SHARP_ZERO_SPACE)) {
            return false;
        }
        results.address = results.value;

        if (!decodePulseDistanceData( SHARP_DATA_BITS, offset + SHARP_ADDR_BITS, SHARP_BIT_MARK_SEND, SHARP_ONE_SPACE,
        SHARP_ZERO_SPACE)) {
            return false;
        }

        //skip exp bit (mark+pause), chk bit (mark+pause), mark and long pause before next burst
        offset += 6;

        //Check if last burst data is equal to this burst (lastData already inverted)
        if (lastData != 0 && results.value != lastData)
            return false;
        //save current burst of data but invert (XOR) the last 10 bits (8 data bits + exp bit + chk bit)
        lastData = results.value ^ 0xFF;
    }

// Success
    results.bits = SHARP_BITS;
    results.decode_type = SHARP;
    return true;
}

bool IRrecv::decodeSharp(decode_results *aResults) {
    bool aReturnValue = decodeSharp();
    *aResults = results;
    return aReturnValue;
}
#endif
