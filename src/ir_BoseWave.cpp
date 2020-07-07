#include "IRremote.h"

//==============================================================================
//                           BBBB    OOO    SSSS  EEEEE
//                           B   B  O   O  S      E
//                           BB B   O   O   SSS   EEEE
//                           B   B  O   O      S  E
//                           BBBB    OOO   SSSS   EEEEE
//==============================================================================
//
//                       Bose Wave Radio CD Remote Control
//                    |-------------------------------------|
//                    |   On/Off        Sleep       VolUp   |
//                    |   Play/Pause    Stop       VolDown  |
//                    |      FM          AM          Aux    |
//                    |   Tune Down    Tune Up       Mute   |
//                    |       1           2           3     |
//                    |       4           5           6     |
//                    |-------------------------------------|
//
// Support for Bose Wave Radio CD provided by https://github.com/uvotguy.
//
// This protocol was reverse engineered by capturing IR signals from a working
// remote.  Multiple signals were captured on my oscilloscope, and the timing
// values were averaged.
//
// IR codes are 8 bits.  Transmission starts with a header:  a mark and a space.
// The header is followed by an 8-bit command, where a bit is a mark and a short
// space (1) or a long space (0).  The command is followed by the complement of
// the command (8 bits).  A transmission ends with a short mark.
//
// As seen on my trusty oscilloscope, there is no repeat code.  Instead, when I
// press and hold a button on my remote, it sends a command, makes a 51.2ms space,
// and resends the command, etc, etc.
//
// It may be worth noting that these values do NOT match those in the LIRC
// remote database (http://lirc.sourceforge.net/remotes/bose/).

#define CMD_ON_OFF     0xff
#define CMD_MUTE       0xfe
#define CMD_VOL_UP     0xfd
#define CMD_VOL_DOWN   0xfc
#define CMD_PRESET_6   0xfb
#define CMD_SLEEP      0xfa
#define CMD_FM         0xf9
#define CMD_AUX        0xf8
#define CMD_AM         0xf7
#define CMD_PLAY_PAUSE 0xf6
#define CMD_STOP       0xf5
#define CMD_TUNE_UP    0xf4
#define CMD_TUNE_DOWN  0xf3
#define CMD_PRESET_1   0xf2
#define CMD_PRESET_2   0xf1
#define CMD_PRESET_3   0xf0
#define CMD_PRESET_4   0xef
#define CMD_PRESET_5   0xee

#define BOSEWAVE_BITS           8
#define BOSEWAVE_HDR_MARK    1061
#define BOSEWAVE_HDR_SPACE   1456
#define BOSEWAVE_BIT_MARK     534
#define BOSEWAVE_ONE_SPACE    468
#define BOSEWAVE_ZERO_SPACE  1447
#define BOSEWAVE_END_MARK     614
#define BOSEWAVE_RPT_SPACE  51200

//+=============================================================================
#if SEND_BOSEWAVE
unsigned int rawSignal[35];
void IRsend::sendBoseWave(unsigned char code) {

    int index = 0;
    // Header
    rawSignal[index++] = BOSEWAVE_HDR_MARK;
    rawSignal[index++] = BOSEWAVE_HDR_SPACE;

    // 8 bit command
    for (unsigned char mask = 0x80; mask; mask >>= 1) {
        rawSignal[index++] = BOSEWAVE_BIT_MARK;
        if (code & mask) {
            rawSignal[index++] = BOSEWAVE_ONE_SPACE;
        } else {
            rawSignal[index++] = BOSEWAVE_ZERO_SPACE;
        }
    }

    // 8 bit command complement
    for (unsigned char mask = 0x80; mask; mask >>= 1) {
        rawSignal[index++] = BOSEWAVE_BIT_MARK;
        if (code & mask) {
            rawSignal[index++] = BOSEWAVE_ZERO_SPACE;
        } else {
            rawSignal[index++] = BOSEWAVE_ONE_SPACE;
        }
    }
    // End transmission
    rawSignal[index++] = BOSEWAVE_END_MARK;

    // Transmit
    this->sendRaw(rawSignal, 35, 38);
}
#endif

