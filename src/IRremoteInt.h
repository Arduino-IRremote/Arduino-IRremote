/**
 * @file IRremoteInt.h
 * @brief Contains all declarations required for the interface to IRremote.
 * Could not be named IRremote.h, since this has another semantic (it must include all *.hpp files) for old example code found in the wild.
 *
 * This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2015-2022 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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
#ifndef _IR_REMOTE_INT_H
#define _IR_REMOTE_INT_H

#include <Arduino.h>

#define MARK   1
#define SPACE  0

#if defined(PARTICLE)
#define F_CPU 16000000 // definition for a board for which F_CPU is not defined
#endif
#if defined(F_CPU) // F_CPU is used to generate the receive send timings in some CPU's
#define CLOCKS_PER_MICRO (F_CPU / MICROS_IN_ONE_SECOND)
#endif

/*
 * For backwards compatibility
 */
#if defined(SYSCLOCK) // allow for processor specific code to define F_CPU
#undef F_CPU
#define F_CPU SYSCLOCK // Clock frequency to be used for timing.
#endif

//#define DEBUG // Activate this for lots of lovely debug output from the IRremote core and all protocol decoders.
//#define TRACE // Activate this for more debug output.

/**
 * For better readability of code
 */
#define DISABLE_LED_FEEDBACK            false
#define ENABLE_LED_FEEDBACK             true
#define USE_DEFAULT_FEEDBACK_LED_PIN    0

#include "IRProtocol.h"

/****************************************************
 * Declarations for the receiver Interrupt Service Routine
 ****************************************************/
// ISR State-Machine : Receiver States
#define IR_REC_STATE_IDLE      0
#define IR_REC_STATE_MARK      1
#define IR_REC_STATE_SPACE     2
#define IR_REC_STATE_STOP      3 // set to IR_REC_STATE_IDLE only by resume()

/**
 * This struct contains the data and control used for receiver static functions and the ISR (interrupt service routine)
 * Only StateForISR needs to be volatile. All the other fields are not written by ISR after data available and before start/resume.
 */
struct irparams_struct {
    // The fields are ordered to reduce memory over caused by struct-padding
    volatile uint8_t StateForISR;       ///< State Machine state
    uint_fast8_t IRReceivePin;          ///< Pin connected to IR data from detector
#if defined(__AVR__)
    volatile uint8_t *IRReceivePinPortInputRegister;
    uint8_t IRReceivePinMask;
#endif
    uint_fast16_t TickCounterForISR;    ///< Counts 50uS ticks. The value is copied into the rawbuf array on every transition.

    bool OverflowFlag;                  ///< Raw buffer OverflowFlag occurred
#if RAW_BUFFER_LENGTH <= 254            // saves around 75 bytes program memory and speeds up ISR
    uint_fast8_t rawlen;                ///< counter of entries in rawbuf
#else
    uint_fast16_t rawlen;               ///< counter of entries in rawbuf
#endif
    unsigned int rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space, first entry is the length of the gap between previous and current command
};

/*
 * Debug directives
 */
#if defined(DEBUG) || defined(TRACE)
#  define IR_DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
#  define IR_DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
/**
 * If DEBUG, print the arguments, otherwise do nothing.
 */
#  define IR_DEBUG_PRINT(...) void()
/**
 * If DEBUG, print the arguments as a line, otherwise do nothing.
 */
#  define IR_DEBUG_PRINTLN(...) void()
#endif

#if defined(TRACE)
#  define IR_TRACE_PRINT(...)    Serial.print(__VA_ARGS__)
#  define IR_TRACE_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#  define IR_TRACE_PRINT(...) void()
#  define IR_TRACE_PRINTLN(...) void()
#endif

/****************************************************
 *                     RECEIVING
 ****************************************************/
/*
 * Definitions for member IRData.flags
 */
