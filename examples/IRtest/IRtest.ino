/*
 * IRremote: IRtest unittest
 * Initially coded 2009 Ken Shirriff http://www.righto.com
 *
 * Note: to run these tests, edit IRremote/IRremote.h to add "#define TEST"
 * You must then recompile the library by removing IRremote.o and restarting
 * the arduino IDE.
 */

#include <IRremote.h>

IRrecv IrReceiver(0);

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump() {
    int count = IrReceiver.results.rawlen;

    if (IrReceiver.results.decode_type == UNKNOWN) {
        Serial.println("Could not decode message");
    } else {
        IrReceiver.printResultShort(&Serial);

        Serial.print(" (");
        Serial.print(IrReceiver.results.bits, DEC);
        Serial.println(" bits)");
    }
    Serial.print("Raw (");
    Serial.print(count, DEC);
    Serial.print("): ");

    for (int i = 0; i < count; i++) {
        if ((i % 2) == 1) {
            Serial.print(IrReceiver.results.rawbuf[i] * MICROS_PER_TICK, DEC);
        } else {
            Serial.print(-(int) IrReceiver.results.rawbuf[i] * MICROS_PER_TICK, DEC);
        }
        Serial.print(" ");
    }
    Serial.println("");
}

class IRsendDummy: public IRsend {
public:
    // For testing, just log the marks/spaces
#define SENDLOG_LEN 128
    int sendlog[SENDLOG_LEN];
    int sendlogcnt = 0;
    IRsendDummy() :
            IRsend() {
    }
    void reset() {
        sendlogcnt = 0;
    }
    void mark(int time) {
        sendlog[sendlogcnt] = time;
        if (sendlogcnt < SENDLOG_LEN)
            sendlogcnt++;
    }
    void space(int time) {
        sendlog[sendlogcnt] = -time;
        if (sendlogcnt < SENDLOG_LEN)
            sendlogcnt++;
    }
    // Copies the dummy buf into the interrupt buf
    void useDummyBuf() {
        int last = SPACE;
        irparams.rcvstate = IR_REC_STATE_STOP;
        irparams.rawlen = 1; // Skip the gap
        for (int i = 0; i < sendlogcnt; i++) {
            if (sendlog[i] < 0) {
                if (last == MARK) {
                    // New space
                    irparams.rawbuf[irparams.rawlen++] = (-sendlog[i] - MARK_EXCESS_MICROS) / MICROS_PER_TICK;
                    last = SPACE;
                } else {
                    // More space
                    irparams.rawbuf[irparams.rawlen - 1] += -sendlog[i] / MICROS_PER_TICK;
                }
            } else if (sendlog[i] > 0) {
                if (last == SPACE) {
                    // New mark
                    irparams.rawbuf[irparams.rawlen++] = (sendlog[i] + MARK_EXCESS_MICROS) / MICROS_PER_TICK;
                    last = MARK;
                } else {
                    // More mark
                    irparams.rawbuf[irparams.rawlen - 1] += sendlog[i] / MICROS_PER_TICK;
                }
            }
        }
        if (irparams.rawlen % 2) {
            irparams.rawlen--; // Remove trailing space
        }
    }
};

IRsendDummy irsenddummy;

void verify(unsigned long val, int bits, int type) {
    irsenddummy.useDummyBuf();
    IrReceiver.decode();
    Serial.print("Testing ");
    Serial.print(val, HEX);
    if (IrReceiver.results.value == val && IrReceiver.results.bits == bits && IrReceiver.results.decode_type == type) {
        Serial.println(": OK");
    } else {
        Serial.println(": Error");
        dump();
    }
}

void testNEC(uint32_t val, int bits) {
    irsenddummy.reset();
    irsenddummy.sendNEC(val, bits);
    verify(val, bits, NEC);
}
void testSony(unsigned long val, int bits) {
    irsenddummy.reset();
    irsenddummy.sendSony(val, bits);
    verify(val, bits, SONY);
}
void testRC5(uint32_t val, int bits) {
    irsenddummy.reset();
    irsenddummy.sendRC5(val, bits);
    verify(val, bits, RC5);
}
void testRC6(uint32_t val, int bits) {
    irsenddummy.reset();
    irsenddummy.sendRC6(val, bits);
    verify(val, bits, RC6);
}

void test() {
    Serial.println("NEC tests");
    testNEC(0x00000000, 32);
    testNEC(0xffffffff, 32);
    testNEC(0xaaaaaaaa, 32);
    testNEC(0x55555555, 32);
    testNEC(0x12345678, 32);
    Serial.println("Sony tests");
    testSony(0xfff, 12);
    testSony(0x000, 12);
    testSony(0xaaa, 12);
    testSony(0x555, 12);
    testSony(0x123, 12);
    Serial.println("RC5 tests");
    testRC5(0xfff, 12);
    testRC5(0x000, 12);
    testRC5(0xaaa, 12);
    testRC5(0x555, 12);
    testRC5(0x123, 12);
    Serial.println("RC6 tests");
    testRC6(0xfffff, 20);
    testRC6(0x00000, 20);
    testRC6(0xaaaaa, 20);
    testRC6(0x55555, 20);
    testRC6(0x12345, 20);
}

void setup() {
    Serial.begin(115200);
    test();
}

void loop() {
}
