/**
 * @file IRremote.h
 * @brief Public API to the library.
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

/****************************************************
 *                     PROTOCOLS
 ****************************************************/
//------------------------------------------------------------------------------
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to 0) all the protocols you do not need/want!
//
//#define DECODE_AIWA_RC_T501  1
//#define SEND_AIWA_RC_T501    1

#define DECODE_BOSEWAVE      1
#define SEND_BOSEWAVE        1

#define DECODE_DENON         1
#define SEND_DENON           1

#define DECODE_DISH          0 // NOT WRITTEN
#define SEND_DISH            1

#define DECODE_JVC           1
#define SEND_JVC             1

#define DECODE_LEGO_PF       1
#define SEND_LEGO_PF         1

#define DECODE_LG            1
#define SEND_LG              1

#define DECODE_MAGIQUEST     1
#define SEND_MAGIQUEST       1

//#define DECODE_MITSUBISHI    1 // Faulty implementation
//#define SEND_MITSUBISHI      0 // NOT WRITTEN

//#define USE_NEC_STANDARD // remove comment to have the standard NEC decoding (LSB first) available.
#if defined(USE_NEC_STANDARD)
#define DECODE_NEC_STANDARD  1
#define DECODE_NEC           0
#define LSB_FIRST_REQUIRED
#else
#define DECODE_NEC_STANDARD  0
#define DECODE_NEC           1
#endif
#define SEND_NEC             1
#define SEND_NEC_STANDARD    1

#define DECODE_PANASONIC     1
#define SEND_PANASONIC       1

#define DECODE_RC5           1
#define SEND_RC5             1

#define DECODE_RC6           1
#define SEND_RC6             1

#define DECODE_SAMSUNG       1
#define SEND_SAMSUNG         1

#define DECODE_SANYO         1
#define SEND_SANYO           0 // NOT WRITTEN

#define DECODE_SHARP         1
#define SEND_SHARP           1

#define DECODE_SHARP_ALT     1
#define SEND_SHARP_ALT       1
#if SEND_SHARP_ALT
#define LSB_FIRST_REQUIRED
#endif

#define DECODE_SONY          1
#define SEND_SONY            1

#define DECODE_WHYNTER       1
#define SEND_WHYNTER         1

#define DECODE_HASH          1 // special decoder for all protocols

/**
 * An enum consisting of all supported formats.
 * You do NOT need to remove entries from this list when disabling protocols!
 */
typedef enum {
    UNKNOWN = -1,
    UNUSED = 0,
//    AIWA_RC_T501,
    BOSEWAVE,
    DENON,
    DISH,
    JVC,
    LEGO_PF,
    LG,
    MAGIQUEST,
//    MITSUBISHI,
    NEC_STANDARD,
    NEC,
    PANASONIC,
    RC5,
    RC6,
    SAMSUNG,
    SANYO,
    SHARP,
    SHARP_ALT,
    SONY,
    WHYNTER,
} decode_type_t;

/**
 * Comment this out for lots of lovely debug output.
 */
//#define DEBUG
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

//------------------------------------------------------------------------------
// Helper macro for getting a macro definition as string
//
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//------------------------------------------------------------------------------
// Mark & Space matching functions
//
int MATCH(unsigned int measured, unsigned int desired);
int MATCH_MARK(uint16_t measured_ticks, unsigned int desired_us);
int MATCH_SPACE(uint16_t measured_ticks, unsigned int desired_us);

/****************************************************
 *                     RECEIVING
 ****************************************************/
/**
 * Results returned from the decoder
 */
struct decode_results {
    decode_type_t decode_type;  ///< UNKNOWN, NEC, SONY, RC5, ...
    uint16_t address;           ///< Used by Panasonic & Sharp6 NEC_standard [16-bits]
    uint32_t value;             ///< Decoded value / command [max 32-bits]
    uint16_t bits;              ///< Number of bits in decoded value
    uint16_t magnitude;         ///< Used by MagiQuest [16-bits]
    bool isRepeat;              ///< True if repeat of value is detected

    // next 3 values are copies of irparams values - see IRremoteint.h
    uint16_t *rawbuf;           ///< Raw intervals in 50uS ticks
    uint16_t rawlen;            ///< Number of records in rawbuf
    bool overflow;              ///< true if IR raw code too long
};

/**
 * DEPRECATED
 * Decoded value for NEC and others when a repeat code is received
 * Use Flag decode_results.isRepeat (see above) instead
 */
#define REPEAT 0xFFFFFFFF

/**
 * Main class for receiving IR
 */
class IRrecv {
public:
    /**
     * Instantiate the IRrecv class. Multiple instantiation is not supported.
     * @param recvpin Arduino pin to use. No sanity check is made.
     */
    IRrecv(int recvpin);
    /**
     * Instantiate the IRrecv class. Multiple instantiation is not supported.
     * @param recvpin Arduino pin to use, where a demodulating IR receiver is connected.
     * @param blinkpin pin to blink when receiving IR. Not supported by all hardware. No sanity check is made.
     */
    IRrecv(int recvpin, int blinkpin);

