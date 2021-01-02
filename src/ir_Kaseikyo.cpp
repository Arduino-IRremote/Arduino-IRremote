/*
 * ir_Kaseikyo.cpp
 *
 *  Contains functions for receiving and sending Kaseikyo/Panasonic IR Protocol in "raw" and standard format with 16 bit Address + 8 bit Data
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

//#define DEBUG // Comment this out for lots of lovely debug output.
#include "IRremote.h"

//==============================================================================
//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC
//==============================================================================
// see: http://www.hifi-remote.com/johnsfine/DecodeIR.html#Panasonic
// see: http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
// The first two (8-bit) bytes are always 2 and 32 (These identify Panasonic within the Kaseikyo standard)
// The next two bytes are 4 independent 4-bit fields or Device and Subdevice
// The second to last byte is the function and the last byte is and xor of the three bytes before it.
// 0_______ 1_______  2______ 3_______ 4_______ 5
// 76543210 76543210 76543210 76543210 76543210 76543210
// 00000010 00100000 Dev____  Sub Dev  Fun____  XOR( B2, B3, B4)

// LSB first, start bit + 16 Vendor + 4 Parity(of vendor) + 4 Genre1 + 4 Genre2 + 10 Command + 2 ID + 8 Parity + stop bit
//
#define KASEIKYO_VENDOR_ID_BITS     16
#define PANASONIC_VENDOR_ID_CODE    0x2002
#define SHARP_VENDOR_ID_CODE        0x5AAA
#define DENON_VENDOR_ID_CODE        0x3254
#define JVC_VENDOR_ID_CODE          0x0103
#define KASEIKYO_ADDRESS_BITS       16
#define KASEIKYO_COMMAND_BITS       8
#define KASEIKYO_PARITY_BITS        8
#define KASEIKYO_BITS               (KASEIKYO_VENDOR_ID_BITS + KASEIKYO_ADDRESS_BITS + KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS)
#define KASEIKYO_UNIT               432 // Pronto 0x70 / 0x10

#define KASEIKYO_HEADER_MARK        (8 * KASEIKYO_UNIT) // 3456
#define KASEIKYO_HEADER_SPACE       (4 * KASEIKYO_UNIT) // 1728

#define KASEIKYO_BIT_MARK           KASEIKYO_UNIT
#define KASEIKYO_ONE_SPACE          (3 * KASEIKYO_UNIT) // 1296
#define KASEIKYO_ZERO_SPACE         KASEIKYO_UNIT

#define KASEIKYO_REPEAT_PERIOD      130000

// for old decoder
#define KASEIKYO_DATA_BITS          32

//+=============================================================================
/*
 * Send with LSB first
 * Address is sub-device << 8 + device
 */
void IRsend::sendKaseikyoStandard(uint16_t aAddress, uint8_t aCommand, uint16_t aVendorCode, uint8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(37); // 36.7kHz is the correct frequency

    uint8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartMillis = millis();

        // Header
        mark(KASEIKYO_HEADER_MARK);
        space(KASEIKYO_HEADER_SPACE);

        // Vendor ID
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aVendorCode,
        KASEIKYO_VENDOR_ID_BITS, false);

        // Address (device and subdevice)
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aAddress,
        KASEIKYO_ADDRESS_BITS, false);

        // Command
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aCommand,
        KASEIKYO_COMMAND_BITS, false);

        // send xor of last 3 bytes
        sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE,
                (aCommand ^ (aAddress & 0xFF) ^ (aAddress >> 8)), KASEIKYO_PARITY_BITS, false);

        // Footer
        mark(KASEIKYO_BIT_MARK);
        space(0);  // Always end with the LED off

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command in a fixed raster
            delay((tStartMillis + KASEIKYO_REPEAT_PERIOD / 1000) - millis());
        }
    }
}

void IRsend::sendPanasonicStandard(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    sendKaseikyoStandard(aAddress, aCommand, PANASONIC_VENDOR_ID_CODE, aNumberOfRepeats);
}