#define IRDATA_FLAGS_EMPTY              0x00
#define IRDATA_FLAGS_IS_REPEAT          0x01
#define IRDATA_FLAGS_IS_AUTO_REPEAT     0x02
#define IRDATA_FLAGS_PARITY_FAILED      0x04 ///< the current (autorepeat) frame violated parity check
#define IRDATA_FLAGS_TOGGLE_BIT         0x08 ///< is set if RC5 or RC6 toggle bit is set
#define IRDATA_FLAGS_EXTRA_INFO         0x10 ///< there is extra info not contained in address and data (e.g. Kaseikyo unknown vendor ID)
#define IRDATA_FLAGS_WAS_OVERFLOW       0x40 ///< irparams.rawlen is 0 in this case to avoid endless OverflowFlag
#define IRDATA_FLAGS_IS_LSB_FIRST       0x00
#define IRDATA_FLAGS_IS_MSB_FIRST       0x80 ///< Just for info. Value is mainly determined by the protocol

// deprecated
#define IRDATA_TOGGLE_BIT_MASK          0x08 ///< is set if RC5 or RC6 toggle bit is set

#define RAW_DATA_ARRAY_SIZE             ((((RAW_BUFFER_LENGTH - 2) - 1) / 64) + 1) // The -2 is for initial gap + stop bit mark, 64 mark + spaces for 32 bit.
/**
 * Data structure for the user application, available as decodedIRData.
 * Filled by decoders and read by print functions or user application.
 */
struct IRData {
    decode_type_t protocol; ///< UNKNOWN, NEC, SONY, RC5, PULSE_DISTANCE, ...
    uint16_t address;       ///< Decoded address, Distance protocol (OneMarkTicks << 8) | OneSpaceTicks
    uint16_t command;       ///< Decoded command, Distance protocol (ZeroMarkTicks << 8) | ZeroSpaceTicks
    uint16_t extra;         ///< Contains upper 16 bit of Magiquest WandID, Kaseikyo unknown vendor ID and Distance protocol (HeaderMarkTicks << 8) | HeaderSpaceTicks.
    uint16_t numberOfBits;  ///< Number of bits received for data (address + command + parity) - to determine protocol length if different length are possible.
    uint8_t flags;          ///< See IRDATA_FLAGS_* definitions above
    uint32_t decodedRawData; ///< Up to 32 bit decoded raw data, to be used for send functions.
#if defined(DECODE_DISTANCE)
    uint32_t decodedRawDataArray[RAW_DATA_ARRAY_SIZE]; ///< 32 bit decoded raw data, to be used for send function.
#endif
    irparams_struct *rawDataPtr; ///< Pointer of the raw timing data to be decoded. Mainly the data buffer filled by receiving ISR.
};

/**
 * Results returned from old decoders !!!deprecated!!!
 */
struct decode_results {
    decode_type_t decode_type;  // deprecated, moved to decodedIRData.protocol ///< UNKNOWN, NEC, SONY, RC5, ...
    uint16_t address;           // Used by Panasonic & Sharp [16-bits]
    uint32_t value;             // deprecated, moved to decodedIRData.decodedRawData ///< Decoded value / command [max 32-bits]
    uint8_t bits;               // deprecated, moved to decodedIRData.numberOfBits ///< Number of bits in decoded value
    uint16_t magnitude;         // deprecated, moved to decodedIRData.extra ///< Used by MagiQuest [16-bits]
    bool isRepeat;              // deprecated, moved to decodedIRData.flags ///< True if repeat of value is detected

// next 3 values are copies of irparams values - see IRremoteint.h
    unsigned int *rawbuf;       // deprecated, moved to decodedIRData.rawDataPtr->rawbuf ///< Raw intervals in 50uS ticks
    uint_fast8_t rawlen;        // deprecated, moved to decodedIRData.rawDataPtr->rawlen ///< Number of records in rawbuf
    bool overflow;              // deprecated, moved to decodedIRData.flags ///< true if IR raw code too long
};

/**
 * Main class for receiving IR signals
 */
class IRrecv {
public:

    IRrecv();
    IRrecv(uint_fast8_t aReceivePin);
    IRrecv(uint_fast8_t aReceivePin, uint_fast8_t aFeedbackLEDPin);
    void setReceivePin(uint_fast8_t aReceivePinNumber);

