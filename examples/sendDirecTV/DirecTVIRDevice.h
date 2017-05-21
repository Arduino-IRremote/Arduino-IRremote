
#ifndef DIRECTVIRDEVICE_H
#define DIRECTVIRDEVICE_H

#endif

#include "Arduino.h"
#include <IRremote.h>


/*
 * DIRECTV / SKY HDTV  HDTV+  Zapper
 * 
 * Controlles: RC66L  RC65SB
 * 
 * 
 * 
 */



/*
 * CONSTANTS FOR SKY/DirecTV SD  SKY/DirecTV HDTV/HDTV+
 * 
 * 
 * 
 */

  const unsigned short NUM0 PROGMEM    = 0xc116;
  const unsigned short NUM1 PROGMEM    = 0xc011;
  const unsigned short NUM2 PROGMEM    = 0xc022;
  const unsigned short NUM3 PROGMEM    = 0xc033;
  const unsigned short NUM4 PROGMEM    = 0xc043;
  const unsigned short NUM5 PROGMEM    = 0xc054;
  const unsigned short NUM6 PROGMEM    = 0xc065;
  const unsigned short NUM7 PROGMEM    = 0xc076;
  const unsigned short NUM8  PROGMEM   = 0xc086;
  const unsigned short NUM9 PROGMEM    = 0xc097;
  const unsigned short NUMD PROGMEM    = 0xc127;
  const unsigned short NUME PROGMEM    = 0xc138;

  const unsigned short POWER PROGMEM   = 0xc105;
  const unsigned short SCRTV PROGMEM   = 0xc739;

  const unsigned short PLAY  PROGMEM   = 0xc30f;
  const unsigned short STOP PROGMEM    = 0xc310;
  const unsigned short PAUSE PROGMEM    = 0xc321;
  const unsigned short RW   PROGMEM    = 0xc332;
  const unsigned short FF   PROGMEM    = 0xc342;
  const unsigned short REC  PROGMEM    = 0xc353;
  const unsigned short CRW  PROGMEM    = 0xc364;
  const unsigned short CFF  PROGMEM    = 0xc375;

  const unsigned short GUIDE  PROGMEM  = 0xc280;
  const unsigned short CENTRAL PROGMEM = 0xc291;
  const unsigned short LIST   PROGMEM  = 0xc2a2;
  const unsigned short EXIT   PROGMEM  = 0xc26f;
  const unsigned short RETURN PROGMEM  = 0xc270;
  const unsigned short MENU   PROGMEM  = 0xc20a;
  const unsigned short INFO  PROGMEM   = 0xc2e5;

  const unsigned short CONFIRM PROGMEM = 0xc25e;
  const unsigned short UP     PROGMEM  = 0xc21b;
  const unsigned short DOWN  PROGMEM   = 0xc22c;
  const unsigned short LEFT   PROGMEM  = 0xc23d;
  const unsigned short RIGHT  PROGMEM  = 0xc24d;

  const unsigned short RED    PROGMEM  = 0xc418;
  const unsigned short GREEN  PROGMEM  = 0xc43a;
  const unsigned short YELLOW PROGMEM  = 0xc429;
  const unsigned short BLUE   PROGMEM  = 0xc44a;

  const unsigned short CHUP   PROGMEM  = 0xc0da;
  const unsigned short CHDOWN PROGMEM  = 0xc0eb;
  const unsigned short CHRETURN PROGMEM = 0xc0fc;



/*
 * 
 * This class works with SKY/DirecTV SD, SKY/DirecTV HDTV/HDTV+ remote controller
 *
 * 
 */

class DirecTVBox{

  private:
  
  IRsend * irsend;
  int nbits;

  public:

  DirecTVBox(IRsend * sender);
  void send(const unsigned short code);

  void btn_num0();
  void btn_num1();
  void btn_num2();
  void btn_num3();
  void btn_num4();
  void btn_num5();
  void btn_num6();
  void btn_num7();
  void btn_num8();
  void btn_num9();
  void btn_numD();
  void btn_numE();

