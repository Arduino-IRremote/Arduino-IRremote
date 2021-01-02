/*
 * ir_Lego.cpp
 *
 *  Contains functions for receiving and sending Kaseikyo/Panasonic IR Protocol in "raw" and standard format with 16 bit Address + 8 bit Data
 *
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

//#define DEBUG // Comment this out for lots of lovely debug output.
#include "IRremote.h"

//==============================================================================
//         L       EEEEEE   EEEE    OOOO
//         L       E       E       O    O
//         L       EEEE    E  EEE  O    O
//         L       E       E    E  O    O
//         LLLLLL  EEEEEE   EEEE    OOOO
//==============================================================================
// from LEGO Power Functions RC Manual 26.02.2010 Version 1.20
// https://github.com/jurriaan/Arduino-PowerFunctions/raw/master/LEGO_Power_Functions_RC_v120.pdf
//
// To ensure correct detection of IR messages six 38 kHz cycles are transmitted as mark.
// Low bit consists of 6 cycles of IR and 10 “cycles” of pause,
// high bit of 6 cycles IR and 21 “cycles” of pause and start bit of 6 cycles IR and 39 “cycles” of pause.
// Low bit range 316 - 526 us
// High bit range 526 – 947 us
// Start/stop bit range 947 – 1579 us

// If tm is the maximum message length (16ms) and Ch is the channel number, then
// The delay before transmitting the first message is: (4 – Ch)*tm
// The time from start to start for the next 2 messages is: 5*tm
// The time from start to start for the following messages is: (6 + 2*Ch)*tm

// Supported Devices
// LEGO Power Functions IR Receiver 8884

// MSB first, 1 start bit + 8 bit control + 4 bit data + 4 bit parity + 1 stop bit.
#define LEGO_CONTROL_BITS        8
#define LEGO_COMMAND_BITS        4
#define LEGO_PARITY_BITS         4

#define LEGO_BITS                (LEGO_CONTROL_BITS + LEGO_COMMAND_BITS + LEGO_PARITY_BITS)

#define LEGO_HEADER_MARK         158    //  6 cycles
#define LEGO_HEADER_SPACE        1026   // 39 cycles

#define LEGO_BIT_MARK            158    //  6 cycles
#define LEGO_ONE_SPACE           553    // 21 cycles
#define LEGO_ZERO_SPACE          263    // 10 cycles

#define LEGO_AUTO_REPEAT_PERIOD_MIN 110000 // Every frame is auto repeated 5 times.
#define LEGO_AUTO_REPEAT_PERIOD_MAX 230000 // space for channel 3

//+=============================================================================
//
bool IRrecv::decodeLegoPowerFunctions() {

    // Check header "mark"
    if (!MATCH_MARK(results.rawbuf[1], LEGO_HEADER_MARK)) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check we have enough data - +4 for initial gap, start bit mark and space + stop bit mark
    if (results.rawlen != (2 * LEGO_BITS) + 4) {
        DBG_PRINT("LEGO: ");
        DBG_PRINT("Data length=");
        DBG_PRINT(results.rawlen);
        DBG_PRINTLN(" is not 36");
        return false;
    }
    // Check header "space"
    if (!MATCH_SPACE(results.rawbuf[2], LEGO_HEADER_SPACE)) {
        DBG_PRINT("LEGO: ");
        DBG_PRINTLN("Header space length is wrong");
        return false;
    }

    if (!decodePulseDistanceData(LEGO_BITS, 3, LEGO_BIT_MARK, LEGO_ONE_SPACE, LEGO_ZERO_SPACE, true)) {
        DBG_PRINT("LEGO: ");
        DBG_PRINTLN("Decode failed");
        return false;
    }

    // Stop bit
    if (!MATCH_MARK(results.rawbuf[3 + (2 * LEGO_BITS)], LEGO_BIT_MARK)) {
        DBG_PRINT("LEGO: ");
        DBG_PRINTLN("Stop bit verify failed");
        return false;
    }

    // Success
    uint16_t tDecodedValue = results.value;
    uint8_t tToggleEscapeChannel = tDecodedValue >> 12;
    uint8_t tModeAddress = (tDecodedValue >> 8) & 0xF;
    uint8_t tData = (tDecodedValue >> 4) & 0xF; // lego calls this field "data"
    uint8_t tParityReceived = tDecodedValue & 0xF;

    uint8_t tParityComputed = tToggleEscapeChannel ^ tModeAddress ^ tData;

    // parity check
    if (tParityReceived != tParityComputed) {
        DBG_PRINT("LEGO: ");
        DBG_PRINT("Parity is not correct. expected=0x");
        DBG_PRINT(tParityComputed, HEX);
        DBG_PRINT(" received=0x");
        DBG_PRINT(tParityReceived, HEX);
        DBG_PRINT(", raw=0x");
        DBG_PRINT(tDecodedValue, HEX);
        DBG_PRINT(", 3 nibbles are 0x");
        DBG_PRINT(tToggleEscapeChannel, HEX);
        DBG_PRINT(", 0x");
        DBG_PRINT(tModeAddress, HEX);
        DBG_PRINT(", 0x");
        DBG_PRINTLN(tData, HEX);
        return false;
    }

    /*
     * Check for autorepeat (should happen 4 times for one press)
     */
    if (results.rawbuf[0] < (LEGO_AUTO_REPEAT_PERIOD_MAX / MICROS_PER_TICK)) {
        decodedIRData.flags = IRDATA_FLAGS_IS_AUTO_REPEAT;
    }
    decodedIRData.address = tDecodedValue >> 8;
    decodedIRData.command = tData;
    decodedIRData.protocol = LEGO_PF;
    decodedIRData.numberOfBits = LEGO_BITS;

    return true;
}
#ifdef DEBUG
namespace {
void logFunctionParameters(uint16_t data, bool repeat) {
    DBG_PRINT("sendLegoPowerFunctions(data=");
    DBG_PRINT(data);
    DBG_PRINT(", repeat=");
    DBG_PRINTLN(repeat?"true)" : "false)");
}
} // anonymous namespace
#endif // DEBUG

