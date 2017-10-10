/*
 Written by rmick (www.bushandbeyond.com.au) Oct 2017
 
 LTTO means Lazer Tag Team Ops. It is a brand of Lazertag that started life in 2004, from a toy dev company called 'Shoot The Moon'.
 It was was originally sold under the Tiger brand, then later under Nerf and Hasbro.
 
 The range of taggers supported by this library includes;
 LTTO/Deluxe
 IRT-2X drone
 LTX/Phoenix
 TMB
 LTAR
 
 The protocol uses a 38kHz carrier with a 1mS Mark and 2mS Space bits. The Header consists of a 3mS Mark, followed by a 3mS or 6mS Space.
 
 This decoder would not have been possible without the help of many people, inlcuding Riley McArdle, TagFerret & Ryan Bales.
 
 For more information on the protocol and how to use it, please visit
 
 https://wiki.lazerswarm.com/wiki/Main_Page
 
 or ask to join the Lazertag Modders group on Facebook.
 https://www.facebook.com/groups/LazerTagModders/
*/

#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                          L      TTTTT  TTTTT   OOO
//                          L        T		T	 O   O
//                          L		 T		T	 O   O
//                          L		 T		T	 O   O
//                          LLLLL    T		T	  OOO
//==============================================================================

#define BITS          32  // The number of bits in the command

#define HDR_MARK		3000	// The length of the Header:Mark
#define HDR_SPACE		6000	// The length of the Header:Space
#define TAG_SYNC		3000	// The lenght of the Sync signal for a Tag:Mark
#define BEACON_SYNC		6000	// The length of the Sync signal for a Beacon:Mark	

#define ZERO_BIT		1000	// The length of a Bit:Mark for 0's
#define ONE_BIT			2000	// The length of a Bit:Mark for 1's
#define BIT_SPACE		2000	// The length of a Bit:Space
#define LONG_PAUSE		25000	// The length between packets.

//+=============================================================================
//
#if SEND_LTTO
void  IRsend::sendLTTO (unsigned long data,  int nbits, bool beacon)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark (HDR_MARK);		//PreSync
	space(HDR_SPACE);		//PreSync Pause

	//Sync
	if (beacon)	mark(BEACON_SYNC);
	else		mark(TAG_SYNC);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			space(BIT_SPACE);
			mark (ONE_BIT);
		} else {
			space(BIT_SPACE);
			mark (ZERO_BIT);
		}
	}

	// Footer
	space(LONG_PAUSE);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_LTTO
bool  IRrecv::decodeLTTO(decode_results *results)
{
	unsigned long  data	  = 0;  // Somewhere to build our code
	int            offset = 0;  // The IF(_GAP) statement sets the Skip the Gap reading
	int headerSize        = 2;	// Number of bits for the header, altered by IF(_GAP) to 4
	int tagLength = 16;
	int beaconLength = 12;
		results->bits = 0;
	// Check initial Mark+Space match
	if (_GAP > 7000)
	{
		// The library default for _GAP is 5000, which means the 6000uS Space is seen as a new packet,
		// therefore the initial 3000uS Mark gets lost, so ignore it unless _GAP > 7000.
		headerSize	 = 4;		// allow for the 2 extra bits in the packet.
		tagLength	 = 18;		// allow for the 2 extra bits in the packet.
		beaconLength = 14;		// allow for the 2 extra bits in the packet.
		offset++;				// Skip the Gap reading
		if (!MATCH_MARK(results->rawbuf[offset++], HDR_MARK))  return false;
	}
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))  return false ;
	

	// Check the Sync Type
	if (MATCH_MARK(results->rawbuf[offset], TAG_SYNC))
	{
		if (results->rawlen < tagLength) return false;
		results->address = 3000;			//TYPE_LAZERTAG_TAG;
	}
	else if (MATCH_MARK(results->rawbuf[offset], BEACON_SYNC))
	{
		if (results->rawlen < beaconLength) return false;
		results->address = 6000;			//TYPE_LAZERTAG_BEACON;
	}
	else return false;
	offset++;

	// Read the bits in
	for (int i = 0; i < ((results->rawlen-headerSize)/2); i++)
	{
		// Each bit looks like: SPACE + MARK_1 -> 1
		//                 or : SPACE + MARK_0 -> 0

		if (!MATCH_SPACE(results->rawbuf[offset++], BIT_SPACE))  return false;
		
		// IR data is big-endian, so we shuffle it in from the right:
		if (MATCH_MARK(results->rawbuf[offset], ONE_BIT))
		{
			data = (data << 1) | 1;
			results->bits++;
		}
		else if (MATCH_MARK(results->rawbuf[offset], ZERO_BIT))
		{
			data = (data << 1) | 0;
			results->bits++;
		}
		offset++;
	}

	// Success
	results->value = data;
	results->decode_type = LTTO;
	
	//Serial.print("\n\nSuccess = LTTO");

	return true;
}
#endif
