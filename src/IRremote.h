/**
 * @file IRremote.h
 * @brief Public API to the library.
 *
 * This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 */

//******************************************************************************
// IRremote
// Version 2.0.1 June, 2015
// Initially coded 2009 Ken Shirriff http://www.righto.com
// Edited by Mitra to add new controller SANYO
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// LG added by Darryl Smith (based on the JVC protocol)
// Whynter A/C ARC-110WD added by Francesco Meschia
// MagiQuest added by E. Stuart Hicks (based on code by mpflaga - https://github.com/mpflaga/Arduino-IRremote/)
//******************************************************************************
#ifndef IRremote_h
#define IRremote_h

//------------------------------------------------------------------------------
#include "private/IRremoteInt.h"

#define VERSION_IRREMOTE "2.9.0"
#define VERSION_IRREMOTE_MAJOR 2
#define VERSION_IRREMOTE_MINOR 9

/****************************************************
 *                     PROTOCOLS
 ****************************************************/

#if VERSION_IRREMOTE_MAJOR > 2
#define USE_STANDARD_DECODE
#else
//#define USE_STANDARD_DECODE // enables the standard NEC and other decoders.
#endif

/**
 * When received, marks  tend to be too long and spaces tend to be too short.
 * To compensate for this, MARK_EXCESS_MICROS is subtracted from all marks, and added to all spaces.
 * If you set MARK_EXCESS to approx. 50us then the TSOP4838 works best.
 * At 100us it also worked, but not as well.
 * Set MARK_EXCESS to 100us and the VS1838 doesn't work at all.
 */
#if !defined(MARK_EXCESS_MICROS)
//#define MARK_EXCESS_MICROS    50
#define MARK_EXCESS_MICROS    20 // recommended for the cheap VS1838 modules
#endif

//------------------------------------------------------------------------------
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to 0) all the protocols you do not need/want!
//
#define DECODE_KASEIKYO     1
// we have no DECODE_SHARP :-)
#define DECODE_SHARP        1
#define DECODE_NEC          1
#define DECODE_SONY         1
#define DECODE_PANASONIC    1
#define DECODE_DENON        1

/*
 * End of new standard protocols
 */
#define DECODE_BOSEWAVE     1
#define DECODE_JVC          1
#define DECODE_LEGO_PF      1
#define DECODE_LG           1
#define DECODE_MAGIQUEST    1
#define DECODE_RC5          1
#define DECODE_RC6          1
#define DECODE_SAMSUNG      1
#define DECODE_SANYO        1
#define DECODE_WHYNTER      1
#define DECODE_HASH         1 // special decoder for all protocols

#if DECODE_MAGIQUEST
#define ENABLE_EXTRA_INFO // for magnitude
#endif

/**
 * An enum consisting of all supported formats.
 * You do NOT need to remove entries from this list when disabling protocols!
 */
typedef enum {
    UNKNOWN = -1,
    UNUSED = 0,
    BOSEWAVE,
    DENON,
    DISH,
    JVC,
    LEGO_PF,
    LG,
    MAGIQUEST,
    NEC,
    PANASONIC,
    KASEIKYO,
    KASEIKYO_JVC,
    KASEIKYO_DENON,
    KASEIKYO_SHARP,
    KASEIKYO_MITSUBISHI,
    RC5,
    RC6,
    SAMSUNG,
    SANYO,
    SHARP,
    SONY,
    WHYNTER,
} decode_type_t;

/*
 * Result required by an application
 */
#define IRDATA_FLAGS_EMPTY              0x00
#define IRDATA_FLAGS_IS_REPEAT          0x01
#define IRDATA_FLAGS_IS_AUTO_REPEAT     0x02
#define IRDATA_FLAGS_PARITY_FAILED      0x04 // the current (autorepeat) frame violated parity check
#define IRDATA_TOGGLE_BIT_MASK          0x08
#define IRDATA_FLAGS_EXTRA_INFO         0x10 // there is unexpected extra info not contained in address and data (e.g. Kaseikyo unknown vendor ID)
#define IRDATA_FLAGS_WAS_OVERFLOW       0x40
#define IRDATA_FLAGS_IS_OLD_DECODER     0x80

