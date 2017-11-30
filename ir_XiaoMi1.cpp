#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//         X   X  III    A     ooo     M   M  III    1
//          X X    I   A   A  o   o    MM MM   I    11
//           X     I   A   A  o   o    M M M   I     1
//          X X    I   A A A  o   o    M   M   I     1
//         X   X  III  A   A   ooo     M   M  III   111
//
// 1st generation of Xiao Mi IR remote controller,
// used on Xiao Mi Box I, Made In China.
//==============================================================================

// Xiao Mi 1 uses quaternary system, 1 bit has four values.
#define XIAOMI1_0SPACE ((unsigned int)11)
#define XIAOMI1_1SPACE ((unsigned int)17)
#define XIAOMI1_2SPACE ((unsigned int)23)
#define XIAOMI1_3SPACE ((unsigned int)29)

// 11-bit of quaternary data.
#define XIAOMI1_BITS 11

#define XIAOMI1_MARK ((unsigned int)12)
#define XIAOMI1_HDR_MARK ((unsigned int)21)

#define XIAOMI1_TOLERANCE ((unsigned int)3)

#define XIAOMI1_MATCH(measured_ticks, desired_ticks)            \
	((measured_ticks) >= ((desired_ticks) - XIAOMI1_TOLERANCE) && \
	 (measured_ticks) <= ((desired_ticks) + XIAOMI1_TOLERANCE))

//+=============================================================================
#if DECODE_XIAOMI1
bool IRrecv::decodeXiaoMi1(decode_results *results)
{
	unsigned long  data   = 0;  // Somewhere to build our code
	int            offset = 1;  // Skip the Gap reading

	if (results->rawlen != 24) return false;

	// Check header "mark"
	if (!XIAOMI1_MATCH(results->rawbuf[offset], XIAOMI1_HDR_MARK)) return false;
	offset++;

	// Read the bits in
	for (int i = 0;  i < XIAOMI1_BITS;  i++) {
		// IR data is big-endian, so we shuffle it in from the right:
		if      (XIAOMI1_MATCH(results->rawbuf[offset], XIAOMI1_0SPACE)) data = (data << 2) | 0 ;
		else if (XIAOMI1_MATCH(results->rawbuf[offset], XIAOMI1_1SPACE)) data = (data << 2) | 1 ;
		else if (XIAOMI1_MATCH(results->rawbuf[offset], XIAOMI1_2SPACE)) data = (data << 2) | 2 ;
		else if (XIAOMI1_MATCH(results->rawbuf[offset], XIAOMI1_3SPACE)) data = (data << 2) | 3 ;
		else return false ;
		offset++;

		// Match a mark
		if (!XIAOMI1_MATCH(results->rawbuf[offset], XIAOMI1_MARK)) return false;
		offset++;
	}

	// Success
	results->bits        = XIAOMI1_BITS * 2;  // 1 quaternary bit equal to 2 binary bits.
	results->value       = data;
	results->decode_type = XIAOMI1;
	return true;
}
#endif
