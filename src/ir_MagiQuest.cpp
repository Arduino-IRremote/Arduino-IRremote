#include <Arduino.h>
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
#define MAGIQUEST_PADDING_BITS     (sizeof(uint8_t) * 8)    // magiquest_t.cmd.padding

#define MAGIQUEST_PERIOD      1150   // Length of time a full MQ "bit" consumes (1100 - 1200 usec)
#define MAGIQUEST_BITS        (MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS)   // Size of the command itself

// The total size of a packet is the sum of all 3 expected fields * 2 to support start/stop bits
#define MAGIQUEST_PACKET_SIZE (2 * (MAGIQUEST_BITS + MAGIQUEST_PADDING_BITS))

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
<<<<<<< HEAD
 * decodes a 56 bit result, which is not really compatible with standard decoder layout
=======
 * decodes a 32 bit result, which is not really compatible with standard decoder layout
>>>>>>> 91f986e (a bit of cleanup & added my email address)
 */
bool IRrecv::decodeMagiQuest() {
    magiquest_t data;  // Somewhere to build our code
    unsigned int offset = 1;  // Skip the gap reading

    unsigned int mark_;
    unsigned int space_;
    unsigned int ratio_;

#ifdef DEBUG
    char bitstring[(MAGIQUEST_PACKET_SIZE+1)];
    bitstring[MAGIQUEST_PACKET_SIZE] = '\0';
#endif

    // Check we have the right amount of data
    if (decodedIRData.rawDataPtr->rawlen != MAGIQUEST_PACKET_SIZE) {
    	DEBUG_PRINT("MagiQuest: Bad packet length - got ");
    	DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
    	DEBUG_PRINT(", expected ");
    	DEBUG_PRINTLN(MAGIQUEST_PACKET_SIZE);
        return false;
    }

    // Read the bits in
    data.llword = 0;
    while (offset < (MAGIQUEST_PACKET_SIZE-1)) {
        mark_ = decodedIRData.rawDataPtr->rawbuf[offset++];
        space_ = decodedIRData.rawDataPtr->rawbuf[offset++];
        ratio_ = space_ / mark_;

        TRACE_PRINT("MagiQuest: mark=");
        TRACE_PRINT(mark_ * MICROS_PER_TICK);
        TRACE_PRINT(" space=");
        TRACE_PRINT(space_ * MICROS_PER_TICK);
        TRACE_PRINT(" ratio=");
        TRACE_PRINTLN(ratio_);

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
    DEBUG_PRINTLN(bitstring);

    // Success
    decodedIRData.protocol = MAGIQUEST;
    decodedIRData.numberOfBits = offset / 2;
    decodedIRData.flags = IRDATA_FLAGS_EXTRA_INFO;
    decodedIRData.extra = data.cmd.magnitude;
    decodedIRData.decodedRawData = data.cmd.wand_id;

    return true;
}
