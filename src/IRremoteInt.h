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
 * Copyright (c) 2015-2026 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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

#define SPACE  0
#define MARK   1
#define NO_MARK_OR_SPACE  2 // Used as return value for getBiphaselevel()

#if defined(PARTICLE)
#define F_CPU 16000000 // definition for a board for which F_CPU is not defined
#endif
#if defined(F_CPU) // F_CPU is used to generate the receive send timings in some CPU's
#define CLOCKS_PER_MICRO (F_CPU / MICROS_IN_ONE_SECOND)
#else
#define CLOCKS_PER_MICRO 48 // just a guess if F_CPU is not specified
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
 * Default is 100 for 512 bytes RAM, 200 for 2k RAM and 750 for more than 2k RAM
 */
#if !defined(RAW_BUFFER_LENGTH)
#  if (defined(RAMSIZE) && RAMSIZE <= 0x200) || (defined(RAMEND) && RAMEND <= 0x2FF) // assuming RAMSTART at 0x100
// For RAMSIZE <= 512 bytes
#define RAW_BUFFER_LENGTH  100  ///< Length of raw duration buffer. Must be even. 100 supports up to 48 bit codings inclusive 1 start and 1 stop bit.
#  elif (defined(RAMSIZE) && RAMSIZE < 0x800) || (defined(RAMEND) && RAMEND <= 0x8FF)  // assuming RAMSTART at 0x100 after the memory mapped registers
// For RAMSIZE <= 2k
#define RAW_BUFFER_LENGTH  200
#  else
// For undefined or bigger RAMsize
#define RAW_BUFFER_LENGTH  750 // The value required for air condition remotes.
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
 * We store IR timing values (ticks) in an 8-bit buffer, using 50 microseconds for one tick.
 * Since 8 bits can hold values from 0 to 255, the maximum measurable timing is 255 × 50 microseconds = 12750 us.
 *
 * This means:
 * - Any timing up to 12750 us is stored accurately.
 * - Any timing greater than 12750 us is clipped and stored as 12750 us.
 * - Therefore, you can detect that a timing exceeded 12750 us (because it appears as 12750 us).
 * - However, you cannot distinguish between two different timings if both are greater than 12750 us — they will both appear as the same maximum value.
 *
 * For decoding purposes, 8 bits are acceptable as long as the protocol does not require distinguishing between two different timings
 * that are both longer than 12750 us.
 * As I am not currently (02/2026) aware of any protocols that require this, we use 8 bits.
 * Use a 16-bit buffer if raw timing capture is required and exact values above 12750 us must be preserved.
 */
#if !defined(USE_16_BIT_TIMING_BUFFER) // Use uint16_t buffer for timing. Doubles the RAM requirement. this can be used to override our selection of 8 the bit array
typedef uint8_t IRRawbufType; // all timings up to 12750 us fit into 8 bit (for MICROS_PER_TICK == 50).
#else
typedef uint16_t IRRawbufType; // Use 16 bit array
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
    // For low ISR reading delay for AVR even with dynamic pin numbers
    volatile uint8_t *IRReceivePinPortInputRegister;
    uint8_t IRReceivePinMask;
#endif
    volatile uint_fast16_t TickCounterForISR; ///< Counts 50uS ticks. The value is copied into the rawbuf array on every transition. Counting is independent of state or resume().
#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
    void (*ReceiveCompleteCallbackFunction)(void); ///< The function to call if a protocol message has arrived, i.e. StateForISR changed to IR_REC_STATE_STOP
#endif
#if defined(DECODE_RC5) || defined(DECODE_RC6) || defined(DECODE_MARANTZ)
    // variables for the getBiphaselevel() function
    uint_fast8_t RawbuffOffsetForNextBiphaseLevel;
    uint16_t NumberOfTimingIntervalsInCurrentInterval; // 1, 2 or 3. Number of aBiphaseTimeUnit intervals.
    uint_fast8_t AlreadyUsedTimingIntervalsOfCurrentInterval;   // Number of already used intervals of current timing interval pointed to by RawbuffOffsetForNextBiphaseLevel.
    uint16_t BiphaseTimeUnit;
#endif
    bool OverflowFlag;                  ///< Raw buffer OverflowFlag occurred
    IRRawlenType rawlen;                ///< counter of entries in rawbuf
    uint16_t initialGapTicks;   ///< Tick counts of the length of the gap between previous and current IR frame. Pre 4.4: rawbuf[0].
    IRRawbufType rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space. With 8 bit we can only store up to 12.7 ms. First entry is empty to be backwards compatible.
};

