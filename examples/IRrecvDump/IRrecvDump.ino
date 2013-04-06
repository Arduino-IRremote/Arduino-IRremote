/*
 * IRremote: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#include <IRremote.h>
uint8_t RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  Serial.println("starting...");
  irrecv.enableIRIn(); // Start the receiver
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {
  int16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } 
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } 
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } 
  else if (results->decode_type == MAGIQUEST) {
    Serial.print("Decoded MAGIQUEST - Magnitude=");
    Serial.print(results->magiquestMagnitude, HEX);
    Serial.print(", wand_id=");
  }
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } 
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == PANASONIC) {	
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->panasonicAddress,HEX);
    Serial.print(", Value: ");
  }
  else if (results->decode_type == SYMA_R5) {
    Serial.print("Decoded SYMA_R5 - ");
    Serial.print(" C="); Serial.print(results->helicopter.symaR5.Channel);
    Serial.print("("); Serial.write(results->helicopter.symaR5.Channel + 0x41); Serial.print(")"); 
    Serial.print(" T="); Serial.print(results->helicopter.symaR5.Throttle,DEC);
    Serial.print(" P="); Serial.print(results->helicopter.symaR5.Pitch,DEC);
    Serial.print(" Y="); Serial.print(results->helicopter.symaR5.Yaw,DEC);
    Serial.print(" t="); Serial.print(results->helicopter.symaR5.Trim,DEC);
  }
  else if (results->decode_type == SYMA_R3) {
    Serial.print("Decoded SYMA_R3 - ");
    Serial.print(" C="); Serial.print(results->helicopter.symaR3.Channel);
    Serial.print("("); Serial.write(results->helicopter.symaR3.Channel + 0x41); Serial.print(")"); 
    Serial.print(" T="); Serial.print(results->helicopter.symaR3.Throttle,DEC);
    Serial.print(" P="); Serial.print(results->helicopter.symaR3.Pitch,DEC);
    Serial.print(" Y="); Serial.print(results->helicopter.symaR3.Yaw,DEC);
  }
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
  else if (results->decode_type == JVC) {
     Serial.print("Decoded JVC: ");
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
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
}


void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
  }
}