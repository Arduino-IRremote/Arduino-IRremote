/*
 * IRSend.hpp
 *
 *  Contains common functions for sending
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2009-2023 Ken Shirriff, Rafi Khan, Armin Joachimsmeyer
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
#ifndef _IR_SEND_HPP
#define _IR_SEND_HPP

#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

#if defined(TRACE) && !defined(LOCAL_TRACE)
#define LOCAL_TRACE
#else
//#define LOCAL_TRACE // This enables debug output only for this file
#endif

/*
 * Low level hardware timing measurement
 */
//#define _IR_MEASURE_TIMING // for mark()
//#define _IR_TIMING_TEST_PIN 7 // "pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);" is executed at begin()
//
/*
 * This improves readability of code by avoiding a lot of #if defined clauses
 */
#if defined(IR_SEND_PIN)
#define sendPin IR_SEND_PIN
#endif

/** \addtogroup Sending Sending IR data for multiple protocols
 * @{
 */

// The sender instance
IRsend IrSender;

IRsend::IRsend() { // @suppress("Class members should be properly initialized")
#if !defined(IR_SEND_PIN)
    sendPin = 0;
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, DO_NOT_ENABLE_LED_FEEDBACK);
#endif
}

#if defined(IR_SEND_PIN)
/**
 * Only required to set LED feedback
 * Simple start with defaults - LED feedback enabled! Used if IR_SEND_PIN is defined. Saves program memory.
 */
void IRsend::begin(){
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(USE_DEFAULT_FEEDBACK_LED_PIN, LED_FEEDBACK_ENABLED_FOR_SEND);
#  endif
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
    pinModeFast(_IR_TIMING_TEST_PIN, OUTPUT);
#endif
}

/**
 * Only required to set LED feedback
 * @param aEnableLEDFeedback    If true / ENABLE_LED_FEEDBACK, the feedback LED is activated while receiving or sending a PWM signal /a mark
 * @param aFeedbackLEDPin       If 0 / USE_DEFAULT_FEEDBACK_LED_PIN, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRsend::begin(bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin) {
#if !defined(NO_LED_FEEDBACK_CODE)
    uint_fast8_t tEnableLEDFeedback = DO_NOT_ENABLE_LED_FEEDBACK;
    if(aEnableLEDFeedback) {
        tEnableLEDFeedback = LED_FEEDBACK_ENABLED_FOR_SEND;
    }
    setLEDFeedback(aFeedbackLEDPin, tEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif
}

#else // defined(IR_SEND_PIN)
IRsend::IRsend(uint_fast8_t aSendPin) { // @suppress("Class members should be properly initialized")
    sendPin = aSendPin;
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(0, DO_NOT_ENABLE_LED_FEEDBACK);
#  endif
}

/**
 * Initializes the send pin and enable LED feedback with board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 * @param aSendPin The Arduino pin number, where a IR sender diode is connected.
 */
void IRsend::begin(uint_fast8_t aSendPin) {
    sendPin = aSendPin;
#  if !defined(NO_LED_FEEDBACK_CODE)
    setLEDFeedback(USE_DEFAULT_FEEDBACK_LED_PIN, LED_FEEDBACK_ENABLED_FOR_SEND);
#  endif
}

void IRsend::setSendPin(uint_fast8_t aSendPin) {
    sendPin = aSendPin;
}

/**
 * Initializes the send and feedback pin
 * @param aSendPin The Arduino pin number, where a IR sender diode is connected.
 * @param aEnableLEDFeedback    If true the feedback LED is activated while receiving or sending a PWM signal /a mark
 * @param aFeedbackLEDPin       If 0 / USE_DEFAULT_FEEDBACK_LED_PIN, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
 */
void IRsend::begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin) {
#if defined(IR_SEND_PIN)
    (void) aSendPin; // for backwards compatibility
#else
    sendPin = aSendPin;
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    uint_fast8_t tEnableLEDFeedback = DO_NOT_ENABLE_LED_FEEDBACK;
    if (aEnableLEDFeedback) {
        tEnableLEDFeedback = LED_FEEDBACK_ENABLED_FOR_SEND;
    }
    setLEDFeedback(aFeedbackLEDPin, tEnableLEDFeedback);
#else
    (void) aEnableLEDFeedback;
    (void) aFeedbackLEDPin;
#endif
}
#endif // defined(IR_SEND_PIN)

/**
 * Interprets and sends a IRData structure.
 * @param aIRSendData The values of protocol, address, command and repeat flag are taken for sending.
 * @param aNumberOfRepeats Number of repeats to send after the initial data if data is no repeat.
 * @return 1 if data sent, 0 if no data sent (i.e. for BANG_OLUFSEN, which is currently not supported here)
 */
/**
 * Interprets and sends a IRData structure.
 * @param aIRSendData The values of protocol, address, command and repeat flag are taken for sending.
 * @param aNumberOfRepeats Number of repeats to send after the initial data if data is no repeat.
 * @return 1 if data sent, 0 if no data sent (i.e. for BANG_OLUFSEN, which is currently not supported here)
 */
