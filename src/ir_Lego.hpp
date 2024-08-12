/*
 * ir_Lego.hpp
 *
 *  Contains functions for receiving and sending Lego Power Functions IR Protocol
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2023 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
#ifndef _IR_LEGO_HPP
#define _IR_LEGO_HPP

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//         L       EEEEEE   EEEE    OOOO
//         L       E       E       O    O
//         L       EEEE    E  EEE  O    O
//         L       E       E    E  O    O
//         LLLLLL  EEEEEE   EEEE    OOOO
//==============================================================================
// from LEGO Power Functions RC Manual 26.02.2010 Version 1.20
// https://github.com/jurriaan/Arduino-PowerFunctions/raw/master/LEGO_Power_Functions_RC_v120.pdf
// https://oberguru.net/elektronik/ir/codes/lego_power_functions_train.lircd.conf
// For original LEGO receiver see: https://www.philohome.com/pfrec/pfrec.htm and https://www.youtube.com/watch?v=KCM4Ug1bPrM
//
// To ensure correct detection of IR messages six 38 kHz cycles are transmitted as mark.
// Low bit consists of 6 cycles of IR and 10 �cycles� of pause,
// high bit of 6 cycles IR and 21 �cycles� of pause and start bit of 6 cycles IR and 39 �cycles� of pause.
// Low bit range 316 - 526 us
// High bit range 526 � 947 us
// Start/stop bit range 947 � 1579 us
// If tm is the maximum message length (16ms) and Ch is the channel number, then
// The delay before transmitting the first message is: (4 � Ch)*tm
// The time from start to start for the next 2 messages is: 5*tm
// The time from start to start for the following messages is: (6 + 2*Ch)*tm
// Supported Devices
// LEGO Power Functions IR Receiver 8884
// MSB first, 1 start bit + 4 bit channel, 4 bit mode + 4 bit command + 4 bit parity + 1 stop bit.
#define LEGO_CHANNEL_BITS       4
#define LEGO_MODE_BITS          4
#define LEGO_COMMAND_BITS       4
#define LEGO_PARITY_BITS        4

#define LEGO_BITS               (LEGO_CHANNEL_BITS + LEGO_MODE_BITS + LEGO_COMMAND_BITS + LEGO_PARITY_BITS)

#define LEGO_HEADER_MARK        158    //  6 cycles
#define LEGO_HEADER_SPACE       1026   // 39 cycles

#define LEGO_BIT_MARK           158    //  6 cycles
#define LEGO_ONE_SPACE          553    // 21 cycles
#define LEGO_ZERO_SPACE         263    // 10 cycles

#define LEGO_AVERAGE_DURATION   11000 // LEGO_HEADER_MARK + LEGO_HEADER_SPACE  + 16 * 600 + 158

#define LEGO_AUTO_REPEAT_PERIOD_MIN 110000 // Every frame is auto repeated 5 times.
#define LEGO_AUTO_REPEAT_PERIOD_MAX 230000 // space for channel 3

#define LEGO_MODE_EXTENDED  0
#define LEGO_MODE_COMBO     1
#define LEGO_MODE_SINGLE    0x4 // here the 2 LSB have meanings like Output A / Output B

struct PulseDistanceWidthProtocolConstants LegoProtocolConstants = { LEGO_PF, 38, LEGO_HEADER_MARK, LEGO_HEADER_SPACE, LEGO_BIT_MARK,
LEGO_ONE_SPACE, LEGO_BIT_MARK, LEGO_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST, (LEGO_AUTO_REPEAT_PERIOD_MIN
        / MICROS_IN_ONE_MILLI), NULL };

/************************************
 * Start of send and decode functions
 ************************************/
/*
 * Here we process the structured data, and call the send raw data function
 * @param aMode one of LEGO_MODE_EXTENDED, LEGO_MODE_COMBO, LEGO_MODE_SINGLE
 */
void IRsend::sendLegoPowerFunctions(uint8_t aChannel, uint8_t aCommand, uint8_t aMode, bool aDoSend5Times) {
    aChannel &= 0x0F; // allow toggle and escape bits too
    aCommand &= 0x0F;
    aMode &= 0x0F;
    uint8_t tParity = 0xF ^ aChannel ^ aMode ^ aCommand;
    // send 4 bit channel, 4 bit mode, 4 bit command, 4 bit parity
    uint16_t tRawData = (((aChannel << LEGO_MODE_BITS) | aMode) << (LEGO_COMMAND_BITS + LEGO_PARITY_BITS))
            | (aCommand << LEGO_PARITY_BITS) | tParity;
    sendLegoPowerFunctions(tRawData, aChannel, aDoSend5Times);
}

