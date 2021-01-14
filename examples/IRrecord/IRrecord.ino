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

#if !defined(USE_STANDARD_DECODE)
#warning "Enable USE_STANDARD_DECODE on line 34 of IRremote.h to enable this improved version of IRrecord example."
#endif

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

// Storage for the recorded code
IRData sStoredIRData;
uint16_t rawCodes[RAW_BUFFER_LENGTH]; // The durations if raw
uint8_t sSendRawCodeLength; // The length of the code

int lastButtonState;

void storeCode();
void sendCode(bool aSendRepeat);

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

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

// Stores the code for later playback
// Most of this code is just logging
void storeCode() {
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
        Serial.println("Ignore repeat");
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
        Serial.println("Ignore autorepeat");
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
        Serial.println("Ignore parity error");
        return;
    }
    /*
     * Copy decoded data
     */
    sStoredIRData = IrReceiver.decodedIRData;

    if (sStoredIRData.protocol == UNKNOWN) {
        Serial.println("Received unknown code, saving as raw");
        sSendRawCodeLength = IrReceiver.results.rawlen - 1;
        // To store raw codes:
        // Drop first value (gap)
        // Convert from ticks to microseconds
        // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
        for (uint16_t i = 1; i <= sSendRawCodeLength; i++) {
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
        sStoredIRData.flags = 0; // clear flags for later (not) printing
        Serial.println();
    }
}

void sendCode(bool aSendRepeat) {
    if (sStoredIRData.protocol == UNKNOWN /* i.e. raw */) {
        // Assume 38 KHz
        IrSender.sendRaw(rawCodes, sSendRawCodeLength, 38);
        Serial.println("Sent raw");
    } else {

        if (sStoredIRData.protocol == NEC) {
            IrSender.sendNECStandard(sStoredIRData.address, sStoredIRData.command, true, 0, aSendRepeat);

        } else if (sStoredIRData.protocol == SAMSUNG) {
            IrSender.sendSamsungStandard(sStoredIRData.address, sStoredIRData.command, 0, aSendRepeat);

        } else if (sStoredIRData.protocol == SONY) {
            IrSender.sendSonyStandard(sStoredIRData.address, sStoredIRData.command, sStoredIRData.numberOfBits, 0);

        } else if (sStoredIRData.protocol == PANASONIC) {
            IrSender.sendPanasonicStandard(sStoredIRData.address, sStoredIRData.command, 0);

        } else if (sStoredIRData.protocol == DENON) {
            IrSender.sendDenonStandard(sStoredIRData.address, sStoredIRData.command, false, 0);

        } else if (sStoredIRData.protocol == SHARP) {
            IrSender.sendSharpStandard(sStoredIRData.address, sStoredIRData.command, 0);

        } else if (sStoredIRData.protocol == SAMSUNG) {
            IrSender.sendSamsungStandard(sStoredIRData.address, sStoredIRData.command, 0);

        } else if (sStoredIRData.protocol == JVC) {
            IrSender.sendJVCStandard(sStoredIRData.address, sStoredIRData.command, 0);

        } else if (sStoredIRData.protocol == RC5 || sStoredIRData.protocol == RC6) {
            // No toggle for repeats
            if (sStoredIRData.protocol == RC5) {
                IrSender.sendRC5Standard(sStoredIRData.address, sStoredIRData.command, !aSendRepeat, 0);
            } else {
                IrSender.sendRC6Standard(sStoredIRData.address, sStoredIRData.command, !aSendRepeat, 0);
            }
        }

        Serial.print("Sent ");
        IrReceiver.printResultShort(&Serial, &sStoredIRData);
    }
}