    /*
     * Stream like API
     */
    void begin(uint_fast8_t aReceivePin, bool aEnableLEDFeedback = false, uint_fast8_t aFeedbackLEDPin =
    USE_DEFAULT_FEEDBACK_LED_PIN);
    void start();
    void enableIRIn(); // alias for start
    void start(uint32_t aMicrosecondsToAddToGapCounter);
    void restartAfterSend();

    bool available();
    IRData* read(); // returns decoded data
    // write is a method of class IRsend below
    // size_t write(IRData *aIRSendData, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void stop();
    void disableIRIn(); // alias for stop
    void end(); // alias for stop

    bool isIdle();

    /*
     * The main functions
     */
    bool decode();  // Check if available and try to decode
    void resume();  // Enable receiving of the next value

    /*
     * Useful info and print functions
     */
    void printIRResultMinimal(Print *aSerial);
    void printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void printIRResultAsCVariables(Print *aSerial);

    /*
     * Next 4 functions are also available as non member functions
     */
    void printIRResultShort(Print *aSerial);
    void printIRSendUsage(Print *aSerial);
    const __FlashStringHelper* getProtocolString();
    static void printActiveIRProtocols(Print *aSerial);

    void compensateAndPrintIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void compensateAndPrintIRResultAsPronto(Print *aSerial, unsigned int frequency = 38000U);

    /*
     * Store the data for further processing
     */
    void compensateAndStoreIRResultInArray(uint8_t *aArrayPtr);
    size_t compensateAndStorePronto(String *aString, unsigned int frequency = 38000U);

    /*
     * The main decoding functions used by the individual decoders
     */
    bool decodePulseDistanceData(uint_fast8_t aNumberOfBits, uint_fast8_t aStartOffset, unsigned int aBitMarkMicros,
            unsigned int aOneSpaceMicros, unsigned int aZeroSpaceMicros, bool aMSBfirst);

    bool decodePulseWidthData(uint_fast8_t aNumberOfBits, uint_fast8_t aStartOffset, unsigned int aOneMarkMicros,
            unsigned int aZeroMarkMicros, unsigned int aBitSpaceMicros, bool aMSBfirst);

    bool decodeBiPhaseData(uint_fast8_t aNumberOfBits, uint_fast8_t aStartOffset, uint_fast8_t aStartClockCount,
            uint_fast8_t aValueOfSpaceToMarkTransition, unsigned int aBiphaseTimeUnit);

    void initBiphaselevel(uint_fast8_t aRCDecodeRawbuffOffset, unsigned int aBiphaseTimeUnit);
    uint_fast8_t getBiphaselevel();

    /*
     * All standard (decode address + command) protocol decoders
     */
    bool decodeBoseWave();
    bool decodeDenon();
    bool decodeJVC();
    bool decodeKaseikyo();
    bool decodeLegoPowerFunctions();
    bool decodeLG();
    bool decodeMagiQuest(); // not completely standard
    bool decodeNEC();
    bool decodeRC5();
    bool decodeRC6();
    bool decodeSamsung();
    bool decodeSharp(); // redirected to decodeDenon()
    bool decodeSony();
    bool decodeWhynter();

    bool decodeDistance();

    bool decodeHash();

    // Template function :-)
    bool decodeShuzu();

    /*
     * Old functions
     */
    bool decodeDenonOld(decode_results *aResults);
    bool decodeJVCMSB(decode_results *aResults);
    bool decodeLGMSB(decode_results *aResults);
    bool decodeNECMSB(decode_results *aResults);
    bool decodePanasonicMSB(decode_results *aResults);
    bool decodeSonyMSB(decode_results *aResults);
    bool decodeSAMSUNG(decode_results *aResults);
    bool decodeHashOld(decode_results *aResults);

    bool decode(
            decode_results *aResults)
                    __attribute__ ((deprecated ("Please use IrReceiver.decode() without a parameter and IrReceiver.decodedIRData.<fieldname> ."))); // deprecated

