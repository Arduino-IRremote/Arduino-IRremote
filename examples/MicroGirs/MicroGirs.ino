/*
This is a minimalistic Girs server (http://harctoolbox.org/Girs.html).
It only depends on (a subset of) IRremote
(https://github.com/z3t0/Arduino-IRremote), It can be used with
IrScrutinizer (Sending/Capturing hw = Girs Client) and Lirc 0.9.4 and
later (driver girs http://lirc.org/html/girs.html).  (Authors of
similar software are encourage to implement support.)

It consists of an interacive IR server, taking one-line commands from
the "user" (which is typically not a person but another program), and
responds with a one-line response. In the language of the Girs
specifications, the modules "base", receive, and transmit are
implemented. (The two latter can be disabled by not defining two
corresponding CPP symbols.)

It understands the following commands:

* The "version" command returns the program name and version, 
* The "modules" command returns the modules implemented, normally base, receive and transmit.
* The "receve" command reads an IR signal using the deployed sensor.
* The "send" commands transmits a supplied raw signal the requested number of times.
        
Only the first character of the command is evaluated in this implementation.
        
The "receive" command returns the received IR sequence as a sequence
of durations, including a (dummy) trailing silence.  On-periods
("marks", "flashes") are prefixed by "+", while off-periods ("spaces",
"gaps") are prefixed by "-". The present version never times out.
        
The "send" command takes the following parameters:
                
      send noSends frequencyHz introLength repeatLength endingLength durations...
                
      where 

* frequencyHz denotes the modulation frequency in Hz
  (NOT khz, as is commonly used in IRremote)
* introLength denotes the length of the intro sequence, must be even,
* repeatLength denotes the length of the repeat sequence, must be even,
* endingLength denotes the length of the ending sequence (normally 0), must be even.
* duration... denotes the microsecond durations to send, 
  starting with the first on-period, ending with a (possibly dummy) trailing silence
                
Semantics: first the intro sequence will be sent once (i.e., the first
repeatLength durations) (if non-empty).  Then the repeat sequence will
be sent (noSends-1) times, unless the intro sequence was empty, in
which case it will be send noSends times.  Finally, the ending
sequence will be send once (if non-empty).
                        
Weaknesses of the IRremote implementation:
* Reception never times out.
* The IRrecv class does its own decoding which is irrelevant for us. It cannot be disabled.
* The timeout at the end on a signal reception is not configurable.
  For example, a NEC1 type signal will cut after the intro sequence,
  and the repeats will be considered independent signals.
* IR reception cannot be turned of. 
* The size of the data is platform dependent (unsigned int).
*/

#include <IRremote.h>
#include <limits.h>

// Define the ones that should be enabled
#define RECEIVE
#define TRANSMIT

// Change the following if desired

// Pin used by the receiver
#define INPUTPIN 11

// (The sending pin is in general not configurable, see the documentation of IRremote.)

// Character that ends the command lines. Do not change unless you known what
// you are doing. IrScrutinizer and Lirc expects \r.
#define EOLCHAR '\r'

// Baud rate for the serial/USB connection.
// 115200 is the default for IrScrutinizer and Lirc.
#define BAUDRATE 115200

////// End of user configurable variables ////////////////////

#define modulesSupported "base transmit receive"
#ifndef PROGNAME
#define PROGNAME "MicroGirs"
#endif
#ifndef VERSION
#define VERSION "2017-04-23"
#endif
#define okString "OK"
#define errorString "ERROR"
#define timeoutString "."

// For compatibility with IRremote, we deliberately use
// the platform dependent types.
typedef unsigned frequency_t;
typedef unsigned microseconds_t;

static const microseconds_t DUMMYENDING = 40000U;
static const frequency_t FREQUENCY_T_MAX = UINT16_MAX;
static const frequency_t MICROSECONDS_T_MAX = UINT16_MAX;

#ifdef RECEIVE
IRrecv irRecv(INPUTPIN);
#endif
#ifdef TRANSMIT
IRsend irSend;
#endif

///////// Tokenizer class. Breaks the command line into tokens. //////
class Tokenizer {
private:
    static const int invalidIndex = -1;

    int index; // signed since invalidIndex is possible
    const String& payload;
    void trim();

public:
    Tokenizer(const String &str);
    virtual ~Tokenizer();

    String getToken();
    String getRest();
    String getLine();
    long getInt();
    microseconds_t getMicroseconds();
    frequency_t getFrequency();

    static const int invalid = INT_MAX;
};

Tokenizer::Tokenizer(const String& str) : index(0), payload(str) {
}

Tokenizer::~Tokenizer() {
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
    index = (i > 0) ? i+1 : invalidIndex;
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
    return (hz + 500)/1000;
}

static void sendRaw(const microseconds_t intro[], unsigned lengthIntro,
        const microseconds_t repeat[], unsigned lengthRepeat,
        const microseconds_t ending[], unsigned lengthEnding,
        frequency_t frequency, unsigned times) {
    if (lengthIntro > 0U)
        irSend.sendRaw(intro, lengthIntro, hz2khz(frequency));
    if (lengthRepeat > 0U)
        for (unsigned i = 0U; i < times - (lengthIntro > 0U); i++)
            irSend.sendRaw(repeat, lengthRepeat, hz2khz(frequency));
    if (lengthEnding > 0U)
        irSend.sendRaw(ending, lengthEnding, hz2khz(frequency));
}
#endif // TRANSMIT

#ifdef RECEIVE
static void receive(Stream& stream) {
    irRecv.resume(); // Receive the next value
    
    decode_results results;
    while (!irRecv.decode(&results)) {
    }

    dump(stream, &results);
}

static void dump(Stream& stream, decode_results* results) {
    unsigned int count = results->rawlen;
    for (unsigned int i = 1; i < count; i++) {
        stream.write(i & 1 ? '+' : '-');
        stream.print(results->rawbuf[i] * USECPERTICK, DEC);
        stream.print(" ");
    }
    stream.print('-');
    stream.println(DUMMYENDING);
}
#endif // RECEIVE

void setup() {
    Serial.begin(BAUDRATE);
    while (!Serial)
        ; // wait for serial port to connect. "Needed for Leonardo only"
    
    Serial.println(F(PROGNAME " " VERSION));
    //Serial.setTimeout(SERIALTIMEOUT);
#ifdef RECEIVE
    // There is unfortunately no disableIRIn in IRremote.
    // Therefore, turn it on, and leave it on.
    // We _hope_ that it will not interfere with sending.
    irRecv.enableIRIn();
#endif
}


static String readCommand(Stream& stream) {
    while (stream.available() == 0) {
    }
    
    String line = stream.readStringUntil(EOLCHAR);
    line.trim();
    return line;
}

static void processCommand(const String& line, Stream& stream) {
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
        case 'a':
        case 'c':
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

void loop() {
    String line = readCommand(Serial);
    processCommand(line, Serial);
}
