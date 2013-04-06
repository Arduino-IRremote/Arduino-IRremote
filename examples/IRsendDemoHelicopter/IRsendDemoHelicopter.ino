/*
* IRremote: IRsendDemoHelicopter - demonstrates sending IR codes Remote Controlled Helicopters
* This code is a proof concept by emulating each of the three 3.5ch IR helicopters Remote transmitter.
* Both the real helicopter or IRrecvDump.ino should recieve signals, decode and either display or react correspondingly.
* Major components of the software layout is: 
*  1) Setup 
*     of Arduino Pins.
*     Model and Channel for selected device.
*  2) Reading of Throttle and other Analog inputs.
*  3) Convert the input from the common console to model specific formats.
*  4) transmitting the corresponding formatted signal
* 
* Much of the common console's signals are converted to the corresponding formats
* Which have been done in easy to read subroutines.
* This allows individual users to hack out un-wanted devices, and or alter the console interface.

* a few items worth noting.
* The remotes begin transmitting only when the throttle is above a minimum threshold.
* Each model and selected channel auto resends the IR signal, as to constantly update the helicopter.
* Each model's auto resend period is different based on channel as to provide a work-a-round for collisions. 
*  This is a poor mans pseudo fall back. But works by brut force.
* Each model of helicoptor's receiver has a unique HOLD time. 
*   As to remember the last received IR signal, until the hold time expires.
* When the throttle goes below the mininum each model will keep sending the IR (equivalent of OFF) for specified period.
*  This ensure prompt shut down. Otherwise it would be possible to drop throttle and not send the Zero Throttle.
*  Causing the Helicopter to remain with last value until hold time expires.
* 
* Where the below code emulates the above behavior of each transmitter remote as found to behave.
* 
* Hardware Requirements:
* An IR LED must be connected to Arduino PWM pin 3.
* Four Potentiometers connected to Analog inputs are used for Throttle, Pitch, Yaw and Trim
* Some Helicoptors have buttons for weapons and such, which the require digital inputs. 
* The analog and digital inputs are defined below. See Used Pins section.
* Selection of Model and Channel can be accomplished one of three possible ways, based on selecting appropiate #if(1).
* 1st method is just to hard code.
* 2nd method is to prompt from Serial Monitor Port
* 3rd method is to determine based off of Digital IO pins.

* Version 0.1 Feb, 2013
* Copyright 2012
*/

#include <IRremote.h>

// enable Serial Debug Prints
#define DEBUG

//Used Pins
// note pin 3 is used by output LED
// avoid pin 11 as it may used by input LED
#define analogInThrottle  A0
#define analogInPitch     A1
#define analogInYaw       A2
#define analogInTrim      A3 // may not be used with SymaR3
#define DigitalInLbutton  5
#define DigitalInRbutton  4
#define DigitalInTurbo    2
// optionally used Pins
#define DigitalInModel0   9
#define DigitalInModel1   8
#define DigitalInChannel0 7
#define DigitalInChannel1 6 // Useries only

//list of different models
#define _SYMAR5   0
#define _SYMAR3   1
#define _USERIES  2
#define _FASTLANE 3

// selected model to use when hard code selection method is defined below
#define _JAMMED_MODEL _SYMAR5
#define _JAMMED_CHANNEL 0

// Throttle time to keep sending off.
// As helicoptor will keep last known value for a timeout before quitting.
#define THROTTLE_HOLD_TIME_SYMA 500
#define THROTTLE_HOLD_TIME_FASTLANE 500
#define THROTTLE_HOLD_TIME_USERIES 3000

// some reason can not include IRremoteInt.h without being in DEBUG mode.
// So these are defined here, again.
#define SYMA_UPDATE_PERIOD_CH_A 120 // 0
#define SYMA_UPDATE_PERIOD_CH_B 180 // 1
#define USERIES_UPDATE_PERIOD_CH_A 150 // 1
#define USERIES_UPDATE_PERIOD_CH_B 110 // 2
#define USERIES_UPDATE_PERIOD_CH_C 190 // 0
#define FASTLANE_UPDATE_PERIOD_CH_A 140 // 2
#define FASTLANE_UPDATE_PERIOD_CH_B 180 // 1
#define FASTLANE_UPDATE_PERIOD_CH_C 220 // 0

//Global attributes
IRsend irsend;
int8_t model;
int8_t channel;
int32_t current_millis;
int32_t prev_millis;
int32_t last_throttle_millis;
int32_t throttle_hold_time;
int32_t update_period;
union helicopter helicopter;