void IRsend::sendLegoPowerFunctions(uint16_t aRawData, uint8_t aChannel, bool aDoSend5Times) {

    IR_DEBUG_PRINT(F("sendLego aRawData=0x"));
    IR_DEBUG_PRINTLN(aRawData, HEX);

    aChannel &= 0x03; // we have 4 channels

    uint_fast8_t tNumberOfRepeats = 0;
    if (aDoSend5Times) {
        tNumberOfRepeats = 4;
    }
// required for repeat timing, see http://www.hackvandedam.nl/blog/?page_id=559
    uint8_t tRepeatPeriod = (LEGO_AUTO_REPEAT_PERIOD_MIN / MICROS_IN_ONE_MILLI) + (aChannel * 40); // from 110 to 230
    LegoProtocolConstants.RepeatPeriodMillis = tRepeatPeriod;
    sendPulseDistanceWidth(&LegoProtocolConstants, aRawData, LEGO_BITS, tNumberOfRepeats);
}

/*
 * Mode is stored in the upper nibble of command
 */
bool IRrecv::decodeLegoPowerFunctions() {

    if (!checkHeader(&LegoProtocolConstants)) {
        return false;
    }

    // Check we have enough data - +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawlen != (2 * LEGO_BITS) + 4) {
        IR_DEBUG_PRINT(F("LEGO: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawDataPtr->rawlen);
        IR_DEBUG_PRINTLN(F(" is not 36"));
        return false;
    }

    if (!decodePulseDistanceWidthData(&LegoProtocolConstants, LEGO_BITS)) {
        IR_DEBUG_PRINT(F("LEGO: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * LEGO_BITS)], LEGO_BIT_MARK)) {
        IR_DEBUG_PRINT(F("LEGO: "));
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    uint16_t tDecodedValue = decodedIRData.decodedRawData;
    uint8_t tToggleEscapeChannel = tDecodedValue >> (LEGO_MODE_BITS + LEGO_COMMAND_BITS + LEGO_PARITY_BITS);
    uint8_t tMode = (tDecodedValue >> (LEGO_COMMAND_BITS + LEGO_PARITY_BITS)) & 0xF;
    uint8_t tData = (tDecodedValue >> LEGO_PARITY_BITS) & 0xF; // lego calls this field "data"
    uint8_t tParityReceived = tDecodedValue & 0xF;

    // This is parity as defined in the specifications
    // But in some scans I saw 0x9 ^ .. as parity formula
    uint8_t tParityComputed = 0xF ^ tToggleEscapeChannel ^ tMode ^ tData;

    // parity check
    if (tParityReceived != tParityComputed) {
        IR_DEBUG_PRINT(F("LEGO: "));
        IR_DEBUG_PRINT(F("Parity is not correct. expected=0x"));
        IR_DEBUG_PRINT(tParityComputed, HEX);
        IR_DEBUG_PRINT(F(" received=0x"));
        IR_DEBUG_PRINT(tParityReceived, HEX);
        IR_DEBUG_PRINT(F(", raw=0x"));
        IR_DEBUG_PRINT(tDecodedValue, HEX);
        IR_DEBUG_PRINT(F(", 3 nibbles are 0x"));
        IR_DEBUG_PRINT(tToggleEscapeChannel, HEX);
        IR_DEBUG_PRINT(F(", 0x"));
        IR_DEBUG_PRINT(tMode, HEX);
        IR_DEBUG_PRINT(F(", 0x"));
        IR_DEBUG_PRINTLN(tData, HEX);
        // might not be an error, so just continue
        decodedIRData.flags = IRDATA_FLAGS_PARITY_FAILED | IRDATA_FLAGS_IS_MSB_FIRST;
    }

    /*
     * Check for autorepeat (should happen 4 times for one press)
     */
    if (decodedIRData.initialGapTicks < (LEGO_AUTO_REPEAT_PERIOD_MAX / MICROS_PER_TICK)) {
        decodedIRData.flags |= IRDATA_FLAGS_IS_AUTO_REPEAT;
    }
    decodedIRData.address = tToggleEscapeChannel;
    decodedIRData.command = tData | tMode << LEGO_COMMAND_BITS;
    decodedIRData.numberOfBits = LEGO_BITS;
    decodedIRData.protocol = LEGO_PF;

    return true;
}

/*********************************************************************************
 * Old deprecated functions, kept for backward compatibility to old 2.0 tutorials
 *********************************************************************************/

void IRsend::sendLegoPowerFunctions(uint16_t aRawData, bool aDoSend5Times) {
    sendLegoPowerFunctions(aRawData, (aRawData >> (LEGO_MODE_BITS + LEGO_COMMAND_BITS + LEGO_PARITY_BITS)) & 0x3, aDoSend5Times);
}

/** @}*/
#endif // _IR_LEGO_HPP
