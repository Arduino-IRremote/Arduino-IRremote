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
 * Copyright (c) 2020-2026 Armin Joachimsmeyer
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

#include "LocalDebugLevelStart.h"

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
// Low bit consists of 6 cycles of IR and 10 cycles of pause,
// High bit of 6 cycles IR and 21 cycles of pause,
// Start/stop of 6 cycles IR and 39 cycles of pause.
//
// If tm is the maximum message length (16ms) and Ch is the channel number (1 to 4), then
// 1st  sent after a delay of (4 − Ch) * tm
// 2nd, 3rd sent 5 * tm = 80 ms after the previous one
// 4th, 5th sent (6 + 2·Ch) * tm after the previous one
//
// Supported Devices
// LEGO Power Functions IR Receiver 8884
// MSB first, 1 start bit + 4 bit channel, 4 bit mode + 4 bit command + 4 bit parity + 1 stop bit. See documentation Version 1.20 from 26.02.2010.
/* Protocol=Lego Address=0x1, Command=0x16, Raw-Data=0x1169 ,16 bits, MSB first, Gap=1050600us, Duration=10000us
 Send with: IrSender.sendLegoPowerFunctionsRaw(0x1, 0x16, <numberOfRepeats>);
 rawData[36]:
 -1050600
 + 250,- 950
 + 250,- 500 + 200,- 200 + 200,- 250 + 200,- 500
 + 200,- 250 + 200,- 500 + 200,- 500 + 200,- 250
 + 200,- 500 + 200,- 200 + 250,- 200 + 200,- 200
 + 250,- 500 + 200,- 200 + 250,- 200 + 200,- 250
 + 200
 Duration=10000us
 */

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
#define LEGO_ONE_THRESHOLD      408    // 15.5 cycles - not used, just for info

#define LEGO_AVERAGE_DURATION   11000 // LEGO_HEADER_MARK + LEGO_HEADER_SPACE  + 16 * 600 + 158
#define LEGO_AUTO_REPEAT_PERIOD_MAX 230000 // space for channel 3

#define LEGO_MESSAGE_LENGTH_MAX_MILLIS      16 // tm in LEGO specification
#define LEGO_REPEAT_PERIOD_FIRST_MILLIS     (5 * LEGO_MESSAGE_LENGTH_MAX_MILLIS) // 80 ms, start to start of message 1 to 3


#define LEGO_MODE_EXTENDED  0
#define LEGO_MODE_COMBO     1
#define LEGO_MODE_SINGLE    0x4 // here the 2 LSB have meanings like Output A / Output B

// Cannot be constant, since we need to change RepeatPeriodMillis during sending
struct PulseDistanceWidthProtocolConstants LegoProtocolConstants = { LEGO_PF, 38, LEGO_HEADER_MARK, LEGO_HEADER_SPACE,
LEGO_BIT_MARK, LEGO_ONE_SPACE, LEGO_BIT_MARK, LEGO_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE,
        0, nullptr }; // RepeatPeriodMillis is set to 0 because it is overwritten on every call to sendLegoPowerFunctionsRaw()

/************************************
 * Start of send and decode functions
 ************************************/
/*
 * Here we process the structured data, and call the send raw data function
 * if command is >= 0x10 then we assume mode in the upper nibble and do not use the mode parameter.
 * This ensures compatibility with the received data and send usage printout.
 * @param aDoSend5Times instead of int_fast8_t aNumberOfRepeats. We only support the standard 5 times repeating.
 * @param aMode one of LEGO_MODE_EXTENDED, LEGO_MODE_COMBO, LEGO_MODE_SINGLE
 */
void IRsend::sendLegoPowerFunctions(uint8_t aChannel, uint8_t aCommand, bool aDoSend5Times, uint8_t aMode) {
    aChannel &= 0x0F; // allow toggle and escape bits too
    if(aCommand >= 0x10) {
        aMode = aCommand >> LEGO_COMMAND_BITS;
    }
    aCommand &= 0x0F;
    aMode &= 0x0F;
    uint8_t tParity = 0xF ^ aChannel ^ aMode ^ aCommand;
    // send 4 bit channel, 4 bit mode, 4 bit command, 4 bit parity
    uint16_t tRawData = (((aChannel << LEGO_MODE_BITS) | aMode) << (LEGO_COMMAND_BITS + LEGO_PARITY_BITS))
            | (aCommand << LEGO_PARITY_BITS) | tParity;
    sendLegoPowerFunctionsRaw(tRawData, aChannel, aDoSend5Times);
}


