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
 * Copyright (c) 2015-2025 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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
 * The RAW_BUFFER_LENGTH determines the length of the byte buffer where the received IR timing data is stored before decoding.
 * 100 is sufficient for standard protocols up to 48 bits, with 1 bit consisting of one mark and space plus 1 byte for initial gap, 2 bytes for header and 1 byte for stop bit.
 * 48 bit protocols are PANASONIC, KASEIKYO, SAMSUNG48, RC6.
 * 32 bit protocols like NEC, SAMSUNG, WHYNTER, SONY(20), LG(28) requires a buffer length of 68.
 * 16 bit protocols like BOSEWAVE, DENON, FAST, JVC, LEGO_PF, RC5, SONY(12 or 15) requires a buffer length of 36.
 * MAGIQUEST requires a buffer length of 112.
 * Air conditioners often send a longer protocol data stream up to 750 bits.
 */
#if !defined(RAW_BUFFER_LENGTH)
#  if (defined(RAMEND) && RAMEND <= 0x2FF) || (defined(RAMSIZE) && RAMSIZE < 0x2FF)
// for RAMsize <= 512 bytes
#define RAW_BUFFER_LENGTH  100  ///< Length of raw duration buffer. Must be even. 100 supports up to 48 bit codings inclusive 1 start and 1 stop bit.
#  elif (defined(RAMEND) && RAMEND <= 0x8FF) || (defined(RAMSIZE) && RAMSIZE < 0x8FF)
// for RAMsize <= 2k
#define RAW_BUFFER_LENGTH  200  ///< Length of raw duration buffer. Must be even. 100 supports up to 48 bit codings inclusive 1 start and 1 stop bit.
#  else
// For undefined or bigger RAMsize
#define RAW_BUFFER_LENGTH  750 // The value for air condition remotes.
#  endif
#endif
#if RAW_BUFFER_LENGTH % 2 == 1
#error RAW_BUFFER_LENGTH must be even, since the array consists of space / mark pairs.
#endif

#if RAW_BUFFER_LENGTH <= 254    // saves around 75 bytes program memory and speeds up ISR
typedef uint_fast8_t IRRawlenType;
#else
typedef unsigned int IRRawlenType;
#endif

/*
 * Use 8 bit buffer for IR timing in 50 ticks units.
 * It is save to use 8 bit if RECORD_GAP_TICKS < 256, since any value greater 255 is interpreted as frame gap of 12750 us.
 * The default for frame gap is currently 8000!
 * But if we assume that for most protocols the frame gap is way greater than the biggest mark or space duration,
 * we can choose to use a 8 bit buffer even for frame gaps up to 200000 us.
 * This enables the use of 8 bit buffer even for more some protocols like B&O or LG air conditioner etc.
 */
#if RECORD_GAP_TICKS <= 400 // Corresponds to RECORD_GAP_MICROS of 200000. A value of 255 is foolproof, but we assume, that the frame gap is way greater than the biggest mark or space duration.
typedef uint8_t IRRawbufType; // all timings up to the gap fit into 8 bit.
#else
typedef uint16_t IRRawbufType; // The gap does not fit into 8 bit ticks value. This must not be a reason to use 16 bit for buffer, but it is at least save.
#endif

#if (__INT_WIDTH__ < 32)
typedef uint32_t IRRawDataType;
#define BITS_IN_RAW_DATA_TYPE   32
#else
typedef uint64_t IRRawDataType;
#define BITS_IN_RAW_DATA_TYPE   64
#endif

/**********************************************************
 * Declarations for the receiver Interrupt Service Routine
 **********************************************************/
// ISR State-Machine : Receiver States
#define IR_REC_STATE_IDLE      0 // Counting the gap time and waiting for the start bit to arrive
#define IR_REC_STATE_MARK      1 // A mark was received and we are counting the duration of it.
#define IR_REC_STATE_SPACE     2 // A space was received and we are counting the duration of it. If space is too long, we assume end of frame.
#define IR_REC_STATE_STOP      3 // Stopped until set to IR_REC_STATE_IDLE which can only be done by resume()

/**
 * This struct contains the data and control used for receiver functions and the ISR (interrupt service routine)
 * Only StateForISR needs to be volatile. All the other fields are not written by ISR after available() == true and before start() / resume().
 */
