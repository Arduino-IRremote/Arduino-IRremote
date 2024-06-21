/*
 * ir_MagiQuest.hpp
 *
 *  Contains functions for receiving and sending MagiQuest Protocol
 *  Based off the Magiquest fork of Arduino-IRremote by mpflaga https://github.com/mpflaga/Arduino-IRremote/
 *
 *  RESULT:
 *  The 31 bit wand ID is available in decodedRawData.
 *  The lower 16 bit of the ID is available in address.
 *  The magnitude is available in command.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2024 E. Stuart Hicks <ehicks@binarymagi.com>, Armin Joachimsmeyer
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
#ifndef _IR_MAGIQUEST_HPP
#define _IR_MAGIQUEST_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif
//
//==============================================================================
//
//       M   M   AA    GGG   III   QQQ    U   U  EEEE   SSS  TTTTTT
//       MM MM  A  A  G       I   Q   Q   U   U  E     S       TT
//       M M M  AAAA  G  GG   I   Q   Q   U   U  EEE    SSS    TT
//       M   M  A  A  G   G   I   Q  QQ   U   U  E         S   TT
//       M   M  A  A   GGG   III   QQQQ    UUU   EEEE  SSSS    TT
//                                     Q
//==============================================================================
/*
 * https://github.com/kitlaan/Arduino-IRremote/blob/master/ir_Magiquest.cpp
 * https://github.com/Arduino-IRremote/Arduino-IRremote/discussions/1027#discussioncomment-3636857
 * https://github.com/Arduino-IRremote/Arduino-IRremote/issues/1015#issuecomment-1222247231

 Protocol=MagiQuest Address=0xFF00 Command=0x176 Raw-Data=0x6BCDFF00 56 bits MSB first
 + 250,- 800 + 250,- 850 + 250,- 850 + 250,- 850 // 8 zero start bits
 + 250,- 850 + 300,- 800 + 250,- 850 + 250,- 850

 // 31 ID bits
 + 550,- 600 + 550,- 550 + 350,- 800 + 600,- 600 // 110 1 6
 + 200,- 950 + 550,- 600 + 550,- 600 + 550,- 600 // 011 1 B - 1(from above)011 => B
 + 550,- 600 + 250,- 900 + 300,- 850 + 550,- 600 // 100 1 C
 + 550,- 600 + 300,- 850 + 550,- 600 + 550,- 600
 + 550,- 600 + 550,- 600 + 550,- 600 + 550,- 600
 + 550,- 600 + 550,- 600 + 550,- 600 + 300,- 800
 + 350,- 850 + 300,- 850 + 300,- 850 + 300,- 850
 + 300,- 850 + 300,- 850 + 300,- 850 + 550,- 600 // 000 1 - 3 LSB ID bits 000 + 1 MSB magnitude bit 1

 // 8 bit magnitude
 + 300,- 850 + 550,- 600 + 550,- 600 + 550,- 600
 + 300,- 850 + 550,- 600 + 550,- 600 + 250,- 900

 // Checksum (+ sum of the 5 bytes before == 0)
 + 250,- 900 + 300,- 900 + 250,- 850 + 550,- 600
 + 600,- 550 + 300,- 900 + 250,- 850 + 550

 // No stop bit!
 */
// MSB first, 8 start bits (zero), 31 wand id bits, 9 magnitude bits 8 checksum bits and no stop bit => 56 bits
#define MAGIQUEST_CHECKSUM_BITS     8   // magiquest_t.cmd.checksum
#define MAGIQUEST_MAGNITUDE_BITS    9   // magiquest_t.cmd.magnitude
#define MAGIQUEST_WAND_ID_BITS     31   // magiquest_t.cmd.wand_id -> wand-id is handled as 32 bit and always even
#define MAGIQUEST_START_BITS        8    // magiquest_t.cmd.StartBits

#define MAGIQUEST_PERIOD         1150   // Time for a full MagiQuest "bit" (1100 - 1200 usec)

#define MAGIQUEST_DATA_BITS     (MAGIQUEST_CHECKSUM_BITS + MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS) // 48 Size of the command without the start bits
#define MAGIQUEST_BITS          (MAGIQUEST_CHECKSUM_BITS + MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_WAND_ID_BITS + MAGIQUEST_START_BITS) // 56 Size of the command with the start bits

/*
 * 0 = 25% mark & 75% space across 1 period
 *     1150 * 0.25 = 288 usec mark
 *     1150 - 288 = 862 usec space
 * 1 = 50% mark & 50% space across 1 period
 *     1150 * 0.5 = 575 usec mark
 *     1150 - 575 = 575 usec space
 */
#define MAGIQUEST_UNIT          (MAGIQUEST_PERIOD / 4) // 287.5

#define MAGIQUEST_ONE_MARK      (2 * MAGIQUEST_UNIT) // 576
#define MAGIQUEST_ONE_SPACE     (2 * MAGIQUEST_UNIT) // 576
#define MAGIQUEST_ZERO_MARK     MAGIQUEST_UNIT       // 287.5
#define MAGIQUEST_ZERO_SPACE    (3 * MAGIQUEST_UNIT) // 864

// assume 110 as repeat period
struct PulseDistanceWidthProtocolConstants MagiQuestProtocolConstants = { MAGIQUEST, 38, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE,
MAGIQUEST_ONE_MARK, MAGIQUEST_ONE_SPACE, MAGIQUEST_ZERO_MARK, MAGIQUEST_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST | SUPPRESS_STOP_BIT, 110,
        NULL };
//+=============================================================================
//
/**
 * @param aWandId       31 bit ID
 * @param aMagnitude    9 bit Magnitude
 */
