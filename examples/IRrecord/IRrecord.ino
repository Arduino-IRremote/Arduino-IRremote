/*
 * IRrecord: record and play back IR signals as a minimal
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected between the input SEND_BUTTON_PIN and ground.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Initially coded 2009 Ken Shirriff http://www.righto.com
 */

#include <IRremote.h>

#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
int SEND_BUTTON_PIN = 16; // RX2 pin
#else
int IR_RECEIVE_PIN = 11;
int SEND_BUTTON_PIN = 12;
#endif
int STATUS_PIN = LED_BUILTIN;

int DELAY_BETWEEN_REPEAT = 50;

IRrecv IrReceiver(IR_RECEIVE_PIN);
IRsend IrSender;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    IrReceiver.enableIRIn();  // Start the receiver
    IrReceiver.blink13(true); // Enable feedback LED

    pinMode(SEND_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STATUS_PIN, OUTPUT);

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);

#if defined(SENDING_SUPPORTED)
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.print(IR_SEND_PIN);
    Serial.print(F(" on press of button at pin "));
    Serial.println(SEND_BUTTON_PIN);

#else
    Serial.println(F("Sending not supported for this board!"));
#endif
}

// Storage for the recorded code
int codeType = -1; // The type of code
uint32_t codeValue; // The code value if not raw
uint16_t address; // The address value if not raw
unsigned int rawCodes[RAW_BUFFER_LENGTH]; // The durations if raw
uint8_t codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
// Most of this code is just logging
void storeCode() {
    if (IrReceiver.results.isRepeat) {
        Serial.println("Ignore repeat");
        return;
    }
    codeType = IrReceiver.results.decode_type;
    address = IrReceiver.results.address;

//  int count = IrReceiver.results.rawlen;
    if (codeType == UNKNOWN) {
        Serial.println("Received unknown code, saving as raw");
        codeLen = IrReceiver.results.rawlen - 1;
        // To store raw codes:
        // Drop first value (gap)
        // Convert from ticks to microseconds
        // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
        for (int i = 1; i <= codeLen; i++) {
            if (i % 2) {
                // Mark
                rawCodes[i - 1] = IrReceiver.results.rawbuf[i] * MICROS_PER_TICK - MARK_EXCESS_MICROS;
                Serial.print(" m");
            } else {
                // Space
                rawCodes[i - 1] = IrReceiver.results.rawbuf[i] * MICROS_PER_TICK + MARK_EXCESS_MICROS;
                Serial.print(" s");
            }
            Serial.print(rawCodes[i - 1], DEC);
        }
        Serial.println();
    } else {
        IrReceiver.printResultShort(&Serial);
        Serial.println();

        codeValue = IrReceiver.results.value;
        codeLen = IrReceiver.results.bits;
    }
}

void sendCode(bool aSendRepeat) {
    if (codeType == NEC) {
        if (aSendRepeat) {
            IrSender.sendNEC(REPEAT, codeLen);
            Serial.println("Sent NEC repeat");
        } else {
            IrSender.sendNEC(codeValue, codeLen);
            Serial.print("Sent NEC ");
            Serial.println(codeValue, HEX);
        }
    } else if (codeType == NEC_STANDARD) {
        if (aSendRepeat) {
            IrSender.sendNECRepeat();
            Serial.println("Sent NEC repeat");
        } else {
            IrSender.sendNECStandard(address, codeValue);
            Serial.print("Sent NEC_STANDARD address=0x");
            Serial.print(address, HEX);
            Serial.print(", command=0x");
            Serial.println(codeValue, HEX);
        }
    } else if (codeType == SONY) {
        IrSender.sendSony(codeValue, codeLen);
        Serial.print("Sent Sony ");
        Serial.println(codeValue, HEX);
    } else if (codeType == PANASONIC) {
        IrSender.sendPanasonic(codeValue, codeLen);
        Serial.print("Sent Panasonic");
        Serial.println(codeValue, HEX);
    } else if (codeType == JVC) {
        IrSender.sendJVC(codeValue, codeLen, false);
        Serial.print("Sent JVC");
        Serial.println(codeValue, HEX);
    } else if (codeType == RC5 || codeType == RC6) {
        if (!aSendRepeat) {
            // Flip the toggle bit for a new button press
            toggle = 1 - toggle;
        }
        // Put the toggle bit into the code to send
        codeValue = codeValue & ~(1 << (codeLen - 1));
        codeValue = codeValue | (toggle << (codeLen - 1));
        if (codeType == RC5) {
            Serial.print("Sent RC5 ");
            Serial.println(codeValue, HEX);
            IrSender.sendRC5(codeValue, codeLen);
        } else {
            IrSender.sendRC6(codeValue, codeLen);
            Serial.print("Sent RC6 ");
            Serial.println(codeValue, HEX);
        }
    } else if (codeType == UNKNOWN /* i.e. raw */) {
        // Assume 38 KHz
        IrSender.sendRaw(rawCodes, codeLen, 38);
        Serial.println("Sent raw");
    }
}

int lastButtonState;

void loop() {
    // If button pressed, send the code.
    int buttonState = digitalRead(SEND_BUTTON_PIN); // Button pin is active LOW
    if (lastButtonState == LOW && buttonState == HIGH) {
        Serial.println("Button released");
        IrReceiver.enableIRIn(); // Re-enable receiver
    }

    if (buttonState == LOW) {
        Serial.println("Button pressed, now sending");
        digitalWrite(STATUS_PIN, HIGH);
        sendCode(lastButtonState == buttonState);
        digitalWrite(STATUS_PIN, LOW);
        delay(DELAY_BETWEEN_REPEAT); // Wait a bit between retransmissions
    } else if (IrReceiver.decode()) {
        storeCode();
        IrReceiver.resume(); // resume receiver
    }
    lastButtonState = buttonState;
}
