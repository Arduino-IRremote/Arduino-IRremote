/*
 * ir_Xiomi.cpp
 *
 *  Contains functions for receiving and sending Xiomi IR Protocol in "raw" and 
 *  standard format with 12 address and 8 bit command.
 *  This is a remote control that also has the BLE function.
 *
 *  Copyright (C) 2021  Like Pi Domotica
 *  humbertokramm@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2017-2021 Like Pi Domotica
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */

//#define DEBUG // Activate this for lots of lovely debug output.
#include "IRremoteInt.h"

//#define SEND_XIOMI  1 // for testing
//#define DECODE_XIOMI  1 // for testing
//==============================================================================
//                XX    XX IIIII  OOOOO  MM    MM IIIII 
//                 XX  XX   III  OO   OO MMM  MMM  III  
//                  XXXX    III  OO   OO MM MM MM  III  
//                 XX  XX   III  OO   OO MM    MM  III  
//                XX    XX IIIII  OOOO0  MM    MM IIIII 
//==============================================================================
// see: https://www....

// LSB first, 1 start bit + 16 bit address + 8 bit command + 1 stop bit.
#define XIOMI_ADDRESS_BITS			12 // 16 bit address
#define XIOMI_COMMAND_BITS			8 // Command

#define XIOMI_BITS					(XIOMI_ADDRESS_BITS + XIOMI_COMMAND_BITS) // The number of bits in the protocol
#define XIOMI_UNIT					65

#define XIOMI_HEADER_MARK			(16 * XIOMI_UNIT) // The length of the Header:Mark
#define XIOMI_HEADER_SPACE			(8 * XIOMI_UNIT)  // The length of the Header:Space

#define XIOMI_BIT_MARK				XIOMI_UNIT		// The length of a Bit:Mark
#define XIOMI_MIN_SPACE_MARK		300
#define XIOMI_MAX_SPACE_MARK		1800


#define XIOMI_REPEAT_SPACE			13350


#define XIOMI_QUEBRA_1			750//711
#define XIOMI_QUEBRA_2			1000//948
#define XIOMI_QUEBRA_3			1300//1287



/*
//		TECLA		ADDR	CMD
#define	TCL_POWER	0x3CC	0xCF
#define	TCL_DOWN	0x490	0x94
#define	TCL_RIGHT	0x490	0xC1
#define	TCL_PLUS	0x490	0xD0
#define	TCL_HOME	0x490	0x49
#define	TCL_MENU	0x490	0x85
#define	TCL_UP		0x490	0xA7
#define	TCL_LEFT	0x490	0x7A
#define	TCL_ESC		0x490	0xB6
#define	TCL_CENTER	0x490	0xE3
#define	TCL_MINUS	0x490	0xF2
*/

//+========================================================
// MSB First
//+========================================================
bool IRrecv::decodePulseDistanceDataXiomi(){
	uint8_t len;
	uint32_t tDecodedData = 0, temp,value;
	len = IrReceiver.decodedIRData.rawDataPtr->rawlen;
	
	for (uint16_t i = len-1; i > 3; i--) {
		temp = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i]* MICROS_PER_TICK;
		if((temp < XIOMI_MIN_SPACE_MARK) || (temp > XIOMI_MAX_SPACE_MARK)) {
			DBG_PRINT("erro: ");
			DBG_PRINT(i);
			DBG_PRINT(" - ");
			DBG_PRINTLN(temp);
			return false;
		}
		if(!(i&1)) {
			DBG_PRINT(i);
			DBG_PRINT("\t");
			DBG_PRINT(temp);
			DBG_PRINT("\t");
			if		(temp >= XIOMI_QUEBRA_3)	value = 0b11;
			else if	(temp >= XIOMI_QUEBRA_2)	value = 0b01;
			else if	(temp >= XIOMI_QUEBRA_1)	value = 0b10;
			else						value = 0b00;
			tDecodedData |= value << len - i-2;
		}
	}
	decodedIRData.decodedRawData = tDecodedData;
	return true;
}

//+=============================================================================
//
void IRsend::sendXiomi(uint16_t aAddress, uint8_t aCommand, uint_fast8_t aNumberOfRepeats) {
	// Set IR carrier frequency
	enableIROut(37); // 36.7kHz is the correct frequency

	uint_fast8_t tNumberOfCommands = aNumberOfRepeats + 1;
	while (tNumberOfCommands > 0) {

		noInterrupts();

		// Header
		mark(XIOMI_HEADER_MARK);
		space(XIOMI_HEADER_SPACE);

		// Address (device and subdevice)
		//sendPulseDistanceWidthData(XIOMI_BIT_MARK, XIOMI_ONE_SPACE, XIOMI_BIT_MARK, XIOMI_ZERO_SPACE, aAddress,
		//XIOMI_ADDRESS_BITS, PROTOCOL_IS_LSB_FIRST); // false -> LSB first

		// Command + stop bit
		//sendPulseDistanceWidthData(XIOMI_BIT_MARK, XIOMI_ONE_SPACE, XIOMI_BIT_MARK, XIOMI_ZERO_SPACE, aCommand,
		//XIOMI_COMMAND_BITS, PROTOCOL_IS_LSB_FIRST, SEND_STOP_BIT); // false, true -> LSB first, stop bit

		interrupts();

		tNumberOfCommands--;
		// skip last delay!
		if (tNumberOfCommands > 0) {
			// send repeated command in a fixed raster
			delay(XIOMI_REPEAT_SPACE / 1000);
		}
	}
}
//+=============================================================================
//
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 */
bool IRrecv::decodeXiomi() {

	// Check we have the right amount of data (28). The +4 is for initial gap, start bit mark and space + stop bit mark
	if (decodedIRData.rawDataPtr->rawlen != (XIOMI_BITS + 4)) {
		// no debug output, since this check is mainly to determine the received protocol
		return false;
	}
	
	// Check header "space"
	if (!MATCH_MARK(decodedIRData.rawDataPtr->rawbuf[1], XIOMI_HEADER_MARK) || !MATCH_SPACE(decodedIRData.rawDataPtr->rawbuf[2], XIOMI_HEADER_SPACE)) {
		DBG_PRINT("Xiomi: ");
		DBG_PRINTLN("Header mark or space length is wrong");
		return false;
	}

	// false -> LSB first
	if (!decodePulseDistanceDataXiomi()) {
		DBG_PRINT(F("Xiomi: "));
		DBG_PRINTLN(F("Decode failed"));
		return false;
	}


	// Success
//	decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST; // Not required, since this is the start value
	uint16_t tAddress = decodedIRData.decodedRawData >> XIOMI_COMMAND_BITS;  // upper 16 bits of MSB first value
	uint16_t tCommand = decodedIRData.decodedRawData & 0xFF;	// lowest 8 bit of MSB first value

	/*
	 *  Check for repeat
	 */
	/*if (decodedIRData.rawDataPtr->rawbuf[0] < ((XIOMI_REPEAT_SPACE + (XIOMI_REPEAT_SPACE / 2)) / MICROS_PER_TICK)) {
		decodedIRData.flags = IRDATA_FLAGS_IS_REPEAT | IRDATA_FLAGS_IS_LSB_FIRST;
	}*/
	decodedIRData.command = tCommand;
	decodedIRData.address = tAddress;
	decodedIRData.numberOfBits = XIOMI_BITS;
	decodedIRData.protocol = XIOMI; // we have no XIOMI code

	return true;
}