struct irparams_struct {
    // The fields are ordered to reduce memory overflow caused by struct-padding
    volatile uint8_t StateForISR;       ///< State Machine state
    uint_fast8_t IRReceivePin;          ///< Pin connected to IR data from detector
#if defined(__AVR__)
    volatile uint8_t *IRReceivePinPortInputRegister;
    uint8_t IRReceivePinMask;
#endif
    volatile uint_fast16_t TickCounterForISR; ///< Counts 50uS ticks. The value is copied into the rawbuf array on every transition. Counting is independent of state or resume().
#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
    void (*ReceiveCompleteCallbackFunction)(void); ///< The function to call if a protocol message has arrived, i.e. StateForISR changed to IR_REC_STATE_STOP
#endif
    bool OverflowFlag;                  ///< Raw buffer OverflowFlag occurred
    IRRawlenType rawlen;                ///< counter of entries in rawbuf
    uint16_t initialGapTicks;   ///< Tick counts of the length of the gap between previous and current IR frame. Pre 4.4: rawbuf[0].
    IRRawbufType rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space. With 8 bit we can only store up to 12.7 ms. First entry is empty to be backwards compatible.
};

extern unsigned long sMicrosAtLastStopTimer; // Used to adjust TickCounterForISR with uncounted ticks between stopTimer() and restartTimer()

#define DECODED_RAW_DATA_ARRAY_SIZE     ((((RAW_BUFFER_LENGTH - 2) - 1) / (2 * BITS_IN_RAW_DATA_TYPE)) + 1) // The -2 is for initial gap + stop bit mark, 128 mark + spaces for 64 bit.
/**
 * Data structure for the user application, available as decodedIRData.
 * Filled by decoders and read by print functions or user application.
 */
struct IRData {
    decode_type_t protocol; ///< UNKNOWN, NEC, SONY, RC5, PULSE_DISTANCE, ...
    uint16_t address; ///< Decoded address, Distance protocol (tMarkTicksLong (if tMarkTicksLong == 0, then tMarkTicksShort) << 8) | tSpaceTicksLong
    uint16_t command;       ///< Decoded command, Distance protocol (tMarkTicksShort << 8) | tSpaceTicksShort
    uint16_t extra; ///< Contains upper 16 bit of Magiquest WandID, Kaseikyo unknown vendor ID and Distance protocol (HeaderMarkTicks << 8) | HeaderSpaceTicks.
    IRRawDataType decodedRawData; ///< Up to 32/64 bit decoded raw data, to be used for send<protocol>Raw functions.
#if defined(DECODE_DISTANCE_WIDTH)
    // This replaces the address, command, extra and decodedRawData in case of protocol == PULSE_DISTANCE or -rather seldom- protocol == PULSE_WIDTH.
    DistanceWidthTimingInfoStruct DistanceWidthTimingInfo; // 12 bytes
    IRRawDataType decodedRawDataArray[DECODED_RAW_DATA_ARRAY_SIZE]; ///< 32/64 bit decoded raw data, to be used for sendPulseDistanceWidthFromArray functions.
#endif
    uint16_t numberOfBits; ///< Number of bits received for data (address + command + parity) - to determine protocol length if different length are possible.
    uint8_t flags;          ///< IRDATA_FLAGS_IS_REPEAT, IRDATA_FLAGS_WAS_OVERFLOW etc. See IRDATA_FLAGS_* definitions above

    /*
     * These 2 variables allow to call resume() directly after decode.
     * After resume(), irparams.initialGapTicks and irparams.rawlen are
     * the first variables, which are overwritten by the next received frame.
     * since 4.3.0.
     */
    IRRawlenType rawlen;        ///< Counter of entries in rawbuf of last received frame.
    uint16_t initialGapTicks;   ///< Contains the initial gap (pre 4.4: the value in rawbuf[0]) of the last received frame.
};

/*
 * Debug directives
 * Outputs with IR_DEBUG_PRINT can only be activated by defining DEBUG!
 * If LOCAL_DEBUG is defined in one file, all outputs with IR_DEBUG_PRINT are still suppressed.
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

// next 3 values are copies of irparams_struct values - see above
    uint16_t *rawbuf;           // deprecated, moved to irparams.rawbuf ///< Raw intervals in 50uS ticks
    uint_fast8_t rawlen;        // deprecated, moved to irparams.rawlen ///< Number of records in rawbuf
    bool overflow;              // deprecated, moved to decodedIRData.flags ///< true if IR raw code too long
};

/**
 * Main class for receiving IR signals
 */
#define USE_DEFAULT_FEEDBACK_LED_PIN        0 // we need it here
class IRrecv {
public:

