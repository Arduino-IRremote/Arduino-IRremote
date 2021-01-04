#include "IRremote.h"

//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================
// MSB first, timing and repeat is like LG but 28 data bits
// MSB! first, 1 start bit + 8 bit address + 16 bit command + 4 bit parity + 1 stop bit.
#define LG_ADDRESS_BITS          8
#define LG_COMMAND_BITS         16
#define LG_CHECKSUM_BITS         4
#define LG_BITS                 (LG_ADDRESS_BITS + LG_COMMAND_BITS + LG_CHECKSUM_BITS) // 28

#define LG_UNIT                 560 // like LG

#define LG_HEADER_MARK          (16 * LG_UNIT) // 9000
#define LG_HEADER_SPACE         (8 * LG_UNIT)  // 4500

#define LG_BIT_MARK             LG_UNIT
#define LG_ONE_SPACE            (3 * LG_UNIT)  // 1690
#define LG_ZERO_SPACE           LG_UNIT

#define LG_REPEAT_HEADER_SPACE  (4 * LG_UNIT)  // 2250
#define LG_REPEAT_PERIOD        110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.

//+=============================================================================
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendLGRepeat() {
    enableIROut(38);
    mark(LG_HEADER_MARK);
    space(LG_REPEAT_HEADER_SPACE);
    mark(LG_BIT_MARK);
    space(0); // Always end with the LED off
}

/*
 * Repeat commands should be sent in a 110 ms raster.
 * There is NO delay after the last sent repeat!
 */
void IRsend::sendLGStandard(uint8_t aAddress, uint16_t aCommand, uint8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(38);

    unsigned long tStartMillis = millis();
    // Header
    mark(LG_HEADER_MARK);
    space(LG_HEADER_SPACE);

    uint32_t tData = ((uint32_t) aAddress << (LG_COMMAND_BITS + LG_CHECKSUM_BITS)) | (aCommand << LG_CHECKSUM_BITS);

    /*
     * My guess of the checksum
     */
    uint8_t tChecksum = 0;
    uint16_t tTempForChecksum = aCommand;
    for (int i = 0; i < 4; ++i) {
        tChecksum += tTempForChecksum & 0xF; // add low nibble
        tTempForChecksum >>= 4; // shift by a nibble
    }
    tData |= tChecksum;

    // MSB first
    sendPulseDistanceWidthData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE, tData, LG_BITS, true);

    mark(LG_BIT_MARK); // Stop bit
    space(0); // Always end with the LED off

    for (uint8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        delay((tStartMillis + (LG_REPEAT_PERIOD / 1000)) - millis());
        tStartMillis = millis();
        // send repeat
        sendLGRepeat();
    }
}

//+=============================================================================
// LGs has a repeat like NEC
//
#if defined(USE_STANDARD_DECODE)
bool IRrecv::decodeLG() {

    // Check header "mark"
    if (!MATCH_MARK(results.rawbuf[1], LG_HEADER_MARK)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check for repeat
    if ((results.rawlen == 4) && MATCH_SPACE(results.rawbuf[2], LG_REPEAT_HEADER_SPACE)
            && MATCH_MARK(results.rawbuf[3], LG_BIT_MARK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.address = lastDecodedAddress;
        decodedIRData.command = lastDecodedCommand;
        return true;
    }

    // Check we have enough data - +4 for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != (2 * LG_BITS) + 4) {
        DBG_PRINT(F("LG: "));
        DBG_PRINT(F("Data length="));
        DBG_PRINT(results.rawlen);
        DBG_PRINTLN(F(" is not 60"));
        return false;
    }
    // Check header "space"
    if (!MATCH_SPACE(results.rawbuf[2], LG_HEADER_SPACE)) {
        DBG_PRINT(F("LG: "));
        DBG_PRINTLN(F("Header space length is wrong"));
        return false;
    }

    if (!decodePulseDistanceData(LG_BITS, 3, LG_BIT_MARK, LG_ONE_SPACE, LG_ZERO_SPACE, true)) {
        DBG_PRINT(F("LG: "));
        DBG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[3 + (2 * LG_BITS)], LG_BIT_MARK)) {
        DBG_PRINT(F("LG: "));
        DBG_PRINTLN(F("Stop bit verify failed"));
        return false;
    }

    // Success
    decodedIRData.command = (results.value >> LG_CHECKSUM_BITS) & 0xFFFF;
    decodedIRData.address = results.value >> (LG_COMMAND_BITS + LG_CHECKSUM_BITS); // first 8 bit

    /*
     * My guess of the checksum
     */
    uint8_t tChecksum = 0;
    uint16_t tTempForChecksum = decodedIRData.command;
    for (int i = 0; i < 4; ++i) {
        tChecksum += tTempForChecksum & 0xF; // add low nibble
        tTempForChecksum >>= 4; // shift by a nibble
    }
    // Parity check
    if (tChecksum != (results.value & 0xF)) {
        DBG_PRINT(F("LG: "));
        DBG_PRINT("4 bit checksum is not correct. expected=0x");
        DBG_PRINT(tChecksum, HEX);
        DBG_PRINT(" received=0x");
        DBG_PRINT((results.value & 0xF), HEX);
        DBG_PRINT(" data=0x");
        DBG_PRINTLN(decodedIRData.command, HEX);
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
    }

    decodedIRData.protocol = LG;
    decodedIRData.numberOfBits = LG_BITS;

    return true;
}
#else

#warning "Old decoder functions decodeLG() and decodeLG(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeLG() instead."

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
    decodedIRData.address = results.value >> (LG_COMMAND_BITS + LG_CHECKSUM_BITS);
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
#endif

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
    space(0);// Always end with the LED off
}

