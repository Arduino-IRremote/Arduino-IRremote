#include <ArduinoLowPower.h>

// Arduino SigFox for MKRFox1200 - Version: Latest 
#include <SigFox.h>

#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

bool fet1State = false;
bool fet2State = false;
bool fet3State = false;
bool outputRelay = false;
bool chargeRelay = false;
bool oshLed = false;

int loopCounter  =0;
bool debug = true;

// First run request downlink data
// from the Sigfox backend.
bool requestDownlinkData = true;

// status::uint:8 version::uint:8 temperature::int:8 current::int:16 voltage::uint:16 humidity::int:16 light::uint:8 light2::uint:8 lastStatus::uint:8
typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t status;     // status::uint:8 -> Split to 8 bits
  uint8_t version;    // version::uint:8
  int16_t temperature; // temperature::int:16:little-endian
  int16_t current;    // current::int:16:little-endian (e.g. 1159 mA -> 0487 but stored as 8704)
  uint16_t voltage;   // voltage::uint:16:little-endian
  int16_t currentOffCharge;   // currentOffCharge::int:16:little-endian
  uint16_t voltageOffCharge;   // voltageOffCharge::uint:16:little-endian
} SigfoxMessage;

// stub for message which will be sent
SigfoxMessage msg;

void setup() {
  pinMode(0, OUTPUT); // led
  pinMode(1, OUTPUT); // Fet1
  pinMode(2, OUTPUT); // Fet2
  pinMode(3, OUTPUT); // Fet3
  pinMode(4, OUTPUT); // charge relay FIN
  pinMode(5, OUTPUT); // charge relay RIN
  pinMode(6, OUTPUT); // output relay FIN
  pinMode(7, OUTPUT); // output relay RIN
  
  setFetOutput(1, fet1State);
  setFetOutput(2, fet2State);
  setFetOutput(3, fet3State);
  // On to indicate in setup
  setOshLed(true);
  
  // Intialize the relays to off
  // whilst in set-up. Force state so we know
  // the state their in and can then track it.
  setChargeRelay(false, true);
  setOutputRelay(false, true);
  
  //Initialize serial and wait for port to open:
  if (debug) {
    Serial.begin(9600);
  }
     
  // Initialize the INA219.
  // By default to (32V, 2A).  
  ina219.begin();
 
  setupSigfox();  

  if (debug) {
    Serial.println("System Monitor Setup Complete... V0.0.4");
  }
  
  // turn off the OSH Led now set-up is complete.
  setOshLed(false);
}

void setupSigfox() {
  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }
  
  // Enable debug led and disable automatic deep sleep
  // Comment this line when shipping your project :)
  if (debug) {
    SigFox.debug();
  }

  // Output the ID and PAC needed to register the 
  // device at the Sigfox backend.
  String version = SigFox.SigVersion();
  String ID = SigFox.ID();
  String PAC = SigFox.PAC();

  // Display module informations
  Serial.println("MKRFox1200 Sigfox first configuration");
  Serial.println("SigFox FW version " + version);
  Serial.println("ID  = " + ID);
  Serial.println("PAC = " + PAC);

  Serial.println("");

  Serial.print("Module temperature: ");
  Serial.println(SigFox.internalTemperature());

  Serial.println("Register your board on https://backend.sigfox.com/activate with provided ID and PAC");

  delay(100);

  // Send the module to the deepest sleep
  SigFox.end();
}