/*
 * Timing according to LEGO Power Functions RC Manual v1.20, without the initial delay of (4 - Ch) * tm:
 * Message 1 to 3 with a start to start period of 5 * tm = 80 ms
 * Message 4 and 5 with a start to start period of (6 + 2 * Ch) * tm = 128, 160, 192, 224 ms for Ch = 1 to 4
 * @param aChannel 0 to 3 for channel 1 to 4
 */
void IRsend::sendLegoPowerFunctionsRaw(uint16_t aRawData, uint8_t aChannel, bool aDoSend5Times) {

    DEBUG_PRINT(F("sendLego aRawData=0x"));
    DEBUG_PRINTLN(aRawData, HEX);

    if (!aDoSend5Times) {
        sendPulseDistanceWidth(&LegoProtocolConstants, aRawData, LEGO_BITS, NO_REPEATS);
        return;
    }

    aChannel &= 0x03; // 0 to 3 for channel 1 to 4

    /*
     * Message 1 to 3
     */
    unsigned long tStartMillis = millis();
    LegoProtocolConstants.RepeatPeriodMillis = LEGO_REPEAT_PERIOD_FIRST_MILLIS;
    sendPulseDistanceWidth(&LegoProtocolConstants, aRawData, LEGO_BITS, 2);

    /*
     * Message 4 and 5
     */
    uint8_t tRepeatPeriodMillis = (6 + 2 * (aChannel + 1)) * LEGO_MESSAGE_LENGTH_MAX_MILLIS; // 128 to 224
    // Duration of message 3 is the total duration minus the two 80 ms periods before it
    unsigned long tLastFrameDurationMillis = (millis() - tStartMillis) - (2 * LEGO_REPEAT_PERIOD_FIRST_MILLIS);
    if (tRepeatPeriodMillis > tLastFrameDurationMillis) {
        delay(tRepeatPeriodMillis - tLastFrameDurationMillis);
    }
    LegoProtocolConstants.RepeatPeriodMillis = tRepeatPeriodMillis;
    sendPulseDistanceWidth(&LegoProtocolConstants, aRawData, LEGO_BITS, 1);
}

/*
 * Mode is stored in the upper nibble of command
 */
bool IRrecv::decodeLegoPowerFunctions() {

    /*
     * Check header timings
     * Since LEGO_HEADER_MARK is just 158 us use a relaxed threshold compare (237) for it instead of matchMark()
     */
    if (!(matchMarkWithGreaterRange(irparams.rawbuf[1], LEGO_HEADER_MARK) && (matchSpace(irparams.rawbuf[2], LEGO_HEADER_SPACE)))) {
        DEBUG_PRINTLN(F("LEGO: No header mark and space"));
        return false;
    }

    // Check we have enough data - +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawlen != (2 * LEGO_BITS) + 4) {
        DEBUG_PRINT(F("LEGO: Data length="));
        DEBUG_PRINT(irparams.rawlen);
        DEBUG_PRINTLN(F(" is not 36"));
        return false;
    }

    decodePulseDistanceWidthData(&LegoProtocolConstants, LEGO_BITS);

    // Stop bit, use threshold decoding - not required :-)
//    if (irparams.rawbuf[3 + (2 * LEGO_BITS)] > (2 * LEGO_BIT_MARK)) {
//        DEBUG_PRINT(F("LEGO: "));
//        DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
//        return false;
//    }

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
        DEBUG_PRINT(F("LEGO: Parity is not correct. expected=0x"));
        DEBUG_PRINT(tParityComputed, HEX);
        DEBUG_PRINT(F(" received=0x"));
        DEBUG_PRINT(tParityReceived, HEX);
        DEBUG_PRINT(F(", raw=0x"));
        DEBUG_PRINT(tDecodedValue, HEX);
        DEBUG_PRINT(F(", 3 nibbles are 0x"));
        DEBUG_PRINT(tToggleEscapeChannel, HEX);
        DEBUG_PRINT(F(", 0x"));
        DEBUG_PRINT(tMode, HEX);
        DEBUG_PRINT(F(", 0x"));
        DEBUG_PRINTLN(tData, HEX);
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
    decodedIRData.command = tData | (tMode << LEGO_COMMAND_BITS);
    decodedIRData.numberOfBits = LEGO_BITS;
    decodedIRData.protocol = LEGO_PF;

    return true;
}

/** @}*/
#include "LocalDebugLevelEnd.h"

#endif // _IR_LEGO_HPP
