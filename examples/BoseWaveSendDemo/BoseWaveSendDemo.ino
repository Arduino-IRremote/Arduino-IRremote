/*
 * Based on IRremote: IRsendDemo by Ken Shirriff
 *
 * Prompt user for a code to send.  Make sure your 940-950nm IR LED is
 * connected to the default digital output.  Place your Bose Wave Radio
 * CD in the line of sight of your LED, and send commands!
 */
#include <IRremote.h>

IRsend IrSender;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

bool prompt;
void menu();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    prompt = true;
}



void loop() {
    if (prompt) {
        prompt = false;
        menu();
    }

    if (Serial.available()) {
        int answer = Serial.read();
        prompt = true;
        if (answer == -1) {
            delay(300);
        } else if (answer == 48) {    // 0
            IrSender.sendBoseWaveStandard(BOSE_CMD_ON_OFF);  // On/Off
        } else if (answer == 49) {    // 1
            IrSender.sendBoseWaveStandard(BOSE_CMD_VOL_UP);  // Volume Up
        } else if (answer == 50) {    // 2
            IrSender.sendBoseWaveStandard(BOSE_CMD_VOL_DOWN);  // Volume Down
        } else if (answer == 51) {    // 3
            IrSender.sendBoseWaveStandard(BOSE_CMD_TUNE_UP);  // Tune Up
        } else if (answer == 52) {    // 4
            IrSender.sendBoseWaveStandard(BOSE_CMD_TUNE_DOWN);  // Tune Down
        } else if (answer == 53) {    // 5
            IrSender.sendBoseWaveStandard(BOSE_CMD_AM);  // AM
        } else if (answer == 54) {    // 6
            IrSender.sendBoseWaveStandard(BOSE_CMD_FM);  // FM
        } else if (answer == 55) {    // 7
            IrSender.sendBoseWaveStandard(BOSE_CMD_PRESET_1);  // Preset 1
        } else if (answer == 56) {    // 8
            IrSender.sendBoseWaveStandard(BOSE_CMD_PRESET_2);  // Preset 2
        } else if (answer == 57) {    // 9
            IrSender.sendBoseWaveStandard(BOSE_CMD_PRESET_3);  // Preset 3
        } else if (answer == 97) {    // a
            IrSender.sendBoseWaveStandard(BOSE_CMD_PRESET_4);  // Preset 4
        } else if (answer == 98) {    // b
            IrSender.sendBoseWaveStandard(BOSE_CMD_PRESET_5);  // Preset 5
        } else if (answer == 99) {    // c
            IrSender.sendBoseWaveStandard(BOSE_CMD_PRESET_6);  // Preset 6
        } else if (answer == 100) {   // d
            IrSender.sendBoseWaveStandard(BOSE_CMD_MUTE);  // Mute
        } else if (answer == 101) {   // e
            IrSender.sendBoseWaveStandard(BOSE_CMD_PLAY_PAUSE);  // Pause
        } else if (answer == 102) {   // f
            IrSender.sendBoseWaveStandard(BOSE_CMD_STOP);  // Stop
        } else if (answer == 103) {   // g
            IrSender.sendBoseWaveStandard(BOSE_CMD_AUX);  // Aux
        } else if (answer == 104) {   // h
            IrSender.sendBoseWaveStandard(BOSE_CMD_SLEEP);  // Sleep
        } else {
            prompt = false;
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