    // for backward compatibility. Now in IRFeedbackLED.hpp
    void blink13(uint8_t aEnableLEDFeedback)
            __attribute__ ((deprecated ("Please use setLEDFeedback() or enableLEDFeedback() / disableLEDFeedback()."))); // deprecated

    /*
     * Internal functions
     */
    void initDecodedIRData();
    uint_fast8_t compare(unsigned int oldval, unsigned int newval);

    IRData decodedIRData;       // New: decoded IR data for the application

    // Last decoded IR data for repeat detection
    decode_type_t lastDecodedProtocol;
    uint32_t lastDecodedAddress;
    uint32_t lastDecodedCommand;

    uint8_t repeatCount;        // Used e.g. for Denon decode for autorepeat decoding.
};

extern uint_fast8_t sBiphaseDecodeRawbuffOffset; //

/*
 * Mark & Space matching functions
 */
bool matchTicks(unsigned int aMeasuredTicks, unsigned int aMatchValueMicros);
bool matchMark(unsigned int aMeasuredTicks, unsigned int aMatchValueMicros);
bool matchSpace(unsigned int aMeasuredTicks, unsigned int aMatchValueMicros);

/*
 * Old function names
 */
bool MATCH(unsigned int measured, unsigned int desired);
bool MATCH_MARK(unsigned int measured_ticks, unsigned int desired_us);
bool MATCH_SPACE(unsigned int measured_ticks, unsigned int desired_us);

int getMarkExcessMicros();
/*
 * Next 4 functions are also available as member functions
 */
void printIRResultShort(Print *aSerial, IRData *aIRDataPtr, bool aPrintGap);
void printIRSendUsage(Print *aSerial, IRData *aIRDataPtr);
const __FlashStringHelper* getProtocolString();
void printActiveIRProtocols(Print *aSerial);

/****************************************************
 * Feedback LED related functions
 ****************************************************/
#define DO_NOT_ENABLE_LED_FEEDBACK          0x00
#define LED_FEEDBACK_DISABLED_COMPLETELY    0x00
#define LED_FEEDBACK_ENABLED_FOR_RECEIVE    0x01
#define LED_FEEDBACK_ENABLED_FOR_SEND       0x02
void setFeedbackLED(bool aSwitchLedOn);
void setLEDFeedback(uint8_t aFeedbackLEDPin, uint8_t aEnableLEDFeedback); // if aFeedbackLEDPin == 0, then take board BLINKLED_ON() and BLINKLED_OFF() functions
void setLEDFeedback(bool aEnableLEDFeedback); // Direct replacement for blink13()
void enableLEDFeedback();
constexpr auto enableLEDFeedbackForReceive = enableLEDFeedback; // alias for enableLEDFeedback
void disableLEDFeedback();
constexpr auto disableLEDFeedbackForReceive = disableLEDFeedback; // alias for enableLEDFeedback
void enableLEDFeedbackForSend();
void disableLEDFeedbackForSend();

void setBlinkPin(uint8_t aFeedbackLEDPin) __attribute__ ((deprecated ("Please use setLEDFeedback()."))); // deprecated

/*
 * Pulse parms are ((X*50)-MARK_EXCESS_MICROS) for the Mark and ((X*50)+MARK_EXCESS_MICROS) for the Space.
 * First MARK is the one after the long gap
 * Pulse parameters in microseconds
 */
#if !defined(TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING)
#define TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING    25 // Relative tolerance (in percent) for matchTicks(), matchMark() and matchSpace() functions used for protocol decoding.
#endif

/** Lower tolerance for comparison of measured data */
//#define LTOL            (1.0 - (TOLERANCE/100.))
#define LTOL            (100 - TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING)
/** Upper tolerance for comparison of measured data */
//#define UTOL            (1.0 + (TOLERANCE/100.))
#define UTOL            (100 + TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING)