/*
 * Here toggle and escape bits are set to 0
 */
void IRsend::sendLegoPowerFunctions(uint8_t aChannel, uint8_t aMode, uint8_t aCommand, bool aDoRepeat5Times) {
    aChannel &= 0x0F; // allow toggle and escape bits too
    aCommand &= 0x0F;
    aMode &= 0x0F;
    uint8_t tParity = aChannel ^ aMode ^ aCommand;
    uint16_t tRawData = (((aChannel << 4) | aMode) << 8) | (aCommand << 4) | tParity;
    sendLegoPowerFunctions(tRawData, aChannel, aDoRepeat5Times);
}

void IRsend::sendLegoPowerFunctions(uint16_t aRawData, bool aDoRepeat5Times) {
    sendLegoPowerFunctions(aRawData, (aRawData >> 12) & 0x3, aDoRepeat5Times);
}

void IRsend::sendLegoPowerFunctions(uint16_t aRawData, uint8_t aChannel, bool aDoRepeat5Times) {
    enableIROut(38);
    unsigned long tStartMillis = millis();

    DBG_PRINT("aRawData=0x");
    DBG_PRINTLN(aRawData, HEX);

    uint8_t tNumberOfCommands = 1;
    if (aDoRepeat5Times) {
        tNumberOfCommands = 5;
    }
// required for repeat timing, see http://www.hackvandedam.nl/blog/?page_id=559
    uint8_t tRepeatPeriod = 110 + (aChannel * 40); // from 110 to 230

    while (tNumberOfCommands > 0) {
        // Header
        mark(LEGO_HEADER_MARK);
        space(LEGO_HEADER_SPACE);

        sendPulseDistanceWidthData(LEGO_BIT_MARK, LEGO_ONE_SPACE, LEGO_BIT_MARK, LEGO_ZERO_SPACE, aRawData, LEGO_BITS, true); // MSB first

        mark(LEGO_BIT_MARK); // Stop bit
        space(0); // Always end with the LED off

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            // send repeated command with a fixed space gap
            delay((tStartMillis + tRepeatPeriod) - millis());
            tStartMillis = millis();
        }
    }
}
