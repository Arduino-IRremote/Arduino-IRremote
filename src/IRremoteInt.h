/**
 * @file IRremoteInt.h
 * @brief Contains all declarations required for the internal functions.
 *
 * This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2015-2021 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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
#ifndef IRremoteInt_h
#define IRremoteInt_h

#if ! defined(RAW_BUFFER_LENGTH)
#define RAW_BUFFER_LENGTH  101  ///< Maximum length of raw duration buffer. Must be odd! 101 supports up to 48 bit codings.
#endif

#define VERSION_IRREMOTE "3.0.1"
#define VERSION_IRREMOTE_MAJOR 3
#define VERSION_IRREMOTE_MINOR 0
/*
 * Just for better readability of code
 */
#define DISABLE_LED_FEEDBACK false
#define ENABLE_LED_FEEDBACK true

#define SEND_STOP_BIT true
#define SEND_REPEAT_COMMAND true // used for e.g. NEC, where a repeat is different from just repeating the data.

/*
 * Try to activate it, if you have legacy code to compile with version >= 3
 */
//#define USE_OLD_DECODE // enables the old NEC and other old decoders.
//------------------------------------------------------------------------------
#include <Arduino.h>

#include "irProtocol.h"

// All board specific stuff have been moved to its own file, included here.
#include "private/IRremoteBoardDefs.h"

//------------------------------------------------------------------------------
// Information for the Interrupt Service Routine
//

// ISR State-Machine : Receiver States
#define IR_REC_STATE_IDLE      0
#define IR_REC_STATE_MARK      1
#define IR_REC_STATE_SPACE     2
#define IR_REC_STATE_STOP      3

/**
 * This struct contains the data and control used for static functions and the ISR (interrupt service routine)
 * Only rcvstate needs to be volatile. All the other fields are not written by ISR during decoding in loop.
 */
struct irparams_struct {
    // The fields are ordered to reduce memory over caused by struct-padding
    volatile uint8_t rcvstate;      ///< State Machine state
    uint8_t recvpin;                ///< Pin connected to IR data from detector
    uint8_t blinkpin;               ///< 0 means not valid pin
    bool blinkflag;                 ///< true -> enable blinking of pin on IR processing
#if RAW_BUFFER_LENGTH <= 255        // saves around 75 bytes program space and speeds up ISR
    uint8_t rawlen;                 ///< counter of entries in rawbuf
#else
    unsigned int rawlen;            ///< counter of entries in rawbuf
#endif
    uint16_t timer;              ///< State timer, counts 50uS ticks. The value is copied into the rawbuf array on every transition.
    uint8_t overflow;               ///< Raw buffer overflow occurred
    uint16_t rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space, first entry is the length of the gap between previous and current command
};

extern struct irparams_struct irparams;

/*
 * Result required by an application
 */
#define IRDATA_FLAGS_EMPTY              0x00
#define IRDATA_FLAGS_IS_REPEAT          0x01
#define IRDATA_FLAGS_IS_AUTO_REPEAT     0x02
#define IRDATA_FLAGS_PARITY_FAILED      0x04 // the current (autorepeat) frame violated parity check
#define IRDATA_TOGGLE_BIT_MASK          0x08
#define IRDATA_FLAGS_EXTRA_INFO         0x10 // there is unexpected extra info not contained in address and data (e.g. Kaseikyo unknown vendor ID)
#define IRDATA_FLAGS_WAS_OVERFLOW       0x40 // irparams.rawlen is 0 in this case to avoid endless overflow
#define IRDATA_FLAGS_IS_LSB_FIRST       0x00
#define IRDATA_FLAGS_IS_MSB_FIRST       0x80 // Just for info. Value is simply determined by the protocol

struct IRData {
    decode_type_t protocol;     ///< UNKNOWN, NEC, SONY, RC5, ...
    uint16_t address;           ///< Decoded address
    uint16_t command;           ///< Decoded command
    uint16_t extra;             ///< Used by MagiQuest and for Kaseikyo unknown vendor ID
    uint8_t numberOfBits; ///< Number of bits received for data (address + command + parity) - to determine protocol length if different length are possible (currently only Sony).
    uint8_t flags;              ///< See definitions above
    uint32_t decodedRawData;    ///< up to 32 bit decoded raw data, formerly used for send functions.
    irparams_struct *rawDataPtr; /// pointer of the raw timing data to be decoded
};