  void btn_power();
  void btn_scrtv();

  void btn_play();
  void btn_stop();
  void btn_pause();
  void btn_rw();
  void btn_ff();
  void btn_rec();
  void btn_crw();
  void btn_cff();

  void btn_guide();
  void btn_central();
  void btn_list();
  void btn_exit();
  void btn_return();
  void btn_menu();
  void btn_info();

  void btn_confirm();
  void btn_up();
  void btn_down();
  void btn_left();
  void btn_right();

  void btn_red();
  void btn_green();
  void btn_yellow();
  void btn_blue();

  void btn_chup();
  void btn_chdown();
  void btn_chreturn();
  
};



DirecTVBox::DirecTVBox(IRsend * sender){

  irsend = sender;
  nbits = 16;
  
  }



void DirecTVBox::send(const unsigned short code){
  
  irsend->sendDirecTV(code,nbits);

  
  }


void DirecTVBox::btn_num0(){
  send(NUM0);
  }
void DirecTVBox::btn_num1(){
  send(NUM1);
  }
void DirecTVBox::btn_num2(){
  send(NUM2);
  }
void DirecTVBox::btn_num3(){
  send(NUM3);
  }
void DirecTVBox::btn_num4(){
  send(NUM4);
  }
void DirecTVBox::btn_num5(){
  send(NUM5);
  }
void DirecTVBox::btn_num6(){
  send(NUM6);
  }
void DirecTVBox::btn_num7(){
  send(NUM7);
  }
void DirecTVBox::btn_num8(){
  send(NUM8);
  }
void DirecTVBox::btn_num9(){
  send(NUM9);
  }
void DirecTVBox::btn_numD(){
  send(NUMD);
  }
void DirecTVBox::btn_numE(){
  send(NUME);
  }




void DirecTVBox::btn_power(){
  send(POWER);
  }
void DirecTVBox::btn_scrtv(){
  send(SCRTV);
  }

void DirecTVBox::btn_play(){
  send(PLAY);
  }
void DirecTVBox::btn_stop(){
  send(STOP);
  }
void DirecTVBox::btn_pause(){
  send(PAUSE);
  }
void DirecTVBox::btn_rw(){
  send(RW);
  }
void DirecTVBox::btn_ff(){
  send(FF);
  }
void DirecTVBox::btn_rec(){
  send(REC);
  }
void DirecTVBox::btn_crw(){
  send(CRW);
  }
void DirecTVBox::btn_cff(){
  send(CFF);
  }

void DirecTVBox::btn_guide(){
  send(GUIDE);
  }
void DirecTVBox::btn_central(){
  send(CENTRAL);
  }
void DirecTVBox::btn_list(){
  send(LIST);
  }
void DirecTVBox::btn_exit(){
  send(EXIT);
  }
void DirecTVBox::btn_return(){
  send(RETURN);
  }
void DirecTVBox::btn_menu(){
  send(MENU);
  }
void DirecTVBox::btn_info(){
  send(INFO);
  }

void DirecTVBox::btn_confirm(){
  send(CONFIRM);
  }
void DirecTVBox::btn_up(){
  send(UP);
  }
void DirecTVBox::btn_down(){
  send(DOWN);
  }
void DirecTVBox::btn_left(){
  send(LEFT);
  }
void DirecTVBox::btn_right(){
  send(RIGHT);
  }

void DirecTVBox::btn_red(){
  send(RED);
  }
void DirecTVBox::btn_green(){
  send(GREEN);
  }
void DirecTVBox::btn_yellow(){
  send(YELLOW);
  }
void DirecTVBox::btn_blue(){
  send(BLUE);
  }

void DirecTVBox::btn_chup(){
  send(CHUP);
  }
void DirecTVBox::btn_chdown(){
  send(CHDOWN);
  }

void DirecTVBox::btn_chreturn(){
  send(CHRETURN);
  }











