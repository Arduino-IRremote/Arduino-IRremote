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
 * Copyright (c) 2015-2023 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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

/**
 * The length of the buffer where the IR timing data is stored before decoding
 * 100 is sufficient for most standard protocols, but air conditioners often send a longer protocol data stream
 */
#if !defined(RAW_BUFFER_LENGTH)
#  if defined(DECODE_MAGIQUEST)
#define RAW_BUFFER_LENGTH  112  // MagiQuest requires 112 bytes.
#  else
#define RAW_BUFFER_LENGTH  100  ///< Length of raw duration buffer. Must be even. 100 supports up to 48 bit codings inclusive 1 start and 1 stop bit.
//#define RAW_BUFFER_LENGTH  750  // 750 (600 if we have only 2k RAM) is the value for air condition remotes.
#  endif
#endif
#if RAW_BUFFER_LENGTH % 2 == 1
#error RAW_BUFFER_LENGTH must be even, since the array consists of space / mark pairs.
#endif

/****************************************************
 * Declarations for the receiver Interrupt Service Routine
 ****************************************************/
// ISR State-Machine : Receiver States
#define IR_REC_STATE_IDLE      0 // Counting the gap time and waiting for the start bit to arrive
#define IR_REC_STATE_MARK      1 // A mark was received and we are counting the duration of it.
#define IR_REC_STATE_SPACE     2 // A space was received and we are counting the duration of it. If space is too long, we assume end of frame.
#define IR_REC_STATE_STOP      3 // Stopped until set to IR_REC_STATE_IDLE which can only be done by resume()

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
    volatile uint_fast16_t TickCounterForISR; ///< Counts 50uS ticks. The value is copied into the rawbuf array on every transition.
#if !IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK
    void (*ReceiveCompleteCallbackFunction)(void); ///< The function to call if a protocol message has arrived, i.e. StateForISR changed to IR_REC_STATE_STOP
#endif
    bool OverflowFlag;                  ///< Raw buffer OverflowFlag occurred
#if RAW_BUFFER_LENGTH <= 254            // saves around 75 bytes program memory and speeds up ISR
    uint_fast8_t rawlen;                ///< counter of entries in rawbuf
#else
    uint_fast16_t rawlen;               ///< counter of entries in rawbuf
#endif
    uint16_t rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space, first entry is the length of the gap between previous and current command
};

#if (__INT_WIDTH__ < 32)
typedef uint32_t IRRawDataType;
#define BITS_IN_RAW_DATA_TYPE   32
#else
typedef uint64_t IRRawDataType;
#define BITS_IN_RAW_DATA_TYPE   64
#endif
#include "IRProtocol.h"

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
    uint16_t *rawbuf;       // deprecated, moved to decodedIRData.rawDataPtr->rawbuf ///< Raw intervals in 50uS ticks
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
    void registerReceiveCompleteCallback(void (*aReceiveCompleteCallbackFunction)(void));
    /*
     * Stream like API
     */
    void begin(uint_fast8_t aReceivePin, bool aEnableLEDFeedback = false, uint_fast8_t aFeedbackLEDPin =
    USE_DEFAULT_FEEDBACK_LED_PIN);
    void start();
    void enableIRIn(); // alias for start
    void start(uint32_t aMicrosecondsToAddToGapCounter);
    void startWithTicksToAdd(uint16_t aTicksToAddToGapCounter);
    void restartAfterSend();

    void addTicksToInternalTickCounter(uint16_t aTicksToAddToInternalTickCounter);
    void addMicrosToInternalTickCounter(uint16_t aMicrosecondsToAddToInternalTickCounter);

    bool available();
    IRData* read(); // returns decoded data
    // write is a method of class IRsend below
    // size_t write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats = NO_REPEATS);
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
    uint32_t getTotalDurationOfRawData();

    /*
     * Next 4 functions are also available as non member functions
     */
    bool printIRResultShort(Print *aSerial, bool aPrintRepeatGap = true, bool aCheckForRecordGapsMicros = true);
    void printDistanceWidthTimingInfo(Print *aSerial, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo);
    void printIRSendUsage(Print *aSerial);