//+=============================================================================
#if DECODE_BOSEWAVE
bool IRrecv::decodeBoseWave(decode_results *results) {
    unsigned char command = 0;      // Decoded command
    unsigned char complement = 0;   // Decoded command complement

    int index = 0;   // Index in to results array

    DBG_PRINTLN("Decoding Bose Wave ...");

    // Check we have enough data
    if (irparams.rawlen < (2 * BOSEWAVE_BITS * 2) + 3) {
        DBG_PRINT("\tInvalid data length found:  ");
        DBG_PRINTLN(results->rawlen);
        return false;
    }

    // Check header "mark"
    index = 1;
    if (!MATCH_MARK(results->rawbuf[index], BOSEWAVE_HDR_MARK)) {
        DBG_PRINT("\tInvalid Header Mark.  Expecting ");
        DBG_PRINT(BOSEWAVE_HDR_MARK);
        DBG_PRINT(".  Got ");
        DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
        return false;
    }
    index++;

    // Check header "space"
    if (!MATCH_SPACE(results->rawbuf[index], BOSEWAVE_HDR_SPACE)) {
        DBG_PRINT("\tInvalid Header Space.  Expecting ");
        DBG_PRINT(BOSEWAVE_HDR_SPACE);
        DBG_PRINT(".  Got ");
        DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
        return false;
    }
    index++;

    // Decode the data bits
    for (int ii = 7; ii >= 0; ii--) {
        // Check bit "mark".  Mark is always the same length.
        if (!MATCH_MARK(results->rawbuf[index], BOSEWAVE_BIT_MARK)) {
            DBG_PRINT("\tInvalid command Mark.  Expecting ");
            DBG_PRINT(BOSEWAVE_BIT_MARK);
            DBG_PRINT(".  Got ");
            DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
            return false;
        }
        index++;

        // Check bit "space"
        if (MATCH_SPACE(results->rawbuf[index], BOSEWAVE_ONE_SPACE)) {
            command |= (0x01 << ii);
        } else if (MATCH_SPACE(results->rawbuf[index], BOSEWAVE_ZERO_SPACE)) {
            // Nothing to do for zeroes.
        } else {
            DBG_PRINT("\tInvalid command Space.  Got ");
            DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
            return false;
        }
        index++;
    }

    // Decode the command complement bits.  We decode it here as the complement
    // of the complement (0=1 and 1=0) so we can easily compare it to the command.
    for (int ii = 7; ii >= 0; ii--) {
        // Check bit "mark".  Mark is always the same length.
        if (!MATCH_MARK(results->rawbuf[index], BOSEWAVE_BIT_MARK)) {
            DBG_PRINT("\tInvalid complement Mark.  Expecting ");
            DBG_PRINT(BOSEWAVE_BIT_MARK);
            DBG_PRINT(".  Got ");
            DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
            return false;
        }
        index++;

        // Check bit "space"
        if (MATCH_SPACE(results->rawbuf[index], BOSEWAVE_ONE_SPACE)) {
            // Nothing to do.
        } else if (MATCH_SPACE(results->rawbuf[index], BOSEWAVE_ZERO_SPACE)) {
            complement |= (0x01 << ii);
        } else {
            DBG_PRINT("\tInvalid complement Space.  Got ");
            DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
            return false;
        }
        index++;
    }

    if (command != complement) {
        DBG_PRINT("\tComplement is not correct.  Command=0x");
        DBG_PRINT(command, HEX);
        DBG_PRINT("  Complement=0x");
        DBG_PRINTLN(complement, HEX);
        return false;
    } else {
        DBG_PRINTLN("\tValid command");
    }

    // Check end "mark"
    if (MATCH_MARK(results->rawbuf[index], BOSEWAVE_END_MARK) == 0) {
        DBG_PRINT("\tInvalid end Mark.  Got ");
        DBG_PRINTLN(results->rawbuf[index] * MICROS_PER_TICK);
        return false;
    }

    // Success
    results->bits = BOSEWAVE_BITS;
    results->value = command;
    results->decode_type = BOSEWAVE;

    return true;
}
#endif
