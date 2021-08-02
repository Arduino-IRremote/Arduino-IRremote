#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for DEBUG_PRINT

// MagiQuest added by E. Stuart Hicks <ehicks@binarymagi.com>
// Based off the Magiquest fork of Arduino-IRremote by mpflaga
// https://github.com/mpflaga/Arduino-IRremote/
//==============================================================================
//
//
//                            M A G I Q U E S T
//
//
//==============================================================================

// MagiQuest packet is both Wand ID and magnitude of swish and flick
union magiquest_t {
    uint64_t llword;
    struct {
        uint16_t magnitude;
        uint32_t wand_id;
        uint8_t padding;
        uint8_t scrap;  // just to pad the struct out to 64 bits so we can union with llword
    } cmd;
};

#define MAGIQUEST_MAGNITUDE_BITS   (sizeof(uint16_t) * 8)   // magiquest_t.cmd.magnitude
#define MAGIQUEST_WAND_ID_BITS     (sizeof(uint32_t) * 8)   // magiquest_t.cmd.wand_id

#define MAGIQUEST_BITS        (MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS)     // The number of bits in the command itself
#define MAGIQUEST_PERIOD      1150   // Length of time a full MQ "bit" consumes (1100 - 1200 usec)
/*
 * 0 = 25% mark & 75% space across 1 period
 *     1150 * 0.25 = 288 usec mark
 *     1150 - 288 = 862 usec space
 * 1 = 50% mark & 50% space across 1 period
 *     1150 * 0.5 = 575 usec mark
 *     1150 - 575 = 575 usec space
 */
#define MAGIQUEST_UNIT          (MAGIQUEST_PERIOD / 4)

#define MAGIQUEST_ONE_MARK      (2 * MAGIQUEST_UNIT) // 576
#define MAGIQUEST_ONE_SPACE     (2 * MAGIQUEST_UNIT) // 576
#define MAGIQUEST_ZERO_MARK     MAGIQUEST_UNIT
#define MAGIQUEST_ZERO_SPACE    (3 * MAGIQUEST_UNIT) // 864

//+=============================================================================
//
void IRsend::sendMagiQuest(uint32_t wand_id, uint16_t magnitude) {

    // Set IR carrier frequency
    enableIROut(38);

    // 2 start bits
    sendPulseDistanceWidthData(
        MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE,
        0, 2, PROTOCOL_IS_MSB_FIRST
	);

    // Data
    sendPulseDistanceWidthData(
        MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE,
        wand_id, MAGIQUEST_WAND_ID_BITS, PROTOCOL_IS_MSB_FIRST
	);
    sendPulseDistanceWidthData(
        MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE,
        magnitude, MAGIQUEST_MAGNITUDE_BITS, PROTOCOL_IS_MSB_FIRST,
		SEND_STOP_BIT
	);
}

//+=============================================================================
//
/*
 * decodes a 32 bit result, which is not really compatible with standard decoder layout
 */
bool IRrecv::decodeMagiQuest() {
    magiquest_t data;  // Somewhere to build our code
    unsigned int offset = 1;  // Skip the gap reading

    unsigned int mark_;
    unsigned int space_;
    unsigned int ratio_;

#ifdef DEBUG
    char bitstring[(2 * MAGIQUEST_BITS) + 6];
    memset(bitstring, 0, sizeof(bitstring));
#endif

    DEBUG_PRINTLN("MagiQuest: Entered decodeMagiQuest()");
    TRACE_PRINTLN("MagiQuest: Entered decodeMagiQuest()");

    // Check we have enough data (102), + 6 for 2 start and 1 stop bit
    if (decodedIRData.rawDataPtr->rawlen != (2 * MAGIQUEST_BITS) + 6) {
    	DEBUG_PRINT("MagiQuest: Bad packet length - got ");
    	DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
    	DEBUG_PRINTLN(", expected (48 * 2) + 6 = 102");
        return false;
    }

    // Read the bits in
    data.llword = 0;
    while (offset + 1 < decodedIRData.rawDataPtr->rawlen) {
        mark_ = decodedIRData.rawDataPtr->rawbuf[offset++];
        space_ = decodedIRData.rawDataPtr->rawbuf[offset++];
        ratio_ = space_ / mark_;

        DEBUG_PRINT("MagiQuest: mark=");
        DEBUG_PRINT(mark_ * MICROS_PER_TICK);
        DEBUG_PRINT(" space=");
        DEBUG_PRINT(space_ * MICROS_PER_TICK);
        DEBUG_PRINT(" ratio=");
        DEBUG_PRINTLN(ratio_);

        if (matchMark(space_ + mark_, MAGIQUEST_PERIOD)) {
            if (ratio_ > 1) {
                // It's a 0
                data.llword <<= 1;
#ifdef DEBUG
                bitstring[(offset / 2) - 1] = '0';
#endif
            } else {
                // It's a 1
                data.llword = (data.llword << 1) | 1;
#ifdef DEBUG
                bitstring[(offset / 2) - 1] = '1';
#endif
            }
        } else {
            DEBUG_PRINTLN("MATCH_MARK failed");
            return false;
        }
    }
#ifdef DEBUG
    DEBUG_PRINTLN(bitstring);
#endif

    // Success
    decodedIRData.protocol = MAGIQUEST;
    decodedIRData.numberOfBits = offset / 2;
    decodedIRData.flags = IRDATA_FLAGS_EXTRA_INFO;
    decodedIRData.extra = data.cmd.magnitude;
    decodedIRData.decodedRawData = data.cmd.wand_id;

    return true;
}
