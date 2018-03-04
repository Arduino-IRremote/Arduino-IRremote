/*
 * IRinterfaceAC
 * Version 1.0 March, 2018
 * by Marcelo Buregio
 *
 * This is an interface for IRremote library with AC add-on to use Air Conditioner that uses 48-bit protocol.
 * Tested successfully on Midea, Electrolux and Hitachi (some).
 *
 * This library needs to be used with IRremote 2.x version by Marcelo Buregio
 * This version can be downloaded on https://github.com/marceloburegio/Arduino-IRremote
 *
 * For more details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 */
#ifndef IRinterfaceAC_h
#define IRinterfaceAC_h

// Operating mode
#define AC_MODE_AUTO 0x00
#define AC_MODE_COOL 0x01
#define AC_MODE_DRY  0x02
#define AC_MODE_HEAT 0x03
#define AC_MODE_FAN  0x04

// Fan Speed
#define AC_FAN_AUTO  0xBF
#define AC_FAN_LOW   0x9F
#define AC_FAN_MED   0x5F
#define AC_FAN_HIGH  0x3F

// Follow Me Mode
#define AC_FOLLOW_DISABLE 0x09

// Including IRremoteAC library and constants
#include "IRremote.h"

// Init add-on IRsendAC
IRsend irsend;

// main class for receiving IR
class IRinterfaceAC
{
public:
	// Default values
	uint8_t mode = AC_MODE_COOL;
	uint8_t temp = 0xC0; // Temp = 25c
	uint8_t fan  = AC_FAN_MED;
	bool flagFollowMe = false;
	bool flagTurnOn   = false;
	
	IRinterfaceAC() {}

	void setTemperature(uint8_t temp1) {
		switch (temp1) {
			case 17 : temp = 0x00; break;
			case 18 : temp = 0x10; break;
			case 19 : temp = 0x30; break;
			case 20 : temp = 0x20; break;
			case 21 : temp = 0x60; break;
			case 22 : temp = 0x70; break;
			case 23 : temp = 0x50; break;
			case 24 : temp = 0x40; break;
			case 25 : temp = 0xC0; break;
			case 26 : temp = 0xD0; break;
			case 27 : temp = 0x90; break;
			case 28 : temp = 0x80; break;
			case 29 : temp = 0xA0; break;
			case 30 : temp = 0xB0; break;
		}
	}

	uint8_t getTemperature() {
		switch (temp) {
			case 0x00 : return 17;
			case 0x10 : return 18;
			case 0x30 : return 19;
			case 0x20 : return 20;
			case 0x60 : return 21;
			case 0x70 : return 22;
			case 0x50 : return 23;
			case 0x40 : return 24;
			case 0xC0 : return 25;
			case 0xD0 : return 26;
			case 0x90 : return 27;
			case 0x80 : return 28;
			case 0xA0 : return 29;
			case 0xB0 : return 30;
		}
	}

	void setFanSpeed(uint8_t fan1) {
		switch (fan1) {
			case AC_FAN_AUTO : 
			case AC_FAN_LOW  : 
			case AC_FAN_MED  : 
			case AC_FAN_HIGH : fan = fan1; break;
		}
	}

	uint8_t getFanSpeed() {
		return fan;
	}

	void setMode(uint8_t mode1) {
		switch (mode1) {
			case AC_MODE_AUTO : 
			case AC_MODE_COOL : 
			case AC_MODE_DRY  : 
			case AC_MODE_HEAT :
			case AC_MODE_FAN  : 
				flagFollowMe = false;
				mode = mode1;
				break;
		}
	}

	uint8_t getMode() {
		return mode;
	}

	// Can be used with FollowMe function active.
	void send() {
		uint8_t b1 = 0xB2;
		uint8_t b2 = 0x1F;
		uint8_t b3 = temp;
		switch (mode) {
			case AC_MODE_AUTO : // FAN = Disabled / Temp = Enabled
				b3 |= 0x08;
				break;
			case AC_MODE_COOL : // FAN = Enabled  / Temp = Enabled
				b2 = fan;
				break;
			case AC_MODE_DRY :  // FAN = Disabled / Temp = Enabled
				b3 |= 0x04;
				break;
			case AC_MODE_HEAT : // FAN = Enabled  / Temp = Enabled
				b2 = fan;
				b3 |= 0x0C;
				break;
			case AC_MODE_FAN :  // FAN = Enabled  / Temp = Disabled
				b2 = fan;
				b3 = 0xE4;
				break;
			default : return;
		}
		flagTurnOn = true;
		convertAndSend(b1, b2, b3);
	}

	void turnOn() {
		send();
	}

	void turnOff() {
		flagTurnOn = false;
		flagFollowMe = false;
		convertAndSend(0xB2, 0x7B, 0xE0);
	}

	bool getTurnOnStatus() {
		return flagTurnOn;
	}

	void swing() {
		convertAndSend(0xB2, 0x6B, 0xE0);
	}

	void airDirection() {
		convertAndSend(0xB2, 0x0F, 0xE0);
	}

	void display() {
		convertAndSend(0xB5, 0xF5, 0xA5);
	}

	void turbo() {
		convertAndSend(0xB5, 0xF5, 0xA2);
	}

	void economicRunning() {
		convertAndSend(0xB2, 0xE0, 0x03);
	}

	bool activeFollowMe(uint8_t currentTemp) {
		bool flagActiveFollowMe = sendFollowMe(mode, currentTemp, false);
		if (flagActiveFollowMe) flagFollowMe = true;
		return flagActiveFollowMe;
	}

	bool deactiveFollowMe(uint8_t currentTemp) {
		bool flagDeactiveFollowMe = sendFollowMe(AC_FOLLOW_DISABLE, currentTemp, false);
		if (flagDeactiveFollowMe) flagFollowMe = false;
		return flagDeactiveFollowMe;
	}

	bool sendStatusFollowMe(uint8_t currentTemp) {
		return sendFollowMe(mode, currentTemp, true);
	}

	bool getFollowMeStatus() {
		return flagFollowMe;
	}

	bool sendFollowMe(uint8_t followMode, uint8_t currentTemp, bool flagStatus) {
		uint8_t b1 = 0xBA;
		uint8_t b2 = currentTemp;
		uint8_t b3;
		if (currentTemp < 0 || currentTemp > 50) return false;
		switch (followMode) {
			case AC_MODE_AUTO :
				b3 = 0x0A;
				break;
			case AC_MODE_COOL :
				b3 = 0x02;
				break;
			case AC_FOLLOW_DISABLE :
				b3 = 0x06;
				break;
			default : return false;
		}
		if (flagStatus) b2 |= 0xC0;
		else b2 |= 0x40;
		b3 |= temp;
		convertAndSend(b1, b2, b3);
		return true;
	}

private:
	// Convert and Sending AC code...
	bool convertAndSend(unsigned int b1, unsigned int b2, unsigned int b3) {
		b1 = (b1 << 8) | (b1 ^ 0xFF);
		b2 = (b2 << 8) | (b2 ^ 0xFF);
		b3 = (b3 << 8) | (b3 ^ 0xFF);
		irsend.sendAC(b1, (((unsigned long) b2) << 16) | b3);
	}
};

#endif