//#define TICKS_LOW(us)   ((int)(((us)*LTOL/MICROS_PER_TICK)))
//#define TICKS_HIGH(us)  ((int)(((us)*UTOL/MICROS_PER_TICK + 1)))
#if MICROS_PER_TICK == 50 && TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING == 25           // Defaults
#define TICKS_LOW(us)   ((us)/67 )     // (us) / ((MICROS_PER_TICK:50 / LTOL:75 ) * 100)
#define TICKS_HIGH(us)  (((us)/40) + 1)  // (us) / ((MICROS_PER_TICK:50 / UTOL:125) * 100) + 1
#else
#define TICKS_LOW(us)   ((unsigned int ) ((long) (us) * LTOL / (MICROS_PER_TICK * 100) ))
#define TICKS_HIGH(us)  ((unsigned int ) ((long) (us) * UTOL / (MICROS_PER_TICK * 100) + 1))
#endif

/*
 * The receiver instance
 */
extern IRrecv IrReceiver;

/****************************************************
 *                     SENDING
 ****************************************************/

/**
 * Just for better readability of code
 */
#define NO_REPEATS  0
#define SEND_STOP_BIT true
#define SEND_NO_STOP_BIT false
#define SEND_REPEAT_COMMAND true ///< used for e.g. NEC, where a repeat is different from just repeating the data.

/**
 * Main class for sending IR signals
 */
class IRsend {
public:
    IRsend();

    /*
     * IR_SEND_PIN is defined
     */
#if defined(IR_SEND_PIN)
    void begin();
    void begin(bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin = USE_DEFAULT_FEEDBACK_LED_PIN);
#else
    IRsend(uint_fast8_t aSendPin);
    void begin(uint_fast8_t aSendPin);
    void setSendPin(uint_fast8_t aSendPin); // required if we use IRsend() as constructor
#endif

    // Not guarded for backward compatibility
    void begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin = USE_DEFAULT_FEEDBACK_LED_PIN);

    size_t write(IRData *aIRSendData, uint_fast8_t aNumberOfRepeats = NO_REPEATS);

    void enableIROut(uint_fast8_t aFrequencyKHz);

    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, unsigned int aHeaderMarkMicros, unsigned int aHeaderSpaceMicros, unsigned int aOneMarkMicros,
            unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros, unsigned int aZeroSpaceMicros,
            uint32_t *aDecodedRawDataArray, unsigned int aNumberOfBits, bool aMSBfirst, unsigned int aRepeatPeriodMillis = 110,
            uint_fast8_t aNumberOfRepeats = 0);
    void sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, unsigned int aHeaderMarkMicros, unsigned int aHeaderSpaceMicros, unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
            unsigned int aZeroSpaceMicros, uint32_t aData, uint_fast8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit, unsigned int aRepeatPeriodMillis = 110,
            uint_fast8_t aNumberOfRepeats = 0);
    void sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
            unsigned int aZeroSpaceMicros, uint32_t aData, uint_fast8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit = false);
    void sendBiphaseData(unsigned int aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits);

    void mark(unsigned int aMarkMicros);
    void space(unsigned int aSpaceMicros);
    void IRLedOff();

// 8 Bit array
    void sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);

