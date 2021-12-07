/**
 * @file MicroGirs.ino
 *
 * @brief This is a minimalistic <a href="http://harctoolbox.org/Girs.html">Girs server</a>.
 * It only depends on (a subset of) IRremote. It can be used with
 * <a href="http://www.harctoolbox.org/IrScrutinizer.html">IrScrutinizer</a>
 * (select Sending/Capturing hw = Girs Client) as well as
 * <a href="http://lirc.org">Lirc</a>
 * version  0.9.4 and later, using the driver <a href="http://lirc.org/html/girs.html">Girs</a>).
 * (Authors of similar software are encourage to implement support.)
 *
 * It runs on all hardware on which IRemote runs.
 *
 * It consists of an interactive IR server, taking one-line commands from
 * the "user" (which is typically not a person but another program), and
 * responds with a one-line response. In the language of the Girs
 * specifications, the modules "base", receive, and transmit are
 * implemented. (The two latter can be disabled by not defining two
 * corresponding CPP symbols.)
 *
 * It understands the following commands:
 *
 * The "version" command returns the program name and version,
 * The "modules" command returns the modules implemented, normally base, receive and transmit.
 * The "receive" command reads an IR signal using the used, demodulating IR sensor.
 * The "send" commands transmits a supplied raw signal the requested number of times.
 *
 * Only the first character of the command is evaluated in this implementation.
 *
 * The "receive" command returns the received IR sequence as a sequence
 * of durations, including a (dummy) trailing silence.  On-periods
 * ("marks", "flashes") are prefixed by "+", while off-periods ("spaces",
 * "gaps") are prefixed by "-". The present version never times out.
 *
 * The \c send command takes the following parameters:
 *
 *       send noSends frequencyHz introLength repeatLength endingLength durations...

 * where
 *
 * * frequencyHz denotes the modulation frequency in Hz
 * (\e not khz, as is normally used in IRremote)
 * * introLength denotes the length of the intro sequence, must be even,
 * * repeatLength denotes the length of the repeat sequence, must be even,
 * * endingLength denotes the length of the ending sequence (normally 0), must be even.
 * * duration... denotes the microsecond durations to send,
 *   starting with the first on-period, ending with a (possibly dummy) trailing silence
 *
 * Semantics: first the intro sequence will be sent once (i.e., the first
 * repeatLength durations) (if non-empty).  Then the repeat sequence will
 * be sent (noSends-1) times, unless the intro sequence was empty, in
 * which case it will be send noSends times.  Finally, the ending
 * sequence will be send once (if non-empty).
 *
 * Weaknesses of the IRremote implementation:
 * * Reception never times out if no IR is seen.
 * * The IRrecv class does its own decoding which is irrelevant for us.
 * * The timeout at the end on a signal reception is not configurable.
 *   For example, a NEC1 type signal will cut after the intro sequence,
 *   and the repeats will be considered independent signals.
 *  In IrSqrutinizer, recognition of repeating signals will therefore not work.
 * The size of the data is platform dependent ("unsigned int", which is 16 bit on AVR boards, 32 bits on 32 bit boards).
 *
 */
#include <Arduino.h>

//#define RAW_BUFFER_LENGTH  750  // 750 is the value for air condition remotes.

/*
 * Define macros for input and output pin etc.
 */
#include "PinDefinitionsAndMore.h"

// Change the following two entries if desired

/**
 * Baud rate for the serial/USB connection.
 * (115200 is the default for IrScrutinizer and Lirc.)
 */
#define BAUDRATE 115200

#define NO_DECODER
#include "IRremote.hpp"
#include <limits.h>

/**
 * Define to support reception of IR.
 */
#define RECEIVE

/**
 * Define to support transmission of IR signals.
 */
#define TRANSMIT

// (The sending pin is in general not configurable, see the documentation of IRremote.)

/**
 *  Character that ends the command lines. Do not change unless you known what
 *  you are doing. IrScrutinizer and Lirc expects \r.
 */
#define EOLCHAR '\r'

////// End of user configurable variables ////////////////////

/**
 * The modules supported, as given by the "modules" command.
 */
