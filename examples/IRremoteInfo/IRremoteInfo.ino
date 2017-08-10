/*
 * IRremote: IRremoteInfo - prints relevant config info & settings for IRremote over serial
 * Intended to help identify & troubleshoot the various settings of IRremote
 * For example, sometimes users are unsure of which pin is used for Tx or the RAWBUF values
 * This example can be used to assist the user directly or with support.
 * Intended to help identify & troubleshoot the various settings of IRremote
 * Hopefully this utility will be a useful tool for support & troubleshooting for IRremote
 * Check out the blog post describing the sketch via http://www.analysir.com/blog/2015/11/28/helper-utility-for-troubleshooting-irremote/
 * Version 1.0 November 2015
 * Original Author: AnalysIR - IR software & modules for Makers & Pros, visit http://www.AnalysIR.com
 */


#include <IRremote.h>

void setup()
{
  Serial.begin(115200); //You may alter the BAUD rate here as needed
  while (!Serial); //wait until Serial is established - required on some Platforms

  //Runs only once per restart of the Arduino.
  dumpHeader();
  dumpRAWBUF();
  dumpTIMER();
  dumpTimerPin();
  dumpClock();
  dumpPlatform();
  dumpPulseParams();
  dumpSignalParams();
  dumpArduinoIDE();
  dumpDebugMode();
  dumpProtocols();
  dumpFooter();
}

void loop() {
  //nothing to do!
}

void dumpRAWBUF() {
  Serial.print(F("RAWBUF: "));
  Serial.println(RAWBUF);
}

void dumpTIMER() {
  boolean flag = false;
#ifdef IR_USE_TIMER1
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer1")); flag = true;
#endif
#ifdef IR_USE_TIMER2
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer2")); flag = true;
#endif
#ifdef IR_USE_TIMER3
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer3")); flag = true;
#endif
#ifdef IR_USE_TIMER4
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer4")); flag = true;
#endif
#ifdef IR_USE_TIMER5
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer5")); flag = true;
#endif
#ifdef IR_USE_TIMER4_HS
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer4_HS")); flag = true;
#endif
#ifdef IR_USE_TIMER_CMT
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer_CMT")); flag = true;
#endif
#ifdef IR_USE_TIMER_TPM1
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer_TPM1")); flag = true;
#endif
#ifdef IR_USE_TIMER_TINY0
  Serial.print(F("Timer defined for use: ")); Serial.println(F("Timer_TINY0")); flag = true;
#endif

  if (!flag) {
    Serial.print(F("Timer Error: ")); Serial.println(F("not defined"));
  }
}

void dumpTimerPin() {
  Serial.print(F("IR Tx Pin: "));
  Serial.println(SEND_PIN);
}

void dumpClock() {
  Serial.print(F("MCU Clock: "));
  Serial.println(F_CPU);
}

void dumpPlatform() {
  Serial.print(F("MCU Platform: "));

#if defined(__AVR_ATmega1280__)
  Serial.println(F("Arduino Mega1280"));
#elif  defined(__AVR_ATmega2560__)
  Serial.println(F("Arduino Mega2560"));
#elif defined(__AVR_AT90USB162__)
  Serial.println(F("Teensy 1.0 / AT90USB162"));
  // Teensy 2.0
#elif defined(__AVR_ATmega32U4__)
  Serial.println(F("Arduino Leonardo / Yun / Teensy 1.0 / ATmega32U4"));
#elif defined(__MK20DX128__) || defined(__MK20DX256__)
  Serial.println(F("Teensy 3.0 / Teensy 3.1 / MK20DX128 / MK20DX256"));
#elif defined(__MKL26Z64__)
  Serial.println(F("Teensy-LC / MKL26Z64"));
#elif defined(__AVR_AT90USB646__)
  Serial.println(F("Teensy++ 1.0 / AT90USB646"));
#elif defined(__AVR_AT90USB1286__)
  Serial.println(F("Teensy++ 2.0 / AT90USB1286"));
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
  Serial.println(F("ATmega1284"));
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__)
  Serial.println(F("ATmega644"));
#elif defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324PA__)
  Serial.println(F("ATmega324")); 
#elif defined(__AVR_ATmega164A__) || defined(__AVR_ATmega164P__)
  Serial.println(F("ATmega164"));
#elif defined(__AVR_ATmega128__)
  Serial.println(F("ATmega128"));
#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega88P__)
  Serial.println(F("ATmega88"));  
#elif defined(__AVR_ATmega64__)
  Serial.println(F("ATmega64"));
#elif defined(__AVR_ATmega48__) || defined(__AVR_ATmega48P__)
  Serial.println(F("ATmega48"));
#elif defined(__AVR_ATmega32__)
  Serial.println(F("ATmega32"));  
#elif defined(__AVR_ATmega16__)
  Serial.println(F("ATmega16"));
#elif defined(__AVR_ATmega8535__)
  Serial.println(F("ATmega8535"));  
#elif defined(__AVR_ATmega8__)
  Serial.println(F("Atmega8"));
#elif defined(__AVR_ATtiny84__)
  Serial.println(F("ATtiny84"));
#elif defined(__AVR_ATtiny85__)
  Serial.println(F("ATtiny85"));