size_t IRsend::write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats) {

    auto tProtocol = aIRSendData->protocol;
    auto tAddress = aIRSendData->address;
    auto tCommand = aIRSendData->command;
    bool tIsRepeat = (aIRSendData->flags & IRDATA_FLAGS_IS_REPEAT);
    if (tIsRepeat) {
        aNumberOfRepeats = -1; // if aNumberOfRepeats < 0 then only a special repeat frame will be sent
    }
//    switch (tProtocol) { // 26 bytes bigger than if, else if, else
//    case NEC:
//        sendNEC(tAddress, tCommand, aNumberOfRepeats, tSendRepeat);
//        break;
//    case SAMSUNG:
//        sendSamsung(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case SONY:
//        sendSony(tAddress, tCommand, aNumberOfRepeats, aIRSendData->numberOfBits);
//        break;
//    case PANASONIC:
//        sendPanasonic(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case DENON:
//        sendDenon(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case SHARP:
//        sendSharp(tAddress, tCommand, aNumberOfRepeats);
//        break;
//    case JVC:
//        sendJVC((uint8_t) tAddress, (uint8_t) tCommand, aNumberOfRepeats); // casts are required to specify the right function
//        break;
//    case RC5:
//        sendRC5(tAddress, tCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats
//        break;
//    case RC6:
//        // No toggle for repeats//        sendRC6(tAddress, tCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats
//        break;
//    default:
//        break;
//    }

    /*
     * Order of protocols is in guessed relevance :-)
     */
    if (tProtocol == NEC) {
        sendNEC(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SAMSUNG) {
        sendSamsung(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SAMSUNG48) {
        sendSamsung48(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SAMSUNGLG) {
        sendSamsungLG(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SONY) {
        sendSony(tAddress, tCommand, aNumberOfRepeats, aIRSendData->numberOfBits);

    } else if (tProtocol == PANASONIC) {
        sendPanasonic(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == DENON) {
        sendDenon(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == SHARP) {
        sendSharp(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == LG) {
        sendLG(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == JVC) {
        sendJVC((uint8_t) tAddress, (uint8_t) tCommand, aNumberOfRepeats); // casts are required to specify the right function

    } else if (tProtocol == RC5) {
        sendRC5(tAddress, tCommand, aNumberOfRepeats, !tIsRepeat); // No toggle for repeats

    } else if (tProtocol == RC6) {
        sendRC6(tAddress, tCommand, aNumberOfRepeats, !tIsRepeat); // No toggle for repeats

    } else if (tProtocol == KASEIKYO_JVC) {
        sendKaseikyo_JVC(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_DENON) {
        sendKaseikyo_Denon(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_SHARP) {
        sendKaseikyo_Sharp(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == KASEIKYO_MITSUBISHI) {
        sendKaseikyo_Mitsubishi(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == NEC2) {
        sendNEC2(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == ONKYO) {
        sendOnkyo(tAddress, tCommand, aNumberOfRepeats);

    } else if (tProtocol == APPLE) {
        sendApple(tAddress, tCommand, aNumberOfRepeats);

#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    } else if (tProtocol == BOSEWAVE) {
        sendBoseWave(tCommand, aNumberOfRepeats);

    } else if (tProtocol == MAGIQUEST) {
        // we have a 32 bit ID/address
        sendMagiQuest(aIRSendData->decodedRawData, tCommand);

    } else if (tProtocol == FAST) {
        // We have only 8 bit command
        sendFAST(tCommand, aNumberOfRepeats);

    } else if (tProtocol == LEGO_PF) {
        sendLegoPowerFunctions(tAddress, tCommand, tCommand >> 4, tIsRepeat); // send 5 autorepeats
#endif

    } else {
        return 0; // Not supported by write. E.g for BANG_OLUFSEN
    }
    return 1;
}

/**
 * Simple version of write without support for MAGIQUEST and numberOfBits for SONY protocol
 * @param aNumberOfRepeats  If aNumberOfRepeats < 0 then only a special repeat frame without leading and trailing space
 *                          will be sent by calling NECProtocolConstants.SpecialSendRepeatFunction().
 */
size_t IRsend::write(decode_type_t aProtocol, uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats) {

//    switch (aProtocol) { // 26 bytes bigger than if, else if, else
//    case NEC:
//        sendNEC(aAddress, aCommand, aNumberOfRepeats, tSendRepeat);
//        break;
//    case SAMSUNG:
//        sendSamsung(aAddress, aCommand, aNumberOfRepeats);
//        break;
//    case SONY:
//        sendSony(aAddress, aCommand, aNumberOfRepeats, aIRSendData->numberOfBits);
//        break;
//    case PANASONIC:
//        sendPanasonic(aAddress, aCommand, aNumberOfRepeats);
//        break;
//    case DENON:
//        sendDenon(aAddress, aCommand, aNumberOfRepeats);
//        break;
//    case SHARP:
//        sendSharp(aAddress, aCommand, aNumberOfRepeats);
//        break;
//    case JVC:
//        sendJVC((uint8_t) aAddress, (uint8_t) aCommand, aNumberOfRepeats); // casts are required to specify the right function
//        break;
//    case RC5:
//        sendRC5(aAddress, aCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats
//        break;
//    case RC6:
//        // No toggle for repeats//        sendRC6(aAddress, aCommand, aNumberOfRepeats, !tSendRepeat); // No toggle for repeats
//        break;
//    default:
//        break;
//    }

    /*
     * Order of protocols is in guessed relevance :-)
     */
    if (aProtocol == NEC) {
        sendNEC(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == SAMSUNG) {
        sendSamsung(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == SAMSUNG48) {
        sendSamsung48(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == SAMSUNGLG) {
        sendSamsungLG(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == SONY) {
        sendSony(aAddress, aCommand, aNumberOfRepeats, SIRCS_12_PROTOCOL);

    } else if (aProtocol == PANASONIC) {
        sendPanasonic(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == DENON) {
        sendDenon(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == SHARP) {
        sendSharp(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == LG) {
        sendLG(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == JVC) {
        sendJVC((uint8_t) aAddress, (uint8_t) aCommand, aNumberOfRepeats); // casts are required to specify the right function

    } else if (aProtocol == RC5) {
        sendRC5(aAddress, aCommand, aNumberOfRepeats, (aNumberOfRepeats > 0)); // No toggle for repeats

    } else if (aProtocol == RC6) {
        sendRC6(aAddress, aCommand, aNumberOfRepeats, (aNumberOfRepeats > 0)); // No toggle for repeats

    } else if (aProtocol == KASEIKYO_JVC) {
        sendKaseikyo_JVC(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == KASEIKYO_DENON) {
        sendKaseikyo_Denon(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == KASEIKYO_SHARP) {
        sendKaseikyo_Sharp(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == KASEIKYO_MITSUBISHI) {
        sendKaseikyo_Mitsubishi(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == NEC2) {
        sendNEC2(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == ONKYO) {
        sendOnkyo(aAddress, aCommand, aNumberOfRepeats);

    } else if (aProtocol == APPLE) {
        sendApple(aAddress, aCommand, aNumberOfRepeats);

#if !defined(EXCLUDE_EXOTIC_PROTOCOLS)
    } else if (aProtocol == BOSEWAVE) {
        sendBoseWave(aCommand, aNumberOfRepeats);

    } else if (aProtocol == FAST) {
        // We have only 8 bit command
        sendFAST(aCommand, aNumberOfRepeats);

    } else if (aProtocol == LEGO_PF) {
        sendLegoPowerFunctions(aAddress, aCommand, aCommand >> 4, (aNumberOfRepeats < 0)); // send 5 autorepeats, except for dedicated repeats
#endif

    } else {
        return 0; // Not supported by write. E.g for BANG_OLUFSEN
    }
    return 1;
}

/**
 * Function using an 16 byte microsecond timing array for every purpose.
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    /*
     * Raw data starts with a mark.
     */
    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        if (i & 1) {
            // Odd
            space(aBufferWithMicroseconds[i]);
        } else {
            mark(aBufferWithMicroseconds[i]);
        }
    }
}

/**
 * Function using an 8 byte tick timing array to save program memory
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        if (i & 1) {
            // Odd
            space(aBufferWithTicks[i] * MICROS_PER_TICK);
        } else {
            mark(aBufferWithTicks[i] * MICROS_PER_TICK);
        }
    }
    IRLedOff();  // Always end with the LED off
}

/**
 * Function using an 16 byte microsecond timing array in FLASH for every purpose.
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw_P(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer,
        uint_fast8_t aIRFrequencyKilohertz) {
#if !defined(__AVR__)
    sendRaw(aBufferWithMicroseconds, aLengthOfBuffer, aIRFrequencyKilohertz); // Let the function work for non AVR platforms
#else
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);
    /*
     * Raw data starts with a mark
     */
    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        auto duration = pgm_read_word(&aBufferWithMicroseconds[i]);
        if (i & 1) {
            // Odd
            space(duration);
#  if defined(LOCAL_DEBUG)
            Serial.print(F("S="));
#  endif
        } else {
            mark(duration);
#  if defined(LOCAL_DEBUG)
            Serial.print(F("M="));
#  endif
        }
#  if defined(LOCAL_DEBUG)
        Serial.println(duration);
#  endif
    }
#endif
}

/**
 * New function using an 8 byte tick (50 us) timing array in FLASH to save program memory
 * Raw data starts with a Mark. No leading space as in received timing data!
 */
void IRsend::sendRaw_P(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz) {
#if !defined(__AVR__)
    sendRaw(aBufferWithTicks, aLengthOfBuffer, aIRFrequencyKilohertz); // Let the function work for non AVR platforms
#else
// Set IR carrier frequency
    enableIROut(aIRFrequencyKilohertz);

    uint_fast16_t duration;
    for (uint_fast16_t i = 0; i < aLengthOfBuffer; i++) {
        duration = pgm_read_byte(&aBufferWithTicks[i]) * (uint_fast16_t) MICROS_PER_TICK;
        if (i & 1) {
            // Odd
            space(duration);
#  if defined(LOCAL_DEBUG)
            Serial.print(F("S="));
#  endif
        } else {
            mark(duration);
#  if defined(LOCAL_DEBUG)
            Serial.print(F("M="));
#  endif
        }
    }
    IRLedOff();  // Always end with the LED off
#  if defined(LOCAL_DEBUG)
    Serial.println(duration);
#  endif
#endif
}

/**
 * Sends PulseDistance data from array
 * For LSB First the LSB of array[0] is sent first then all bits until MSB of array[0]. Next is LSB of array[1] and so on.
 * The output always ends with a space
 * Stop bit is always sent
 * @param aFlags    Evaluated flags are PROTOCOL_IS_MSB_FIRST and SUPPRESS_STOP_BIT. Stop bit is otherwise sent for all pulse distance protocols.
 */
void IRsend::sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo,
        IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
        int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidthFromArray(aFrequencyKHz, aDistanceWidthTimingInfo->HeaderMarkMicros,
            aDistanceWidthTimingInfo->HeaderSpaceMicros, aDistanceWidthTimingInfo->OneMarkMicros,
            aDistanceWidthTimingInfo->OneSpaceMicros, aDistanceWidthTimingInfo->ZeroMarkMicros,
            aDistanceWidthTimingInfo->ZeroSpaceMicros, aDecodedRawDataArray, aNumberOfBits, aFlags, aRepeatPeriodMillis,
            aNumberOfRepeats);
}
void IRsend::sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
        uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
        IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
        int_fast8_t aNumberOfRepeats) {

    // Set IR carrier frequency
    enableIROut(aFrequencyKHz);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    uint_fast8_t tNumberOf32Or64BitChunks = ((aNumberOfBits - 1) / BITS_IN_RAW_DATA_TYPE) + 1;

#if defined(LOCAL_DEBUG)
    // fist data
    Serial.print(F("Data[0]=0x"));
    Serial.print(aDecodedRawDataArray[0], HEX);
    if (tNumberOf32Or64BitChunks > 1) {
        Serial.print(F(" Data[1]=0x"));
        Serial.print(aDecodedRawDataArray[1], HEX);
    }
    Serial.print(F(" #="));
    Serial.println(aNumberOfBits);
    Serial.flush();
#endif

    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        // Header
        mark(aHeaderMarkMicros);
        space(aHeaderSpaceMicros);

        for (uint_fast8_t i = 0; i < tNumberOf32Or64BitChunks; ++i) {
            uint8_t tNumberOfBitsForOneSend;

            // Manage stop bit
            uint8_t tFlags;
            if (i == (tNumberOf32Or64BitChunks - 1)) {
                // End of data
                tNumberOfBitsForOneSend = aNumberOfBits;
                tFlags = aFlags;
            } else {
                // intermediate data
                tNumberOfBitsForOneSend = BITS_IN_RAW_DATA_TYPE;
                tFlags = aFlags | SUPPRESS_STOP_BIT; // No stop bit for leading data
            }

            sendPulseDistanceWidthData(aOneMarkMicros, aOneSpaceMicros, aZeroMarkMicros, aZeroSpaceMicros, aDecodedRawDataArray[i],
                    tNumberOfBitsForOneSend, tFlags);
            aNumberOfBits -= BITS_IN_RAW_DATA_TYPE;
        }

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (aRepeatPeriodMillis > tFrameDurationMillis) {
                delay(aRepeatPeriodMillis - tFrameDurationMillis);
            }
        }
    }
}

/**
 * Sends PulseDistance data from array using PulseDistanceWidthProtocolConstants
 * For LSB First the LSB of array[0] is sent first then all bits until MSB of array[0]. Next is LSB of array[1] and so on.
 * The output always ends with a space
 * Stop bit is always sent
 * @param aNumberOfBits     Number of bits from aDecodedRawDataArray to be actually sent.
 * @param aNumberOfRepeats  If < 0 and a aProtocolConstants->SpecialSendRepeatFunction() is specified
 *                          then it is called without leading and trailing space.
 */
void IRsend::sendPulseDistanceWidthFromArray(PulseDistanceWidthProtocolConstants *aProtocolConstants,
        IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, int_fast8_t aNumberOfRepeats) {

// Calling sendPulseDistanceWidthFromArray() costs 68 bytes program memory compared to the implementation below
//    sendPulseDistanceWidthFromArray(aProtocolConstants->FrequencyKHz, aProtocolConstants->DistanceWidthTimingInfo.HeaderMarkMicros,
//            aProtocolConstants->DistanceWidthTimingInfo.HeaderSpaceMicros,
//            aProtocolConstants->DistanceWidthTimingInfo.OneMarkMicros, aProtocolConstants->DistanceWidthTimingInfo.OneSpaceMicros,
//            aProtocolConstants->DistanceWidthTimingInfo.ZeroMarkMicros, aProtocolConstants->DistanceWidthTimingInfo.ZeroSpaceMicros,
//            aDecodedRawDataArray, aNumberOfBits, aProtocolConstants->Flags, aProtocolConstants->RepeatPeriodMillis,
//            aNumberOfRepeats);
    // Set IR carrier frequency
    enableIROut(aProtocolConstants->FrequencyKHz);

    uint_fast8_t tNumberOf32Or64BitChunks = ((aNumberOfBits - 1) / BITS_IN_RAW_DATA_TYPE) + 1;

#if defined(LOCAL_DEBUG)
    // fist data
    Serial.print(F("Data[0]=0x"));
    Serial.print(aDecodedRawDataArray[0], HEX);
    if (tNumberOf32Or64BitChunks > 1) {
        Serial.print(F(" Data[1]=0x"));
        Serial.print(aDecodedRawDataArray[1], HEX);
    }
    Serial.print(F(" #="));
    Serial.println(aNumberOfBits);
    Serial.flush();
#endif

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        auto tStartOfFrameMillis = millis();
        auto tNumberOfBits = aNumberOfBits; // refresh value for repeats

        // Header
        mark(aProtocolConstants->DistanceWidthTimingInfo.HeaderMarkMicros);
        space(aProtocolConstants->DistanceWidthTimingInfo.HeaderSpaceMicros);
        uint8_t tOriginalFlags = aProtocolConstants->Flags;

        for (uint_fast8_t i = 0; i < tNumberOf32Or64BitChunks; ++i) {
            uint8_t tNumberOfBitsForOneSend;

            uint8_t tFlags;
            if (i == (tNumberOf32Or64BitChunks - 1)) {
                // End of data
                tNumberOfBitsForOneSend = tNumberOfBits;
                tFlags = tOriginalFlags;
            } else {
                // intermediate data
                tNumberOfBitsForOneSend = BITS_IN_RAW_DATA_TYPE;
                tFlags = tOriginalFlags | SUPPRESS_STOP_BIT; // No stop bit for leading data
            }

            sendPulseDistanceWidthData(aProtocolConstants->DistanceWidthTimingInfo.OneMarkMicros,
                    aProtocolConstants->DistanceWidthTimingInfo.OneSpaceMicros,
                    aProtocolConstants->DistanceWidthTimingInfo.ZeroMarkMicros,
                    aProtocolConstants->DistanceWidthTimingInfo.ZeroSpaceMicros, aDecodedRawDataArray[i], tNumberOfBitsForOneSend,
                    tFlags);
            tNumberOfBits -= BITS_IN_RAW_DATA_TYPE;
        }

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (aProtocolConstants->RepeatPeriodMillis > tFrameDurationMillis) {
                delay(aProtocolConstants->RepeatPeriodMillis - tFrameDurationMillis);
            }
        }
    }
}

/**
 * Sends PulseDistance frames and repeats
 * @param aProtocolConstants    The constants to use for sending this protocol.
 * @param aData             uint32 or uint64 holding the bits to be sent.
 * @param aNumberOfBits     Number of bits from aData to be actually sent.
 * @param aNumberOfRepeats  If < 0 and a aProtocolConstants->SpecialSendRepeatFunction() is specified
 *                          then it is called without leading and trailing space.
 */
void IRsend::sendPulseDistanceWidth(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRRawDataType aData,
        uint_fast8_t aNumberOfBits, int_fast8_t aNumberOfRepeats) {

#if defined(LOCAL_DEBUG)
    Serial.print(F("Data=0x"));
    Serial.print(aData, HEX);
    Serial.print(F(" #="));
    Serial.println(aNumberOfBits);
    Serial.flush();
#endif

    if (aNumberOfRepeats < 0) {
        if (aProtocolConstants->SpecialSendRepeatFunction != NULL) {
            /*
             * Send only a special repeat and return
             */
            aProtocolConstants->SpecialSendRepeatFunction();
            return;
        } else {
            // Send only one plain frame (as repeat)
            aNumberOfRepeats = 0;
        }
    }

    // Set IR carrier frequency
    enableIROut(aProtocolConstants->FrequencyKHz);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        if (tNumberOfCommands < ((uint_fast8_t) aNumberOfRepeats + 1) && aProtocolConstants->SpecialSendRepeatFunction != NULL) {
            // send special repeat, if specified and we are not in the first loop
            aProtocolConstants->SpecialSendRepeatFunction();
        } else {
            /*
             * Send Header and regular frame
             */
            mark(aProtocolConstants->DistanceWidthTimingInfo.HeaderMarkMicros);
            space(aProtocolConstants->DistanceWidthTimingInfo.HeaderSpaceMicros);
            sendPulseDistanceWidthData(aProtocolConstants, aData, aNumberOfBits);
        }

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            auto tCurrentFrameDurationMillis = millis() - tStartOfFrameMillis;
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            if (aProtocolConstants->RepeatPeriodMillis > tCurrentFrameDurationMillis) {
                delay(aProtocolConstants->RepeatPeriodMillis - tCurrentFrameDurationMillis);
            }
        }
    }
}

/**
 * Sends PulseDistance frames and repeats.
 * @param aFrequencyKHz, aHeaderMarkMicros, aHeaderSpaceMicros, aOneMarkMicros, aOneSpaceMicros, aZeroMarkMicros, aZeroSpaceMicros, aFlags, aRepeatPeriodMillis     Values to use for sending this protocol, also contained in the PulseDistanceWidthProtocolConstants of this protocol.
 * @param aData             uint32 or uint64 holding the bits to be sent.
 * @param aNumberOfBits     Number of bits from aData to be actually sent.
 * @param aFlags            Evaluated flags are PROTOCOL_IS_MSB_FIRST and SUPPRESS_STOP_BIT. Stop bit is otherwise sent for all pulse distance protocols.
 * @param aNumberOfRepeats  If < 0 and a aProtocolConstants->SpecialSendRepeatFunction() is specified
 *                          then it is called without leading and trailing space.
 * @param aSpecialSendRepeatFunction    If NULL, the first frame is repeated completely, otherwise this function is used for sending the repeat frame.
 */
void IRsend::sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
        uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros, IRRawDataType aData,
        uint_fast8_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats,
        void (*aSpecialSendRepeatFunction)()) {

    if (aNumberOfRepeats < 0) {
        if (aSpecialSendRepeatFunction != NULL) {
            aSpecialSendRepeatFunction();
            return;
        } else {
            aNumberOfRepeats = 0; // send a plain frame as repeat
        }
    }

    // Set IR carrier frequency
    enableIROut(aFrequencyKHz);

    uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
    while (tNumberOfCommands > 0) {
        unsigned long tStartOfFrameMillis = millis();

        if (tNumberOfCommands < ((uint_fast8_t) aNumberOfRepeats + 1) && aSpecialSendRepeatFunction != NULL) {
            // send special repeat
            aSpecialSendRepeatFunction();
        } else {
            // Header and regular frame
            mark(aHeaderMarkMicros);
            space(aHeaderSpaceMicros);
            sendPulseDistanceWidthData(aOneMarkMicros, aOneSpaceMicros, aZeroMarkMicros, aZeroSpaceMicros, aData, aNumberOfBits,
                    aFlags);
        }

        tNumberOfCommands--;
        // skip last delay!
        if (tNumberOfCommands > 0) {
            /*
             * Check and fallback for wrong RepeatPeriodMillis parameter. I.e the repeat period must be greater than each frame duration.
             */
            auto tFrameDurationMillis = millis() - tStartOfFrameMillis;
            if (aRepeatPeriodMillis > tFrameDurationMillis) {
                delay(aRepeatPeriodMillis - tFrameDurationMillis);
            }
        }
    }
}

/**
 * Sends PulseDistance from data contained in parameter using ProtocolConstants structure for timing etc.
 * The output always ends with a space
 * Each additional call costs 16 bytes program memory
 * @param aProtocolConstants    The constants to use for sending this protocol.
 * @param aData                 uint32 or uint64 holding the bits to be sent.
 * @param aNumberOfBits         Number of bits from aData to be actually sent.
 */
void IRsend::sendPulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRRawDataType aData,
        uint_fast8_t aNumberOfBits) {

    sendPulseDistanceWidthData(aProtocolConstants->DistanceWidthTimingInfo.OneMarkMicros,
            aProtocolConstants->DistanceWidthTimingInfo.OneSpaceMicros, aProtocolConstants->DistanceWidthTimingInfo.ZeroMarkMicros,
            aProtocolConstants->DistanceWidthTimingInfo.ZeroSpaceMicros, aData, aNumberOfBits, aProtocolConstants->Flags);
}

/**
 * Sends PulseDistance data with timing parameters and flag parameters.
 * The output always ends with a space
 * @param aOneMarkMicros    Timing for sending this protocol.
 * @param aData             uint32 or uint64 holding the bits to be sent.
 * @param aNumberOfBits     Number of bits from aData to be actually sent.
 * @param aFlags            Evaluated flags are PROTOCOL_IS_MSB_FIRST and SUPPRESS_STOP_BIT. Stop bit is otherwise sent for all pulse distance protocols.
 */
void IRsend::sendPulseDistanceWidthData(uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros,
        uint16_t aZeroSpaceMicros, IRRawDataType aData, uint_fast8_t aNumberOfBits, uint8_t aFlags) {

#if defined(LOCAL_DEBUG)
    Serial.print(aData, HEX);
    Serial.print('|');
    Serial.println(aNumberOfBits);
    Serial.flush();
#endif

    // For MSBFirst, send data from MSB to LSB until mask bit is shifted out
    IRRawDataType tMask = 1ULL << (aNumberOfBits - 1);
    for (uint_fast8_t i = aNumberOfBits; i > 0; i--) {
        if (((aFlags & PROTOCOL_IS_MSB_FIRST) && (aData & tMask)) || (!(aFlags & PROTOCOL_IS_MSB_FIRST) && (aData & 1))) {
#if defined(LOCAL_TRACE)
            Serial.print('1');
#endif
            mark(aOneMarkMicros);
            space(aOneSpaceMicros);
        } else {
#if defined(LOCAL_TRACE)
            Serial.print('0');
#endif
            mark(aZeroMarkMicros);
            space(aZeroSpaceMicros);
        }
        if (aFlags & PROTOCOL_IS_MSB_FIRST) {
            tMask >>= 1;
        } else {
            aData >>= 1;
        }
    }
    /*
     * Stop bit is sent for all pulse distance protocols i.e. aOneSpaceMicros != aZeroSpaceMicros.
     * Therefore it is not sent for Sony :-)
     * For sending from an array, no intermediate stop bit must be sent for all but last data chunk.
     */
    if ((!(aFlags & SUPPRESS_STOP_BIT)) && (abs(aOneSpaceMicros - aZeroSpaceMicros) > (aOneSpaceMicros / 4))) {
        // Send stop bit here
#if defined(LOCAL_TRACE)
        Serial.print('S');
#endif
        mark(aOneMarkMicros); // Use aOneMarkMicros for stop bits. This seems to be correct for all protocols :-)
    }
#if defined(LOCAL_TRACE)
    Serial.println();
#endif
}

/**
 * Sends Biphase data MSB first
 * Always send start bit, do not send the trailing space of the start bit
 * 0 -> mark+space
 * 1 -> space+mark
 * The output always ends with a space
 * can only send 31 bit data, since we put the start bit as 32th bit on front
 * @param aData             uint32 or uint64 holding the bits to be sent.
 * @param aNumberOfBits     Number of bits from aData to be actually sent.
 */
void IRsend::sendBiphaseData(uint16_t aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits) {

    IR_TRACE_PRINT(F("0x"));
    IR_TRACE_PRINT(aData, HEX);

#if defined(LOCAL_TRACE)
    Serial.print('S');
#endif

// Data - Biphase code MSB first
// prepare for start with sending the start bit, which is 1
    uint32_t tMask = 1UL << aNumberOfBits;    // mask is now set for the virtual start bit
    uint_fast8_t tLastBitValue = 1;    // Start bit is a 1
    bool tNextBitIsOne = 1;    // Start bit is a 1
    for (uint_fast8_t i = aNumberOfBits + 1; i > 0; i--) {
        bool tCurrentBitIsOne = tNextBitIsOne;
        tMask >>= 1;
        tNextBitIsOne = ((aData & tMask) != 0) || (i == 1); // true for last bit to avoid extension of mark
        if (tCurrentBitIsOne) {
#if defined(LOCAL_TRACE)
            Serial.print('1');
#endif
            space(aBiphaseTimeUnit);
            if (tNextBitIsOne) {
                mark(aBiphaseTimeUnit);
            } else {
                // if next bit is 0, extend the current mark in order to generate a continuous signal without short breaks
                mark(2 * aBiphaseTimeUnit);
            }
            tLastBitValue = 1;

        } else {
#if defined(LOCAL_TRACE)
            Serial.print('0');
#endif
            if (!tLastBitValue) {
                mark(aBiphaseTimeUnit);
            }
            space(aBiphaseTimeUnit);
            tLastBitValue = 0;
        }
    }
    IR_TRACE_PRINTLN(F(""));
}

/**
 * Sends an IR mark for the specified number of microseconds.
 * The mark output is modulated at the PWM frequency if USE_NO_SEND_PWM is not defined.
 * The output is guaranteed to be OFF / inactive after after the call of the function.
 * This function may affect the state of feedback LED.
 * Period time is 26 us for 38.46 kHz, 27 us for 37.04 kHz, 25 us for 40 kHz.
 * On time is 8 us for 30% duty cycle
 *
 * The mark() function relies on the correct implementation of:
 * delayMicroseconds() for pulse time, and micros() for pause time.
 * The delayMicroseconds() of pulse time is guarded on AVR CPU's with noInterrupts() / interrupts().
 * At the start of pause time, interrupts are enabled once, the rest of the pause is also guarded on AVR CPU's with noInterrupts() / interrupts().
 * The maximum length of an interrupt during sending should not exceed 26 us - 8 us = 18 us, otherwise timing is disturbed.
 * This disturbance is no problem, if the exceedance is small and does not happen too often.
 */
void IRsend::mark(uint16_t aMarkMicros) {

#if defined(SEND_PWM_BY_TIMER) || defined(USE_NO_SEND_PWM)
#  if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
        setFeedbackLED(true);
    }
#  endif
#endif

#if defined(SEND_PWM_BY_TIMER)
    /*
     * Generate hardware PWM signal
     */
    enableSendPWMByTimer(); // Enable timer or ledcWrite() generated PWM output
    customDelayMicroseconds(aMarkMicros);
    IRLedOff(); // disables hardware PWM and manages feedback LED
    return;

#elif defined(USE_NO_SEND_PWM)
    /*
     * Here we generate no carrier PWM, just simulate an active low receiver signal.
     */
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
    // Here we have no hardware supported Open Drain outputs, so we must mimicking it
    pinModeFast(sendPin, OUTPUT); // active state for mimicking open drain
#  elif defined(USE_ACTIVE_HIGH_OUTPUT_FOR_SEND_PIN)
    digitalWriteFast(sendPin, HIGH); // Set output to active high.
#  else
    digitalWriteFast(sendPin, LOW); // Set output to active low.
#  endif

    customDelayMicroseconds(aMarkMicros);
    IRLedOff();
#  if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
        setFeedbackLED(false);
    }
    return;
#  endif

#else // defined(SEND_PWM_BY_TIMER)
    /*
     * Generate PWM by bit banging
     */
    unsigned long tStartMicros = micros();
    unsigned long tNextPeriodEnding = tStartMicros;
    unsigned long tMicros;
#  if !defined(NO_LED_FEEDBACK_CODE)
    bool FeedbackLedIsActive = false;
#  endif

    do {
//        digitalToggleFast(_IR_TIMING_TEST_PIN);
        /*
         * Output the PWM pulse
         */
        noInterrupts(); // do not let interrupts extend the short on period
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, LOW); // set output with pin mode OUTPUT_OPEN_DRAIN to active low
#    else
        pinModeFast(sendPin, OUTPUT); // active state for mimicking open drain
#    endif
#  else
        // 3.5 us from FeedbackLed on to pin setting. 5.7 us from call of mark() to pin setting incl. setting of feedback pin.
        // 4.3 us from do{ to pin setting if sendPin is no constant
        digitalWriteFast(sendPin, HIGH);
#  endif
        delayMicroseconds (periodOnTimeMicros); // On time is 8 us for 30% duty cycle. This is normally implemented by a blocking wait.

        /*
         * Output the PWM pause
         */
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
#    if defined(OUTPUT_OPEN_DRAIN)
        digitalWriteFast(sendPin, HIGH); // Set output with pin mode OUTPUT_OPEN_DRAIN to inactive high.
#    else
        pinModeFast(sendPin, INPUT); // to mimic the open drain inactive state
#    endif

#  else
        digitalWriteFast(sendPin, LOW);
#  endif
        /*
         * Enable interrupts at start of the longer off period. Required at least to keep micros correct.
         * If receive interrupt is still active, it takes 3.4 us from now until receive ISR is active (for 7 us + pop's)
         */
        interrupts();

#  if !defined(NO_LED_FEEDBACK_CODE)
        /*
         * Delayed call of setFeedbackLED() to get better startup timing, especially required for consecutive marks
         */
        if (!FeedbackLedIsActive) {
            FeedbackLedIsActive = true;
            if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
                setFeedbackLED(true);
            }
        }
#  endif
        /*
         * PWM pause timing
         * Measured delta between pause duration values are 13 us for a 16 MHz Uno (from 13 to 26), if interrupts are disabled below
         * Measured delta between pause duration values are 20 us for a 16 MHz Uno (from 7.8 to 28), if interrupts are not disabled below
         * Minimal pause duration is 5.2 us with NO_LED_FEEDBACK_CODE enabled
         * and 8.1 us with NO_LED_FEEDBACK_CODE disabled.
         */
        tNextPeriodEnding += periodTimeMicros;
#if defined(__AVR__) // micros() for STM sometimes give decreasing values if interrupts are disabled. See https://github.com/stm32duino/Arduino_Core_STM32/issues/1680
        noInterrupts(); // disable interrupts (especially the 20 us receive interrupts) only at start of the PWM pause. Otherwise it may extend the pause too much.
#endif
        do {
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
            digitalWriteFast(_IR_TIMING_TEST_PIN, HIGH); // 2 clock cycles
#endif
            /*
             * For AVR @16MHz we have only 4 us resolution.
             * The duration of the micros() call itself is 3 us.
             * It takes 0.9 us from signal going low here.
             * The rest of the loop takes 1.2 us with NO_LED_FEEDBACK_CODE enabled
             * and 3 us with NO_LED_FEEDBACK_CODE disabled.
             */
#if defined(_IR_MEASURE_TIMING) && defined(_IR_TIMING_TEST_PIN)
            digitalWriteFast(_IR_TIMING_TEST_PIN, LOW); // 2 clock cycles
#endif
            /*
             * Exit the forever loop if aMarkMicros has reached
             */
            tMicros = micros();
            uint16_t tDeltaMicros = tMicros - tStartMicros;
#if defined(__AVR__)
            // reset feedback led in the last pause before end
//            tDeltaMicros += (160 / CLOCKS_PER_MICRO); // adding this once increases program size, so do it below !
#  if !defined(NO_LED_FEEDBACK_CODE)
            if (tDeltaMicros >= aMarkMicros - (30 + (112 / CLOCKS_PER_MICRO))) { // 30 to be constant. Using periodTimeMicros increases program size too much.
                if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
                    setFeedbackLED(false);
                }
            }
#  endif
            // Just getting variables and check for end condition takes minimal 3.8 us
            if (tDeltaMicros >= aMarkMicros - (112 / CLOCKS_PER_MICRO)) { // To compensate for call duration - 112 is an empirical value
#else
            if (tDeltaMicros >= aMarkMicros) {
#  if !defined(NO_LED_FEEDBACK_CODE)
                if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
                    setFeedbackLED(false);
                }
#  endif
#endif
#if defined(__AVR__)
                interrupts();
#endif
                return;
            }
        } while (tMicros < tNextPeriodEnding);
    } while (true);
#  endif
}

/**
 * Just switch the IR sending LED off to send an IR space
 * A space is "no output", so the PWM output is disabled.
 * This function may affect the state of feedback LED.
 */
void IRsend::IRLedOff() {
#if defined(SEND_PWM_BY_TIMER)
    disableSendPWMByTimer(); // Disable PWM output
#elif defined(USE_NO_SEND_PWM)
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && !defined(OUTPUT_OPEN_DRAIN)
    digitalWriteFast(sendPin, LOW); // prepare for all next active states.
    pinModeFast(sendPin, INPUT);// inactive state for open drain
#  elif defined(USE_ACTIVE_HIGH_OUTPUT_FOR_SEND_PIN)
    digitalWriteFast(sendPin, LOW); // Set output to inactive low.
#  else
    digitalWriteFast(sendPin, HIGH); // Set output to inactive high.
#  endif
#else
#  if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
#    if defined(OUTPUT_OPEN_DRAIN)
    digitalWriteFast(sendPin, HIGH); // Set output to inactive high.
#    else
    pinModeFast(sendPin, INPUT); // inactive state to mimic open drain
#    endif
#  else
    digitalWriteFast(sendPin, LOW);
#  endif
#endif

#if !defined(NO_LED_FEEDBACK_CODE)
    if (FeedbackLEDControl.LedFeedbackEnabled == LED_FEEDBACK_ENABLED_FOR_SEND) {
        setFeedbackLED(false);
    }
#endif
}

/**
 * Sends an IR space for the specified number of microseconds.
 * A space is "no output", so just wait.
 */
void IRsend::space(uint16_t aSpaceMicros) {
    customDelayMicroseconds(aSpaceMicros);
}

/**
 * Custom delay function that circumvents Arduino's delayMicroseconds 16 bit limit
 * and is (mostly) not extended by the duration of interrupt codes like the millis() interrupt
 */
void IRsend::customDelayMicroseconds(unsigned long aMicroseconds) {
#if defined(ESP32) || defined(ESP8266)
    // from https://github.com/crankyoldgit/IRremoteESP8266/blob/00b27cc7ea2e7ac1e48e91740723c805a38728e0/src/IRsend.cpp#L123
    // Invoke a delay(), where possible, to avoid triggering the WDT.
    // see https://github.com/Arduino-IRremote/Arduino-IRremote/issues/1114 for the reason of checking for > 16383)
    // delayMicroseconds() is only accurate to 16383 us. Ref: https://www.arduino.cc/en/Reference/delayMicroseconds
    if (aMicroseconds > 16383) {
        delay(aMicroseconds / 1000UL);  // Delay for as many whole milliseconds as we can.
        // Delay the remaining sub-millisecond.
        delayMicroseconds(static_cast<uint16_t>(aMicroseconds % 1000UL));
    } else {
        delayMicroseconds(aMicroseconds);
    }
#else

#  if defined(__AVR__)
    unsigned long start = micros() - (64 / clockCyclesPerMicrosecond()); // - (64 / clockCyclesPerMicrosecond()) for reduced resolution and additional overhead
#  else
    unsigned long start = micros();
#  endif
// overflow invariant comparison :-)
    while (micros() - start < aMicroseconds) {
    }
#endif
}

/**
 * Enables IR output. The kHz value controls the modulation frequency in kilohertz.
 * IF PWM should be generated by a timer, it uses the platform specific timerConfigForSend() function,
 * otherwise it computes the delays used by the mark() function.
 * If IR_SEND_PIN is defined, maximum PWM frequency for an AVR @16 MHz is 170 kHz (180 kHz if NO_LED_FEEDBACK_CODE is defined)
 */
void IRsend::enableIROut(uint_fast8_t aFrequencyKHz) {
#if defined(SEND_PWM_BY_TIMER)
    timerConfigForSend(aFrequencyKHz); // must set output pin mode and disable receive interrupt if required, e.g. uses the same resource

#elif defined(USE_NO_SEND_PWM)
    (void) aFrequencyKHz;

#else
    periodTimeMicros = (1000U + (aFrequencyKHz / 2)) / aFrequencyKHz; // rounded value -> 26 for 38.46 kHz, 27 for 37.04 kHz, 25 for 40 kHz.
#  if defined(IR_SEND_PIN)
    periodOnTimeMicros = (((periodTimeMicros * IR_SEND_DUTY_CYCLE_PERCENT) + 50) / 100U); // +50 for rounding -> 830/100 for 30% and 16 MHz
#  else
// Heuristics! We require a nanosecond correction for "slow" digitalWrite() functions
    periodOnTimeMicros = (((periodTimeMicros * IR_SEND_DUTY_CYCLE_PERCENT) + 50 - (PULSE_CORRECTION_NANOS / 10)) / 100U); // +50 for rounding -> 530/100 for 30% and 16 MHz
#  endif
#endif // defined(SEND_PWM_BY_TIMER)

#if defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN) && defined(OUTPUT_OPEN_DRAIN) // the mode INPUT for mimicking open drain is set at IRLedOff()
#  if defined(IR_SEND_PIN)
    pinModeFast(IR_SEND_PIN, OUTPUT_OPEN_DRAIN);
#  else
    pinModeFast(sendPin, OUTPUT_OPEN_DRAIN);
#  endif
#else

// For Non AVR platforms pin mode for SEND_PWM_BY_TIMER must be handled by the timerConfigForSend() function
// because ESP 2.0.2 ledcWrite does not work if pin mode is set, and RP2040 requires gpio_set_function(IR_SEND_PIN, GPIO_FUNC_PWM);
#  if defined(__AVR__) || !defined(SEND_PWM_BY_TIMER)
#    if defined(IR_SEND_PIN)
    pinModeFast(IR_SEND_PIN, OUTPUT);
#    else
    pinModeFast(sendPin, OUTPUT);
#    endif
#  endif
#endif // defined(USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN)
}

#if defined(SEND_PWM_BY_TIMER)
// Used for Bang&Olufsen
void IRsend::enableHighFrequencyIROut(uint_fast16_t aFrequencyKHz) {
    timerConfigForSend(aFrequencyKHz); // must set output pin mode and disable receive interrupt if required, e.g. uses the same resource
    // For Non AVR platforms pin mode for SEND_PWM_BY_TIMER must be handled by the timerConfigForSend() function
    // because ESP 2.0.2 ledcWrite does not work if pin mode is set, and RP2040 requires gpio_set_function(IR_SEND_PIN, GPIO_FUNC_PWM);
#  if defined(__AVR__)
#    if defined(IR_SEND_PIN)
    pinModeFast(IR_SEND_PIN, OUTPUT);
#    else
    pinModeFast(sendPin, OUTPUT);
#    endif
#  endif
}
#endif

uint16_t IRsend::getPulseCorrectionNanos() {
    return PULSE_CORRECTION_NANOS;
}

/** @}*/
#if defined(_IR_MEASURE_TIMING)
#undef _IR_MEASURE_TIMING
#endif
#if defined(LOCAL_TRACE)
#undef LOCAL_TRACE
#endif
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _IR_SEND_HPP
