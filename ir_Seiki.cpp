#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              S E I K I
//
//
//==============================================================================

#define SEIKI_BITS        16  // The number of bits in the command

#define SEIKI_HDR_MARK    9057 // The length of the Header:Mark
#define SEIKI_HDR_SPACE   4467 // The length of the Header:Space

#define SEIKI_BIT_MARK    606  // The length of a Bit:Mark
#define SEIKI_ONE_SPACE   1646  // The length of a Bit:Space for 1's
#define SEIKI_ZERO_SPACE  520  // The length of a Bit:Space for 0's


//+=============================================================================
//
#if SEND_SEIKI
void  IRsend::sendSeiki (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	unsigned long pre = 0x40BE;  // 16 bits

	// Header
	mark (SEIKI_HDR_MARK);
	space(SEIKI_HDR_SPACE);

    // Send "pre" data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		mark(SEIKI_BIT_MARK);
		if (pre & mask)  space(SEIKI_ONE_SPACE) ;
		else             space(SEIKI_ZERO_SPACE) ;
	}

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark (SEIKI_BIT_MARK);
			space(SEIKI_ONE_SPACE);
		} else {
			mark (SEIKI_BIT_MARK);
			space(SEIKI_ZERO_SPACE);
		}
	}

	// Footer
	mark(SEIKI_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif
