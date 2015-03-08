#include "IRremote.h"
#include "CppList.h"

bool _initialized = false;

int rcv_count;
IRrecv **all_rcv;

void setup() {
  if (_initialized) return;
  
  Serial.begin(9600);
  
  rcv_count = 3;
  all_rcv = (IRrecv **)malloc(rcv_count*sizeof(int));
  all_rcv[0] = new IRrecv(2);
  all_rcv[1] = new IRrecv(3);
  all_rcv[2] = new IRrecv(4);
  
  for (int i=0; i<rcv_count; ++i){
    all_rcv[i]->enableIRIn();
  }
  
  _initialized = true;
}

void loop() {
  for (int i=0; i<rcv_count; ++i){
    decode_results results;
    if (all_rcv[i]->decode(&results)) {
      int btn = DecodeButton(results.value);
      Serial.print("Rcv_");
      Serial.print(i);
      Serial.print(":");
      Serial.println(btn);
      all_rcv[i]->resume();
    }
  }
}

const int BTN_EMPTY = 0;
const int BTN_POWER = 99;
const int BTN_0 = 10;
const int BTN_1 = 1;
const int BTN_2 = 2;
const int BTN_3 = 3;
const int BTN_4 = 4;
const int BTN_5 = 5;
const int BTN_STOP = 6;
const int BTN_DOWN = 7;
const int BTN_UP = 8;
const int BTN_BACK = 9;
const int BTN_FWD = 11;
const int BTN_EQ = 12;
const int BTN_REPEAT = 100;
const int BTN_UNKNOWN = 101;
const int BTN_ERROR = 102;

int DecodeButton(int param){
	int rez = BTN_UNKNOWN;
	switch (param){
	case 0x0: {
						rez = BTN_ERROR;
						break;
			  }
	case 0x00FD00FF:
	case 0x00FF728D:
	case 0x20DF4EB1: {
						rez = BTN_POWER;
						break;
					 }
	case 0x00FD30CF:{
						rez = BTN_0;
						break;
					}
	case 0x00FD08F7:{
						rez = BTN_1;
						break;
					}
	case 0x00FD8877:{
						rez = BTN_2;
						break;
					}
	case 0x00FD48B7:{
						rez = BTN_3;
						break;
					}
	case 0x00FD28D7:{
						rez = BTN_4;
						break;
					}
	case 0x00FDA857:{
						rez = BTN_5;
						break;
					}
	case 0x00FD40BF:{
						rez = BTN_STOP;
						break;
					}
	case 0x00FD10EF:{
						rez = BTN_DOWN;
						break;
					}
	case 0x00FD50AF:{
						rez = BTN_UP;
						break;
					}
	case 0x00FD20DF:{
						rez = BTN_BACK;
						break;
					}
	case 0x00FD609F:{
						rez = BTN_FWD;
						break;
					}
	case 0x00FDB04F:{
						rez = BTN_EQ;
						break;
					}
	case 0xFFFFFFFF:{
						rez = BTN_REPEAT;
						break;
					}
	}
	return rez;
}