#ifdef TRANSMIT
#ifdef RECEIVE
#define modulesSupported "base transmit receive"
#else // ! RECEIVE
#define modulesSupported "base transmit"
#endif
#else // !TRANSMIT
#ifdef RECEIVE
#define modulesSupported "base receive"
#else // ! RECETVE
#error At lease one of TRANSMIT and RECEIVE must be defined
#endif
#endif

/**
 * Name of program, as reported by the "version" command.
 */
#define PROGNAME "MicroGirs"

/**
 * Version of program, as reported by the "version" command.
 */
#define VERSION "2020-07-05"

#define okString "OK"
#define errorString "ERROR"
#define timeoutString "."

// For compatibility with IRremote, we deliberately use
// the platform dependent types.
// (Although it is a questionable idea ;-) )
/**
 * Type used for modulation frequency in Hz (\e not kHz).
 */
typedef unsigned frequency_t; // max 65535, unless 32-bit

/**
 * Type used for durations in micro seconds.
 */
typedef uint16_t microseconds_t; // max 65535

static const microseconds_t DUMMYENDING = 40000U;
static const frequency_t FREQUENCY_T_MAX = __UINT16_MAX__;
static const frequency_t MICROSECONDS_T_MAX = __UINT16_MAX__;

/**
 * Our own tokenizer class. Breaks the command line into tokens.
 * Usage outside of this package is discouraged.
 */
class Tokenizer {
private:
    static const int invalidIndex = -1;

    int index; // signed since invalidIndex is possible
    const String &payload;
    void trim();

public:
    Tokenizer(const String &str);

    String getToken();
    String getRest();
    String getLine();
    long getInt();
    microseconds_t getMicroseconds();
    frequency_t getFrequency();

    static const int invalid = INT_MAX;
};

Tokenizer::Tokenizer(const String &str) :
        index(0), payload(str) {
}

String Tokenizer::getRest() {
    String result = index == invalidIndex ? String("") : payload.substring(index);
    index = invalidIndex;
    return result;
}

String Tokenizer::getLine() {
    if (index == invalidIndex)
        return String("");

    int i = payload.indexOf('\n', index);
    String s = (i > 0) ? payload.substring(index, i) : payload.substring(index);
    index = (i > 0) ? i + 1 : invalidIndex;
    return s;
}

String Tokenizer::getToken() {
    if (index < 0)
        return String("");

    int i = payload.indexOf(' ', index);
    String s = (i > 0) ? payload.substring(index, i) : payload.substring(index);
    index = (i > 0) ? i : invalidIndex;
    if (index != invalidIndex)
        if (index != invalidIndex)
            while (payload.charAt(index) == ' ')
                index++;
    return s;
}

long Tokenizer::getInt() {
    String token = getToken();
    return token == "" ? (long) invalid : token.toInt();
}

microseconds_t Tokenizer::getMicroseconds() {
    long t = getToken().toInt();
    return (microseconds_t) ((t < MICROSECONDS_T_MAX) ? t : MICROSECONDS_T_MAX);
}

frequency_t Tokenizer::getFrequency() {
    long t = getToken().toInt();
    return (frequency_t) ((t < FREQUENCY_T_MAX) ? t : FREQUENCY_T_MAX);
}
///////////////// end Tokenizer /////////////////////////////////

#ifdef TRANSMIT
static inline unsigned hz2khz(frequency_t hz) {
    return (hz + 500) / 1000;
}

/**
 * Transmits the IR signal given as argument.
 *
 * The intro sequence (possibly empty) is first sent. Then the repeat signal
 * (also possibly empty) is sent, "times" times, except for the case of
 * the intro signal being empty, in which case it is sent "times" times.
 * Finally the ending sequence (possibly empty) is sent.
 *
 * @param intro Sequence to be sent exactly once at the start.
 * @param lengthIntro Number of durations in intro sequence, must be even.
 * @param repeat Sequence top possibly be sent multiple times
 * @param lengthRepeat Number of durations in repeat sequence.
 * @param ending Sequence to be sent at the end, possibly empty
 * @param lengthEnding Number of durations in ending sequence
 * @param frequency Modulation frequency, in Hz (not kHz as normally in IRremote)
 * @param times Number of times to send the signal, in the sense above.
 */
