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
  Remote.clear();
   */

#include <IRremote.h>
#define T 15
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;
//boolean flechas = false;
boolean rotacion = false;

void setup()
{
  pinMode(0,OUTPUT);
  pinMode(1,OUTPUT);
  digitalWrite(0,HIGH);
  digitalWrite(1,LOW);//using pin 0 and 1 to power the IRreciver, pin 2 is serial input.
  Serial.begin(57600);
  Keyboard.begin();
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    //DellRemote(results.value);
    PanasonicRemote(results.value);
    //mapping(results.value);
    delay(300);//estaba en 100//500 es muy largo
    irrecv.resume(); // Receive the next value
  }
  else{  delay(200);}//estaba en 100
}

void mapping(long code){
  //use this funtion to map the codes of all your buttons. some buttons send different codes if they are hold down
  if ((code == 0x801C2B2E) or (code == 0xB4AFB411) or (code == 0xD17D8037)){Serial.println("adelante/fordware button");  }
  else if ((code == 0xB7AFB8C8) or (code == 0xD07D7EA2) or (code == 0x801CAB2F)){Serial.println("atras/previous button");  }
  else {Serial.print(results.value, HEX); Serial.println("-----DESCONOCIDO/unkonw code! ");  }
}

void PanasonicRemote(long code){
  //here i have mapped some buttons of my blueray remote.
  if (code == 0xD00A0AD){/*adelante*/    Keyboard.releaseAll();delay(T); Keyboard.press(KEY_LEFT_ALT); Keyboard.press(KEY_RIGHT_ARROW); delay(T); Keyboard.releaseAll(); /*Remote.forward();  Remote.clear();*/}//Ctrl + ->    fordward 10 seconds VLC
  else if (code == 0xD00202D){/*atras*/  Keyboard.releaseAll();delay(T);  Keyboard.press(KEY_LEFT_ALT); Keyboard.press(KEY_LEFT_ARROW); delay(T); Keyboard.releaseAll(); /*Remote.rewind();  Remote.clear();*/} //Ctrl + <-    rewind 10 seconds VLC      
  else if (code == 0xD00525F){/*next*/   Keyboard.releaseAll();delay(T);  Remote.next();  Remote.clear();}
  else if (code == 0xD00929F){/*prev*/   Keyboard.releaseAll();delay(T);  Remote.previous();  Remote.clear();}
  //else if (code == 0xD00000D){/*stop*/   Keyboard.releaseAll();delay(T);  Remote.stop();  Remote.clear();}
  else if (code == 0x1000405){/*vol +*/    Keyboard.releaseAll();delay(T);Remote.increase();  Remote.clear();}
  else if (code == 0x1008485){/*vol -*/    Keyboard.releaseAll();delay(T);Remote.decrease();  Remote.clear(); }
  else if (code == 0xD00BCB1){/*POWER*/    Keyboard.releaseAll();delay(T);Remote.mute();  Remote.clear();}
  else if ((code == 0xD00606D) or (code == 0xD00505D)){/*play/pause*/  Keyboard.releaseAll();delay(T);  Remote.play();  Remote.clear();}
  else if (code == 0xD00818C){/*return*/   Keyboard.releaseAll(); delay(T); Keyboard.write(KEY_ESC);}
  else if (code == 0xD00414C){/*OK*/       Keyboard.releaseAll(); delay(T); Keyboard.print(" "); /*evitar doble pulsaciondelay(300);*/}
  else if (code == 0x100BCBD){/*tvPower*/  Keyboard.releaseAll();delay(T);Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_UP_ARROW); delay(T); Keyboard.releaseAll();}//volumen en VLC
  else if (code == 0x100A0A1){/*input AV*/ Keyboard.releaseAll();delay(T);Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_DOWN_ARROW); delay(T); Keyboard.releaseAll();}//volumen en VLC      
  else if (code == 0xD00808D){/*open/close*/Keyboard.releaseAll();delay(T);Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_PAGE_UP); delay(T); Keyboard.releaseAll();}//cambiar canal HEXCHAT
  else if (code == 0xD004944){/*display*/  Keyboard.releaseAll();delay(T);Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_PAGE_DOWN); delay(T); Keyboard.releaseAll();}//cambiar canal HEXCHAT
  else if (code == 0xD803AB7){/*DLNA*/     if (rotacion){Keyboard.releaseAll();rotacion=false;delay(T);}
                                           else{Keyboard.releaseAll(); delay(T); Keyboard.press(KEY_LEFT_GUI); delay(T); Keyboard.write(KEY_TAB);rotacion=true;}}
                                           //rotation mode  //iniciar animacion de rotacion de ventanas  //start window rotation with compiz      
  else if (code == 0xD00A1AC){/*arrow up*/     Keyboard.write(KEY_UP_ARROW);}
  else if (code == 0xD00616C){/*arrow down*/   Keyboard.releaseAll();delay(T);Keyboard.write(KEY_DOWN_ARROW);}
  else if (code == 0xD00E1EC){/*arrow right*/  Keyboard.write(KEY_LEFT_ARROW);}
  else if (code == 0xD00111C){/*arrow left*/   Keyboard.write(KEY_RIGHT_ARROW);}
  else if (code == 0xD00D9D4){/*TOP MENU*/     Keyboard.print("f");}//f pone a pantalla completa en VLC    fullscreen
  else if (code == 0xD00010C){/*S sub menu*/   Keyboard.print("v");}//v cambia subs en VLC   change subs
  else if (code == 0xD00CCC1){/*AUDIO*/    Keyboard.print("b");}//b cambia audio en VLC    change audio track
  else if (code == 0xD002825){/*#5*/       Keyboard.write(KEY_RETURN);}//useful to press enter on the file you want to play.
  else {Serial.print(results.value, HEX); Serial.println("-----DESCONOCIDO!");  }
}


