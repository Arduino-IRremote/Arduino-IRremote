/*
 * IRremote: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * RCMM protocol added by Matthias Neeracher
 */

#include <IRremote.h>

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  #define IS_AVTINY
  int RECV_PIN = 2;
  #define ENABLE_MagiQuest 
  // add others as to fit in Tiny
#else
  int RECV_PIN = 11;
  #define ENABLE_MagiQuest
  #define ENABLE_NEC
  #define ENABLE_SONY
  #define ENABLE_Sanyo
  #define ENABLE_Mitsubishi
  #define ENABLE_Panasonic
  #define ENABLE_RC5
  #define ENABLE_RC6
  #define ENABLE_SymaR3
  #define ENABLE_SymaR5
  #define ENABLE_Useries
  #define ENABLE_FastLane
  #define ENABLE_JVC
  #define ENABLE_RCMM
#endif


IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  Serial.println("starting...");
  irrecv.enableIRIn(); // Start the receiver
#ifdef IS_AVTINY
  pinMode(1, OUTPUT); //LED on Model A   


  for (int i = 0; i < 5; i++) {
    digitalWrite(1, HIGH);
    delay(125);               // wait for a second
    digitalWrite(1, LOW); 
    delay(125);               // wait for a second
  }

#endif //IS_AVTINY
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {

  int count = results->rawlen;

//#ifndef IS_AVTINY
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } 
//#endif //IS_AVTINY

#ifdef ENABLE_NEC
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } 
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } 
#endif //ENABLE_NEC

#ifdef ENABLE_MagiQuest
  else if (results->decode_type == MAGIQUEST) {
  #ifndef IS_AVTINY
    Serial.print("Decoded MAGIQUEST - Magnitude=");
    Serial.print(results->magiquestMagnitude, HEX);
    Serial.print(", wand_id=");
  #endif //IS_AVTINY
    if (results->value == 0x4FAB881) {
      for (int i = 0; i < 2; i++) {
        digitalWrite(1, HIGH);
        delay(250);               // wait for a second
        digitalWrite(1, LOW); 
        delay(125);               // wait for a second
      }
    }
  }
#endif //ENABLE_MagiQuest

#ifdef ENABLE_RC5
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } 
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
#endif //ENABLE_RC5

#ifdef ENABLE_Panasonic
  else if (results->decode_type == PANASONIC) {	
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->panasonicAddress,HEX);
    Serial.print(", Value: ");
    for (int i = 0; i < 3; i++) {
      digitalWrite(1, HIGH);
      delay(250);               // wait for a second
      digitalWrite(1, LOW); 
      delay(125);               // wait for a second
    }
  }
#endif //ENABLE_Panasonic

#ifdef ENABLE_Syma
  else if (results->decode_type == SYMA_R5) {
    Serial.print("Decoded SYMA_R5 - ");
    Serial.print(" C="); Serial.print(results->helicopter.symaR5.Channel);
    Serial.print("("); Serial.write(results->helicopter.symaR5.Channel + 0x41); Serial.print(")"); 
    Serial.print(" T="); Serial.print(results->helicopter.symaR5.Throttle,DEC);
    Serial.print(" P="); Serial.print(results->helicopter.symaR5.Pitch,DEC);
    Serial.print(" Y="); Serial.print(results->helicopter.symaR5.Yaw,DEC);
    Serial.print(" t="); Serial.print(results->helicopter.symaR5.Trim,DEC);
  }
#endif //ENABLE_Syma

#ifdef ENABLE_Syma
  else if (results->decode_type == SYMA_R3) {
    Serial.print("Decoded SYMA_R3 - ");
    Serial.print(" C="); Serial.print(results->helicopter.symaR3.Channel);
    Serial.print("("); Serial.write(results->helicopter.symaR3.Channel + 0x41); Serial.print(")"); 
    Serial.print(" T="); Serial.print(results->helicopter.symaR3.Throttle,DEC);
    Serial.print(" P="); Serial.print(results->helicopter.symaR3.Pitch,DEC);
    Serial.print(" Y="); Serial.print(results->helicopter.symaR3.Yaw,DEC);
  }
#endif //ENABLE_Syma

