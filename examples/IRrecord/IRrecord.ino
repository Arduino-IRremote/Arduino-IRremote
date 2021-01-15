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
uint8_t rawCodes[RAW_BUFFER_LENGTH]; // The durations if raw
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
        Serial.println(F("Button released"));
        IrReceiver.enableIRIn(); // Re-enable receiver
    }

    if (buttonState == LOW) {
        Serial.println(F("Button pressed, now sending"));
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
        Serial.println(F("Ignore repeat"));
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
        Serial.println(F("Ignore autorepeat"));
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
        Serial.println(F("Ignore parity error"));
        return;
    }
    /*
     * Copy decoded data
     */
    sStoredIRData = IrReceiver.decodedIRData;

    if (sStoredIRData.protocol == UNKNOWN) {
        Serial.print(F("Received unknown code saving "));
        Serial.print(IrReceiver.results.rawlen - 1);
        Serial.println(F(" TickCounts as raw "));
        sSendRawCodeLength = IrReceiver.results.rawlen - 1;
        IrReceiver.compensateAndStoreIRResultInArray(rawCodes);
    } else {
        IrReceiver.printIRResultShort(&Serial);
        sStoredIRData.flags = 0; // clear flags for later (not) printing
        Serial.println();
    }
}

void sendCode(bool aSendRepeat) {
    if (sStoredIRData.protocol == UNKNOWN /* i.e. raw */) {
        // Assume 38 KHz
        IrSender.sendRaw(rawCodes, sSendRawCodeLength, 38);
        Serial.print(F("Sent raw "));
        Serial.print(sSendRawCodeLength);
        Serial.println(F(" marks or spaces"));
    } else {

        if (sStoredIRData.protocol == NEC) {
            IrSender.sendNEC(sStoredIRData.address, sStoredIRData.command, NO_REPEATS, aSendRepeat);

        } else if (sStoredIRData.protocol == SAMSUNG) {
            IrSender.sendSamsung(sStoredIRData.address, sStoredIRData.command, NO_REPEATS, aSendRepeat);

        } else if (sStoredIRData.protocol == SONY) {
            IrSender.sendSony(sStoredIRData.address, sStoredIRData.command, NO_REPEATS, sStoredIRData.numberOfBits);

        } else if (sStoredIRData.protocol == PANASONIC) {
            IrSender.sendPanasonic(sStoredIRData.address, sStoredIRData.command, NO_REPEATS);

        } else if (sStoredIRData.protocol == DENON) {
            IrSender.sendDenon(sStoredIRData.address, sStoredIRData.command, NO_REPEATS, false);

        } else if (sStoredIRData.protocol == SHARP) {
            IrSender.sendSharp(sStoredIRData.address, sStoredIRData.command, NO_REPEATS);

        } else if (sStoredIRData.protocol == SHARP) {
            IrSender.sendSharp(sStoredIRData.address, sStoredIRData.command, NO_REPEATS);

        } else if (sStoredIRData.protocol == JVC) {
            // casts are required to specify the right function
            IrSender.sendJVC((uint8_t)sStoredIRData.address, (uint8_t)sStoredIRData.command, NO_REPEATS);

        } else if (sStoredIRData.protocol == RC5 || sStoredIRData.protocol == RC6) {
            // No toggle for repeats
            if (sStoredIRData.protocol == RC5) {
                IrSender.sendRC5(sStoredIRData.address, sStoredIRData.command, NO_REPEATS, !aSendRepeat);
            } else {
                IrSender.sendRC6(sStoredIRData.address, sStoredIRData.command, NO_REPEATS, !aSendRepeat);
            }
        }

        Serial.print(F("Sent: "));
        IrReceiver.printIRResultShort(&Serial, &sStoredIRData);
    }
}

