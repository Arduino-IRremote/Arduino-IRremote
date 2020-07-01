/*
 * Based on IRremote: IRsendDemo by Ken Shirriff
 *
 * Prompt user for a code to send.  Make sure your 940-950nm IR LED is
 * connected to the default digital output.  Place your Bose Wave Radio
 * CD in the line of sight of your LED, and send commands!
 */
#include <IRremote.h>

IRsend irsend;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

bool prompt;
void menu();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial)
        ; //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    prompt = true;
}

void loop() {
    if (prompt) {
        menu();
    }
    prompt = false;

    if (Serial.available()) {
        int answer = Serial.read();
        if (answer == -1) {
            delay(300);
        } else if (answer == 48) {    // 0
            irsend.sendBoseWave(0xFF);  // On/Off
            prompt = true;
        } else if (answer == 49) {    // 1
            irsend.sendBoseWave(0xFD);  // Volume Up
            prompt = true;
        } else if (answer == 50) {    // 2
            irsend.sendBoseWave(0xFC);  // Volume Down
            prompt = true;
        } else if (answer == 51) {    // 3
            irsend.sendBoseWave(0xF4);  // Tune Up
            prompt = true;
        } else if (answer == 52) {    // 4
            irsend.sendBoseWave(0xF3);  // Tune Down
            prompt = true;
        } else if (answer == 53) {    // 5
            irsend.sendBoseWave(0xF7);  // AM
            prompt = true;
        } else if (answer == 54) {    // 6
            irsend.sendBoseWave(0xF9);  // FM
            prompt = true;
        } else if (answer == 55) {    // 7
            irsend.sendBoseWave(0xF2);  // Preset 1
            prompt = true;
        } else if (answer == 56) {    // 8
            irsend.sendBoseWave(0xF1);  // Preset 2
            prompt = true;
        } else if (answer == 57) {    // 9
            irsend.sendBoseWave(0xF0);  // Preset 3
            prompt = true;
        } else if (answer == 97) {    // a
            irsend.sendBoseWave(0xEF);  // Preset 4
            prompt = true;
        } else if (answer == 98) {    // b
            irsend.sendBoseWave(0xEE);  // Preset 5
            prompt = true;
        } else if (answer == 99) {    // c
            irsend.sendBoseWave(0xFB);  // Preset 6
            prompt = true;
        } else if (answer == 100) {   // d
            irsend.sendBoseWave(0xFE);  // Mute
            prompt = true;
        } else if (answer == 101) {   // e
            irsend.sendBoseWave(0xF6);  // Pause
            prompt = true;
        } else if (answer == 102) {   // f
            irsend.sendBoseWave(0xF5);  // Stop
            prompt = true;
        } else if (answer == 103) {   // g
            irsend.sendBoseWave(0xF8);  // Aux
            prompt = true;
        } else if (answer == 104) {   // h
            irsend.sendBoseWave(0xFA);  // Sleep
            prompt = true;
        }
        delay(300);
    }
}

void menu() {
    Serial.println("0:  On / Off");
    Serial.println("1:  Volume Up");
    Serial.println("2:  Volume Down");
    Serial.println("3:  Tune Up");
    Serial.println("4:  Tune Down");
    Serial.println("5:  AM");
    Serial.println("6:  FM");
    Serial.println("7:  Preset 1");
    Serial.println("8:  Preset 2");
    Serial.println("9:  Preset 3");
    Serial.println("a:  Preset 4");
    Serial.println("b:  Preset 5");
    Serial.println("c:  Preset 6");
    Serial.println("d:  Mute");
    Serial.println("e:  Play/Pause");
    Serial.println("f:  Stop");
    Serial.println("g:  Aux");
    Serial.println("h:  Sleep");
}