    /**
     * TODO: Why is this public???
     * @param blinkflag
     */
    void blink13(int blinkflag);

    /**
     * Attempt to decode the recently receive IR signal
     * @param results decode_results instance returning the decode, if any.
     * @return success of operation.
     */
    bool decode(decode_results *aResults);__attribute__ ((deprecated ("You should use decode() without a parameter.")))
    ;               // deprecated
    bool decode();

    /**
     * Enable IR reception.
     */
    void enableIRIn();

    /**
     * Disable IR reception.
     */
    void disableIRIn();

    /**
     * Returns status of reception
     * @return true if no reception is on-going.
     */
    bool isIdle();

    /**
     * Returns status of reception and copies IR-data to decode_results buffer if true.
     * @return true if data is available.
     */
    bool available();

    /**
     * Called to re-enable IR reception.
     */
    void resume();

    const char* getProtocolString();
    void printResultShort(Print *aSerial);
    void printIRResultRaw(Print *aSerial);
    void printIRResultRawFormatted(Print *aSerial);
    void printIRResultAsCArray(Print *aSerial);
    void printIRResultAsCVariables(Print *aSerial);
    /**
     * Print the result (second argument) as Pronto Hex on the Stream supplied as argument.
     * @param stream The Stream on which to write, often Serial
     * @param results the decode_results as delivered from irrecv.decode.
     * @param frequency Modulation frequency in Hz. Often 38000Hz.
     */
    void dumpPronto(Print *aSerial, unsigned int frequency = 38000U);
    void printIRResultAsPronto(Print *aSerial, unsigned int frequency = 38000U);

    bool decodePulseDistanceData(uint8_t aNumberOfBits, uint8_t aStartOffset, unsigned int aBitMarkMicros,
            unsigned int aOneSpaceMicros, unsigned int aZeroSpaceMicros, bool aMSBfirst = true);

    decode_results results; // the instance for decoding

private:
#if DECODE_HASH
    bool decodeHash();
    bool decodeHash(decode_results *aResults);
    int compare(unsigned int oldval, unsigned int newval);
#endif