//#define DEBUG // Activate this for lots of lovely debug output.
//------------------------------------------------------------------------------
// Debug directives
//
#ifdef DEBUG
#  define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
#  define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
/**
 * If DEBUG, print the arguments, otherwise do nothing.
 */
#  define DBG_PRINT(...) void()
/**
 * If DEBUG, print the arguments as a line, otherwise do nothing.
 */
#  define DBG_PRINTLN(...) void()
#endif

#ifdef TRACE
#  define TRACE_PRINT(...)    Serial.print(__VA_ARGS__)
#  define TRACE_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#  define TRACE_PRINT(...) void()
#  define TRACE_PRINTLN(...) void()
#endif
//------------------------------------------------------------------------------
// Helper macro for getting a macro definition as string
//
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//------------------------------------------------------------------------------
// Mark & Space matching functions
//
bool MATCH(uint16_t measured, uint16_t desired);
bool MATCH_MARK(uint16_t measured_ticks, uint16_t desired_us);
bool MATCH_SPACE(uint16_t measured_ticks, uint16_t desired_us);

int getMarkExcessMicros();

void setFeedbackLED(bool aSwitchLedOn);

/****************************************************
 *                     RECEIVING
 ****************************************************/
/**
 * Results returned from old decoders !!!deprecated!!!
 */
struct decode_results {
    decode_type_t decode_type;  // deprecated, moved to decodedIRData.protocol ///< UNKNOWN, NEC, SONY, RC5, ...
//    uint16_t address;         ///< Used by Panasonic & Sharp & NEC_standard [16-bits]
    uint32_t value;             // deprecated, moved to decodedIRData.decodedRawData ///< Decoded value / command [max 32-bits]
    uint8_t bits;               // deprecated, moved to decodedIRData.numberOfBits ///< Number of bits in decoded value
    uint16_t magnitude;         // deprecated, moved to decodedIRData.extra ///< Used by MagiQuest [16-bits]
    bool isRepeat;              // deprecated, moved to decodedIRData.flags ///< True if repeat of value is detected

// next 3 values are copies of irparams values - see IRremoteint.h
    uint16_t *rawbuf;           // deprecated, moved to decodedIRData.rawDataPtr->rawbuf ///< Raw intervals in 50uS ticks
    uint16_t rawlen;            // deprecated, moved to decodedIRData.rawDataPtr->rawlen ///< Number of records in rawbuf
    bool overflow;              // deprecated, moved to decodedIRData.flags ///< true if IR raw code too long
};

/*
 * Just for better readability of code
 */
#define DISABLE_LED_FEEDBACK false
#define ENABLE_LED_FEEDBACK true

#define SEND_STOP_BIT true
#define SEND_REPEAT_COMMAND true // used for e.g. NEC, where a repeat is different from just repeating the data.

#define USE_DEFAULT_FEEDBACK_LED_PIN 0
/**
 * Main class for receiving IR
 */
class IRrecv {
public:

    IRrecv();
    IRrecv(uint8_t recvpin);
    IRrecv(uint8_t recvpin, uint8_t blinkpin);
    static void blink13(bool aEnableLEDFeedback);
    static void setBlinkPin(uint8_t aBlinkPin); // if 0, then take board BLINKLED_ON() and BLINKLED_OFF() functions

    void enableIRIn();
    void disableIRIn();

    /*
     * Stream like API
     */
    void begin(uint8_t aReceivePin, bool aEnableLEDFeedback = false, uint8_t aLEDFeedbackPin = USE_DEFAULT_FEEDBACK_LED_PIN); // if aBlinkPin == 0 then take board default BLINKPIN
    void start(); // alias for enableIRIn
    bool available();
    IRData* read(); // returns decoded data
    // write is a method of class IRsend below
    // size_t write(IRData *aIRSendData, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void stop(); // alias for disableIRIn
    void end();

    bool isIdle();

    /*
     * The main functions
     */
    bool decode();  // Check if available and try to decode
    void resume();  // Enable receiving of the next value