//void DellRemote(long code){
//  if ((code == 0x801C2B2E) or (code == 0xB4AFB411) or (code == 0xD17D8037))
//      {/*adelante*/    Keyboard.releaseAll();delay(T);flechas = false;Keyboard.press(KEY_LEFT_ALT); Keyboard.press(KEY_RIGHT_ARROW); delay(T); Keyboard.releaseAll(); /*Remote.forward();  Remote.clear();*/}
//      //adelantar en VLC      forward
//  else if ((code == 0xB7AFB8C8) or (code == 0xD07D7EA2) or (code == 0x801CAB2F))
//      {/*atras*/       Keyboard.releaseAll();delay(T);flechas = false;Keyboard.press(KEY_LEFT_ALT); Keyboard.press(KEY_LEFT_ARROW); delay(T); Keyboard.releaseAll(); /*Remote.rewind();  Remote.clear();*/}
//      //retroceder en VLC   backwards
//      
//  else if ((code == 0x98BD82C2) or (code == 0xD218D2C)  or (code == 0xBD218D2C))
//      {/*next*/   Keyboard.releaseAll();delay(T);flechas = false;Remote.next();  Remote.clear();}
//  else if ((code == 0x7CA73789) or (code == 0x63D971B3) or (code == 0x801CAB21))
//      {/*prev*/   Keyboard.releaseAll();delay(T);flechas = false;Remote.previous();  Remote.clear();}
//  else if ((code == 0x851FCFD5) or (code == 0xB599B51F) or (code == 0x801CAB31))
//      {/*stop*/   Keyboard.releaseAll();delay(T);flechas = false;Remote.stop();  Remote.clear();}
//  else if ((code == 0x5ECE5800) or (code == 0x419B5F52) or (code == 0x801CAB2C))
//      {/*play/pause*/  Keyboard.releaseAll();delay(T);flechas = false;Remote.play();  Remote.clear();}
//  else if ((code == 0x948C3B0)  or (code == 0x17B591EA) or (code == 0x801CABA4))
//      {/*backspace*/   Keyboard.write(KEY_ESC);delay(T);flechas = false;Keyboard.releaseAll();}
//  else if ((code == 0x2AF7C446) or (code == 0x31D33AF4) or (code == 0x801CAB5C))
//      {/*check*/      if(flechas){Keyboard.releaseAll();delay(T);flechas = false;}else{ Keyboard.print(" ");}}
//      
//  else if ((code == 0x801CABCE) or (code == 0xC838DBEF) or (code == 0x11A1DED))  
//      {/*page up*/      Keyboard.releaseAll();delay(T);flechas = false;Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_UP_ARROW); delay(T); Keyboard.releaseAll();}
//      //volumen en VLC
//  else if ((code == 0x41A22A4)  or (code == 0xC738DA5A) or (code == 0x801CABCF))
//      {/*page down*/   Keyboard.releaseAll();delay(T);flechas = false;Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_DOWN_ARROW); delay(T); Keyboard.releaseAll();}
//      //volumen en VLC
//      
//  else if ((code == 0x1EBE32)   or (code == 0x529FB6FC) or (code == 0x801C2B10)) 
//      {/*vol up*/      Keyboard.releaseAll();delay(T);flechas = false;Remote.increase();  Remote.clear();}
//  else if ((code == 0x801CAB11) or (code == 0xA2AB03A3) or (code == 0xE48B8D9))
//      {/*vol down*/     Keyboard.releaseAll();delay(T);flechas = false;Remote.decrease();  Remote.clear(); }
//  else if ((code == 0xFB6B19B4) or (code == 0x1662CD72) or (code == 0x801C2B0D))
//      {/*mute*/      Keyboard.releaseAll();delay(T);flechas = false;Remote.mute();  Remote.clear();}
//      
//  else if ((code == 0x2E88114A) or (code == 0x4B9B7430) or (code == 0x801CAB58)){/*arrow up*/
//  //iniciar animacion de rotacion de ventanas
//  //start window rotation with compiz
//      Keyboard.releaseAll();
//      delay(T);
//      Keyboard.write(KEY_UP_ARROW);
//      Keyboard.press(KEY_LEFT_GUI);
//      delay(T);
//      Keyboard.write(KEY_TAB);flechas=true;}//rotation mode
//      
//  else if ((code == 0x2EA26AFB) or (code == 0xAD79DDED) or (code == 0x801CAB59))
//      {/*arrow down*/  Keyboard.releaseAll();delay(T);flechas = false;Keyboard.print("b");delay(T);Keyboard.write(KEY_DOWN_ARROW);}
//      //b cambia audio en VLC    change audio track
//  else if ((code == 0x3B726EB4) or (code == 0x54A4E562) or (code == 0x801C2B5B))
//      {/*arrow right*/  if (flechas){Keyboard.write(KEY_LEFT_ARROW);}else{Keyboard.print("v");}}
//      //v cambia subs en VLC   change subs
//  else if ((code == 0x801CAB5A) or (code == 0x3C727047) or (code == 0x55A4E6F5))
//      {/*arrow left*/  if (flechas){Keyboard.write(KEY_RIGHT_ARROW);}else{Keyboard.print("f");}}
//      //f pone a pantalla completa en VLC    fullscreen
//  //else {Serial.print(results.value, HEX); Serial.println("-----DESCONOCIDO!");  }
//}