    //......................................................................
#if DECODE_RC5
    /**
     * Try to decode the recently received IR signal as an RC5 signal-
     * @param results decode_results instance returning the decode, if any.
     * @return Success of the operation.
     */
    bool decodeRC5();
    bool decodeRC5(decode_results *aResults);
#endif
#if DECODE_RC6
    bool decodeRC6();
    bool decodeRC6(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_NEC
    bool decodeNEC();
    bool decodeNEC(decode_results *aResults);
#endif
#if DECODE_NEC_STANDARD
    bool decodeNECStandard();
#endif

    //......................................................................
#if DECODE_SONY
    bool decodeSony();
    bool decodeSony(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_PANASONIC
    bool decodePanasonic();
    bool decodePanasonic(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_JVC
    bool decodeJVC();
    bool decodeJVC(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_SAMSUNG
    bool decodeSAMSUNG();
    bool decodeSAMSUNG(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_WHYNTER
    bool decodeWhynter();
    bool decodeWhynter(decode_results *aResults);
#endif
    //......................................................................
//#if DECODE_AIWA_RC_T501
//    bool decodeAiwaRCT501();
//    bool decodeAiwaRCT501(decode_results *aResults);
//#endif
    //......................................................................
#if DECODE_LG
    bool decodeLG();
    bool decodeLG(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_SANYO
    bool decodeSanyo();
    bool decodeSanyo(decode_results *aResults);
#endif
    //......................................................................
//#if DECODE_MITSUBISHI
//    bool decodeMitsubishi();
//    bool decodeMitsubishi(decode_results *aResults);
//#endif
    //......................................................................
#if DECODE_DISH
      bool  decodeDish () ; // NOT WRITTEN
#endif
    //......................................................................
#if DECODE_SHARP
    bool decodeSharp();
    bool decodeSharp(decode_results *aResults);
#endif
#if DECODE_SHARP_ALT
    bool decodeSharpAlt();
    bool decodeSharpAlt(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_DENON
    bool decodeDenon();
    bool decodeDenon(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_LEGO_PF
      bool  decodeLegoPowerFunctions () ;
#endif
    //......................................................................
#if DECODE_BOSEWAVE
    bool decodeBoseWave();
    bool decodeBoseWave(decode_results *aResults);
#endif
    //......................................................................
#if DECODE_MAGIQUEST
    bool decodeMagiQuest();
    bool decodeMagiQuest(decode_results *aResults);
#endif
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

    void custom_delay_usec(unsigned long uSecs);
    void enableIROut(int khz);
    void sendPulseDistanceWidthData(unsigned int aOneMarkMicros, unsigned int aOneSpaceMicros, unsigned int aZeroMarkMicros,
            unsigned int aZeroSpaceMicros, unsigned long aData, uint8_t aNumberOfBits, bool aMSBfirst = true);
    void mark(uint16_t timeMicros);
    void mark_long(uint32_t timeMicros);
    void space(uint16_t timeMicros);
    void space_long(uint32_t timeMicros);
    void sendRaw(const unsigned int buf[], unsigned int len, unsigned int hz);
    void sendRaw_P(const unsigned int buf[], unsigned int len, unsigned int hz);

    //......................................................................
#if SEND_RC5
    void sendRC5(uint32_t data, uint8_t nbits);
    void sendRC5ext(uint8_t addr, uint8_t cmd, boolean toggle);
#endif
#if SEND_RC6
    void sendRC6(uint32_t data, uint8_t nbits);
    void sendRC6(uint64_t data, uint8_t nbits);
#endif
    //......................................................................
#if SEND_NEC || SEND_NEC_STANDARD
    void sendNECRepeat();
#endif
#if SEND_NEC
    void sendNEC(uint32_t data, uint8_t nbits, bool repeat = false);
#endif
#if SEND_NEC_STANDARD
    void sendNECStandard(uint16_t aAddress, uint8_t aCommand, uint8_t aNumberOfRepeats = 0);
#endif
    //......................................................................
#if SEND_SONY
    void sendSony(unsigned long data, int nbits);
#endif
    //......................................................................
#if SEND_PANASONIC
    void sendPanasonic(unsigned int address, unsigned long data);
#endif
    //......................................................................
#if SEND_JVC
    // JVC does NOT repeat by sending a separate code (like NEC does).
    // The JVC protocol repeats by skipping the header.
    // To send a JVC repeat signal, send the original code value
    //   and set 'repeat' to true
    void sendJVC(unsigned long data, int nbits, bool repeat = false);
#endif
    //......................................................................
#if SEND_SAMSUNG
    void sendSAMSUNG(unsigned long data, int nbits);
#endif
    //......................................................................
#if SEND_WHYNTER
    void sendWhynter(unsigned long data, int nbits);
#endif
    //......................................................................
//#if SEND_AIWA_RC_T501
//    void sendAiwaRCT501(int code);
//#endif
    //......................................................................
#if SEND_LG
    void sendLG(unsigned long data, int nbits);
#endif
    //......................................................................
#if SEND_SANYO
      void  sendSanyo      ( ) ; // NOT WRITTEN
#endif
    //......................................................................
//#if SEND_MISUBISHI
//      void  sendMitsubishi ( ) ; // NOT WRITTEN
//#endif
    //......................................................................
#if SEND_DISH
    void sendDISH(unsigned long data, int nbits);
#endif
    //......................................................................
#if SEND_SHARP
    void sendSharpRaw(unsigned long data, int nbits);
    void sendSharp(unsigned int address, unsigned int command);
#endif
#if SEND_SHARP_ALT
    void sendSharpAltRaw(unsigned int data, int nbits);
    void sendSharpAlt(uint8_t address, uint8_t command);
#endif
    //......................................................................
#if SEND_DENON
    void sendDenon(unsigned long data, int nbits);
#endif
    //......................................................................
#if SEND_LEGO_PF
    void sendLegoPowerFunctions(uint16_t data, bool repeat = true);
#endif
    //......................................................................
#if SEND_BOSEWAVE
    void sendBoseWave(unsigned char code);
#endif
    //......................................................................
#if SEND_MAGIQUEST
    void sendMagiQuest(unsigned long wand_id, unsigned int magnitude);
#endif

    /**
     * Parse the string given as Pronto Hex, and send it a number of times given
     * as the second argument. Thereby the division of the Pronto Hex into
     * an intro-sequence and a repeat sequence is taken into account:
     * First the intro sequence is sent, then the repeat sequence is sent times-1 times.
     * However, if the intro sequence is empty, the repeat sequence is sent times times.
     * <a href="http://www.harctoolbox.org/Glossary.html#ProntoSemantics">Reference</a>.
     *
     * Note: Using this function is very wasteful for the memory consumption on
     * a small board.
     * Normally it is a much better ide to use a tool like e.g. IrScrutinizer
     * to transform Pronto type signals offline
     * to a more memory efficient format.
     *
     * @param prontoHexString C type string (null terminated) containing a Pronto Hex representation.
     * @param times Number of times to send the signal.
     */
    void sendPronto(const char* prontoHexString, unsigned int times = 1U);

    void sendPronto(const uint16_t* data, unsigned int length, unsigned int times = 1U);

#if HAS_FLASH_READ || defined(DOXYGEN)
    void sendPronto_PF(uint_farptr_t str, unsigned int times = 1U);

    /**
     * Version of sendPronto that reads from PROGMEM, saving RAM memory.
     * @param pronto C type string (null terminated) containing a Pronto Hex representation.
     * @param times Number of times to send the signal.
     */
    void sendPronto_PF(const char *str, unsigned int times = 1U);
    void sendPronto(const __FlashStringHelper *str, unsigned int times = 1U);
#endif

private:
#if (DECODE_RC5 || DECODE_RC6)
    /**
     *  This helper function is shared by RC5 and RC6
     */
    int getRClevel(decode_results *results, unsigned int *offset, int *used, int t1);
#endif

#if defined(USE_SOFT_SEND_PWM) || defined(USE_NO_SEND_PWM)
    int sendPin;

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