struct IRData {
    decode_type_t protocol;     ///< UNKNOWN, NEC, SONY, RC5, ...
    uint16_t address;           ///< Decoded address
    uint16_t command;           ///< Decoded command
#if defined(ENABLE_EXTRA_INFO)
    uint16_t extra;             ///< Used by MagiQuest and for Kaseikyo unknown vendor ID
#endif
    uint8_t numberOfBits;    ///< Number of bits received for data (address + command + parity + etc.) to determine protocol length.
    uint8_t flags;              ///< See definitions above
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
bool MATCH(unsigned int measured, unsigned int desired);
bool MATCH_MARK(uint16_t measured_ticks, unsigned int desired_us);
bool MATCH_SPACE(uint16_t measured_ticks, unsigned int desired_us);

/****************************************************
 *                     RECEIVING
 ****************************************************/
/**
 * Results returned from old decoders !!!deprecated!!!
 */
struct decode_results {
    decode_type_t decode_type; // deprecated ///< UNKNOWN, NEC, SONY, RC5, ...
//    uint16_t address;           ///< Used by Panasonic & Sharp & NEC_standard [16-bits]
    uint32_t value;             ///< Decoded value / command [max 32-bits]
    uint8_t bits;               // deprecated - only for backwards compatibility ///< Number of bits in decoded value
#if DECODE_MAGIQUEST
    uint16_t magnitude;         ///< Used by MagiQuest [16-bits]
#endif
    bool isRepeat;         // deprecated              ///< True if repeat of value is detected

// next 3 values are copies of irparams values - see IRremoteint.h
    uint16_t *rawbuf;           ///< Raw intervals in 50uS ticks
    uint16_t rawlen;            ///< Number of records in rawbuf
    bool overflow;              // deprecated ///< true if IR raw code too long
};

/**
 * DEPRECATED
 * Decoded value for NEC and others when a repeat code is received
 * Use Flag decode_decodedIRData.isRepeat (see above) instead
 */
#define REPEAT 0xFFFFFFFF

/**
 * Main class for receiving IR
 */
class IRrecv {
public:

    IRrecv(int recvpin);
    IRrecv(int recvpin, int blinkpin);
    void blink13(int blinkflag);

    void enableIRIn();
    void disableIRIn();
    void start(); // alias for enableIRIn
    void stop(); // alias for disableIRIn

    bool isIdle();
    bool available();

    /*
     * The main functions
     */
    bool decode();
    void resume();

    /*
     * Useful info and print functions
     */
    void printIRResultShort(Print *aSerial);
    void printIRResultShort(Print *aSerial, IRData *aDecodedDataPtr, uint16_t aLeadingSpaceDuration = 0);
    void printIRResultRawFormatted(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void printIRResultAsCVariables(Print *aSerial);

    void compensateAndPrintIRResultAsCArray(Print *aSerial, bool aOutputMicrosecondsInsteadOfTicks = true);
    void compensateAndPrintIRResultAsPronto(Print *aSerial, unsigned int frequency = 38000U);

    const char* getProtocolString();

    /*
     * Store the data for further processing
     */
    void compensateAndStoreIRResultInArray(uint8_t *aArrayPtr);
    size_t compensateAndStorePronto(String *aString, unsigned int frequency = 38000U);

    /*
     * The main decoding functions used by the individual decoders
     */
    bool decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, unsigned int aBitMarkMicros,
            unsigned int aOneSpaceMicros, unsigned int aZeroSpaceMicros, bool aMSBfirst = true);

    bool decodePulseWidthData(uint8_t aNumberOfBits, uint8_t aStartOffset, unsigned int aOneMarkMicros,
            unsigned int aZeroMarkMicros, unsigned int aBitSpaceMicros, bool aMSBfirst = true);

    bool decodeBiPhaseData(uint8_t aNumberOfBits, uint8_t aStartOffset, uint8_t aValueOfSpaceToMarkTransition,
            unsigned int aBiphaseTimeUnit);

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
    bool decodeWhynter();

