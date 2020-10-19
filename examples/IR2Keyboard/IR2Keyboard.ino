/*
 this is just an example of creative (maybe useful) use of IR in a PC, as a media center or some other automatizations.
 the idea is fordward the IR commands (from any spare remote you may have) to keyboard pulses, combos, shortcuts, etc.
 Keyboard funtions only available on "leonardo" and "micro". for more keys read into  https://www.arduino.cc/en/Reference/KeyboardModifiers
 for IR decoding im using the amazing lib from:  http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html
 Remote class emulate media keys. need to be added manually to the arduino lib.  http://stefanjones.ca/blog/arduino-leonardo-remote-multimedia-keys/
 this is optional, you dont need Remote class to run the IR or Keyboard; its only for media keys
 */

/*
 Remote methods:
 http://stefanjones.ca/blog/arduino-leonardo-remote-multimedia-keys/
 Volume
 Remote.increase(void);
 Remote.decrease(void);
 Remote.mute(void);
 Playback
 Remote.play(void);
 Remote.pause(void);
 Remote.stop(void);
 Track Controls
 Remote.next(void);
 Remote.previous(void);
 Remote.forward(void);
 Remote.rewind(void);
 Send an empty report to prevent repeated actions
 Keyboard.releaseAll();
 */

#include <IRremote.h>
#include <Keyboard.h>
#define T 15

bool rotation = false;

#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#elif defined(ARDUINO_AVR_PROMICRO)
int IR_RECEIVE_PIN = 10;
#else
int IR_RECEIVE_PIN = 11;
#endif
IRrecv IrReceiver(IR_RECEIVE_PIN);

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void PanasonicRemote();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    Keyboard.begin();
    // In case the interrupt driver crashes on setup, give a clue
    // to the user what's going on.
    Serial.println("Enabling IRin");
    IrReceiver.enableIRIn();  // Start the receiver
    IrReceiver.blink13(true); // Enable feedback LED

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

void loop() {
    if (IrReceiver.decode()) {
        //DellRemote(results.value);
        PanasonicRemote();
        //mapping();
        delay(300);
        IrReceiver.resume(); // Receive the next value
    }
    delay(100);
}

void mapping() {
    uint32_t tCode = IrReceiver.results.value;
    //use this function to map the codes of all your buttons. some buttons send different codes if they are hold down
    if ((tCode == 0x801C2B2E) or (tCode == 0xB4AFB411) or (tCode == 0xD17D8037)) {
        Serial.println("forward button");
    } else if ((tCode == 0xB7AFB8C8) or (tCode == 0xD07D7EA2) or (tCode == 0x801CAB2F)) {
        Serial.println("previous button");
    } else {
        Serial.print(IrReceiver.results.value, HEX);
        Serial.println("unknown code!");
    }
}

void PanasonicRemote() {
    uint32_t code = IrReceiver.results.value;

    //here i have mapped some buttons of my blue ray remote.
    if (code == 0xD00A0AD) {/*forward*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_RIGHT_ARROW);
        delay(T);
        Keyboard.releaseAll(); /*Remote.forward();   Keyboard.releaseAll(); */
    } //Ctrl + ->    forward 10 seconds VLC
    else if (code == 0xD00202D) {/*atras*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_ARROW);
        delay(T);
        Keyboard.releaseAll(); /*Remote.rewind();   Keyboard.releaseAll(); */
    } //Ctrl + <-    rewind 10 seconds VLC
    else if (code == 0xD00525F) {/*next*/
        Keyboard.releaseAll();
        delay(T);
//        Remote.next();
        Keyboard.releaseAll();
    } else if (code == 0xD00929F) {/*prev*/
        Keyboard.releaseAll();
        delay(T);
//        Remote.previous();
        Keyboard.releaseAll();
    }
    //else if (code == 0xD00000D){/*stop*/   Keyboard.releaseAll();delay(T);  Remote.stop();   Keyboard.releaseAll(); }
    else if (code == 0x1000405) {/*vol +*/
        Keyboard.releaseAll();
        delay(T);
//        Remote.increase();
        Keyboard.releaseAll();
    } else if (code == 0x1008485) {/*vol -*/
        Keyboard.releaseAll();
        delay(T);
//        Remote.decrease();
        Keyboard.releaseAll();
    } else if (code == 0xD00BCB1) {/*POWER*/
        Keyboard.releaseAll();
        delay(T);
//        Remote.mute();
        Keyboard.releaseAll();
    } else if ((code == 0xD00606D) or (code == 0xD00505D)) {/*play/pause*/
        Keyboard.releaseAll();
        delay(T);
//        Remote.play();
        Keyboard.releaseAll();
    } else if (code == 0xD00818C) {/*return*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.write(KEY_ESC);
    } else if (code == 0xD00414C) {/*OK*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.print(" "); /*evitar doble pulsaciondelay(300);*/
    } else if (code == 0x100BCBD) {/*tvPower*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_UP_ARROW);
        delay(T);
        Keyboard.releaseAll();
    } //volumen en VLC
    else if (code == 0x100A0A1) {/*input AV*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_DOWN_ARROW);
        delay(T);
        Keyboard.releaseAll();
    } //volumen en VLC
    else if (code == 0xD00808D) {/*open/close*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_PAGE_UP);
        delay(T);
        Keyboard.releaseAll();
    } //cambiar canal HEXCHAT
    else if (code == 0xD004944) {/*display*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_PAGE_DOWN);
        delay(T);
        Keyboard.releaseAll();
    } //cambiar canal HEXCHAT
    else if (code == 0xD803AB7) {/*DLNA*/
        if (rotation) {
            Keyboard.releaseAll();
            rotation = false;
            delay(T);
        } else {
            Keyboard.releaseAll();
            delay(T);
            Keyboard.press(KEY_LEFT_GUI);
            delay(T);
            Keyboard.write(KEY_TAB);
            rotation = true;
        }
    }
    //rotation mode  //iniciar animacion de rotation de ventanas  //start window rotation with compiz
    else if (code == 0xD00A1AC) {/*arrow up*/
        Keyboard.write(KEY_UP_ARROW);
    } else if (code == 0xD00616C) {/*arrow down*/
        Keyboard.releaseAll();
        delay(T);
        Keyboard.write(KEY_DOWN_ARROW);
    } else if (code == 0xD00E1EC) {/*arrow right*/
        Keyboard.write(KEY_LEFT_ARROW);
    } else if (code == 0xD00111C) {/*arrow left*/
        Keyboard.write(KEY_RIGHT_ARROW);
    } else if (code == 0xD00D9D4) {/*TOP MENU*/
        Keyboard.print("f");
    } //f pone a pantalla completa en VLC    fullscreen
    else if (code == 0xD00010C) {/*S sub menu*/
        Keyboard.print("v");
    } //v cambia subs en VLC   change subs
    else if (code == 0xD00CCC1) {/*AUDIO*/
        Keyboard.print("b");
    } //b cambia audio en VLC    change audio track
    else if (code == 0xD002825) {/*#5*/
        Keyboard.write(KEY_RETURN);
    } //useful to press enter on the file you want to play.
    else {
        Serial.print(IrReceiver.results.value, HEX);
        Serial.println(" Unknown!");
    }
}