#ifdef ENABLE_Useries
  else if (results->decode_type == USERIES) {
    // temp variables to hold Channel from propietary format. 
    uint8_t Channel = results->helicopter.uSeries.Channel;

    //convert Channel to easy A,B,C
    switch (Channel) {
      case 1:
      case 2:
        Channel--;
        break;
      case 0:
        Channel = 2;
        break;
    } // ~case
    
    Serial.print("Decoded Useries - ");
    Serial.print(" C="); Serial.print(results->helicopter.uSeries.Channel,DEC);
    Serial.print("("); Serial.write(Channel + 0x41); Serial.print(")"); 
    Serial.print(" T="); Serial.print(results->helicopter.uSeries.Throttle,DEC);
    Serial.print(" P="); Serial.print(results->helicopter.uSeries.Pitch,DEC);
    Serial.print(" Y="); Serial.print(results->helicopter.uSeries.Yaw,DEC);
    Serial.print(" t="); Serial.print(results->helicopter.uSeries.Trim,DEC);
    Serial.print(" Turbo="); Serial.print(results->helicopter.uSeries.Turbo,DEC);
    Serial.print(" Lbutton="); Serial.print(results->helicopter.uSeries.Lbutton,DEC);
    Serial.print(" Rbutton="); Serial.print(results->helicopter.uSeries.Rbutton,DEC);
    Serial.print(" Leftover="); Serial.print(results->helicopter.uSeries.cksum,DEC);
    Serial.print(" Parity="); Serial.print(results->parity,DEC);
  }
#endif //ENABLE_Useries

#ifdef ENABLE_FastLane
  else if (results->decode_type == FASTLANE) {
    // temp variables to hold FastLane's unsigned integers from propietary magnitude and direction bits. 
    uint8_t Yaw;
    uint8_t Trim;
    uint8_t Channel;

    //convert Yaw's direction bit to unsigned integers, for easy scaling
    if (results->helicopter.fastlane.Yaw_dir) {
      Yaw = 16 - results->helicopter.fastlane.Yaw;
    } else {
      Yaw = 16 + results->helicopter.fastlane.Yaw;
    }

    //convert Trim's direction bit to unsigned integers, for easy scaling
    if (results->helicopter.fastlane.Trim_dir) {
      Trim = 16 - results->helicopter.fastlane.Trim;
    } else {
      Trim = 16 + results->helicopter.fastlane.Trim;
    }
    
    //convert Channel to easy A,B,C
    Channel = ((results->helicopter.fastlane.Channel >> 1) & 0x55) | ((results->helicopter.fastlane.Channel << 1) & 0xaa); 

    Serial.print("Decoded FastLane - ");
    Serial.print(millis()); Serial.print("ms ");
    Serial.print(" C="); Serial.print(results->helicopter.fastlane.Channel,DEC);
    Serial.print("("); Serial.write(Channel + 0x41); Serial.print(")"); 
    Serial.print(" T="); Serial.print(results->helicopter.fastlane.Throttle,DEC);
    Serial.print(" P="); Serial.print(results->helicopter.fastlane.Pitch,DEC);
    Serial.print(" Yd="); Serial.print(results->helicopter.fastlane.Yaw_dir,DEC);
    Serial.print(" Y="); Serial.print(results->helicopter.fastlane.Yaw,DEC);
    Serial.print("(u"); Serial.print(Yaw,DEC);Serial.print(")");
    Serial.print(" td="); Serial.print(results->helicopter.fastlane.Trim_dir,DEC);
    Serial.print(" t="); Serial.print(results->helicopter.fastlane.Trim,DEC);
    Serial.print("(u"); Serial.print(Trim,DEC); Serial.print(")");
    Serial.print(" F="); Serial.print(results->helicopter.fastlane.Fire,DEC);

  }
#endif //ENABLE_FastLane

#ifdef ENABLE_JVC
  else if (results->decode_type == JVC) {
     Serial.print("Decoded JVC: ");
  }
#endif //ENABLE_JVC

#ifdef ENABLE_RCMM
  else if (results->decode_type == RCMM) {
     Serial.print("Decoded RCMM: ");
  }
  Serial.print(" value=");
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(long)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
#endif //ENABLE_RCMM
}


void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
  }
}
