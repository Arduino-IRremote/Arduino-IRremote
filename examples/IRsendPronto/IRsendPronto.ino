/*
   IRsendPronto: Deomonstrates how to send a Pronto code.
   An IR LED must be connected to Arduino PWM pin 3.
   Version 1.0 July, 2018
   By Nathan Seidle @ SparkFun

   If you have the remote for the device you are trying to control
   it is fairly ease to inspect codes using IRrecvDump and then re-send
   those codes using IRsendDemo. But what if you've lost the remote?

   Pronto is a pseudo-universal format. The downside is that each code takes
   up a lot of program memory (400 bytes vs 4 bytes).

   If you're in a pinch and only have a few buttons you need pressed
   then Pronto codes can help.

   Checkout these websites to get Pronto codes for most devices:
   http://irdb.tk/codes/
   http://www.remotecentral.com/cgi-bin/codes/samsung/tv_functions/
*/

#include <IRremote.h>

IRsend irsend;

//Pronto commands for Samsung TV
const char volUp[] PROGMEM = "0000 006d 0022 0003 00a9 00a8 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 003f 0015 003f 0015 003f 0015 0702 00a9 00a8 0015 0015 0015 0e6e";
const char volDown[] PROGMEM = "0000 006d 0022 0003 00a9 00a8 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 0015 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 0015 0015 003f 0015 003f 0015 003f 0015 003f 0015 0702 00a9 00a8 0015 0015 0015 0e6e";

const char* const command_table[] PROGMEM = {volUp, volDown};

char buffer[400]; //Must be large enough to store longest Pronto command

void setup()
{
  Serial.begin(9600);
  Serial.println("Sending");
}

void loop() {
  strcpy_P(buffer, (char*)pgm_read_word(&(command_table[1]))); //Copy pronto command into buffer
  
  for (int i = 0; i < 3; i++) {
    //irsend.sendSAMSUNG(0xE0E0E01F, 32); //Found by using Samsung remote to send code to IRrecvDump

    //If you don't have the original remote to get codes from then use Pronto codes!
    //Code from http://www.remotecentral.com/cgi-bin/codes/samsung/tv_functions/
    irsend.sendPronto(buffer, PRONTO_ONCE, PRONTO_FALLBACK); // once code

    delay(100);
  }

  Serial.println("Done");
  while (1);
}
