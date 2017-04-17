#include <IRremote.h>

int recvPin = 11;
IRrecv irrecv(recvPin);
IRsend irsend;

// Credits to https://github.com/barakwei/IRelectra for decode this
// That configuration has a total of 34 bits
//    33: Power bit, if this bit is ON, the A/C will toggle it's power.
// 32-30: Mode - Cool, heat etc.
// 29-28: Fan - Low, medium etc.
// 27-26: Zeros
//    25: Swing On/Off
// 24-23: Zeros
// 22-19: Temperature, where 15 is 0000, 30 is 1111
//    18: Sleep mode On/Off
// 17- 2: Zeros
//     1: One
//     0: Zero
typedef union ElectraCode {
    uint64_t num;
    struct {
        uint8_t zeros1 : 1;
        uint8_t ones1 : 1;
        uint16_t zeros2 : 16;
        uint8_t sleep : 1;
        uint8_t temperature : 4;
        uint8_t zeros3 : 2;
        uint8_t swing : 1;
        uint8_t zeros4 : 2;
        uint8_t fan : 2;
        uint8_t mode : 3;
        uint8_t power : 1;
    };
} ElectraUnion;

typedef enum IRElectraMode {
    IRElectraModeCool = 0b001,
    IRElectraModeHeat = 0b010,
    IRElectraModeAuto = 0b011,
    IRElectraModeDry  = 0b100,
    IRElectraModeFan  = 0b101
} IRElectraMode;

typedef enum IRElectraFan {
    IRElectraFanLow    = 0b00,
    IRElectraFanMedium = 0b01,
    IRElectraFanHigh   = 0b10,
    IRElectraFanAuto   = 0b11
} IRElectraFan;

void  setup ( )
{
  Serial.begin(115200);   // Status message will be sent to PC at 9600 baud
  irrecv.enableIRIn();  // Start the receiver
}

void  dumpCode (ElectraCode *code)
{  
  Serial.print("Toggle Power: ");
  Serial.println(code->power);

  dumpElectraMode(code->mode);
  dumpElectraFan(code->fan);

  Serial.print("Temperature: ");
  Serial.println(code->temperature + 15, DEC);

  Serial.print("Sleep: ");
  Serial.println(code->sleep);
  
  Serial.print("Swing: ");
  Serial.println(code->swing);
}

void  dumpElectraMode (IRElectraMode mode)
{
  Serial.print("Mode: ");
  switch (mode) {
    case IRElectraModeCool:
      Serial.println("Cool");
      break;
    case IRElectraModeHeat:
      Serial.println("Heat");
      break;
    case IRElectraModeAuto:
      Serial.println("Auto");
      break;
    case IRElectraModeDry:
      Serial.println("Dry");
      break;
    case IRElectraModeFan:
      Serial.println("Fan");
      break;
    default:
      Serial.println(mode, BIN);
      break;
  }
}

void  dumpElectraFan (IRElectraFan fan)
{
  Serial.print("Fan: ");
  switch (fan) {
    case IRElectraFanLow:
      Serial.println("Low");
      break;
    case IRElectraFanMedium:
      Serial.println("Medium");
      break;
    case IRElectraFanHigh:
      Serial.println("High");
      break;
    case IRElectraFanAuto:
      Serial.println("Auto");
      break;
    default:
      Serial.println(fan, BIN);
      break;
  }
}

ElectraCode code = { 0 };

void  loop ( )
{
  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == 'z')
    {
      Serial.println("Sending Code!");
      irsend.sendElectraRC3(code.num, 34);
      dumpCode(&code);
      Serial.println("");
    }
  }

  decode_results results;

  if (irrecv.decode(&results))
  {
    if (results.decode_type == ELECTRARC3)
    {
      code.num = results.value;
      dumpCode(&code);
    }
    else
    {
      Serial.println("Not Electra!");
    }
    Serial.println("");
    irrecv.resume();
  }

}