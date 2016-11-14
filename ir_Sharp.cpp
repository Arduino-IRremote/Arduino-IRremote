#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                       SSSS  H   H   AAA   RRRR   PPPP
//                      S      H   H  A   A  R   R  P   P
//                       SSS   HHHHH  AAAAA  RRRR   PPPP
//                          S  H   H  A   A  R  R   P
//                      SSSS   H   H  A   A  R   R  P
//==============================================================================

// Sharp and DISH support by Todd Treece: http://unionbridge.org/design/ircommand
//
// The send function has the necessary repeat built in because of the need to
// invert the signal.
//
// Sharp protocol documentation:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
//
// Here is the LIRC file I found that seems to match the remote codes from the
// oscilloscope:
//   Sharp LCD TV:
//   http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

#define SHARP_BITS             15
#define SHARP_ADDR_BITS         5
#define SHARP_DATA_BITS         8
#define SHARP_BIT_MARK_SEND   250
#define SHARP_BIT_MARK_RECV   150
#define SHARP_ONE_SPACE      1850
#define SHARP_ZERO_SPACE      795
#define SHARP_GAP           42680
#define SHARP_RPT_SPACE      3000

#define SHARP_TOGGLE_MASK  0x3FF
#define SHARP_ADRDAT_MASK  0x7FFC

//+=============================================================================
#if SEND_SHARP
void  IRsend::sendSharpRaw (unsigned long data,  int nbits)
{
	enableIROut(38);

	// Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
	// much more reliable. That's the exact behaviour of CD-S6470 remote control.
	for (int n = 0;  n < 3;  n++) {
		for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
			if (data & mask) {
				mark(SHARP_BIT_MARK_SEND);
				space(SHARP_ONE_SPACE);
			} else {
				mark(SHARP_BIT_MARK_SEND);
				space(SHARP_ZERO_SPACE);
			}
		}

		mark(SHARP_BIT_MARK_SEND);
		space(SHARP_GAP);
		//delay(40);

		data = data ^ SHARP_TOGGLE_MASK;
	}
}
#endif

//+=============================================================================
// Sharp send compatible with data obtained through decodeSharp()
//
#if SEND_SHARP
void  IRsend::sendSharp (unsigned int address,  unsigned int command)
{
	//Change address to big-endian (five bits swap place)
	address = (address & 0x10) >> 4 | (address & 0x01) << 4 | (address & 0x08) >> 2 | (address & 0x02) << 2 | (address & 0x04) ;
	
	//Change command to big-endian (eight bit swap place)
	command = (command & 0xF0) >> 4 | (command & 0x0F) << 4;
    command = (command & 0xCC) >> 2 | (command & 0x33) << 2;
    command = (command & 0xAA) >> 1 | (command & 0x55) << 1;
	
	sendSharpRaw((address << 10) | (command << 2) | 0, SHARP_BITS);
}
#endif

//+=============================================================================
// Sharp decode function written based on Sharp protocol documentation:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
// Tesded on a DENON AVR-1804 reciever

#if DECODE_SHARP
bool  IRrecv::decodeSharp(decode_results *results)
{
	unsigned long  addr = 0;  // Somewhere to build our address
	unsigned long  data = 0;  // Somewhere to build our data
	unsigned long  lastData = 0;  // Somewhere to store last data
	int            offset = 1;  //skip long space


	// Check we have the right amount of data  
	if (irparams.rawlen != (SHARP_BITS + 1) * 2)  return false;
	
	// Check the first mark to see if it fits the SHARP_BIT_MARK_RECV length
	if (!MATCH_MARK(results->rawbuf[offset], SHARP_BIT_MARK_RECV))  return false;
	//check the first pause and see if it fits the SHARP_ONE_SPACE or SHARP_ZERO_SPACE length
	if (!(MATCH_SPACE(results->rawbuf[offset+1], SHARP_ONE_SPACE) || MATCH_SPACE(results->rawbuf[offset + 1], SHARP_ZERO_SPACE)))  return false;

	// Read the bits in
//	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < SHARP_ADDR_BITS; i++) {
			// Each bit looks like: SHARP_BIT_MARK_RECV + SHARP_ONE_SPACE -> 1
			//                 or : SHARP_BIT_MARK_RECV + SHARP_ZERO_SPACE -> 0
			if (!MATCH_MARK(results->rawbuf[offset++], SHARP_BIT_MARK_RECV))  return false;
			// IR data is big-endian, so we shuffle it in from the right:
			if (MATCH_SPACE(results->rawbuf[offset], SHARP_ONE_SPACE)) addr += 1<<i ;//   addr = (addr << 1) | 1;
			else if (MATCH_SPACE(results->rawbuf[offset], SHARP_ZERO_SPACE)) addr = addr;  //addr = (addr << 1) | 0;
			else                                                        return false;
			offset++;
		}
		for (int i = 0; i < SHARP_DATA_BITS; i++) {
			// Each bit looks like: SHARP_BIT_MARK_RECV + SHARP_ONE_SPACE -> 1
			//                 or : SHARP_BIT_MARK_RECV + SHARP_ZERO_SPACE -> 0
			if (!MATCH_MARK(results->rawbuf[offset++], SHARP_BIT_MARK_RECV))  return false;
			// IR data is big-endian, so we shuffle it in from the right:
			if (MATCH_SPACE(results->rawbuf[offset], SHARP_ONE_SPACE)) data += 1<<i ;//data = (data << 1) | 1;
			else if (MATCH_SPACE(results->rawbuf[offset], SHARP_ZERO_SPACE))  data = data;  //data = (data << 1) | 0;
			else                                                        return false;
			offset++;
			//Serial.print(i);
			//Serial.print(":");
			//Serial.println(data, HEX);
		}
		//skip exp bit (mark+pause), chk bit (mark+pause), mark and long pause before next burst
//		offset+=6;

		//Check if last burst data is equal to this burst (lastData allready inverted)
//		if (lastData != 0 && data != lastData) return false;
		//save current burst of data but invert (XOR) the last 10 bits (8 data bits + exp bit + chk bit)
//		lastData = data ^ 0xFF;
//	}

	// Success
	results->bits = SHARP_BITS;
	results->value = data;
	results->address = addr;
	results->decode_type = SHARP;
	return true;
}
#endif