    IRrecv();
#if defined(SUPPORT_MULTIPLE_RECEIVER_INSTANCES)
    IRrecv(uint_fast8_t aReceivePin);
    IRrecv(uint_fast8_t aReceivePin, uint_fast8_t aFeedbackLEDPin);
#else
    IRrecv(
            uint_fast8_t aReceivePin)
                    __attribute__ ((deprecated ("Please use the default IRrecv instance \"IrReceiver\" and IrReceiver.begin(), and not your own IRrecv instance.")));
    IRrecv(uint_fast8_t aReceivePin,
            uint_fast8_t aFeedbackLEDPin)
                    __attribute__ ((deprecated ("Please use the default IRrecv instance \"IrReceiver\" and IrReceiver.begin(), and not your own IRrecv instance..")));
#endif
    void setReceivePin(uint_fast8_t aReceivePinNumber);
#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
    void registerReceiveCompleteCallback(void (*aReceiveCompleteCallbackFunction)(void));
#endif
    void ReceiveInterruptHandler();

    /*
     * Stream like API
     */
    void begin(uint_fast8_t aReceivePin, bool aEnableLEDFeedback = false, uint_fast8_t aFeedbackLEDPin =
    USE_DEFAULT_FEEDBACK_LED_PIN);
    void start();
    void enableIRIn(); // alias for start
    void restartTimer();
    void restartTimer(uint32_t aMicrosecondsToAddToGapCounter);
    void restartTimerWithTicksToAdd(uint16_t aTicksToAddToGapCounter);
    void restartAfterSend();

    bool available();
    IRData* read(); // returns decoded data
    // write is a method of class IRsend below
    // size_t write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    void stopTimer();
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
    void printIRDuration(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks);
    void printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void printIRResultAsCVariables(Print *aSerial);
    uint8_t getMaximumMarkTicksFromRawData();
    uint8_t getMaximumSpaceTicksFromRawData();
    uint8_t getMaximumTicksFromRawData(bool aSearchSpaceInsteadOfMark);
    uint32_t getTotalDurationOfRawData();

    /*
     * Next 4 functions are also available as non member functions
     */
    bool printIRResultShort(Print *aSerial, bool aPrintRepeatGap, bool aCheckForRecordGapsMicros)
            __attribute__ ((deprecated ("Remove second parameter, it is not supported any more.")));
    bool printIRResultShort(Print *aSerial, bool aCheckForRecordGapsMicros = true);
    void printDistanceWidthTimingInfo(Print *aSerial, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo);
    void printIRSendUsage(Print *aSerial);
#if defined(__AVR__)
    const __FlashStringHelper* getProtocolString();
#else
    const char* getProtocolString();
#endif
    static void printActiveIRProtocols(Print *aSerial);

    void printIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true, bool aDoCompensate = true);
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
#if defined(USE_STRICT_DECODER)
    bool
#else
    void
#endif
    decodePulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, uint_fast8_t aNumberOfBits,
            IRRawlenType aStartOffset = 3);

    void decodePulseDistanceWidthData_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM,
            uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset = 3);

    void decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMicros,
            bool aIsPulseWidthProtocol, bool aMSBfirst);

    void decodeWithThresholdPulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset,
            uint16_t aOneThresholdMicros, bool aIsPulseWidthProtocol, bool aMSBfirst);

    void decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, bool aMSBfirst);

    void decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aZeroMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst)
                    __attribute__ ((deprecated ("Please use decodePulseDistanceWidthData() with 6 parameters.")));

    bool decodeStrictPulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst);

    bool decodeBiPhaseData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint_fast8_t aStartClockCount,
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

    bool decode_old(decode_results *aResults);

    bool decode(
            decode_results *aResults)
                    __attribute__ ((deprecated ("Please use IrReceiver.decode() without a parameter and IrReceiver.decodedIRData.<fieldname> .")));

    // for backward compatibility. Now in IRFeedbackLED.hpp
    void blink13(uint8_t aEnableLEDFeedback)
            __attribute__ ((deprecated ("Please use setLEDFeedback() or enableLEDFeedback() / disableLEDFeedback().")));

    /*
     * Internal functions
     */
    void initDecodedIRData();
    uint_fast8_t compare(uint16_t oldval, uint16_t newval);
    bool checkHeader(PulseDistanceWidthProtocolConstants *aProtocolConstants);
    bool checkHeader_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM);
    void checkForRepeatSpaceTicksAndSetFlag(uint16_t aMaximumRepeatSpaceTicks);
    bool checkForRecordGapsMicros(Print *aSerial);

    irparams_struct irparams;
    IRData decodedIRData;       // Decoded IR data for the application

    // Last decoded IR data for repeat detection and to fill in JVC, LG, NEC repeat values. Parity for Denon autorepeat
    decode_type_t lastDecodedProtocol;
    uint16_t lastDecodedAddress;
    uint16_t lastDecodedCommand;
