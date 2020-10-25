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
// The repeating section of the code
//
void loop() {
    if (IrReceiver.decode()) {  // Grab an IR code
        dumpInfo();             // Output the results
        IrReceiver.printIRResultRawFormatted(&Serial);  // Output the results in RAW format
        Serial.println();                               // blank line between entries
        IrReceiver.printIRResultAsCArray(&Serial);      // Output the results as source code array
        IrReceiver.printIRResultAsCVariables(&Serial);  // Output address and data as source code variables
        IrReceiver.printIRResultAsPronto(&Serial);
        Serial.println();                               // 2 blank lines between entries
        Serial.println();
        IrReceiver.resume();                            // Prepare for the next value
    }
}
