/*
 * ir_NEC.cpp
 *
 *  Contains functions for receiving and sending NEC IR Protocol in "raw" and standard format with 16 or 8 bit Address and 8 bit Data
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

//#define DEBUG // Activate this  for lots of lovely debug output.
#include "IRremote.h"
#include "LongUnion.h"

//==============================================================================
//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//==============================================================================
// see http://www.hifi-remote.com/wiki/index.php?title=DecodeIR#Samsung
// LSB first, 1 start bit + 16 bit address + 16,32,20 bit data + 1 stop bit.
// repeats are like NEC but with 2 stop bits

#define SAMSUNG_ADDRESS_BITS        16
#define SAMSUNG_COMMAND16_BITS      16
#define SAMSUNG_COMMAND32_BITS      32
#define SAMSUNG_BITS                (SAMSUNG_ADDRESS_BITS + SAMSUNG_COMMAND16_BITS)
#define SAMSUNG48_BITS              (SAMSUNG_ADDRESS_BITS + SAMSUNG_COMMAND32_BITS)

#define SAMSUNG_UNIT                550
#define SAMSUNG_HEADER_MARK         (8 * SAMSUNG_UNIT) // 4400
#define SAMSUNG_HEADER_SPACE        (8 * SAMSUNG_UNIT) // 4400
#define SAMSUNG_BIT_MARK            SAMSUNG_UNIT
#define SAMSUNG_ONE_SPACE           (3 * SAMSUNG_UNIT) // 1650
#define SAMSUNG_ZERO_SPACE          SAMSUNG_UNIT
#define SAMSUNG_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.

//+=============================================================================
/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 */
void IRsend::sendSamsungRepeat() {
    enableIROut(38);
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);
    mark(SAMSUNG_BIT_MARK);
    space(SAMSUNG_ZERO_SPACE);
    mark(SAMSUNG_BIT_MARK);
    space(0); // Always end with the LED off
}

void IRsend::sendSamsungStandard(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats) {
    // Set IR carrier frequency
    enableIROut(38);

    unsigned long tStartMillis = millis();

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Address
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aAddress,
    SAMSUNG_ADDRESS_BITS, false);

    // Command

    // send 8 address bits and then 8 inverted address bits LSB first
    aCommand = aCommand & 0xFF;
    aCommand = ((~aCommand) << 8) | aCommand;

    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aCommand,
    SAMSUNG_COMMAND16_BITS, false);

    // Footer
    mark(SAMSUNG_BIT_MARK);
    space(0);  // Always end with the LED off

    for (uint8_t i = 0; i < aNumberOfRepeats; ++i) {
        // send repeat in a 110 ms raster
        delay((tStartMillis + (SAMSUNG_REPEAT_PERIOD / 1000)) - millis());
        tStartMillis = millis();
        // send repeat
        sendSamsungRepeat();
    }
}

//+=============================================================================
#if defined(USE_STANDARD_DECODE)

