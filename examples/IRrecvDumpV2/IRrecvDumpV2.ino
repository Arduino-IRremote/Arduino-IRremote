//------------------------------------------------------------------------------
// Include the IRremote library header
//
#include <IRremote.h>

//------------------------------------------------------------------------------
// Tell IRremote which Arduino pin is connected to the IR Receiver (TSOP4838)
//
#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#else
int IR_RECEIVE_PIN = 11;
#endif
IRrecv irrecv(IR_RECEIVE_PIN);

//+=============================================================================
// Configure the Arduino
//
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);   // Status message will be sent to PC at 9600 baud
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    irrecv.enableIRIn();  // Start the receiver

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

//+=============================================================================
// Display IR code
//
void ircode(decode_results *results) {
    // Panasonic has an Address
    if (results->decode_type == PANASONIC) {
        Serial.print(results->address, HEX);
        Serial.print(":");
    }

    // Print Code
    Serial.print(results->value, HEX);
}

//+=============================================================================
// Display encoding type
//
void encoding(decode_results *results) {
    switch (results->decode_type) {
    default:
    case UNKNOWN:
        Serial.print("UNKNOWN");
        break;
    case NEC:
        Serial.print("NEC");
        break;
    case SONY:
        Serial.print("SONY");
        break;
    case RC5:
        Serial.print("RC5");
        break;
    case RC6:
        Serial.print("RC6");
        break;
    case DISH:
        Serial.print("DISH");
        break;
    case SHARP:
        Serial.print("SHARP");
        break;
    case SHARP_ALT:
        Serial.print("SHARP_ALT");
        break;
    case JVC:
        Serial.print("JVC");
        break;
    case SANYO:
        Serial.print("SANYO");
        break;
    case MITSUBISHI:
        Serial.print("MITSUBISHI");
        break;
    case SAMSUNG:
        Serial.print("SAMSUNG");
        break;
    case LG:
        Serial.print("LG");
        break;
    case WHYNTER:
        Serial.print("WHYNTER");
        break;
    case AIWA_RC_T501:
        Serial.print("AIWA_RC_T501");
        break;
    case PANASONIC:
        Serial.print("PANASONIC");
        break;
    case DENON:
        Serial.print("Denon");
        break;
    case BOSEWAVE:
        Serial.print("BOSEWAVE");
        break;
    }
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpInfo(decode_results *results) {
    // Check if the buffer overflowed
    if (results->overflow) {
        Serial.println("IR code too long. Edit IRremoteInt.h and increase RAW_BUFFER_LENGTH");
        return;
    }

    // Show Encoding standard
    Serial.print("Encoding  : ");
    encoding(results);
    Serial.println("");

    // Show Code & length
    Serial.print("Code      : 0x");
    ircode(results);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpRaw(decode_results *results) {
    // Print Raw data
    Serial.print("Timing[");
    Serial.print(results->rawlen - 1, DEC);
    Serial.println("]: ");

    for (unsigned int i = 1; i < results->rawlen; i++) {
        unsigned long x = results->rawbuf[i] * MICROS_PER_TICK;
        if (!(i & 1)) {  // even
            Serial.print("-");
            if (x < 1000)
                Serial.print(" ");
            if (x < 100)
                Serial.print(" ");
            Serial.print(x, DEC);
        } else {  // odd
            Serial.print("     ");
            Serial.print("+");
            if (x < 1000)
                Serial.print(" ");
            if (x < 100)
                Serial.print(" ");
            Serial.print(x, DEC);
            if (i < results->rawlen - 1)
                Serial.print(", "); //',' not needed for last one
        }
        if (!(i % 8))
            Serial.println("");
    }
    Serial.println("");                    // Newline
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpCode(decode_results *results) {
    // Start declaration
    Serial.print("unsigned int  ");          // variable type
    Serial.print("rawData[");                // array name
    Serial.print(results->rawlen - 1, DEC);  // array size
    Serial.print("] = {");                   // Start declaration

    // Dump data
    for (unsigned int i = 1; i < results->rawlen; i++) {
        Serial.print(results->rawbuf[i] * MICROS_PER_TICK, DEC);
        if (i < results->rawlen - 1)
            Serial.print(","); // ',' not needed on last one
        if (!(i & 1))
            Serial.print(" ");
    }

    // End declaration
    Serial.print("};");  //

    // Comment
    Serial.print("  // ");
    encoding(results);
    Serial.print(" ");
    ircode(results);

    // Newline
    Serial.println("");

    // Now dump "known" codes
    if (results->decode_type != UNKNOWN) {

        // Some protocols have an address
        if (results->decode_type == PANASONIC) {
            Serial.print("unsigned int  addr = 0x");
            Serial.print(results->address, HEX);
            Serial.println(";");
        }

        // All protocols have data
        Serial.print("unsigned int  data = 0x");
        Serial.print(results->value, HEX);
        Serial.println(";");
    }
}

//+=============================================================================
// Dump out the raw data as Pronto Hex.
//
void dumpPronto(decode_results *results) {
    Serial.print("Pronto Hex: ");
    irrecv.dumpPronto(Serial, results);
    Serial.println();
}

//+=============================================================================
// The repeating section of the code
//
void loop() {
    decode_results results;        // Somewhere to store the results

    if (irrecv.decode(&results)) {  // Grab an IR code
        dumpInfo(&results);           // Output the results
        dumpRaw(&results);            // Output the results in RAW format
        dumpPronto(&results);
        dumpCode(&results);           // Output the results as source code
        Serial.println("");           // Blank line between entries
        irrecv.resume();              // Prepare for the next value
    }
}