#if defined(__AVR__)
    const __FlashStringHelper* getProtocolString();
#else
    const char* getProtocolString();
#endif
    static void printActiveIRProtocols(Print *aSerial);

    void compensateAndPrintIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void compensateAndPrintIRResultAsPronto(Print *aSerial, uint16_t frequency = 38000U);

    /*
     * Store the data for further processing
     */
    void compensateAndStoreIRResultInArray(uint8_t *aArrayPtr);
    size_t compensateAndStorePronto(String *aString, uint16_t frequency = 38000U);

    /*
     * The main decoding functions used by the individual decoders
     */
    bool decodePulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, uint_fast8_t aNumberOfBits,
            uint_fast8_t aStartOffset = 3);

    bool decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, uint_fast8_t aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aZeroMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst);

    bool decodeBiPhaseData(uint_fast8_t aNumberOfBits, uint_fast8_t aStartOffset, uint_fast8_t aStartClockCount,
            uint_fast8_t aValueOfSpaceToMarkTransition, uint16_t aBiphaseTimeUnit);

    void initBiphaselevel(uint_fast8_t aRCDecodeRawbuffOffset, uint16_t aBiphaseTimeUnit);
    uint_fast8_t getBiphaselevel();

    /*
     * All standard (decode address + command) protocol decoders
     */
    bool decodeBangOlufsen();
    bool decodeBoseWave();
    bool decodeDenon();
    bool decodeFAST();
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

    bool decodeDistanceWidth();

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
    uint_fast8_t compare(uint16_t oldval, uint16_t newval);
    bool checkHeader(PulseDistanceWidthProtocolConstants *aProtocolConstants);
    void checkForRepeatSpaceTicksAndSetFlag(uint16_t aMaximumRepeatSpaceTicks);
    bool checkForRecordGapsMicros(Print *aSerial);

    IRData decodedIRData;       // New: decoded IR data for the application

    // Last decoded IR data for repeat detection and parity for Denon autorepeat
    decode_type_t lastDecodedProtocol;
    uint32_t lastDecodedAddress;
    uint32_t lastDecodedCommand;

    uint8_t repeatCount;        // Used e.g. for Denon decode for autorepeat decoding.
};

extern uint_fast8_t sBiphaseDecodeRawbuffOffset; //

/*
 * Mark & Space matching functions
 */
bool matchTicks(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
bool matchMark(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
bool matchSpace(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);

/*
 * Old function names
 */
bool MATCH(uint16_t measured, uint16_t desired);
bool MATCH_MARK(uint16_t measured_ticks, uint16_t desired_us);
bool MATCH_SPACE(uint16_t measured_ticks, uint16_t desired_us);

int getMarkExcessMicros();

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
#define TICKS_LOW(us)   ((uint16_t ) ((long) (us) * LTOL / (MICROS_PER_TICK * 100) ))
#define TICKS_HIGH(us)  ((uint16_t ) ((long) (us) * UTOL / (MICROS_PER_TICK * 100) + 1))
#endif

/*
 * The receiver instance
 */
extern IRrecv IrReceiver;

/*
 * The receiver interrupt handler for timer interrupt
 */
void IRReceiveTimerInterruptHandler();

/****************************************************
 *                     SENDING
 ****************************************************/

/**
 * Just for better readability of code
 */
#define NO_REPEATS  0
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
    // The next function is a dummy to avoid acceptance of pre 4.0 calls to begin(IR_SEND_PIN, DISABLE_LED_FEEDBACK);
    void begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback)
#  if !defined (DOXYGEN)
            __attribute__ ((deprecated ("You must use begin(ENABLE_LED_FEEDBACK) or begin(DISABLE_LED_FEEDBACK) since version 4.0.")));
#  endif
#else
    IRsend(uint_fast8_t aSendPin);
    void begin(uint_fast8_t aSendPin);
    void setSendPin(uint_fast8_t aSendPin); // required if we use IRsend() as constructor
    // Since 4.0 guarded and without default parameter
    void begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin); // aFeedbackLEDPin can be USE_DEFAULT_FEEDBACK_LED_PIN