bool IRrecv::decodeSamsung() {

    // Check header "mark"
    if (!MATCH_MARK(results.rawbuf[1], SAMSUNG_HEADER_MARK)) {
        return false;
    }

    // Check for repeat
    if ((results.rawlen == 6) && MATCH_SPACE(results.rawbuf[1], SAMSUNG_HEADER_MARK)
            && MATCH_MARK(results.rawbuf[2], SAMSUNG_HEADER_SPACE)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.address = lastDecodedAddress;
        decodedIRData.command = lastDecodedCommand;
        return true;
    }

    // Check we have enough data (68). +4 for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != (2 * SAMSUNG_BITS) + 4) {
        DBG_PRINT("Samsung: ");
        DBG_PRINT("Data length=");
        DBG_PRINT(results.rawlen);
        DBG_PRINTLN(" is not 68");
        return false;
    }
    // Check header "space"
    if (!MATCH_SPACE(results.rawbuf[2], SAMSUNG_HEADER_SPACE)) {
        DBG_PRINT("Samsung: ");
        DBG_PRINTLN("Header space length is wrong");
        return false;
    }

    if (results.rawlen == (2 * SAMSUNG48_BITS) + 4) {
        // decode address
        if (!decodePulseDistanceData(SAMSUNG_ADDRESS_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, false)) {
            DBG_PRINT("Samsung: ");
            DBG_PRINTLN("Decode failed");
            return false;
        }
        decodedIRData.address = results.value;

        // decode 32 bit command
        if (!decodePulseDistanceData(SAMSUNG_COMMAND32_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, false)) {
            DBG_PRINT("Samsung: ");
            DBG_PRINTLN("Decode failed");
            return false;
        }
        LongUnion tValue;
        tValue.ULong = results.value;
        // receive 2 * 8 bits then 8 inverted bits LSB first
        if (tValue.UByte.HighByte != (uint8_t) (~(tValue.UByte.MidHighByte))
                && tValue.UByte.MidLowByte != (uint8_t) (~(tValue.UByte.LowByte))) {
            decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
        }
        decodedIRData.command = tValue.UByte.HighByte << 8 | tValue.UByte.MidLowByte;
        decodedIRData.numberOfBits = SAMSUNG48_BITS;

    } else {
        if (!decodePulseDistanceData(SAMSUNG_BITS, 3, SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_ZERO_SPACE, false)) {
            DBG_PRINT("Samsung: ");
            DBG_PRINTLN("Decode failed");
            return false;
        }
        decodedIRData.address = results.value & 0xFFFF;

        WordUnion tCommand;
        tCommand.UWord = results.value >> SAMSUNG_ADDRESS_BITS;
        if (tCommand.UByte.LowByte == (uint8_t) (~tCommand.UByte.HighByte)) {
            // 8 bit command protocol
            decodedIRData.command = tCommand.UByte.LowByte; // first 8 bit
        } else {
            // 16 bit command protocol
            decodedIRData.command = tCommand.UWord; // first 16 bit
        }
        decodedIRData.numberOfBits = SAMSUNG_BITS;
    }

    decodedIRData.protocol = SAMSUNG;

    return true;
}

#else

#warning "Old decoder functions decodeSAMSUNG() and decodeSAMSUNG(decode_results *aResults) are enabled. Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable new version of decodeSamsung() instead."

bool IRrecv::decodeSAMSUNG() {
    return decodeSamsung();
}
bool IRrecv::decodeSamsung() {
    unsigned int offset = 1;  // Skip first space

    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], SAMSUNG_HEADER_MARK)) {
        return false;
    }
    offset++;

// Check for repeat -- like a NEC repeat
    if ((results.rawlen == 4) && MATCH_SPACE(results.rawbuf[offset], 2250)
            && MATCH_MARK(results.rawbuf[offset + 1], SAMSUNG_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER | IRDATA_FLAGS_IS_REPEAT;
        decodedIRData.protocol = SAMSUNG;
        return true;
    }
    if (results.rawlen < (2 * SAMSUNG_BITS) + 4) {
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
    decodedIRData.protocol = SAMSUNG;
    decodedIRData.flags = IRDATA_FLAGS_IS_OLD_DECODER;
    return true;
}

bool IRrecv::decodeSAMSUNG(decode_results *aResults) {
    bool aReturnValue = decodeSAMSUNG();
    *aResults = results;
    return aReturnValue;
}
#endif

void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
    sendSamsung(data, nbits);
}

void IRsend::sendSamsung(uint32_t aData, uint8_t aNumberOfBits) {
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(SAMSUNG_HEADER_MARK);
    space(SAMSUNG_HEADER_SPACE);

    // Data
    sendPulseDistanceWidthData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK, SAMSUNG_ZERO_SPACE, aData, aNumberOfBits);

    // Footer
    mark(SAMSUNG_BIT_MARK);
    space(0);  // Always end with the LED off
}