    /*
     * Useful info and print functions
     */
    void printIRResultShort(Print *aSerial);
    void printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void printIRResultAsCVariables(Print *aSerial);

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
    bool decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint16_t aBitMarkMicros,
            uint16_t aOneSpaceMicros, uint16_t aZeroSpaceMicros, bool aMSBfirst);

    bool decodePulseWidthData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint16_t aOneMarkMicros,
            uint16_t aZeroMarkMicros, uint16_t aBitSpaceMicros, bool aMSBfirst);

    bool decodeBiPhaseData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint8_t aValueOfSpaceToMarkTransition,
            uint16_t aBiphaseTimeUnit);

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
    bool decodePanasonic();
    bool decodeRC5();
    bool decodeRC6();
    bool decodeSamsung();
    bool decodeSharp(); // redirected to decodeDenon()
    bool decodeSony();

    bool decodeHash();

    // Template function :-)
    bool decodeShuzu();

    /*
     * Old functions
     */
    bool decode(decode_results *aResults) __attribute__ ((deprecated ("Please use decode() without a parameter."))); // deprecated
    bool decodeWhynter();

    bool decodeSAMSUNG();

    /*
     * Internal functions
     */
    void initDecodedIRData();
    uint8_t compare(unsigned int oldval, unsigned int newval);

    decode_results results; // deprecated, only for legacy compatibility
    IRData decodedIRData;           // decoded IR data for the application

    // Last decoded IR data for repeat detection
    uint32_t lastDecodedAddress;
    uint32_t lastDecodedCommand;

    uint8_t repeatCount;            // Used e.g. for Denon decode for autorepeat decoding.
};

// The receiver instance
extern IRrecv IrReceiver;
// static function
void printIRResultShort(Print *aSerial, IRData *aIRDataPtr, uint16_t aLeadingSpaceDuration = 0);

/****************************************************
 *                     SENDING
 ****************************************************/
/**
 * Define to use no carrier PWM, just simulate an active low receiver signal.
 */
//#define USE_NO_SEND_PWM
/**
 * Define to use carrier PWM generation in software, instead of hardware PWM.
 */
//#define USE_SOFT_SEND_PWM
/**
 * If USE_SOFT_SEND_PWM, this amount is subtracted from the on-time of the pulses.
 */
#ifndef PULSE_CORRECTION_MICROS
#define PULSE_CORRECTION_MICROS 3
#endif
/**
 * If USE_SOFT_SEND_PWM, use spin wait instead of delayMicros().
 */
//#define USE_SPIN_WAIT
/*
 * Just for better readability of code
 */
#define NO_REPEATS  0
/**
 * Main class for sending IR
 */
class IRsend {
public:
#if defined(USE_SOFT_SEND_PWM) || defined(USE_NO_SEND_PWM)
    IRsend(int pin = IR_SEND_PIN);
#else
    IRsend();
#endif

    void begin(bool aEnableLEDFeedback, uint8_t aLEDFeedbackPin = USE_DEFAULT_FEEDBACK_LED_PIN);

    size_t write(IRData *aIRSendData, uint_fast8_t aNumberOfRepeats = NO_REPEATS);

    void enableIROut(uint8_t aFrequencyKHz);

    void sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
            unsigned int aZeroSpaceMicros, uint32_t aData, uint8_t aNumberOfBits, bool aMSBfirst, bool aSendStopBit = false);
    void sendBiphaseData(unsigned int aBiphaseTimeUnit, uint32_t aData, uint_fast8_t aNumberOfBits);

    void mark(unsigned int timeMicros);
    void space(unsigned int timeMicros);
    void ledOff();

