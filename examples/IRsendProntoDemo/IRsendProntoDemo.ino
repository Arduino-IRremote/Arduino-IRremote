// Comment this out if you to send from FLASH
#define VAR_IN_PROGMEM

#define TIMES_TO_SEND 10U

#include <IRremote.h>

const char yamahaVolDown[]
#if defined(VAR_IN_PROGMEM) && HAS_FLASH_READ
PROGMEM
#endif
= "0000 006C 0022 0002 "
        "015B 00AD 0016 0016 0016 0041 0016 0016 0016 0041 0016 0041 0016 0041 0016 0041 0016 0016 0016 0041 0016 0016 0016 0041 "
        "0016 0016 0016 0016 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 0016 0016 0041 0016 0041 0016 0016 0016 0016 "
        "0016 0016 0016 0016 0016 0016 0016 0041 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 05F7 015B 0057 0016 0E6C";

IRsend irsend;

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ;

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    Serial.print(F("Will send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

void loop() {

#if defined(VAR_IN_PROGMEM) && HAS_FLASH_READ
    Serial.println(F("Sending from PROGMEM"));
    irsend.sendPronto_PF(yamahaVolDown, TIMES_TO_SEND);
#else
    Serial.println(F("Sending from normal memory"));
    irsend.sendPronto(yamahaVolDown, TIMES_TO_SEND);
#endif

    delay(2000);
#if HAS_FLASH_READ
    Serial.println(F("Sending Yamaha (Nec) using the F()-form"));
    irsend.sendPronto(
            F(
                    "0000 006C 0022 0002 "
                            "015B 00AD 0016 0016 0016 0041 0016 0016 0016 0041 0016 0041 0016 0041 0016 0041 0016 0016 0016 0041 0016 0016 0016 0041 "
                            "0016 0016 0016 0016 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 0016 0016 0041 0016 0041 0016 0016 0016 0016 "
                            "0016 0016 0016 0016 0016 0016 0016 0041 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 05F7 015B 0057 0016 0E6C"),
            TIMES_TO_SEND);
    delay(2000);
#endif

    // send Nec code aquired by IRreceiveDumpV2.cpp
    Serial.println(F("Sending Nec: address 0xFF00, data 0x15"));
    // 006D -> 38029 Hz
    irsend.sendPronto(
            "0000 006D 0022 0000 015C 00AB 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 0017 0015 "
                    "0017 003F 0017 003E 0017 003F 0017 003E 0017 003F 0015 003F 0017 003F 0015 003F 0017 003E 0017 0015 0017 003F 0017 0015 0017 "
                    "003E 0017 0015 0017 0017 0015 0017 0017 0015 0017 003E 0017 0015 0017 003F 0015 0017 0017 003E 0017 003F 0015 003F 0017 0806",
            1); // no repeats

    delay(5000);
}