#if (__INT_WIDTH__ < 32)
typedef uint32_t IRDecodedRawDataType;
typedef int32_t IRDecodedRawDataSignedType;
#define BITS_IN_DECODED_RAW_DATA_TYPE   32
#else
typedef uint64_t IRDecodedRawDataType;
typedef int64_t IRDecodedRawDataSignedType;
#define BITS_IN_DECODED_RAW_DATA_TYPE   64
#endif
typedef IRDecodedRawDataType IRRawDataType; // Define old IRRawDataType (removed in 4.6.0) for backward compatibility

#define DECODED_RAW_DATA_ARRAY_SIZE     ((((RAW_BUFFER_LENGTH - 2) - 1) / (2 * BITS_IN_DECODED_RAW_DATA_TYPE)) + 1) // The -2 is for initial gap + stop bit mark, 128 mark + spaces for 64 bit.
/**
 * Data structure for the user application, available as decodedIRData.
 * Filled by decoders and read by print functions or user application.
 */
struct IRData {
    decode_type_t protocol; ///< UNKNOWN, NEC, SONY, RC5, PULSE_DISTANCE, ...
    uint16_t address; ///< Decoded address, Distance protocol (tMarkTicksLong (if tMarkTicksLong == 0, then tMarkTicksShort) << 8) | tSpaceTicksLong
    uint16_t command;       ///< Decoded command, Distance protocol (tMarkTicksShort << 8) | tSpaceTicksShort
    uint16_t extra; ///< Contains upper 16 bit of Magiquest WandID, Kaseikyo unknown vendor ID and Distance protocol (HeaderMarkTicks << 8) | HeaderSpaceTicks.
    IRDecodedRawDataType decodedRawData; ///< Up to 32/64 bit decoded raw data, to be used for send<protocol>Raw functions.
#if defined(DECODE_DISTANCE_WIDTH)
    // This replaces the address, command, extra and decodedRawData in case of protocol == PULSE_DISTANCE or -rather seldom- protocol == PULSE_WIDTH.
    DistanceWidthTimingInfoStruct DistanceWidthTimingInfo; // 12 bytes
    IRDecodedRawDataType decodedRawDataArray[DECODED_RAW_DATA_ARRAY_SIZE]; ///< 32/64 bit decoded raw data, to be used for sendPulseDistanceWidthFromArray functions.
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

/****************************************************
 *                     RECEIVING
 ****************************************************/
extern unsigned long sMicrosAtLastStopTimer; // Used to adjust TickCounterForISR with uncounted ticks between stopTimer() and restartTimer()

/**
 * Main class for receiving IR signals
 */
#define USE_DEFAULT_FEEDBACK_LED_PIN        0xFF // we need it here
class IRrecv {
public:

    IRrecv();
    IRrecv(uint_fast8_t aReceivePin);
    IRrecv(uint_fast8_t aReceivePin, uint_fast8_t aFeedbackLEDPin);
    void setReceivePin(uint_fast8_t aReceivePin);
#if !defined(IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK)
    void registerReceiveCompleteCallback(void (*aReceiveCompleteCallbackFunction)(void));
#endif
    void ReceiveInterruptHandler();

    /*
     * Stream like API
     */
    void begin(uint_fast8_t aReceivePin, uint_fast8_t aFeedbackLEDPin, bool aEnableLEDFeedback);
    void begin(uint_fast8_t aReceivePin);
    void start();
    void restartTimer();
    void restartTimer(uint32_t aMicrosecondsToAddToGapCounter);
    void restartTimerWithTicksToAdd(uint16_t aTicksToAddToGapCounter);
    void restartAfterSend();

    bool available();
    IRData* read(); // returns decoded data
    // write is a method of class IRsend below
    // size_t write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats);
    void stopTimer();
    void stop();
    void end() {
        // the same functionality as stop() is sufficient :-)
        stop();
    }

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
    void printIRDuration(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void printIRResultAsCVariables(Print *aSerial);
    uint8_t getMaximumMarkTicksFromRawData();
    uint8_t getMaximumSpaceTicksFromRawData();
    uint8_t getMaximumTicksFromRawData(bool aSearchSpaceInsteadOfMark);
    uint32_t getTotalDurationOfRawData();

    /*
     * Next 4 functions are also available as non member functions
     */
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
    void compensateAndPrintIRResultAsPronto(Print *aSerial, uint16_t aFrequencyHertz = 38000U);
    // Store the data for further processing
    void compensateAndStoreIRResultInArray(uint8_t *aDestinationArrayPtr, uint_fast16_t aLengthOfDestinationArray);
    size_t compensateAndStorePronto(String *aString, uint16_t aFrequencyHertz = 38000U);

    /*
     * The main decoding functions used by the individual decoders
     */
    bool decodePulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, uint_fast8_t aNumberOfBits,
            IRRawlenType aStartOffset = 3);

