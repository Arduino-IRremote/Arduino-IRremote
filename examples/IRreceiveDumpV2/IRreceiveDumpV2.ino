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
IRrecv IrReceiver(IR_RECEIVE_PIN);

//+=============================================================================
// Configure the Arduino
//
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);   // Status message will be sent to PC at 9600 baud
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    IrReceiver.enableIRIn();  // Start the receiver
    IrReceiver.blink13(true); // Enable feedback LED

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpInfo() {
    // Check if the buffer overflowed
    if (IrReceiver.results.overflow) {
        Serial.println("IR code too long. Edit IRremoteInt.h and increase RAW_BUFFER_LENGTH");
        return;
    }

    IrReceiver.printResultShort(&Serial);

    Serial.print(" (");
    Serial.print(IrReceiver.results.bits, DEC);
    Serial.println(" bits)");
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpRaw() {
    // Print Raw data
    Serial.print("Timing[");
    Serial.print(IrReceiver.results.rawlen - 1, DEC);
    Serial.println("]: ");

    for (unsigned int i = 1; i < IrReceiver.results.rawlen; i++) {
        unsigned long x = IrReceiver.results.rawbuf[i] * MICROS_PER_TICK;
        if (!(i & 1)) {  // even
            Serial.print("-");
            if (x < 1000) {
                Serial.print(" ");
            }
            if (x < 100) {
                Serial.print(" ");
            }
            Serial.print(x, DEC);
        } else {  // odd
            Serial.print("     ");
            Serial.print("+");
            if (x < 1000) {
                Serial.print(" ");
            }
            if (x < 100) {
                Serial.print(" ");
            }
            Serial.print(x, DEC);
            if (i < IrReceiver.results.rawlen - 1) {
                Serial.print(", "); //',' not needed for last one
            }
        }
        if (!(i % 8)) {
            Serial.println("");
        }
    }
    Serial.println("");                    // Newline
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpCode() {
    // Start declaration
    Serial.print("unsigned int  ");          // variable type
    Serial.print("rawData[");                // array name
    Serial.print(IrReceiver.results.rawlen - 1, DEC);  // array size
    Serial.print("] = {");                   // Start declaration

    // Dump data
    for (unsigned int i = 1; i < IrReceiver.results.rawlen; i++) {
        Serial.print(IrReceiver.results.rawbuf[i] * MICROS_PER_TICK, DEC);
        if (i < IrReceiver.results.rawlen - 1)
            Serial.print(","); // ',' not needed on last one
        if (!(i & 1))
            Serial.print(" ");
    }

    // End declaration
    Serial.print("};");  //

    // Comment
    Serial.print("  // ");
    IrReceiver.printResultShort(&Serial);

    // Newline
    Serial.println("");

    // Now dump "known" codes
    if (IrReceiver.results.decode_type != UNKNOWN) {

        // Some protocols have an address
        if(IrReceiver.results.address != 0){
            Serial.print("unsigned int  addr = 0x");
            Serial.print(IrReceiver.results.address, HEX);
            Serial.println(";");
        }

        // All protocols have data
        Serial.print("unsigned int  data = 0x");
        Serial.print(IrReceiver.results.value, HEX);
        Serial.println(";");
        Serial.println();
    }
}

//+=============================================================================
// Dump out the raw data as Pronto Hex.
//
void dumpPronto() {
    Serial.print("Pronto Hex: ");
    IrReceiver.dumpPronto(Serial);
    Serial.println();
}

//+=============================================================================
// The repeating section of the code
//
void loop() {
    if (IrReceiver.decode()) {  // Grab an IR code
        dumpInfo();           // Output the results
        dumpRaw();            // Output the results in RAW format
        dumpCode();           // Output the results as source code
        dumpPronto();
        Serial.println();           // 2 blank lines between entries
        Serial.println();
        IrReceiver.resume();              // Prepare for the next value
    }
}