void setup()
{
  Serial.begin(9600);
  // update rate is fast, so 9600 may need to be bumped up,
  // depending on what else you do or added more debug.

  pinMode(analogInThrottle,  INPUT);
  pinMode(analogInPitch,     INPUT);
  pinMode(analogInYaw,       INPUT);
  pinMode(analogInTrim,      INPUT);
  pinMode(DigitalInLbutton,  INPUT_PULLUP);
  pinMode(DigitalInRbutton,  INPUT_PULLUP);
  pinMode(DigitalInTurbo,    INPUT_PULLUP);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // This is alot of prep. Just make Model and Channel selectable.
  #if (1)
  // just jam the values
  model   = _JAMMED_MODEL;
  Serial.print(F("Model = "));
  Serial.println(model, DEC);
  channel = _JAMMED_CHANNEL;
  #elif (0)
  // select based on input pins.
  pinMode(DigitalInModel0,   INPUT_PULLUP);
  pinMode(DigitalInModel1,   INPUT_PULLUP);
  model   = digitalRead(DigitalInModel1)   << 1 || digitalRead(DigitalInModel0);

  pinMode(DigitalInChannel0, INPUT_PULLUP);
  pinMode(DigitalInChannel1, INPUT_PULLUP);
  channel = digitalRead(DigitalInChannel1) << 1 || digitalRead(DigitalInChannel0);
  #else
  // select based on serial input, with rule checks
  model = -1;
  channel = -1;
  Serial.println(F("To begin."));
  while (model < 0) {
    Serial.println(F("Enter Model of Helicopter?"));
    Serial.println(F("0 = SymaR5"));
    Serial.println(F("1 = SymaR3"));
    Serial.println(F("2 = Useries"));
    Serial.println(F("3 = FastLane"));
    while (!Serial.available()) {
      // wait for Serial input
    };
    model = Serial.read() - 0x30; // substract ASCII offset for zero
    if (model > _USERIES) model = -1;
  }
  Serial.print(F("Model = "));
  Serial.println(model, DEC);

  while (channel < 0) {
    Serial.println(F("Enter Model of Helicopter?"));
    Serial.print(F("A, B"));
    if (model == _USERIES) { // accomidate U-Series third channel
      Serial.println(F(", C"));
    } else {
      Serial.println();
    }
    while (!Serial.available()) {
      // wait for Serial input
    };
    channel = (toupper(Serial.read()) - 0x41) ; // substract ASCII offset for "A" or "a"
  } // while channel
  #endif

  #ifdef DEBUG
  Serial.print(F("Channel = "));
  Serial.println(channel, DEC);
  Serial.print(F("update_period = "));
  Serial.println(update_period, DEC);
  #endif

  // determine varianaces based on Model and Channel
  switch (model) {
    case _SYMAR5:
    case _SYMAR3:
      throttle_hold_time = THROTTLE_HOLD_TIME_SYMA;
      switch (channel) {
        case 0:
          update_period = SYMA_UPDATE_PERIOD_CH_A;
        break;
        case 1:
          update_period = SYMA_UPDATE_PERIOD_CH_B;
        break;
      }
    break; //_SYMARx

    case _USERIES:
      throttle_hold_time = THROTTLE_HOLD_TIME_USERIES;
      switch (channel) {
        case 0:
          update_period = USERIES_UPDATE_PERIOD_CH_A;
          channel = 1;
        break;
        case 1:
          update_period = USERIES_UPDATE_PERIOD_CH_B;
          channel = 2;
        break;
        case 2:
          update_period = USERIES_UPDATE_PERIOD_CH_C;
          channel = 0;
        break;
      }
    break; //_USERIES

    case _FASTLANE:
      throttle_hold_time = THROTTLE_HOLD_TIME_FASTLANE;
      switch (channel) {
        case 0:
          update_period = FASTLANE_UPDATE_PERIOD_CH_A;
          channel = 0;
        break;
        case 1:
          update_period = FASTLANE_UPDATE_PERIOD_CH_B;
          channel = 2;
        break;
        case 2:
          update_period = FASTLANE_UPDATE_PERIOD_CH_C;
          channel = 1;
        break;
      }
    break; //_FASTLANE
  }

  // prime periodic update
  prev_millis = millis();
  last_throttle_millis = current_millis - throttle_hold_time;

} // setup()