#if defined(DECODE_DISTANCE_WIDTH)
    IRRawDataType lastDecodedRawData;
#endif

    uint8_t repeatCount;        // Used e.g. for Denon decode for autorepeat decoding.
};

void printIRResultShort(Print *aSerial, IRData *aIRDataPtr, bool aPrintRepeatGap)
        __attribute__ ((deprecated ("Remove last parameter, it is not supported any more.")));
void printIRResultShort(Print *aSerial, IRData *aIRDataPtr)
        __attribute__ ((deprecated ("Use member function or printIRDataShort() instead.")));
;
// A static function to be able to print send or copied received data.
void printIRDataShort(Print *aSerial, IRData *aIRDataPtr);

extern uint_fast8_t sBiphaseDecodeRawbuffOffset;

/*
 * Mark & Space matching functions
 */
bool matchTicks(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
bool matchTicks(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros, int16_t aCompensationMicrosForTicks);
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
#define DISABLE_LED_FEEDBACK                false
#define ENABLE_LED_FEEDBACK                 true
//#define USE_DEFAULT_FEEDBACK_LED_PIN        0 // repeated definition for info
void setLEDFeedback(bool aEnableLEDFeedback); // Direct replacement for blink13()
void setLEDFeedbackPin(uint8_t aFeedbackLEDPin);
void setFeedbackLED(bool aSwitchLedOn);
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
#if !defined(TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING_PERCENT)
#define TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING_PERCENT    25 // Relative tolerance (in percent) for matchTicks(), matchMark() and matchSpace() functions used for protocol decoding.
#endif

#define TICKS(us)       ((us)/MICROS_PER_TICK)  // (us)/50
#if MICROS_PER_TICK == 50 && TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING_PERCENT == 25           // Defaults
#define TICKS_LOW(us)   ((us)/67 )       // =(us * 0.75 /MICROS_PER_TICK), 67 = MICROS_PER_TICK / ((100-25)/100) = (MICROS_PER_TICK * 100) / (100-25)
#define TICKS_HIGH(us)  (((us)/40) + 1)  // =(us * 1,25 /MICROS_PER_TICK), 40 = MICROS_PER_TICK / ((100+25)/100) = (MICROS_PER_TICK * 100) / (100+25)
#else
/** Lower tolerance for comparison of measured data */
//#define LTOL            (1.0 - (TOLERANCE/100.))
#define LTOL            (100 - TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING_PERCENT)
/** Upper tolerance for comparison of measured data */
//#define UTOL            (1.0 + (TOLERANCE/100.))
#define UTOL            (100 + TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING_PERCENT)
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
     * IR_SEND_PIN is defined or fixed by timer, value of IR_SEND_PIN is then "DeterminedByTimer"
     */
#if defined(IR_SEND_PIN)
    void begin();
    // The default parameter allowed to specify IrSender.begin(7); without errors, if IR_SEND_PIN was defined. But the semantics is not the one the user expect.
    void begin(uint_fast8_t aFeedbackLEDPin);
#else
    IRsend(uint_fast8_t aSendPin);
    void begin(uint_fast8_t aSendPin);
    void setSendPin(uint_fast8_t aSendPin); // required if we use IRsend() as constructor
#endif
    void begin(uint_fast8_t aSendPin, uint_fast8_t aFeedbackLEDPin); // aFeedbackLEDPin is by default USE_DEFAULT_FEEDBACK_LED_PIN
    void begin(uint_fast8_t aSendPin, bool aEnableLEDFeedback, uint_fast8_t aFeedbackLEDPin)
#  if !defined (DOXYGEN)
            __attribute__ ((deprecated ("Use begin(aSendPin, aFeedbackLEDPin) instead.")));
#  endif

    size_t write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats = NO_REPEATS);
    size_t write(decode_type_t aProtocol, uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats = NO_REPEATS);

    void enableIROut(uint_fast8_t aFrequencyKHz);
#if defined(SEND_PWM_BY_TIMER)
    void enableHighFrequencyIROut(uint_fast16_t aFrequencyKHz); // Used for Bang&Olufsen