    bool decodePulseDistanceWidthData_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM,
            uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset = 3);

    bool decodeStrictPulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst);

    void decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMicros,
            bool aIsPulseWidthProtocol, bool aMSBfirst);

    void decodeWithThresholdPulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset,
            uint16_t aOneThresholdMicros, bool aIsPulseWidthProtocol, bool aMSBfirst);

    void decodePulseDistanceWidthData(uint_fast8_t aNumberOfBits, IRRawlenType aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, bool aMSBfirst);

#if defined(DECODE_RC5) || defined(DECODE_MARANTZ) || defined(DECODE_RC6)
    void initBiphaselevel(uint_fast8_t aRCDecodeRawbuffOffset, uint16_t aBiphaseTimeUnit);
    static uint8_t getNumberOfUnitsInInterval(uint16_t aCurrentInterval, uint16_t aTimeUnit);
    uint_fast8_t getBiphaselevel();
#endif

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
    bool decodeOpenLASIR();
#if defined(DECODE_RC5) || defined(DECODE_MARANTZ)
    bool decodeRC5();
#endif
#if defined(DECODE_RC6)
    bool decodeRC6();
#endif
    bool decodeSamsung();
    bool decodeSharp(); // redirected to decodeDenon()
    bool decodeSony();
    bool decodeWhynter();

    bool decodeDistanceWidth();

    bool decodeHash();

    // Template function :-)
    bool decodeShuzu();

    /*
     * Internal functions
     */
    void initDecodedIRData();
    uint_fast8_t compare(uint16_t aOldValue, uint16_t aNewValue);
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
    IRDecodedRawDataType lastDecodedRawData;
#endif

    uint8_t repeatCount;        // Used e.g. for Denon decode for autorepeat decoding.
};

// A static function to be able to print send or copied received data.
void printIRDataShort(Print *aSerial, IRData *aIRDataPtr);

/*
 * Mark & Space matching functions
 */
