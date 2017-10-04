#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              R-STEP
//
//
//==============================================================================

/* -----------------------------------------------------------------------------
 r-step (Ruwido Standard Engineering Protocol) is used by some set top boxes
 (like canal+ cube).
 Few informations are available on internet except this pdf : http://bit.ly/2fRMTmu

 type of transmission : bi-phase, carried
 carrier frequency : 56 kHz / 17.9 usec
 burst : 250 usec (14 carrier pulse)
 pause : 250 usec
 carrier : ++++++__________
           | 6.0 |  11.9  |
 logic 1 : +++++++++++__________
 logic 0 : ___________++++++++++
 For continuous transmission, IR message is retransmit every 100 ms

 IR Frame structure :

 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 |
 |STA|        custom ID      |BAT|  device    |   address    |REP |

 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 |
 |        function code (data)           |

 STA: start bit, always 1
 Custom ID: 5 (000101) for canalsat cube and 9 (001001) for Telefonica
 BAT: Battery indication, always 1
 Device id: 000
 Address : 000
 REP: repeat code: 1st frame 0 then 1
 Function code: data
 frame length : 23 bits
   ----------------------------------------------------------------------------- */

#define RS_BITS         23
#define RS_SHORT_LEN    250     // usec
#define RS_LONG_LEN     500
#define RS_TOLERANCE    USECPERTICK

typedef struct {
  byte  sta; //start bit
  byte  custom_id;
  byte  bat;
  byte  device_id;
  byte  address;
  byte  rep; //repeat code
  byte  function_code;
} rstep_t;

//+=============================================================================
//
#if SEND_RSTEP
void  IRsend::sendRstep (byte custom_id,  byte data, byte repeat_code)
{
  byte bin_code[RS_BITS];

  int i = 0;

  // sta, always 1
  bin_code[i++] = 1;

  // custom_id on 6 bits
  for (int k = 5; k >= 0; k--) {
    bin_code[i++] = (custom_id & ( 1 << k )) >> k;
  }

  // bat, always 1
  bin_code[i++] = 1;

  // device_id and address
  for (int k = 0; k < 6; k++) {
    bin_code[i++] = 0;
  }

  // repeat_code
  if (repeat_code == 1)
    bin_code[i++] = 1;
  else
    bin_code[i++] = 0;

  // function_code aka data
  for (int k = 7; k >= 0; k--) {
    bin_code[i++] = (data & ( 1 << k )) >> k;
  }

  /*
  for (i = 0; i < RS_BITS; i++) {
    Serial.print(bin_code[i], DEC);
  }
  Serial.println("");
  */

  // Set IR carrier frequency
  enableIROut(56);
  for (i = 0; i < RS_BITS; i++) {
    if (bin_code[i] == 0) {
      space(RS_SHORT_LEN);
      mark (RS_SHORT_LEN);
    }
    else {
      mark (RS_SHORT_LEN);
      space(RS_SHORT_LEN);
    }
  }
  // Always end with the LED off
  space(0);
}
#endif

//+=============================================================================
//
#if DECODE_RSTEP
static void convert_to_byte(byte *dest, byte *array, byte len)
{
  for (byte i = 0; i < len; i++) {
    *dest <<= 1;
    if (array[i] == 1) *dest ^= 1;
  }
}

bool  IRrecv::decodeRstep (decode_results *results)
{
  rstep_t rstep;
  byte bin_map[RS_BITS * 2];
  byte bin_code[RS_BITS];

  bin_map[RS_BITS * 2 - 1] = 0;
  memset(&rstep, 0, sizeof(rstep));

  for (int i = 1, j = 0; i <= results->rawlen; i++) {
    int t = results->rawbuf[i] * USECPERTICK;

    if (!t) continue;
    if (t < RS_SHORT_LEN - RS_TOLERANCE) return false;
    if (t > RS_LONG_LEN + RS_TOLERANCE) return false;

    // first mark should be 250usec
    if (i == 1 && (t > RS_SHORT_LEN + RS_TOLERANCE ||
                   t < RS_SHORT_LEN - RS_TOLERANCE))
      return false;

    if (i % 2) { // Mark
      bin_map[j++] = 1;
      if (t >= RS_LONG_LEN - RS_TOLERANCE)
        bin_map[j++] = 1;

    }
    else {
      bin_map[j++] = 0;
      if (t >= RS_LONG_LEN - RS_TOLERANCE)
        bin_map[j++] = 0;
    }

#ifdef DEBUG
    if (t <= RS_SHORT_LEN + RS_TOLERANCE)
      Serial.print("250");
    else
      Serial.print("500");
    Serial.print(",");
#endif
  }

  for (int i = 0, j = 0; i < RS_BITS * 2; j++, i += 2) {
    if (bin_map[i] == 0 && bin_map[i+1] == 1) bin_code[j] = 0;
    if (bin_map[i] == 1 && bin_map[i+1] == 0) bin_code[j] = 1;
  }

  for (int i = 0; i < RS_BITS; i++) {
    Serial.print(bin_code[i], DEC);
  }
  Serial.println("");

  rstep.sta = bin_code[0];
  convert_to_byte(&rstep.custom_id, &bin_code[1], 6);
  rstep.bat = bin_code[7];
  convert_to_byte(&rstep.device_id, &bin_code[8], 3);
  convert_to_byte(&rstep.address, &bin_code[11], 3);
  rstep.rep = bin_code[14];
  convert_to_byte(&rstep.function_code, &bin_code[15], 8);

#ifdef DEBUG
  Serial.println("");
  Serial.print("STA: ");
  Serial.println(rstep.sta, DEC);
  Serial.print("Custom id: ");
  Serial.println(rstep.custom_id, DEC);
  Serial.print("BAT: ");
  Serial.println(rstep.bat, DEC);
  Serial.print("Device id: ");
  Serial.println(rstep.device_id, DEC);
  Serial.print("Address: ");
  Serial.println(rstep.address, DEC);
  Serial.print("Repeat code: ");
  Serial.println(rstep.rep, DEC);
  Serial.print("Function code: ");
  Serial.println(rstep.function_code, DEC);
#endif

  // Success
  results->bits        = 8;
  results->address     = rstep.custom_id;
  results->value       = rstep.function_code;
  results->decode_type = RSTEP;
  return true;
}
#endif