void loop(void) 
{ 
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  
  // V and I when not charging
  // to give an idea of the state of the battery
  float current_off_charge_mA = 0;
  float busvoltage_off_charge = 0;
 
  // Ensure output relay and charge relay are on by default
  // now that we're monitoring.
  setOutputRelay(true, false);
  setChargeRelay(true, false);
     
  // Flip the LED to show we're doing stuff.
  setOshLed(true);

  // Meaure normal running voltage/current.
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
    
  if (debug) {    
    Serial.println("-------------------------------------------------------");
    Serial.println("Charger connected: ");    
    Serial.print("Bus Voltage:   "); 
    Serial.print(busvoltage); 
    Serial.println(" V");
    
    Serial.print("Current:       "); 
    Serial.print(current_mA); 
    Serial.println(" mA");
    Serial.println("");
    
    Serial.print("Loop counter:  "); 
    Serial.print(loopCounter); 
    Serial.println("");
    
    Serial.println("");
  }
          
  // Flip the LED to show we're doing stuff.
  setOshLed(false);
       
  // 6 (loops) * 10s (delay) gives approx 1 minute 
  // so send an update to Sigfox rather than (when debugging)
  // having to wait the full 15 minutes for a transmission.
  if (loopCounter == 12) {

    // TODO: Don't disable charge relay when voltage is low
    // as we might not be able to re-enable it.
    //
    // Once every Sigfox transmit cycle, disconnect the charger
    // to measure the raw battery voltage and actual load current
    // (otherwise load current excludes current from the charger).
    setChargeRelay(false, false);
    // Little delay to let the battery chemistry settle a little.
    delay(2000);
    current_off_charge_mA = ina219.getCurrent_mA();
    busvoltage_off_charge = ina219.getBusVoltage_V();
    setChargeRelay(true, false);  

    if (debug) {
      Serial.println("Charger disconnected: ");
      Serial.print("Charge Off Voltage:  "); 
      Serial.print(busvoltage_off_charge); 
      Serial.println(" V");
      
      Serial.print("Charge Off Current: "); 
      Serial.print(current_off_charge_mA); 
      Serial.println(" mA");
      
      Serial.println("");
    }
  
    // remove once we have the device details
    //setupSigfox();
    sendToSigfox(busvoltage, current_mA, busvoltage_off_charge, current_off_charge_mA);
  }

  // Reset the loop counter after (hopefully) 15 minutes.
  if (loopCounter > 6 * 15) {
    loopCounter = 0;
  }

  // Monitor every 10s. (2s delay in relay clickyness).
  delay(8000);
  //LowPower.sleep(8 * 1000);
  loopCounter++;
}

void setOshLed(bool state) {
  //Serial.println("Setting OSH LED");
  digitalWrite(0, state);
  oshLed = state;
}

void setOutputRelay(bool state, bool force) {
  
  // Unless forced, ignore instruction if relay already in correct state
  // to save power.
  if (!force) {
    if (outputRelay == state) {
      return;
    }
  }

  if (debug) {    
    Serial.print("Setting output relay: ");
    Serial.println(state);
  }
  
  digitalWrite(6, state);
  digitalWrite(7, !state);
  outputRelay = state;
  
  delay(100);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
}

void setChargeRelay(bool state, bool force) {
  // Unless forced, ignore instruction if relay already in correct state
  // to save power.
  if (!force) {
    if (chargeRelay == state) {
      return;
    }
  }

  if (debug) {    
    Serial.print("Setting charge relay: ");
    Serial.println(state);
  }
  
  // FIN/RIN order swapped on charge compared to output.
  digitalWrite(5, state);
  digitalWrite(4, !state);
  chargeRelay = state;
  
  delay(100);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
}

void setFetOutput(int fet, bool state) {
  if (debug) {    
    Serial.print("Setting FET: ");
    Serial.print(fet);
    Serial.print(" State: ");
    Serial.println(state);
  }
  
  switch (fet) {
    case 1:
      fet1State = state;
    case 2:
      fet2State = state;
    case 3:
      fet3State = state;
    default:
      // Invalid fet!
      return;
  }
  
  digitalWrite(fet, state);
}

// B7: Battery Flat
// B6: Charge Relay state
// B5: Charging
// B4: Reserved
// B3: Output relay state
// B2: Fet 1 state
// B1: Fet 2 state
// B0: Fet 3 state
byte getStatusFlags(float voltage, float current_mA, float current_charger_off_mA) {
  byte status = 0;
  
  // Upper Nibble (Charging/Battery)
  // Battery flat
  if (voltage < 10.5) {
    status |= 0x80; // 1000 0000
  }

  // Charger relay status
  if (chargeRelay) {
    status |= 0x40; // 0100 0000
  }

  // +ve current flow to the battery means it's charging.
  if (current_mA > 0) {
    status |= 0x20; // 0010 0000
  }

  // Not sure what this means...
  if (current_charger_off_mA > 0) {
    status |= 0x10; // 0001 0000
  }

  // Lower Nibble (Outputs)
  // Output relay enabled.
  if (outputRelay) {
    status |= 0x08; // 0000 1000
  }

  if (fet1State) {
    status |= 0x04; // 0000 0100
  }

  if (fet2State) {
    status |= 0x02; // 0000 0010
  }

  if (fet3State) {
    status |= 0x01; // 0000 0001
  }

  return status;
}

