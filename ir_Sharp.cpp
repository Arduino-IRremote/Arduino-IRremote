//-*- mode: C; c-basic-offset: 8; tab-width: 8; indent-tabs-mode: t; -*-
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                       SSSS  H   H   AAA   RRRR   PPPP
//                      S      H   H  A   A  R   R  P   P
//                       SSS   HHHHH  AAAAA  RRRR   PPPP
//                          S  H   H  A   A  R  R   P
//                      SSSS   H   H  A   A  R   R  P
//==============================================================================

// Sharp and DISH support.
//
// Authors:
// Todd Treece (http://unionbridge.org/design/ircommand)
// Sergiy Kolesnikov
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

#define SHARP_RAWLEN		32
#define SHARP_ADDRESS_BITS	5
#define SHARP_COMMAND_BITS	8
#define SHARP_BIT_MARK		150
#define SHARP_SEND_BIT_MARK	300
#define SHARP_ONE_SPACE		1750
#define SHARP_ZERO_SPACE	700
#define SHARP_RPT_SPACE		50000
#define SHARP_SEND_RPT_SPACE	44000
#define SHARP_TOGGLE_MASK	0x3FF
#define SHARP_SEND_INVERT_MASK	0x7FE0

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
				mark(SHARP_BIT_MARK);
				space(SHARP_ONE_SPACE);
			} else {
				mark(SHARP_BIT_MARK);
				space(SHARP_ZERO_SPACE);
			}
		}

		mark(SHARP_BIT_MARK);
		space(SHARP_ZERO_SPACE);
		delay(40);

		data = data ^ SHARP_TOGGLE_MASK;
	}
}

// The old sendSharpRaw() function expects data bits in the big-endian format,
// but the sharp protocol and correspondingly the decodeSharp() function use
// little-endian. The old sendSharpRaw() was not changed to preserve
// compatibility. sendSharpRawLittleEndian() is used instead to send data
// obtained with decodeSharp().
void  IRsend::sendSharpRawLittleEndian (unsigned long data,  int nbits)
{
	enableIROut(38);

	for (int n = 0;  n < 3;  n++) {
		unsigned long mask = B1;
		for (int i = 0; i < nbits; i++) {
			if (data & mask) {
				mark(SHARP_SEND_BIT_MARK);
				space(SHARP_ONE_SPACE);
			} else {
				mark(SHARP_SEND_BIT_MARK);
				space(SHARP_ZERO_SPACE);
			}
			mask <<= 1;
		}
		mark(SHARP_BIT_MARK);
		space(SHARP_SEND_RPT_SPACE);
		data = data ^ SHARP_SEND_INVERT_MASK;
	}
}

void  IRsend::sendSharp (unsigned int address,  unsigned long command)
{
	unsigned long data = 1; // The expansion and the check bits (01).
	data = (data << SHARP_COMMAND_BITS) | command;
	data = (data << SHARP_ADDRESS_BITS) | address;

	// (+2) is for the expansion and the check bits.
	sendSharpRawLittleEndian(data, SHARP_ADDRESS_BITS + SHARP_COMMAND_BITS + 2);
}
#endif

//+=============================================================================
#if DECODE_SHARP
bool  IRrecv::decodeSharp (decode_results *results)
{

	// Check we have enough data.
	if (irparams.rawlen < (SHARP_RAWLEN)) return false;

	// Check stop mark.
	if (!MATCH_MARK(results->rawbuf[SHARP_RAWLEN - 1], SHARP_BIT_MARK)) return false;

	// Check the "check bit." If this bit is not 0 than it is an inverted
	// frame, which we ignore.
	if (!MATCH_SPACE(results->rawbuf[SHARP_RAWLEN - 2], SHARP_ZERO_SPACE)) return false;

	// Check for repeat.
	static boolean is_first_repeat = true;
        long initial_space = ((long) results->rawbuf[0]) * USECPERTICK;
	if (initial_space <= SHARP_RPT_SPACE) {
		if (!is_first_repeat) {
			results->bits        = 0;
			results->value       = REPEAT;
			results->decode_type = SHARP;
			return true;
		} else {

			// Ignore the first repeat that always comes after the
			// inverted frame (even if the button was pressed only
			// once).
			is_first_repeat = false;
			return false;
		}
	}

	// Decode bits. SHARP_RAWLEN-6 because index starts with 0 (-1) and we
	// omit the timings for the stop mark (-1), the check bit (-2), and the
	// expansion bit (-2).
	uint16_t bits = 0;
	for (int i = SHARP_RAWLEN - 6;  i > 1;  i -= 2) {
		if (MATCH_SPACE(results->rawbuf[i], SHARP_ONE_SPACE )) {
			bits = (bits << 1) | 1;
		} else if (MATCH_SPACE(results->rawbuf[i], SHARP_ZERO_SPACE)) {
			bits = (bits << 1) | 0;
		} else {
			return false ;
		}
	}

	results->bits        = SHARP_ADDRESS_BITS + SHARP_COMMAND_BITS;
	results->address     = bits & (1 << (SHARP_ADDRESS_BITS)) - 1;
	results->value       = bits >> SHARP_ADDRESS_BITS;	// command
	results->decode_type = SHARP;
	is_first_repeat = true;
	return true;
}
#endif