void loop() {
  uint16_t throttle;
  uint16_t pitch;
  uint16_t yaw;
  uint16_t trim;

  current_millis = millis();

  // check if next interval to transmit IR signal
  if (current_millis - prev_millis >= update_period) {
    prev_millis += update_period; // mark time for next interval.
    helicopter.dword = 0; // blank out un-defined bits.

    // read throttle
    throttle = analogRead(analogInThrottle);

    // is the throttle on or just off, if so begin transmitting
    if ((throttle > (1023*0.05)) || ((current_millis - last_throttle_millis) < throttle_hold_time) ) {
      if (throttle > (1023*0.05)) {
        last_throttle_millis = current_millis; // mark time for throttle timeout
      }
      #ifdef DEBUG
      Serial.print(current_millis, DEC); Serial.print(F("ms"));
      #endif

      // read rest of controls
      pitch = analogRead(analogInPitch);
      yaw   = analogRead(analogInYaw);
      trim  = analogRead(analogInTrim);

      // Ready message by mapping into unions bit structure
      switch (model) {
        case _SYMAR5:
          tx_SymaR5(channel, throttle, pitch, yaw, trim);
        break; // _SYMAR5

        case _SYMAR3:
          tx_SymaR3(channel, throttle, pitch, yaw, trim);
        break; // _SYMAR3

        case _USERIES:
          tx_uSeries(channel, throttle, pitch, yaw, trim);
        break; // _USERIES

        case _FASTLANE:
          tx_FastLane(channel, throttle, pitch, yaw, trim);
        break; // _USERIES
      }

      #ifdef DEBUG
      Serial.print(F(" +0x")); Serial.print(helicopter.dword, HEX);
      Serial.println();
      #endif
    }  // ~if throttle
  } // ~if timer service
} // ~loop()

void tx_SymaR5(uint8_t channel, uint16_t throttle, uint16_t pitch, uint16_t yaw, uint16_t trim) {
  helicopter.symaR5.Channel  = channel;

  // rescale analog input to desired scale for model
  helicopter.symaR5.Throttle = map(throttle, 0, 1023,   0, 127);
  helicopter.symaR5.Pitch    = map(pitch,    0, 1023,   0, 127);
  helicopter.symaR5.Yaw      = map(yaw,      0, 1023, 127,   0);
  helicopter.symaR5.Trim     = map(trim,     0, 1023, 127,   0);

  // send the IR
  irsend.sendSymaR5(helicopter.dword);

  #ifdef DEBUG
  Serial.print(F(" C=")); Serial.print(helicopter.symaR5.Channel,DEC);
  Serial.print(F(" T=")); Serial.print(helicopter.symaR5.Throttle,DEC);
  Serial.print(F(" P=")); Serial.print(helicopter.symaR5.Pitch,DEC);
  Serial.print(F(" Y=")); Serial.print(helicopter.symaR5.Yaw,DEC);
  Serial.print(F(" t=")); Serial.print(helicopter.symaR5.Trim,DEC);
  #endif
}

void tx_SymaR3(uint8_t channel, uint16_t throttle, uint16_t pitch, uint16_t yaw, uint16_t trim) {
  helicopter.symaR3.Channel  = channel;

  // rescale analog input to desired scale for model
  helicopter.symaR3.Throttle = map(throttle, 0, 1023,   0, 127);
  helicopter.symaR3.Pitch    = map(pitch,    0, 1023,   0, 127);
  helicopter.symaR3.Yaw      = map(yaw,      0, 1023, 127,   0);

  // send the IR
  irsend.sendSymaR3(helicopter.dword);

  #ifdef DEBUG
  Serial.print(F(" C=")); Serial.print(helicopter.symaR3.Channel,DEC);
  Serial.print(F(" T=")); Serial.print(helicopter.symaR3.Throttle,DEC);
  Serial.print(F(" P=")); Serial.print(helicopter.symaR3.Pitch,DEC);
  Serial.print(F(" Y=")); Serial.print(helicopter.symaR3.Yaw,DEC);
  #endif
}

