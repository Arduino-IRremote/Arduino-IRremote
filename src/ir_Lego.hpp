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

#if defined(DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

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
// If tm is the maximum message length (16ms) and Ch is the channel number, then
// The delay before transmitting the first message is: (4 - Ch) * tm
// The time from start to start for the next 2 messages is: 5 * tm
// The time from start to start for the following messages is: (6 + 2 * Ch) * tm
// Supported Devices
// LEGO Power Functions IR Receiver 8884
// MSB first, 1 start bit + 4 bit channel, 4 bit mode + 4 bit command + 4 bit parity + 1 stop bit.
/* Protocol=Lego Address=0x1, Command=0x16, Raw-Data=0x1169 ,16 bits, MSB first, Gap=1050600us, Duration=10000us
 Send with: IrSender.sendLego(0x1, 0x16, <numberOfRepeats>);
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

#define LEGO_AUTO_REPEAT_PERIOD_MIN 110000 // Every frame is auto repeated 5 times.
#define LEGO_AUTO_REPEAT_PERIOD_MAX 230000 // space for channel 3

#define LEGO_MODE_EXTENDED  0
#define LEGO_MODE_COMBO     1
#define LEGO_MODE_SINGLE    0x4 // here the 2 LSB have meanings like Output A / Output B

// Cannot be constant, since we need to change RepeatPeriodMillis during sending
struct PulseDistanceWidthProtocolConstants LegoProtocolConstants = { LEGO_PF, 38, LEGO_HEADER_MARK, LEGO_HEADER_SPACE,
LEGO_BIT_MARK, LEGO_ONE_SPACE, LEGO_BIT_MARK, LEGO_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE,
        (LEGO_AUTO_REPEAT_PERIOD_MIN / MICROS_IN_ONE_MILLI), nullptr };

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

#if defined(LOCAL_DEBUG)
    Serial.print(F("sendLego aRawData=0x"));
    Serial.println(aRawData, HEX);
#endif

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

    /*
     * Check header timings
     * Since LEGO_HEADER_MARK is just 158 us use a relaxed threshold compare (237) for it instead of matchMark()
     */
    if ((irparams.rawbuf[1] > LEGO_HEADER_MARK + (LEGO_HEADER_MARK / 2)) || (!matchSpace(irparams.rawbuf[2], LEGO_HEADER_SPACE))) {
        return false;
    }

    // Check we have enough data - +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawlen != (2 * LEGO_BITS) + 4) {
        IR_DEBUG_PRINT(F("LEGO: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(irparams.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 36"));
        return false;
    }

    decodePulseDistanceWidthData(&LegoProtocolConstants, LEGO_BITS);

    // Stop bit, use threshold decoding - not required :-)
//    if (irparams.rawbuf[3 + (2 * LEGO_BITS)] > (2 * LEGO_BIT_MARK)) {
//#if defined(LOCAL_DEBUG)
//        Serial.print(F("LEGO: "));
//        Serial.println(F("Stop bit mark length is wrong"));
//#endif
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
#if defined(LOCAL_DEBUG)
        Serial.print(F("LEGO: "));
        Serial.print(F("Parity is not correct. expected=0x"));
        Serial.print(tParityComputed, HEX);
        Serial.print(F(" received=0x"));
        Serial.print(tParityReceived, HEX);
        Serial.print(F(", raw=0x"));
        Serial.print(tDecodedValue, HEX);
        Serial.print(F(", 3 nibbles are 0x"));
        Serial.print(tToggleEscapeChannel, HEX);
        Serial.print(F(", 0x"));
        Serial.print(tMode, HEX);
        Serial.print(F(", 0x"));
        Serial.println(tData, HEX);
#endif
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

/*********************************************************************************
 * Old deprecated functions, kept for backward compatibility to old 2.0 tutorials
 *********************************************************************************/

void IRsend::sendLegoPowerFunctions(uint16_t aRawData, bool aDoSend5Times) {
    sendLegoPowerFunctions(aRawData, (aRawData >> (LEGO_MODE_BITS + LEGO_COMMAND_BITS + LEGO_PARITY_BITS)) & 0x3, aDoSend5Times);
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_LEGO_HPP