#endif

    size_t write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    size_t write(decode_type_t aProtocol, uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats = NO_REPEATS);

    void enableIROut(uint_fast8_t aFrequencyKHz);

    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, bool aMSBFirst, bool aSendStopBit,
            uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats)
                    __attribute__ ((deprecated ("Since version 4.1.0 parameter aSendStopBit is not longer required.")));
    void sendPulseDistanceWidthFromArray(PulseDistanceWidthProtocolConstants *aProtocolConstants,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats);

    void sendPulseDistanceWidth(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRRawDataType aData,
            uint_fast8_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRRawDataType aData,
            uint_fast8_t aNumberOfBits);
    void sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType aData, uint_fast8_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats, void (*aSpecialSendRepeatFunction)() = NULL);
    void sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType aData, uint_fast8_t aNumberOfBits, bool aMSBFirst, bool aSendStopBit, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats, void (*aSpecialSendRepeatFunction)() = NULL)
                    __attribute__ ((deprecated ("Since version 4.1.0 parameter aSendStopBit is not longer required.")));
    void sendPulseDistanceWidthData(uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros,
            uint16_t aZeroSpaceMicros, IRRawDataType aData, uint_fast8_t aNumberOfBits, uint8_t aFlags);
    void sendPulseDistanceWidthData(uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros,
            uint16_t aZeroSpaceMicros, IRRawDataType aData, uint_fast8_t aNumberOfBits, bool aMSBFirst, bool aSendStopBit)
                    __attribute__ ((deprecated ("Since version 4.1.0 last parameter aSendStopBit is not longer required.")));
    void sendBiphaseData(uint16_t aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits);

    void mark(uint16_t aMarkMicros);
    static void space(uint16_t aSpaceMicros);
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
    void sendBangOlufsen(uint16_t aHeader, uint8_t aData, int_fast8_t aNumberOfRepeats = NO_REPEATS,
            int8_t aNumberOfHeaderBits = 8);
    void sendBangOlufsenDataLink(uint32_t aHeader, uint8_t aData, int_fast8_t aNumberOfRepeats = NO_REPEATS,
            int8_t aNumberOfHeaderBits = 8);
    void sendBangOlufsenRaw(uint32_t aRawData, int_fast8_t aBits, bool aBackToBack = false);
    void sendBangOlufsenRawDataLink(uint64_t aRawData, int_fast8_t aBits, bool aBackToBack = false,
            bool aUseDatalinkTiming = false);
    void sendBoseWave(uint8_t aCommand, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendDenon(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aSendSharp = false);
    void sendDenonRaw(uint16_t aRawData, int_fast8_t aNumberOfRepeats = NO_REPEATS)
#if !defined (DOXYGEN)
            __attribute__ ((deprecated ("Please use sendDenon(aAddress, aCommand, aNumberOfRepeats).")));
#endif
    void sendFAST(uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendJVC(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    void sendLG2Repeat();
    uint32_t computeLGRawDataAndChecksum(uint8_t aAddress, uint16_t aCommand);
    void sendLG(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendLG2(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendLGRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats = NO_REPEATS);

    void sendNECRepeat();
    uint32_t computeNECRawDataAndChecksum(uint16_t aAddress, uint16_t aCommand);
    void sendNEC(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendNEC2(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendNECRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    // NEC variants
    void sendOnkyo(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendApple(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    void sendKaseikyo(uint16_t aAddress, uint8_t aData, int_fast8_t aNumberOfRepeats, uint16_t aVendorCode); // LSB first
    void sendPanasonic(uint16_t aAddress, uint8_t aData, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Denon(uint16_t aAddress, uint8_t aData, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Mitsubishi(uint16_t aAddress, uint8_t aData, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Sharp(uint16_t aAddress, uint8_t aData, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_JVC(uint16_t aAddress, uint8_t aData, int_fast8_t aNumberOfRepeats); // LSB first

    void sendRC5(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC6(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendSamsungLGRepeat();
    void sendSamsung(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung48(uint16_t aAddress, uint32_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsungLG(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSharp(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSony(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint8_t numberOfBits = 12); // SIRCS_12_PROTOCOL

    void sendLegoPowerFunctions(uint8_t aChannel, uint8_t tCommand, uint8_t aMode, bool aDoSend5Times = true);
    void sendLegoPowerFunctions(uint16_t aRawData, bool aDoSend5Times = true);
    void sendLegoPowerFunctions(uint16_t aRawData, uint8_t aChannel, bool aDoSend5Times = true);

    void sendMagiQuest(uint32_t aWandId, uint16_t aMagnitude);

    void sendPronto(const __FlashStringHelper *str, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendPronto(const char *prontoHexString, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendPronto(const uint16_t *data, uint16_t length, int_fast8_t aNumberOfRepeats = NO_REPEATS);

#if defined(__AVR__)
    void sendPronto_PF(uint_farptr_t str, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendPronto_P(const char *str, int_fast8_t aNumberOfRepeats);
#endif

// Template protocol :-)
    void sendShuzu(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    /*
     * OLD send functions
     */
    void sendDenon(unsigned long data,
            int nbits)
                    __attribute__ ((deprecated ("The function sendDenon(data, nbits) is deprecated and may not work as expected! Use sendDenonRaw(data, NumberOfRepeats) or better sendDenon(Address, Command, NumberOfRepeats).")));
    void sendDish(uint16_t aData);
    void sendJVC(unsigned long data, int nbits,
            bool repeat)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendJVC(aAddress, aCommand, aNumberOfRepeats)."))) {
        sendJVCMSB(data, nbits, repeat);
    }
    void sendJVCMSB(unsigned long data, int nbits, bool repeat = false);

    void sendLG(unsigned long data,
            int nbits)
                    __attribute__ ((deprecated ("The function sendLG(data, nbits) is deprecated and may not work as expected! Use sendLGRaw(data, NumberOfRepeats) or better sendLG(Address, Command, NumberOfRepeats).")));

    void sendNEC(uint32_t aRawData,
            uint8_t nbits)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendNECMSB() or sendNEC(aAddress, aCommand, aNumberOfRepeats)."))) {
        sendNECMSB(aRawData, nbits);
    }
    void sendNECMSB(uint32_t data, uint8_t nbits, bool repeat = false);
    void sendRC5(uint32_t data, uint8_t nbits);
    void sendRC5ext(uint8_t addr, uint8_t cmd, bool toggle);
    void sendRC6Raw(uint32_t data, uint8_t nbits);
    void sendRC6(uint32_t data, uint8_t nbits) __attribute__ ((deprecated ("Please use sendRC6Raw().")));
    void sendRC6Raw(uint64_t data, uint8_t nbits);
    void sendRC6(uint64_t data, uint8_t nbits) __attribute__ ((deprecated ("Please use sendRC6Raw().")));
    ;
    void sendSharpRaw(unsigned long data, int nbits);
    void sendSharp(uint16_t address, uint16_t command);
    void sendSAMSUNG(unsigned long data, int nbits);
    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendSamsung().")));
    void sendSony(unsigned long data,
            int nbits)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendSony(aAddress, aCommand, aNumberOfRepeats).")));
    ;
    void sendWhynter(uint32_t aData, uint8_t aNumberOfBitsToSend);

#if !defined(IR_SEND_PIN)
    uint8_t sendPin;
#endif
    uint16_t periodTimeMicros;
    uint16_t periodOnTimeMicros; // compensated with PULSE_CORRECTION_NANOS for duration of digitalWrite. Around 8 microseconds for 38 kHz.
    uint16_t getPulseCorrectionNanos();

    static void customDelayMicroseconds(unsigned long aMicroseconds);
};

/*
 * The sender instance
 */
extern IRsend IrSender;

void sendNECSpecialRepeat();
void sendLG2SpecialRepeat();
void sendSamsungLGSpecialRepeat();

#endif // _IR_REMOTE_INT_H
