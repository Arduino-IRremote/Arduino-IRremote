// Define exactly one of these
//#define VAR_IN_PROGMEM
#define VAR_IN_MEM
//#define USE_F_FORM

#define TIMES_TO_SEND 10U

#include <IRremote.h>

const char yamahaVolDown[]
#ifdef VAR_IN_PROGMEM
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


#ifdef VAR_IN_PROGMEM
    Serial.println(F("Sending from PROGMEM"));
    irsend.sendPronto_PF(yamahaVolDown, TIMES_TO_SEND);
#elif defined(VAR_IN_MEM)
    Serial.println(F("Sending from normal memory"));
    irsend.sendPronto(yamahaVolDown, TIMES_TO_SEND);
#else
    Serial.println(F("Sending using the F()-form"));
    irsend.sendPronto(F("0000 006C 0022 0002 "
            "015B 00AD 0016 0016 0016 0041 0016 0016 0016 0041 0016 0041 0016 0041 0016 0041 0016 0016 0016 0041 0016 0016 0016 0041 "
            "0016 0016 0016 0016 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 0016 0016 0041 0016 0041 0016 0016 0016 0016 "
            "0016 0016 0016 0016 0016 0016 0016 0041 0016 0016 0016 0016 0016 0041 0016 0041 0016 0041 0016 05F7 015B 0057 0016 0E6C"), TIMES_TO_SEND);
#endif

    delay(5000);
}