// 16 Bit array
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);

    /*
     * New send functions
     */
    void sendBoseWave(uint8_t aCommand, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendDenon(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendSharp = false);
    void sendDenonRaw(uint16_t aRawData, uint_fast8_t aNumberOfRepeats = 0)
#if !defined (DOXYGEN)
            __attribute__ ((deprecated ("Please use sendDenon(aAddress, aCommand, aNumberOfRepeats).")));
#endif
    void sendJVC(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats);

    void sendLGRepeat(bool aUseLG2Protocol = false);
    void sendLG(uint8_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialLGRepeat = false, bool aUseLG2Protocol =
            false);
    void sendLG2(uint8_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialLGRepeat = false);
    void sendLGRaw(uint32_t aRawData, uint_fast8_t aNumberOfRepeats = 0, bool aSendOnlySpecialLGRepeat = false, bool aUseLG2Protocol = false);

    void sendNECRepeat();
    void sendNEC(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialNECRepeat = false);
    void sendNEC2(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats);
    void sendNECRaw(uint32_t aRawData, uint_fast8_t aNumberOfRepeats = 0, bool aSendOnlySpecialNECRepeat = false);
    // NEC variants
    void sendOnkyo(uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialNECRepeat = false);
    void sendApple(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialNECRepeat = false);

    void sendKaseikyo(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats, uint16_t aVendorCode); // LSB first
    void sendPanasonic(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Denon(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Mitsubishi(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Sharp(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_JVC(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats); // LSB first

    void sendRC5(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC6(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendSamsungLGRepeat();
    void sendSamsung(uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats);
    void sendSamsungLG(uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendOnlySpecialSamsungRepeat = false);
    void sendSharp(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSony(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, uint8_t numberOfBits = SIRCS_12_PROTOCOL);

    void sendLegoPowerFunctions(uint8_t aChannel, uint8_t tCommand, uint8_t aMode, bool aDoSend5Times = true);
    void sendLegoPowerFunctions(uint16_t aRawData, bool aDoSend5Times = true);
    void sendLegoPowerFunctions(uint16_t aRawData, uint8_t aChannel, bool aDoSend5Times = true);

    void sendMagiQuest(uint32_t wand_id, uint16_t magnitude);

    void sendPronto(const __FlashStringHelper *str, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendPronto(const char *prontoHexString, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendPronto(const uint16_t *data, unsigned int length, uint_fast8_t aNumberOfRepeats = NO_REPEATS);

#if defined(__AVR__)
    void sendPronto_PF(uint_farptr_t str, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendPronto_P(const char *str, uint_fast8_t aNumberOfRepeats);
#endif

// Template protocol :-)
    void sendShuzu(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats);

    /*
     * OLD send functions
     */
    void sendDenon(unsigned long data, int nbits);
    void sendDISH(unsigned long data, int nbits);
    void sendJVC(unsigned long data, int nbits,
            bool repeat)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendJVC(aAddress, aCommand, aNumberOfRepeats)."))) {
        sendJVCMSB(data, nbits, repeat);
    }
    void sendJVCMSB(unsigned long data, int nbits, bool repeat = false);

    void sendLG(unsigned long data, int nbits);

    void sendNEC(uint32_t aRawData,
            uint8_t nbits)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendNEC(aAddress, aCommand, aNumberOfRepeats)."))) {
        sendNECMSB(aRawData, nbits);
    }
    void sendNECMSB(uint32_t data, uint8_t nbits, bool repeat = false);
    void sendPanasonic(uint16_t aAddress,
            uint32_t aData)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendPanasonic(aAddress, aCommand, aNumberOfRepeats).")));
    void sendRC5(uint32_t data, uint8_t nbits);
    void sendRC5ext(uint8_t addr, uint8_t cmd, bool toggle);
    void sendRC6(uint32_t data, uint8_t nbits);
    void sendRC6(uint64_t data, uint8_t nbits);
    void sendSharpRaw(unsigned long data, int nbits);
    void sendSharp(unsigned int address, unsigned int command);
    void sendSAMSUNG(unsigned long data, int nbits);
    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendSamsung().")));
    void sendSony(unsigned long data,
            int nbits)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendSony(aAddress, aCommand, aNumberOfRepeats).")));
    ;
    void sendWhynter(unsigned long data, int nbits);

#if !defined(IR_SEND_PIN)
    uint8_t sendPin;
#endif
    unsigned int periodTimeMicros;
    unsigned int periodOnTimeMicros; // compensated with PULSE_CORRECTION_NANOS for duration of digitalWrite. Around 8 microseconds for 38 kHz.
    unsigned int getPulseCorrectionNanos();

    void customDelayMicroseconds(unsigned long aMicroseconds);
};

/*
 * The sender instance
 */
extern IRsend IrSender;

#endif // _IR_REMOTE_INT_H
