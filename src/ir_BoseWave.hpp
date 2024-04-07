/*
 * ir_BoseWave.cpp
 *
 *  Contains functions for receiving and sending Bose IR Protocol
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 */
#ifndef _IR_BOSEWAVE_HPP
#define _IR_BOSEWAVE_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */
//==============================================================================
//                           BBBB    OOO    SSSS  EEEEE
//                           B   B  O   O  S      E
//                           BB B   O   O   SSS   EEEE
//                           B   B  O   O      S  E
//                           BBBB    OOO   SSSS   EEEEE
//==============================================================================
// see http://lirc.sourceforge.net/remotes/bose/WAVERADIO
// see: https://www.mikrocontroller.net/articles/IRMP_-_english#BOSE
//
// Support for Bose Wave Radio CD initially provided by https://github.com/uvotguy.
//
// As seen on my oscilloscope, there is no repeat code. Instead, when I
// press and hold a button on my remote, it sends a command, makes a 51.2ms space,
// and resends the command again, and so on.
// 38 kHz, LSB first, 1 start bit + 8 bit data + 8 bit inverted data + 1 stop bit.
#define BOSEWAVE_BITS             16 // Command and inverted command

#define BOSEWAVE_HEADER_MARK    1014    // 1014 are 39 clock periods (I counted 3 times!)
#define BOSEWAVE_HEADER_SPACE   1468    // 1468(measured), 1456 are 56 clock periods
#define BOSEWAVE_BIT_MARK        520    // 520 are 20 clock periods
#define BOSEWAVE_ZERO_SPACE      468    // 468 are 18 clock periods
#define BOSEWAVE_ONE_SPACE      1468    // 1468(measured), 1456 are 56 clock periods

#define BOSEWAVE_REPEAT_PERIOD              75000
#define BOSEWAVE_REPEAT_DISTANCE            50000
#define BOSEWAVE_MAXIMUM_REPEAT_DISTANCE    62000

struct PulseDistanceWidthProtocolConstants BoseWaveProtocolConstants = { BOSEWAVE, BOSEWAVE_KHZ, BOSEWAVE_HEADER_MARK,
BOSEWAVE_HEADER_SPACE, BOSEWAVE_BIT_MARK, BOSEWAVE_ONE_SPACE, BOSEWAVE_BIT_MARK, BOSEWAVE_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST
       , (BOSEWAVE_REPEAT_PERIOD / MICROS_IN_ONE_MILLI), NULL };

/************************************
 * Start of send and decode functions
 ************************************/

void IRsend::sendBoseWave(uint8_t aCommand, int_fast8_t aNumberOfRepeats) {

    // send 8 command bits and then 8 inverted command bits LSB first
    uint16_t tData = ((~aCommand) << 8) | aCommand;
    sendPulseDistanceWidth(&BoseWaveProtocolConstants, tData, BOSEWAVE_BITS, aNumberOfRepeats);
}

bool IRrecv::decodeBoseWave() {

    if (!checkHeader(&BoseWaveProtocolConstants)) {
        return false;
    }

    // Check we have enough data +4 for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawlen != (2 * BOSEWAVE_BITS) + 4) {
        IR_DEBUG_PRINT(F("Bose: "));
        IR_DEBUG_PRINT(F("Data length="));
        IR_DEBUG_PRINT(decodedIRData.rawlen);
        IR_DEBUG_PRINTLN(F(" is not 36"));
        return false;
    }

    if (!decodePulseDistanceWidthData(&BoseWaveProtocolConstants, BOSEWAVE_BITS)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Bose: "));
        Serial.println(F("Decode failed"));
#endif
        return false;
    }

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * BOSEWAVE_BITS)], BOSEWAVE_BIT_MARK)) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Bose: "));
        Serial.println(F("Stop bit mark length is wrong"));
#endif
        return false;
    }

    // Success
//    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
    uint16_t tDecodedValue = decodedIRData.decodedRawData;
    uint8_t tCommandNotInverted = tDecodedValue & 0xFF; // comes first and is in the lower bits (LSB first :-))
    uint8_t tCommandInverted = tDecodedValue >> 8;
    // parity check for command. Use this variant to avoid compiler warning "comparison of promoted ~unsigned with unsigned [-Wsign-compare]"
    if ((tCommandNotInverted ^ tCommandInverted) != 0xFF) {
#if defined(LOCAL_DEBUG)
        Serial.print(F("Bose: "));
        Serial.println(F("Command and inverted command check failed"));
#endif
        return false;
    }
    decodedIRData.command = tCommandNotInverted;
    decodedIRData.numberOfBits = BOSEWAVE_BITS;
    decodedIRData.protocol = BOSEWAVE;

    // check for repeat
    checkForRepeatSpaceTicksAndSetFlag(BOSEWAVE_MAXIMUM_REPEAT_DISTANCE / MICROS_PER_TICK);

    return true;
}

/** @}*/
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_BOSEWAVE_HPP