bool matchTicks(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros, int16_t aCompensationMicrosForTicks);
bool matchMark(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
bool matchSpace(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
bool matchTicksWithGreaterRange(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros, int16_t aCompensationMicrosForTicks);
bool matchMarkWithGreaterRange(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
bool matchSpaceWithGreaterRange(uint16_t aMeasuredTicks, uint16_t aMatchValueMicros);
int getMarkExcessMicros();

void printActiveIRProtocols(Print *aSerial);

/****************************************************
 * Feedback LED related functions
 ****************************************************/
#define DISABLE_LED_FEEDBACK                false
#define ENABLE_LED_FEEDBACK                 true
//#define USE_DEFAULT_FEEDBACK_LED_PIN      0xFF // repeated definition for info
void setLEDFeedback(bool aEnableLEDFeedback); // Direct replacement for blink13()
void setLEDFeedbackPin(uint_fast8_t aFeedbackLEDPin);
void writeFeedbackLED(bool aSwitchLedOn);
void enableLEDFeedback();
//constexpr auto enableLEDFeedbackForReceive = enableLEDFeedback; // alias for enableLEDFeedback
void disableLEDFeedback();
//constexpr auto disableLEDFeedbackForReceive = disableLEDFeedback; // alias for disableLEDFeedback

/****************************************************
 * Mark and spaces receiving tolerances
 ****************************************************/
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
#else
    IRsend(uint_fast8_t aSendPin);
    void begin(uint_fast8_t aSendPin);
    void setSendPin(uint_fast8_t aSendPin); // required if we use IRsend() as constructor
    void begin(uint_fast8_t aSendPin, uint_fast8_t aFeedbackLEDPin); // aFeedbackLEDPin is by default USE_DEFAULT_FEEDBACK_LED_PIN
#endif

    size_t write(IRData *aIRSendData, int_fast8_t aNumberOfRepeats);
    size_t write(decode_type_t aProtocol, uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);

    void enableIROut(uint_fast8_t aFrequencyKHz);
#if defined(SEND_PWM_BY_TIMER)
    void enableHighFrequencyIROut(uint_fast16_t aFrequencyKHz); // Used for Bang&Olufsen
#endif

    /*
     * Array functions. They do not support special repeat function, therefore aNumberOfRepeats is unsigned
     */
    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRDecodedRawDataType *aDecodedRawDataArray, uint_fast16_t aNumberOfBitsToSend, uint8_t aFlags,
            uint16_t aRepeatPeriodMillis, uint_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromPGMArray(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRDecodedRawDataType const *aDecodedRawDataPGMArray, uint_fast16_t aNumberOfBitsToSend, uint8_t aFlags,
            uint16_t aRepeatPeriodMillis, uint_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray(PulseDistanceWidthProtocolConstants *aProtocolConstants,
            IRDecodedRawDataType *aDecodedRawDataArray, uint_fast16_t aNumberOfBitsToSend, uint_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromPGMArray(PulseDistanceWidthProtocolConstants *aProtocolConstants,
            IRDecodedRawDataType const *aDecodedRawDataPGMArray, uint_fast16_t aNumberOfBitsToSend, uint_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM,
            IRDecodedRawDataType *aDecodedRawDataArray, uint_fast16_t aNumberOfBitsToSend, uint_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromPGMArray_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM,
            IRDecodedRawDataType const *aDecodedRawDataPGMArray, uint_fast16_t aNumberOfBitsToSend, uint_fast8_t aNumberOfRepeats);

    void sendPulseDistanceWidthFromArray(uint_fast8_t aFrequencyKHz, DistanceWidthTimingInfoStruct *aDistanceWidthTimingInfo,
            IRDecodedRawDataType *aDecodedRawDataArray, uint_fast16_t aNumberOfBitsToSend, uint8_t aFlags,
            uint16_t aRepeatPeriodMillis, uint_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthFromArray_P(uint_fast8_t aFrequencyKHz,
            DistanceWidthTimingInfoStruct const *aDistanceWidthTimingInfoPGM, IRDecodedRawDataType *aDecodedRawDataArray,
            uint_fast16_t aNumberOfBitsToSend, uint8_t aFlags, uint16_t aRepeatPeriodMillis, uint_fast8_t aNumberOfRepeats);

    void sendPulseDistanceWidth(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRDecodedRawDataType aData,
            uint_fast8_t aNumberOfBitsToSend, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidth_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM, IRDecodedRawDataType aData,
            uint_fast8_t aNumberOfBitsToSend, int_fast8_t aNumberOfRepeats);
    void sendPulseDistanceWidthData(PulseDistanceWidthProtocolConstants *aProtocolConstants, IRDecodedRawDataType aData,
            uint_fast8_t aNumberOfBitsToSend);
    void sendPulseDistanceWidthData_P(PulseDistanceWidthProtocolConstants const *aProtocolConstantsPGM, IRDecodedRawDataType aData,
            uint_fast8_t aNumberOfBitsToSend);
    void sendPulseDistanceWidth(uint_fast8_t aFrequencyKHz, uint16_t aHeaderMarkMicros, uint16_t aHeaderSpaceMicros,
            uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros, uint16_t aZeroSpaceMicros,
            IRDecodedRawDataType aData, uint_fast8_t aNumberOfBitsToSend, uint8_t aFlags, uint16_t aRepeatPeriodMillis,
            int_fast8_t aNumberOfRepeats, void (*aSpecialSendRepeatFunction)() = nullptr);
    void sendPulseDistanceWidthData(uint16_t aOneMarkMicros, uint16_t aOneSpaceMicros, uint16_t aZeroMarkMicros,
            uint16_t aZeroSpaceMicros, IRDecodedRawDataType aData, uint_fast8_t aNumberOfBitsToSend, uint8_t aFlags);
    void sendBiphaseData(uint16_t aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBitsToSend, bool aSendStartBit = true);

    void mark(uint16_t aMarkMicros);
    static void space(uint16_t aSpaceMicros);
    void IRLedOff();

// 8 Bit array
    void sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz);
    void sendRaw_P(const uint8_t aPGMBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz);
    void sendRaw(const uint8_t aBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz,
            uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);
    void sendRaw_P(const uint8_t aPGMBufferWithTicks[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz,
            uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);

// 16 Bit array
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz);
    void sendRaw_P(const uint16_t aPGMBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz);
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz,
            uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);
    void sendRaw_P(const uint16_t aPGMBufferWithMicroseconds[], uint_fast16_t aLengthOfBuffer, uint_fast8_t aFrequencyKHz,
            uint16_t aRepeatPeriodMillis, int_fast8_t aNumberOfRepeats);

    /*
     * Protocol specific send functions
     */
    void sendBangOlufsen(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint8_t aNumberOfHeaderBits = 8);
    void sendBangOlufsenDataLink(uint32_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats,
            uint8_t aNumberOfHeaderBits = 8);
    void sendBangOlufsenRaw(uint32_t aRawData, uint_fast8_t aNumberOfBitsToSend, bool aBackToBack = false);
    void sendBangOlufsenRawDataLink(uint64_t aRawData, uint_fast8_t aNumberOfBitsToSend, bool aBackToBack = false,
            bool aUseDatalinkTiming = false);
    void sendBoseWave(uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendDenon(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint8_t aSendSharpFrameMarker = 0);

    void sendFAST(uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendFAST16Bit(uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendJVC(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    void sendLGRepeat();
    uint32_t computeLGRawDataAndChecksum(uint8_t aAddress, uint16_t aCommand);
    void sendLG(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendLGRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats);

    void sendNECRepeat();
    uint32_t computeNECRawDataAndChecksum(uint16_t aAddress, uint16_t aCommand);
    void sendNEC(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendNEC2(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendNECRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats);
    // NEC variants
    void sendOnkyo(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendApple(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    void sendOpenLASIRRepeat();
    uint32_t computeOpenLASIRRawDataAndChecksum(uint8_t aAddress, uint16_t aCommand);
    uint16_t computeOpenLASIRRawCommand(uint8_t aDeviceID, uint8_t aMode, uint8_t aData);
    void sendOpenLASIR(uint8_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendOpenLASIR(uint8_t aAddress, uint8_t aDeviceID, uint8_t aMode, uint8_t aData, int_fast8_t aNumberOfRepeats);
    void sendOpenLASIRRaw(uint32_t aRawData, int_fast8_t aNumberOfRepeats);
#define OpenLASIR_GetDeviceId(aCommand)    ((aCommand) & 0xFF)
#define OpenLASIR_GetMode(aCommand)         (((aCommand) >> 8) & 0x1F)
#define OpenLASIR_GetData(aCommand)         ((aCommand) >> 13)

    void sendKaseikyo(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint16_t aVendorCode); // LSB first
    void sendPanasonic(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Denon(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Mitsubishi(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_Sharp(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo_JVC(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // LSB first

    void setNextToggleBitValueForRC5AndRC6(uint8_t aRC5ToggleBitValue);
    void sendRC5(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC5Marantz(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint8_t aMarantzExtension,
            bool aEnableAutomaticToggle = true);
    void sendRC6(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC6A(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint16_t aCustomer,
            bool aEnableAutomaticToggle = true);
    void sendRC6Raw(uint32_t aRawData, uint_fast8_t aNumberOfBitsToSend);
    void sendRC6Raw(uint64_t aRawData, uint_fast8_t aNumberOfBitsToSend);

    void sendSamsungLGRepeat();
    void sendSamsung(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung16BitAddressAnd8BitCommand(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung16BitAddressAndCommand(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsung48(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSamsungLG(uint16_t aAddress, uint16_t aCommand, int_fast8_t aNumberOfRepeats);
    void sendSharp(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSharp2(uint8_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSony(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, uint_fast8_t aNumberOfBitsToSend = 12); // SIRCS_12_PROTOCOL

    void sendLegoPowerFunctions(uint8_t aChannel, uint8_t aCommand, bool aDoSend5Times, uint8_t aMode = 0);
    void sendLegoPowerFunctionsRaw(uint16_t aRawData, uint8_t aChannel, bool aDoSend5Times = true);

    void sendMagiQuest(uint32_t aWandId, uint16_t aMagnitude);

    void sendPronto(const __FlashStringHelper *aProntoHexString, int_fast8_t aNumberOfRepeats);
    void sendPronto(const char *aProntoHexString, int_fast8_t aNumberOfRepeats);
    void sendPronto(const uint16_t *aData, uint16_t aLengthOfData, int_fast8_t aNumberOfRepeats);

#if defined(__AVR__)
    void sendPronto_PF(uint_farptr_t aProntoHexString, int_fast8_t aNumberOfRepeats);
    void sendPronto_P(const char *aProntoHexString, int_fast8_t aNumberOfRepeats);
#endif

// Template protocol :-)
    void sendShuzu(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats);

    /*
     * OTHER send functions
     */
    void sendDish(uint16_t aRawData);
    void sendWhynter(uint32_t aRawData, int_fast8_t aNumberOfRepeats);
    void sendVelux(uint8_t aMotorNumber, uint8_t aMotorSet, uint8_t aCommand, uint16_t aSecurityCode, int_fast8_t aNumberOfRepeats);
    void sendVelux(uint32_t aRawData, int_fast8_t aNumberOfRepeats);

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

/*
 * Special repeat functions
 */
void sendNECSpecialRepeat();
void sendOpenLASIRSpecialRepeat();
void sendLGSpecialRepeat();
void sendSamsungLGSpecialRepeat();

#endif // _IR_REMOTE_INT_H