// 8 Bit array
    void sendRaw(const uint8_t aBufferWithTicks[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint8_t aBufferWithTicks[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);

// 16 Bit array
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint16_t aBufferWithMicroseconds[], uint_fast8_t aLengthOfBuffer, uint_fast8_t aIRFrequencyKilohertz);

    /*
     * New send functions
     */
    void sendBoseWave(uint8_t aCommand, uint_fast8_t aNumberOfRepeats = NO_REPEATS);
    void sendDenon(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendSharp = false);
    void sendDenonRaw(uint16_t aRawData, uint_fast8_t aNumberOfRepeats = 0);
    __attribute__ ((deprecated ("Please use sendDenon(aAddress, aCommand, aNumberOfRepeats).")));
    void sendJVC(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats);
    void sendJVCRaw(uint16_t aRawData, uint_fast8_t aNumberOfRepeats = 0);
    __attribute__ ((deprecated ("Please use sendJVC(aAddress, aCommand, aNumberOfRepeats).")));

    void sendLGRepeat();
    void sendLG(uint8_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat = false);
    void sendLGRaw(uint32_t aRawData, uint_fast8_t aNumberOfRepeats = 0, bool aIsRepeat = false);

    void sendNECRepeat();
    void sendNEC(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat = false);
    void sendNECRaw(uint32_t aRawData, uint_fast8_t aNumberOfRepeats = 0, bool aIsRepeat = false);

    void sendApple(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat = false);

    void sendPanasonic(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo(uint16_t aAddress, uint8_t aData, uint_fast8_t aNumberOfRepeats, uint16_t aVendorCode); // LSB first

    void sendRC5(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC6(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendSamsungRepeat();
    void sendSamsung(uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aIsRepeat = false);
    void sendSharp(uint8_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSony(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats, uint8_t numberOfBits = SIRCS_12_PROTOCOL);

    void sendLegoPowerFunctions(IRData *aIRSendData, bool aDoSend5Times = true);
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
    void sendJVCMSB(unsigned long data, int nbits, bool repeat = false);
    void sendLG(unsigned long data, int nbits);
    void sendNECMSB(uint32_t data, uint8_t nbits, bool repeat = false);
    void sendPanasonic(uint16_t aAddress,
            uint32_t aData)
                    __attribute__ ((deprecated ("This old function sends MSB first! Please use sendPanasonic(aAddress, aCommand, aNumberOfRepeats).")));
    void sendRC5(uint32_t data, uint8_t nbits);
    void sendRC5ext(uint8_t addr, uint8_t cmd, boolean toggle);
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

    uint8_t sendPin;

#if defined(USE_SOFT_SEND_PWM) || defined(USE_NO_SEND_PWM)

#  if defined(USE_SOFT_SEND_PWM)
    unsigned int periodTimeMicros;
    unsigned int periodOnTimeMicros;

    void sleepMicros(unsigned long us);
    void sleepUntilMicros(unsigned long targetTime);
#  endif
#endif

private:
    void custom_delay_usec(unsigned long uSecs);
};

// The sender instance
extern IRsend IrSender;

// Content is from V2 IRremoteint.h
/*
 * Activate this line if your receiver has an external output driver transistor / "inverted" output
 */
//#define IR_INPUT_IS_ACTIVE_HIGH
//------------------------------------------------------------------------------
// Defines for setting and clearing register bits
//
#ifndef cbi
#define cbi(sfr, bit)  (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit)  (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//------------------------------------------------------------------------------
// Pulse parms are ((X*50)-100) for the Mark and ((X*50)+100) for the Space.
// First MARK is the one after the long gap
// Pulse parameters in uSec
//

/** Relative tolerance (in percent) for some comparisons on measured data. */
#define TOLERANCE       25

/** Lower tolerance for comparison of measured data */
//#define LTOL            (1.0 - (TOLERANCE/100.))
#define LTOL            (100 - TOLERANCE)
/** Upper tolerance for comparison of measured data */
//#define UTOL            (1.0 + (TOLERANCE/100.))
#define UTOL            (100 + TOLERANCE)

/** Minimum gap between IR transmissions, in microseconds */
#define RECORD_GAP_MICROS   5000 // NEC header space is 4500

/** Minimum gap between IR transmissions, in MICROS_PER_TICK */
#define RECORD_GAP_TICKS    (RECORD_GAP_MICROS / MICROS_PER_TICK)

//#define TICKS_LOW(us)   ((int)(((us)*LTOL/MICROS_PER_TICK)))
//#define TICKS_HIGH(us)  ((int)(((us)*UTOL/MICROS_PER_TICK + 1)))
#if MICROS_PER_TICK == 50 && TOLERANCE == 25           // Defaults
#define TICKS_LOW(us)   ((us)/67 )     // (us) / ((MICROS_PER_TICK:50 / LTOL:75 ) * 100)
#define TICKS_HIGH(us)  ((us)/40 + 1)  // (us) / ((MICROS_PER_TICK:50 / UTOL:125) * 100) + 1
#else
    #define TICKS_LOW(us)   ((uint16_t) ((long) (us) * LTOL / (MICROS_PER_TICK * 100) ))
    #define TICKS_HIGH(us)  ((uint16_t) ((long) (us) * UTOL / (MICROS_PER_TICK * 100) + 1))
#endif

//------------------------------------------------------------------------------
// IR receivers on a board with an external output transistor may have "inverted" output
#ifdef IR_INPUT_IS_ACTIVE_HIGH
// IR detector output is active high
#define MARK   1 ///< Sensor output for a mark ("flash")
#define SPACE  0 ///< Sensor output for a space ("gap")
#else
// IR detector output is active low
#define MARK   0 ///< Sensor output for a mark ("flash")
#define SPACE  1 ///< Sensor output for a space ("gap")
#endif

#endif // IRremoteInt_h

#pragma once