bool IRrecv::decodeKaseikyo() {
    unsigned int offset = 1;

    decode_type_t tProtocol;
    // Check we have enough data (100)- +4 for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != ((2 * KASEIKYO_BITS) + 4)) {
        return false;
    }

    if (!MATCH_MARK(results.rawbuf[offset], KASEIKYO_HEADER_MARK)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Header mark length is wrong");
        return false;
    }

    offset++;
    if (!MATCH_MARK(results.rawbuf[offset], KASEIKYO_HEADER_SPACE)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Header space length is wrong");
        return false;
    }
    offset++;

    // decode Vendor ID
    if (!decodePulseDistanceData(KASEIKYO_VENDOR_ID_BITS, offset, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_ZERO_SPACE,
            false)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Vendor ID decode failed");
        return false;
    }

    uint16_t tVendorId = results.value;
    if (results.value == PANASONIC_VENDOR_ID_CODE) {
        tProtocol = PANASONIC;
    } else if (results.value == SHARP_VENDOR_ID_CODE) {
        tProtocol = SHARP;
    } else if (results.value == DENON_VENDOR_ID_CODE) {
        tProtocol = DENON;
    } else if (results.value == JVC_VENDOR_ID_CODE) {
        tProtocol = JVC;
    } else {
        tProtocol = KASEIKYO;
    }

    // decode address (device and subdevice)
    offset += (2 * KASEIKYO_VENDOR_ID_BITS);
    if (!decodePulseDistanceData(KASEIKYO_ADDRESS_BITS, offset, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_ZERO_SPACE,
            false)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Address decode failed");
        return false;
    }
    decodedIRData.address = results.value;
    uint8_t tParity = (decodedIRData.address >> 8) ^ (decodedIRData.address & 0xFF);

    if (tProtocol == KASEIKYO) {
        // Include vendor ID in address
        decodedIRData.address |= ((uint32_t) tVendorId) << KASEIKYO_ADDRESS_BITS;
    }

    // decode command + parity
    offset += (2 * KASEIKYO_ADDRESS_BITS);
    if (!decodePulseDistanceData(KASEIKYO_COMMAND_BITS + KASEIKYO_PARITY_BITS, offset, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE,
    KASEIKYO_ZERO_SPACE, false)) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINTLN("Command + parity decode failed");
        return false;
    }
    decodedIRData.command = results.value & 0xFF;
    tParity ^= decodedIRData.command;

    if ((results.value >> KASEIKYO_COMMAND_BITS) != tParity) {
        DBG_PRINT("Kaseikyo: ");
        DBG_PRINT("Parity is not correct. expected=0x");
        DBG_PRINT(tParity, HEX);
        DBG_PRINT(" received=0x");
        DBG_PRINT(results.value, HEX);
        DBG_PRINT(" address=0x");
        DBG_PRINT(decodedIRData.address, HEX);
        DBG_PRINT(" command=0x");
        DBG_PRINTLN(decodedIRData.command, HEX);
        return false;
    }

    // check for repeat
    if (results.rawbuf[0] < (KASEIKYO_REPEAT_PERIOD / MICROS_PER_TICK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
    }

    results.value = 0; // no sensible raw data here
    decodedIRData.protocol = tProtocol;

    decodedIRData.numberOfBits = KASEIKYO_BITS;

    return true;
}

//+=============================================================================
#if !defined(USE_STANDARD_DECODE)
bool IRrecv::decodePanasonic() {
    decodedIRData.flags |= IRDATA_FLAGS_IS_OLD_DECODER;
    unsigned int offset = 1;

    if (results.rawlen < (2 * KASEIKYO_BITS) + 2) {
        return false;
    }

    if (!MATCH_MARK(results.rawbuf[offset], KASEIKYO_HEADER_MARK)) {
        return false;
    }
    offset++;
    if (!MATCH_MARK(results.rawbuf[offset], KASEIKYO_HEADER_SPACE)) {
        return false;
    }
    offset++;

    // decode address
    if (!decodePulseDistanceData(KASEIKYO_ADDRESS_BITS + KASEIKYO_DATA_BITS, offset, KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE,
    KASEIKYO_ZERO_SPACE)) {
        return false;
    }

    decodedIRData.protocol = PANASONIC;
    results.bits = KASEIKYO_BITS;

    return true;
}

bool IRrecv::decodePanasonic(decode_results *aResults) {
    bool aReturnValue = decodePanasonic();
    *aResults = results;
    return aReturnValue;
}
#endif

void IRsend::sendPanasonic(uint16_t aAddress, uint32_t aData) {
    // Set IR carrier frequency
    enableIROut(37); // 36.7kHz is the correct frequency

    // Header
    mark(KASEIKYO_HEADER_MARK);
    space(KASEIKYO_HEADER_SPACE);

    // Address
    sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aAddress,
    KASEIKYO_ADDRESS_BITS);

    // Data
    sendPulseDistanceWidthData(KASEIKYO_BIT_MARK, KASEIKYO_ONE_SPACE, KASEIKYO_BIT_MARK, KASEIKYO_ZERO_SPACE, aData,
    KASEIKYO_DATA_BITS);

    // Footer
    mark(KASEIKYO_BIT_MARK);
    space(0);  // Always end with the LED off
}

