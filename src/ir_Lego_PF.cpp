#include "IRremote.h"
#include "ir_Lego_PF_BitStreamEncoder.h"

//==============================================================================
//    L       EEEEEE   EEEE    OOOO
//    L       E       E       O    O
//    L       EEEE    E  EEE  O    O
//    L       E       E    E  O    O    LEGO Power Functions
//    LLLLLL  EEEEEE   EEEE    OOOO     Copyright (c) 2016 Philipp Henkel
//==============================================================================

// Supported Devices
// LEGO® Power Functions IR Receiver 8884
/*
 * Lego Power Functions receive.
 * As per document
 * http://cache.lego.com/Media/Download/PowerfunctionsElementSpecsDownloads/otherfiles/download9FC026117C091015E81EC28101DACD4E/8884RemoteControlIRReceiver_Download.pdf

 * Receives the 16 bit protocol. It can be decoded with the  Open Powerfunctions code
 * https://bitbucket.org/tinkerer_/openpowerfunctionsrx
 */

//+=============================================================================
//
#if SEND_LEGO_PF
#if DEBUG
namespace {
void logFunctionParameters(uint16_t data, bool repeat) {
  DBG_PRINT("sendLegoPowerFunctions(data=");
  DBG_PRINT(data);
  DBG_PRINT(", repeat=");
  DBG_PRINTLN(repeat?"true)" : "false)");
}
} // anonymous namespace
#endif // DEBUG

void IRsend::sendLegoPowerFunctions(uint16_t data, bool repeat) {
#if DEBUG
  ::logFunctionParameters(data, repeat);
#endif // DEBUG

    enableIROut(38);
    static LegoPfBitStreamEncoder bitStreamEncoder;
    bitStreamEncoder.reset(data, repeat);
    do {
        mark(bitStreamEncoder.getMarkDuration());
        space_long(bitStreamEncoder.getPauseDuration());
    } while (bitStreamEncoder.next());
}

#endif // SEND_LEGO_PF

#if DECODE_LEGO_PF
/*
 * UNTESTED!!!
 */
#define LEGO_PF_STARTSTOP   1579
#define LEGO_PF_LOWBIT      526
#define LEGO_PF_HIBIT       947
#define LEGO_PF_LOWER       315
#define LEGO_PF_BITS        16  // The number of bits in the command

bool IRrecv::decodeLegoPowerFunctions() {
    unsigned long data = 0;  // Somewhere to build our code
    DBG_PRINTLN(results.rawlen, DEC);
    // Check we have the right amount of data
    if (irparams.rawlen != (2 * LEGO_PF_BITS) + 4)
        return false;

    DBG_PRINTLN("Attempting Lego Power Functions Decode");

    uint16_t desired_us = (results.rawbuf[1] + results.rawbuf[2]) * MICROS_PER_TICK;
    DBG_PRINT("PF desired_us = ");
    DBG_PRINTLN(desired_us, DEC);

    if (desired_us > LEGO_PF_HIBIT && desired_us <= LEGO_PF_STARTSTOP) {
        DBG_PRINTLN("Found PF Start Bit");
        int offset = 3;
        for (int i = 0; i < LEGO_PF_BITS; i++) {
            desired_us = (results.rawbuf[offset] + results.rawbuf[offset + 1]) * MICROS_PER_TICK;

            DBG_PRINT("PF desired_us = ");
            DBG_PRINTLN(desired_us, DEC);
            if (desired_us >= LEGO_PF_LOWER && desired_us <= LEGO_PF_LOWBIT) {
                DBG_PRINTLN("PF 0");
                data = (data << 1) | 0;
            } else if (desired_us > LEGO_PF_LOWBIT && desired_us <= LEGO_PF_HIBIT) {
                DBG_PRINTLN("PF 1");
                data = (data << 1) | 1;
            } else {
                DBG_PRINTLN("PF Failed");
                return false;
            }
            offset += 2;
        }

        desired_us = (results.rawbuf[offset]) * MICROS_PER_TICK;

        DBG_PRINT("PF END desired_us = ");
        DBG_PRINTLN(desired_us, DEC);
        if (desired_us < LEGO_PF_LOWER) {
            DBG_PRINTLN("Found PF End Bit");
            DBG_PRINTLN(data, BIN);

            // Success
            results.bits = LEGO_PF_BITS;
            results.value = data;
            results.decode_type = LEGO_PF;
            return true;
        }
    }
    return false;
}
#endif