static void sendRaw(const microseconds_t intro[], unsigned lengthIntro, const microseconds_t repeat[], unsigned lengthRepeat,
        const microseconds_t ending[], unsigned lengthEnding, frequency_t frequency, unsigned times) {
    if (lengthIntro > 0U)
        IrSender.sendRaw(intro, lengthIntro, hz2khz(frequency));
    if (lengthRepeat > 0U)
        for (unsigned i = 0U; i < times - (lengthIntro > 0U); i++)
            IrSender.sendRaw(repeat, lengthRepeat, hz2khz(frequency));
    if (lengthEnding > 0U)
        IrSender.sendRaw(ending, lengthEnding, hz2khz(frequency));
}
#endif // TRANSMIT

#ifdef RECEIVE

static void dump(Stream &stream) {
    unsigned int count = IrReceiver.decodedIRData.rawDataPtr->rawlen;
    // If buffer gets full, count = RAW_BUFFER_LENGTH, which is odd,
    // and IrScrutinizer does not like that.
    count &= ~1;
    for (unsigned int i = 1; i < count; i++) {
        stream.write(i & 1 ? '+' : '-');
        stream.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK, DEC);
        stream.print(" ");
    }
    stream.print('-');
    stream.println(DUMMYENDING);
}

/**
 * Reads a command from the stream given as argument.
 * @param stream Stream to read from, typically Serial.
 */
static void receive(Stream &stream) {
    IrReceiver.enableIRIn();
    IrReceiver.resume(); // Receive the next value

    while (!IrReceiver.decode()) {
    }
    IrReceiver.disableIRIn();

    dump(stream);
}

#endif // RECEIVE

/**
 * Initialization.
 */
void setup() {
    Serial.begin(BAUDRATE);
    while (!Serial)
        ; // wait for serial port to connect.

    Serial.println(F(PROGNAME " " VERSION));
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

#ifdef RECEIVE
    /*
     * Start the receiver, enable feedback LED and (if not 3. parameter specified) take LED feedback pin from the internal boards definition
     */
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.print(F("at pin "));
#endif

    IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin

}

static String readCommand(Stream &stream) {
    while (stream.available() == 0) {
    }

    String line = stream.readStringUntil(EOLCHAR);
    line.trim();
    return line;
}

static void processCommand(const String &line, Stream &stream) {
    Tokenizer tokenizer(line);
    String cmd = tokenizer.getToken();

    // Decode the command in cmd
    if (cmd.length() == 0) {
        // empty command, do nothing
        stream.println(F(okString));
        return;
    }

    switch (cmd[0]) {
    case 'm':
        stream.println(F(modulesSupported));
        break;

#ifdef RECEIVE
    case 'r': // receive
        //case 'a':
        //case 'c':
        receive(stream);
        break;
#endif // RECEIVE

#ifdef TRANSMIT
    case 's': // send
    {
        // TODO: handle unparsable data gracefully
        unsigned noSends = (unsigned) tokenizer.getInt();
        frequency_t frequency = tokenizer.getFrequency();
        unsigned introLength = (unsigned) tokenizer.getInt();
        unsigned repeatLength = (unsigned) tokenizer.getInt();
        unsigned endingLength = (unsigned) tokenizer.getInt();
        microseconds_t intro[introLength];
        microseconds_t repeat[repeatLength];
        microseconds_t ending[endingLength];
        for (unsigned i = 0; i < introLength; i++)
            intro[i] = tokenizer.getMicroseconds();
        for (unsigned i = 0; i < repeatLength; i++)
            repeat[i] = tokenizer.getMicroseconds();
        for (unsigned i = 0; i < endingLength; i++)
            ending[i] = tokenizer.getMicroseconds();
        sendRaw(intro, introLength, repeat, repeatLength, ending, endingLength, frequency, noSends);
        stream.println(F(okString));
    }
        break;
#endif // TRANSMIT

    case 'v': // version
        stream.println(F(PROGNAME " " VERSION));
        break;
    default:
        stream.println(F(errorString));
    }
}

/**
 * Reads a command from the serial line and executes it-
 */
void loop() {
    String line = readCommand(Serial);
    processCommand(line, Serial);
}