void tx_uSeries(uint8_t channel, uint16_t throttle, uint16_t pitch, uint16_t yaw, uint16_t trim) {
  helicopter.uSeries.Channel  = channel;

  // rescale analog input to desired scale for model
  helicopter.uSeries.Throttle = map(throttle, 0, 1023,   0, 127);
  helicopter.uSeries.Pitch    = map(pitch,    0, 1023,   0,  63);
  helicopter.uSeries.Yaw      = map(yaw,      0, 1023,  31,   0);
  helicopter.uSeries.Trim     = map(trim,     0, 1023,  63,   0);

  // read the buttons
  helicopter.uSeries.Turbo    = ~digitalRead(DigitalInTurbo);
  helicopter.uSeries.Lbutton  = ~digitalRead(DigitalInLbutton);
  helicopter.uSeries.Rbutton  = ~digitalRead(DigitalInRbutton);

  // add in calculated parity checksum
  helicopter.uSeries.cksum    = UseriesChecksum(helicopter.dword);

  // send the IR
  irsend.sendUseries(helicopter.dword);

  #ifdef DEBUG
  delay(40);//ms
  Serial.print(F(" C=")); Serial.print(helicopter.uSeries.Channel,DEC);
  Serial.print(F(" T=")); Serial.print(helicopter.uSeries.Throttle,DEC);
  Serial.print(F(" P=")); Serial.print(helicopter.uSeries.Pitch,DEC);
  Serial.print(F(" Y=")); Serial.print(helicopter.uSeries.Yaw,DEC);
  Serial.print(F(" t=")); Serial.print(helicopter.uSeries.Trim,DEC);
  Serial.print(F(" L=")); Serial.print(helicopter.uSeries.Lbutton,DEC);
  Serial.print(F(" R=")); Serial.print(helicopter.uSeries.Rbutton,DEC);
  Serial.print(F(" Turbo=")); Serial.print(helicopter.uSeries.Turbo,DEC);
  Serial.print(F(" cksum=")); Serial.print(helicopter.uSeries.cksum,DEC);
  Serial.flush();
  #endif
}

void tx_FastLane(uint8_t channel, uint16_t throttle, uint16_t pitch, uint16_t yaw, uint16_t trim) {
  helicopter.fastlane.Channel  = channel;

  // rescale analog input to desired scale for model
  helicopter.fastlane.Throttle = map(throttle, 0, 1023,   0, 63);
  helicopter.fastlane.Pitch    = map(pitch,    0, 1023,   0, 15);

  // convert scaler analog input for Yaw to two component: direction bit and magnitude.
  #define YAW_SIZE 32 // input bit size of analog after mapping
  yaw = map(yaw, 0, 1023, (YAW_SIZE-1), 0);
  helicopter.fastlane.Yaw_dir = (yaw & (YAW_SIZE/2))>0?0:1;
  yaw = yaw & ~(YAW_SIZE/2); // strip off MSB, leaving the magnitude
  if (helicopter.fastlane.Yaw_dir == 0) {
    helicopter.fastlane.Yaw = yaw; // if not reverse direction simply use magnitude.
  }
  else {
    helicopter.fastlane.Yaw = (YAW_SIZE/2) - yaw; //otherwise convert to reverse direction
  }

  // convert scaler analog input for Trim to two component: direction bit and magnitude.
  #define TRIM_SIZE 32 // input bit size of analog after mapping
  trim = map(trim,      0, 1023,  (TRIM_SIZE - 1),   0);
  helicopter.fastlane.Trim_dir  = (trim & (TRIM_SIZE/2))>0?0:1;
  trim = trim & ~(TRIM_SIZE/2); // strip off MSB, leaving the magnitude
  if (helicopter.fastlane.Trim_dir == 0) {
    helicopter.fastlane.Trim = trim; // if not reverse direction simply use magnitude.
  }
  else {
    helicopter.fastlane.Trim = (TRIM_SIZE/2) - trim;//otherwise convert to reverse direction
  }
  // read the buttons
  helicopter.fastlane.Fire = ~digitalRead(DigitalInTurbo);

  // send the IR
  irsend.sendFastLane(helicopter.dword);

  #ifdef DEBUG
  delay(40);//ms
  Serial.print(F(" C=")); Serial.print(helicopter.fastlane.Channel,DEC);
  Serial.print(F(" T=")); Serial.print(helicopter.fastlane.Throttle,DEC);
  Serial.print(F(" P=")); Serial.print(helicopter.fastlane.Pitch,DEC);
  Serial.print(F(" Yd=")); Serial.print(helicopter.fastlane.Yaw_dir,DEC);
  Serial.print(F(" Y=")); Serial.print(helicopter.fastlane.Yaw,DEC);
  Serial.print(F(" td=")); Serial.print(helicopter.fastlane.Trim_dir,DEC);
  Serial.print(F(" t=")); Serial.print(helicopter.fastlane.Trim,DEC);
  Serial.print(F(" F=")); Serial.print(helicopter.fastlane.Fire,DEC);
  Serial.flush();
  #endif
}