    bool decode(decode_results *aResults) __attribute__ ((deprecated ("You should use decode() without a parameter."))); // deprecated
    bool decodeBoseWave(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeDenon(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeJVC(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeLG(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeMagiQuest(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeNEC(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodePanasonic(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeRC5(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeRC6(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeSAMSUNG() __attribute__ ((deprecated ("Renamed to decodeSamsung()"))); // deprecated
    bool decodeSAMSUNG(decode_results *aResults) __attribute__ ((deprecated ("You should use decodeSamsung() without a parameter."))); // deprecated
    bool decodeSony(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeWhynter(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated
    bool decodeHash(decode_results *aResults) __attribute__ ((deprecated ("You should use decode*() without a parameter."))); // deprecated

// To be removed
    bool decodeSanyo();
    bool decodeSanyo(decode_results *aResults);

    /*
     * Internal functions
     */
    void initDecodedIRData();
    uint8_t compare(unsigned int oldval, unsigned int newval);

    decode_results results;         // the instance for the ISR which does the decoding
    IRData decodedIRData;           // decoded IR data for the application, used by all new / updated decoders

    // Last decoded IR data for repeat detection, used by all new / updated decoders
    uint32_t lastDecodedAddress;
    uint32_t lastDecodedCommand;

    uint8_t repeatCount;            // Used for Denon decode for autorepeat decoding.
};

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
//
#define NO_REPEATS  0 // for better readability of code
/**
 * Main class for sending IR
 */
class IRsend {
public:
#if defined(USE_SOFT_SEND_PWM) || defined(USE_NO_SEND_PWM)
    IRsend(int pin = IR_SEND_PIN) {
        sendPin = pin;
    }
#else
    IRsend() {
    }
#endif

    void enableIROut(int khz);

    void sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
            unsigned int aZeroSpaceMicros, uint32_t aData, uint8_t aNumberOfBits, bool aMSBfirst = true, bool aSendStopBit = false);
    void sendBiphaseData(unsigned int aBiphaseTimeUnit, uint32_t aData, uint8_t aNumberOfBits);

    void mark(uint16_t timeMicros);
    void space(uint16_t timeMicros);

// 8 Bit array
    void sendRaw(const uint8_t aBufferWithTicks[], uint8_t aLengthOfBuffer, uint8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint8_t aBufferWithTicks[], uint8_t aLengthOfBuffer, uint8_t aIRFrequencyKilohertz);

// 16 Bit array
    void sendRaw(const uint16_t aBufferWithMicroseconds[], uint8_t aLengthOfBuffer, uint8_t aIRFrequencyKilohertz);
    void sendRaw_P(const uint16_t aBufferWithMicroseconds[], uint8_t aLengthOfBuffer, uint8_t aIRFrequencyKilohertz);

    /*
     * Constants for some protocols
     */
#define PANASONIC_VENDOR_ID_CODE    0x2002
#define SHARP_VENDOR_ID_CODE        0x5AAA
#define DENON_VENDOR_ID_CODE        0x3254
#define MITSUBISHI_VENDOR_ID_CODE   0xCB23
#define JVC_VENDOR_ID_CODE          0x0103

#define SIRCS_12_PROTOCOL       12
#define SIRCS_20_PROTOCOL       20

#define LEGO_MODE_EXTENDED  0
#define LEGO_MODE_COMBO     1
#define LEGO_MODE_SINGLE    0x4 // here the 2 LSB have meanings like Output A / Output B

    /*
     * New send functions
     */
    void sendBoseWave(uint8_t aCommand, uint8_t aNumberOfRepeats = NO_REPEATS);
    void sendDenon(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aSendSharp = false);
    void sendJVC(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats);

    void sendLGRepeat();
    void sendLG(uint8_t aAddress, uint16_t aCommand, uint8_t aNumberOfRepeats, bool aIsRepeat = false);
    void sendNECRepeat();
    void sendNEC(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aIsRepeat = false);

    void sendPanasonic(uint16_t aAddress, uint8_t aData, uint8_t aNumberOfRepeats); // LSB first
    void sendKaseikyo(uint16_t aAddress, uint8_t aData, uint8_t aNumberOfRepeats, uint16_t aVendorCode); // LSB first

    void sendRC5(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendRC6(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, bool aEnableAutomaticToggle = true);
    void sendSamsungRepeat();
    void sendSamsung(uint16_t aAddress, uint16_t aCommand, uint8_t aNumberOfRepeats, bool aIsRepeat = false);
    void sendSharp(uint8_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats); // redirected to sendDenon
    void sendSony(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats, uint8_t numberOfBits = SIRCS_12_PROTOCOL);

    void sendLegoPowerFunctions(uint8_t aChannel, uint8_t tCommand, uint8_t aMode, bool aDoSend5Times = true);
    void sendLegoPowerFunctions(uint16_t aRawData, bool aDoSend5Times = true);
    void sendLegoPowerFunctions(uint16_t aRawData, uint8_t aChannel, bool aDoSend5Times = true);

    void sendMagiQuest(uint32_t wand_id, uint16_t magnitude);

    void sendPronto(const __FlashStringHelper *str, uint8_t numberOfRepeats = NO_REPEATS);
    void sendPronto(const char *prontoHexString, uint8_t numberOfRepeats = NO_REPEATS);
    void sendPronto(const uint16_t *data, unsigned int length, uint8_t numberOfRepeats = NO_REPEATS);
#if defined(__AVR__)
    void sendPronto_PF(uint_farptr_t str, uint8_t numberOfRepeats = NO_REPEATS);
    void sendPronto_P(const char *str, uint8_t numberOfRepeats);
#endif

// Template protocol :-)
    void sendShuzu(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats);

    /*
     * OLD send functions
     */
    void sendDenon(unsigned long data, int nbits);
    void sendDISH(unsigned long data, int nbits);
    void sendJVC(unsigned long data, int nbits, bool repeat = false);
    void sendLG(unsigned long data, int nbits);
    void sendNEC(uint32_t data, uint8_t nbits, bool repeat = false);
    void sendPanasonic(uint16_t aAddress, uint32_t aData);
    void sendRC5(uint32_t data, uint8_t nbits);
    void sendRC5ext(uint8_t addr, uint8_t cmd, boolean toggle);
    void sendRC6(uint32_t data, uint8_t nbits);
    void sendRC6(uint64_t data, uint8_t nbits);
    void sendSharpRaw(unsigned long data, int nbits);
    void sendSharp(unsigned int address, unsigned int command);
    void sendSAMSUNG(unsigned long data, int nbits);
    void sendSony(unsigned long data, int nbits);
    void sendWhynter(unsigned long data, int nbits);

#if defined(USE_SOFT_SEND_PWM) || defined(USE_NO_SEND_PWM)
    uint8_t sendPin;

#  if defined(USE_SOFT_SEND_PWM)
    unsigned int periodTimeMicros;
    unsigned int periodOnTimeMicros;

    void sleepMicros(unsigned long us);
    void sleepUntilMicros(unsigned long targetTime);
#  endif

#else
    const int sendPin = IR_SEND_PIN;
#endif
};

#endif // IRremote_h