#endif

    /*
     * Array functions
     */
    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromPGMArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType const *aDecodedRawDataPGMArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray(PulseDistanceWidthProtocolConstants *aProtocolConstants,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromPGMArray(PulseDistanceWidthProtocolConstants *aProtocolConstants,
            IRRawDataType const *aDecodedRawDataPGMArray, uint16_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromPGMArray_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM,
            IRRawDataType const *aDecodedRawDataPGMArray, uint16_t aNumberOfBits, int_fast8_t aNumberOfRepeats);

    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo,
            IRRawDataType *aDecodedRawDataArray, uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray_P(uint_fast8_t aFrequencyKHz,
            DistanceWidthTimingInfoStruct const *aDistanceWidthTimingInfoPGM, IRRawDataType *aDecodedRawDataArray,
            uint16_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);

    void sendPulseDistanceWidth(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRRawDataType aData,
            uint_fast8_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidth_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM, IRRawDataType aData,
            uint_fast8_t aNumberOfBits, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRRawDataType aData,
            uint_fast8_t aNumberOfBits);
    void sendPulseDistanceWidthData_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM, IRRawDataType aData,
            uint_fast8_t aNumberOfBits);
    void sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType aData, uint_fast8_t aNumberOfBits, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats, void (*aSpecialSendRepeatFunction)() = nullptr);
    void sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRRawDataType aData, uint_fast8_t aNumberOfBits, bool aMSBFirst, bool aSendStopBit, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats, void (*aSpecialSendRepeatFunction)() = nullptr)
                    __attribute__ ((deprecated ("Since version 4.1.0 parameter aSendStopBit is not longer required.")));
    void sendPulseDistanceWidthData(uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros,
            uint16_t aZeroSpaceMicros, IRRawDataType aData, uint_fast8_t aNumberOfBits, uint8_t aFlags);
    void sendBiphaseData(uint16_t aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits, bool aSendStartBit = true);

    void mark(uint16_t aMarkMicros);
    static void space(uint16_t aSpaceMicros);
    void IRLedOff();

// 8 Bit array
    void sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint8_t aPGMBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz,
            uint_fast16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);
    void sendRaw_P(const uint8_t aPGMBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz,
            uint_fast16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);

// 16 Bit array
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint16_t aPGMBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz,
            uint_fast16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);
    void sendRaw_P(const uint16_t aPGMBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz,
            uint_fast16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);

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
    void sendDenon(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint8_t aSendSharpFrameMarker = 0);
    void sendDenonRaw(uint16_t aRawData, int_fast8_t aNumberOfRepeats = NO_REPEATS)
            __attribute__ ((deprecated ("Please use sendDenon(aAddress, aCommand, aNumberOfRepeats).")));
    void sendFAST(uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendJVC(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    void sendLG2Repeat();
    uint32_t computeLGRawDataAndChecksum(uint8_t aAddress, uint16_t aCommand);
    void sendLG(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendLG2(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendLGRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats = NO_REPEATS);

    void sendNECRepeat();
    uint32_t computeNECRawDataAndChecksum(uint16_t aAddress, uint16_t aCommand);
    void sendNEC(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendNEC2(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
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
    void sendRC5Marantz(uint8_t aAddress, uint8_t aCommand, uint8_t aMarantzExtension, int_fast8_t aNumberOfRepeats,
            bool aEnableAutomaticToggle = true);
    void sendRC6(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC6A(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint16_t aCustomer,
            bool aEnableAutomaticToggle = true);
    void sendSamsungLGRepeat();
    void sendSamsung(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung16BitAddressAnd8BitCommand(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung16BitAddressAndCommand(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung48(uint16_t aAddress, uint32_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsungLG(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSharp(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSharp2(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // redirected to sendDenon
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

    void sendSharpRaw(unsigned long data, int nbits);
    void sendSharp(uint16_t address, uint16_t command);
    void sendSAMSUNG(unsigned long data, int nbits);
    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendSamsung().")));
    void sendSamsungMSB(unsigned long data, int nbits);
    void sendSonyMSB(unsigned long data, int nbits);
    void sendSony(unsigned long data,
            int nbits)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendSony(aAddress, aCommand, aNumberOfRepeats).")));

    void sendWhynter(uint32_t aData, int_fast8_t aNumberOfRepeats);
    void sendVelux(uint8_t aCommand, uint8_t aMotorNumber, uint8_t aMotorSet, uint16_t aSecurityCode, int_fast8_t aNumberOfRepeats);
    void sendVelux(uint32_t aData, int_fast8_t aNumberOfRepeats);

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