#else
  Serial.println(F("ATmega328(P) / (Duemilanove, Diecimila, LilyPad, Mini, Micro, Fio, Nano, etc)"));
#endif
}

void dumpPulseParams() {
  Serial.print(F("Mark Excess: ")); Serial.print(MARK_EXCESS);; Serial.println(F(" uSecs"));
  Serial.print(F("Microseconds per tick: ")); Serial.print(USECPERTICK);; Serial.println(F(" uSecs"));
  Serial.print(F("Measurement tolerance: ")); Serial.print(TOLERANCE); Serial.println(F("%"));
}

void dumpSignalParams() {
  Serial.print(F("Minimum Gap between IR Signals: ")); Serial.print(_GAP); Serial.println(F(" uSecs"));
}

void dumpDebugMode() {
  Serial.print(F("Debug Mode: "));
#if DEBUG
  Serial.println(F("ON"));
#else
  Serial.println(F("OFF (Normal)"));
#endif

}

void dumpArduinoIDE() {
  Serial.print(F("Arduino IDE version: "));
  Serial.print(ARDUINO / 10000);
  Serial.write('.');
  Serial.print((ARDUINO % 10000) / 100);
  Serial.write('.');
  Serial.println(ARDUINO % 100);
}

void dumpProtocols() {

  Serial.println(); Serial.print(F("IR PROTOCOLS  "));  Serial.print(F("SEND     "));  Serial.println(F("DECODE"));
  Serial.print(F("============= "));  Serial.print(F("======== "));  Serial.println(F("========"));
  Serial.print(F("RC5:          "));  printSendEnabled(SEND_RC5);  printDecodeEnabled(DECODE_RC6);
  Serial.print(F("RC6:          "));  printSendEnabled(SEND_RC6);  printDecodeEnabled(DECODE_RC5);
  Serial.print(F("NEC:          "));  printSendEnabled(SEND_NEC);  printDecodeEnabled(DECODE_NEC);
  Serial.print(F("SONY:         "));  printSendEnabled(SEND_SONY);  printDecodeEnabled(DECODE_SONY);
  Serial.print(F("PANASONIC:    "));  printSendEnabled(SEND_PANASONIC);  printDecodeEnabled(DECODE_PANASONIC);
  Serial.print(F("JVC:          "));  printSendEnabled(SEND_JVC);  printDecodeEnabled(DECODE_JVC);
  Serial.print(F("SAMSUNG:      "));  printSendEnabled(SEND_SAMSUNG);  printDecodeEnabled(DECODE_SAMSUNG);
  Serial.print(F("WHYNTER:      "));  printSendEnabled(SEND_WHYNTER);  printDecodeEnabled(DECODE_WHYNTER);
  Serial.print(F("AIWA_RC_T501: "));  printSendEnabled(SEND_AIWA_RC_T501);  printDecodeEnabled(DECODE_AIWA_RC_T501);
  Serial.print(F("LG:           "));  printSendEnabled(SEND_LG);  printDecodeEnabled(DECODE_LG);
  Serial.print(F("SANYO:        "));  printSendEnabled(SEND_SANYO);  printDecodeEnabled(DECODE_SANYO);
  Serial.print(F("MITSUBISHI:   "));  printSendEnabled(SEND_MITSUBISHI);  printDecodeEnabled(DECODE_MITSUBISHI);
  Serial.print(F("DISH:         "));  printSendEnabled(SEND_DISH);  printDecodeEnabled(DECODE_DISH);
  Serial.print(F("SHARP:        "));  printSendEnabled(SEND_SHARP);  printDecodeEnabled(DECODE_SHARP);
  Serial.print(F("DENON:        "));  printSendEnabled(SEND_DENON);  printDecodeEnabled(DECODE_DENON);
  Serial.print(F("PRONTO:       "));  printSendEnabled(SEND_PRONTO);  Serial.println(F("(Not Applicable)"));
}

void printSendEnabled(int flag) {
  if (flag) {
    Serial.print(F("Enabled  "));
  }
  else {
    Serial.print(F("Disabled "));
  }
}

void printDecodeEnabled(int flag) {
  if (flag) {
    Serial.println(F("Enabled"));
  }
  else {
    Serial.println(F("Disabled"));
  }
}

void dumpHeader() {
  Serial.println(F("IRremoteInfo - by AnalysIR (http://www.AnalysIR.com/)"));
  Serial.println(F("             - A helper sketch to assist in troubleshooting issues with the library by reviewing the settings within the IRremote library"));
  Serial.println(F("             - Prints out the important settings within the library, which can be configured to suit the many supported platforms"));
  Serial.println(F("             - When seeking on-line support, please post or upload the output of this sketch, where appropriate"));
  Serial.println();
  Serial.println(F("IRremote Library Settings"));
  Serial.println(F("========================="));
}

void dumpFooter() {
  Serial.println();
  Serial.println(F("Notes: "));
  Serial.println(F("     - Most of the seetings above can be configured in the following files included as part of the library"));
  Serial.println(F("     - IRremteInt.h"));
  Serial.println(F("     - IRremote.h"));
  Serial.println(F("     - You can save SRAM by disabling the Decode or Send features for any protocol (Near the top of IRremoteInt.h)"));
  Serial.println(F("     - Some Timer conflicts, with other libraries, can be easily resolved by configuring a differnt Timer for your platform"));
}