void sendToSigfox(float busvoltage, float current_mA, float busvoltage_off_charge, float current_charger_off_mA) {
  if (debug) {    
    if (requestDownlinkData) { 
      Serial.println("**** Sending to Sigfox with downlink *****");
    } else {
      Serial.println("**** Sending to Sigfox *****");
    }
  }
  SigFox.begin();

  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  // Bytes 0 and 1
  msg.status = getStatusFlags(busvoltage_off_charge, current_mA, current_charger_off_mA);
  msg.version = 3;
  
  msg.temperature = (int16_t)(SigFox.internalTemperature() * 10);
  
  // Charger connected
  // 2000mA is max for PCB (32768 max for signed int. i.e. +/-32 Amps)
  msg.current = (int16_t)(current_mA); 
  // 12.13 -> 1213 (65535 max i.e. 655 Volts)  
  msg.voltage = (uint16_t)(busvoltage * 100); 
  
  // Charger Off (Isolated).
  msg.currentOffCharge = (int16_t)(current_charger_off_mA); 
  msg.voltageOffCharge = (uint16_t)(busvoltage_off_charge * 100);

  Serial.println("Sending packet...");
  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 12);
  // endPacket actually sends the data.
  uint8_t lastMessageStatus = SigFox.endPacket(requestDownlinkData);
  
  if (requestDownlinkData) {
    parseDownlinkData(lastMessageStatus);
  }
  
  SigFox.end();
  
  // Ensure downliad link is cleared after sending to Sigfox.
  // TODO: Re-enable this every 6 hours (Max 4x per 24 hours).
  requestDownlinkData = false;
  
  if (debug) {    
    Serial.println("Status: " + String(lastMessageStatus));
  
    if (lastMessageStatus > 0) {
      Serial.println("No transmission!");
    }  
  }
}

void parseDownlinkData(uint8_t lastMessageStatus) {
  if (lastMessageStatus > 0) {
    Serial.println("No transmission. Status: " + String(lastMessageStatus));
    return;
  }

  // Max response size is 8 bytes
  // set-up a empty buffer to store this. (0x00 == no action for us.)
  // each byte has meaning for control.
  uint8_t response[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  // Response packet needs parsePacket to be called
  // before being read.
  if (SigFox.parsePacket()) {
    
    Serial.println("Response from server:");
    int i = 0;
    while (SigFox.available()) {
      Serial.print("0x");
      int readValue = SigFox.read();
      Serial.println(readValue, HEX);
      response[i] = (uint8_t)readValue;
      i++;
    }

    // byte 0 - relay control.
    parseRelayControl(response[0]);
    // byte 1 - FETs control
    parseFETControl(response[1]);
    // byte 2 - GPIO output?
    
  } else {
    Serial.println("No response from server");
  }
  Serial.println();
}

void parseRelayControl(uint8_t value) {
  // TODO: implement.
  // 00 : Ignore
  // 01 : Off
  // 10 : on
  // 11 : invalid
  // Bits 7 & 6 = > Output relay
  // Bits 5 & 4 = > Charge relay
  // Bits 3 & 2 = > Ignored
  // Bits 1 & 0 = > Ignored
  Serial.println("Setting relays from:");
  Serial.println(value, HEX);
  Serial.println("");
}

void parseFETControl(uint8_t value) {
  // TODO: Implement.
  // 00 : Ignore
  // 01 : Off
  // 10 : on
  // 11 : invalid
  // Bits 7 & 6 = > Fet 1
  // Bits 5 & 4 = > Fet 2
  // Bits 3 & 2 = > Fet 3
  // Bits 1 & 0 = > Ignored
  Serial.println("Setting FET outputs from:");
  Serial.println(value, HEX);
  Serial.println("");