void IRsend::sendMagiQuest(uint32_t aWandId, uint16_t aMagnitude) {

    // Set IR carrier frequency
    enableIROut(38);

    aMagnitude &= 0x1FF; // we have 9 bit
    LongUnion tWandId;
    tWandId.ULong = aWandId << 1;
    uint8_t tChecksum = (tWandId.Bytes[0]) + tWandId.Bytes[1] + tWandId.Bytes[2] + tWandId.Bytes[3];
    tChecksum += aMagnitude + (aMagnitude >> 8);
    tChecksum = ~tChecksum + 1;

    // 8 start bits
    sendPulseDistanceWidthData(&MagiQuestProtocolConstants, 0, 8);
    // 48 bit data
    sendPulseDistanceWidthData(&MagiQuestProtocolConstants, aWandId, MAGIQUEST_WAND_ID_BITS); // send only 31 bit, do not send MSB here
    sendPulseDistanceWidthData(&MagiQuestProtocolConstants, aMagnitude, MAGIQUEST_MAGNITUDE_BITS);
    sendPulseDistanceWidthData(&MagiQuestProtocolConstants, tChecksum, MAGIQUEST_CHECKSUM_BITS);
#if defined(LOCAL_DEBUG)
    // must be after sending, in order not to destroy the send timing
    Serial.print(F("MagiQuest checksum=0x"));
    Serial.println(tChecksum, HEX);
#endif
}

//+=============================================================================
//
/*
 * decodes a 56 bit result, which is not really compatible with standard decoder layout
 * magnitude is stored in command
 * 31 bit wand_id is stored in decodedRawData
 * lower 16 bit of wand_id is stored in address
 */
bool IRrecv::decodeMagiQuest() {

    // Check we have the right amount of data, magnitude and ID bits and 8 start bits + 0 stop bit
    if (decodedIRData.rawlen != (2 * MAGIQUEST_BITS)) {
        IR_DEBUG_PRINT(F("MagiQuest: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 112"));
        return false;
    }

    /*
     * Check for 8 zero header bits
     */
    if (!decodePulseDistanceWidthData(&MagiQuestProtocolConstants, MAGIQUEST_START_BITS, 1)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("MagiQuest: "));
        Serial.println(F("Start bit decode failed"));
#endif
        return false;
    }
    if (decodedIRData.decodedRawData != 0) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("MagiQuest: "));
        Serial.print(F("Not 8 leading zero start bits received, RawData=0x"));
        Serial.println(decodedIRData.decodedRawData, HEX);
#endif
        return false;
    }

    /*
     * Decode the 31 bit ID
     */
    if (!decodePulseDistanceWidthData(&MagiQuestProtocolConstants, MAGIQUEST_WAND_ID_BITS, (MAGIQUEST_START_BITS * 2) + 1)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("MagiQuest: "));
        Serial.println(F("ID decode failed"));
#endif
        return false;
    }
    LongUnion tDecodedRawData;
#if defined(LOCAL_DEBUG)
    Serial.print(F("31 bit WandId=0x"));
    Serial.println(decodedIRData.decodedRawData, HEX);
#endif
    uint32_t tWandId = decodedIRData.decodedRawData; // save tWandId for later use
    tDecodedRawData.ULong = decodedIRData.decodedRawData << 1; // shift for checksum computation
    uint8_t tChecksum = tDecodedRawData.Bytes[0] + tDecodedRawData.Bytes[1] + tDecodedRawData.Bytes[2] + tDecodedRawData.Bytes[3];
#if defined(LOCAL_DEBUG)
    Serial.print(F("31 bit WandId=0x"));
    Serial.print(decodedIRData.decodedRawData, HEX);
    Serial.print(F(" shifted=0x"));
    Serial.println(tDecodedRawData.ULong, HEX);
#endif
    /*
     * Decode the 9 bit Magnitude + 8 bit checksum
     */
    if (!decodePulseDistanceWidthData(&MagiQuestProtocolConstants, MAGIQUEST_MAGNITUDE_BITS + MAGIQUEST_CHECKSUM_BITS,
            ((MAGIQUEST_WAND_ID_BITS + MAGIQUEST_START_BITS) * 2) + 1)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("MagiQuest: "));
        Serial.println(F("Magnitude + checksum decode failed"));
#endif
        return false;
    }

#if defined(LOCAL_DEBUG)
    Serial.print(F("Magnitude + checksum=0x"));
    Serial.println(decodedIRData.decodedRawData, HEX);
#endif
    tDecodedRawData.ULong = decodedIRData.decodedRawData;
    decodedIRData.command = tDecodedRawData.UBytes[1] | tDecodedRawData.UBytes[2] << 8; // Values observed are 0x102,01,04,37,05,38,2D| 02,06,04|03,103,12,18,0E|09

    tChecksum += tDecodedRawData.UBytes[2] /* only one bit */+ tDecodedRawData.UBytes[1] + tDecodedRawData.UBytes[0];
    if (tChecksum != 0) {
        decodedIRData.flags |= IRDATA_FLAGS_PARITY_FAILED;
#if defined(LOCAL_DEBUG)
        Serial.print(F("Checksum 0x"));
        Serial.print(tChecksum, HEX);
        Serial.println(F(" is not 0"));
#endif
    }

    // Success
    decodedIRData.decodedRawData = tWandId;     // 31 bit wand_id
    decodedIRData.address = tWandId;            // lower 16 bit of wand_id
    decodedIRData.extra = tWandId >> 16;        // upper 15 bit of wand_id

    decodedIRData.protocol = MAGIQUEST;
    decodedIRData.numberOfBits = MAGIQUEST_BITS;
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;

    return true;
}
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_MAGIQUEST_HPP
