#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              P I O N E E R
//
//
//==============================================================================
// Device Code in Hex Format
// e.g. 165 for AVR Receiver

#define PIONEER_BITS          32  // The number of bits in the command

#define PIONEER_HDR_MARK    9000  // The length of the Header:Mark
#define PIONEER_HDR_SPACE   4500  // The lenght of the Header:Space

#define PIONEER_BIT_MARK    550  // The length of a Bit:Mark
#define PIONEER_ONE_SPACE   1675  // The length of a Bit:Space for 1's
#define PIONEER_ZERO_SPACE  550  // The length of a Bit:Space for 0's

//+=============================================================================
//
#if SEND_PIONEER
void  IRsend::sendPioneer (unsigned int devicecode,  unsigned long data)
{
	// Set IR carrier frequency
	enableIROut(39.2);

	// Header
	mark (PIONEER_HDR_MARK);
	space(PIONEER_HDR_SPACE);

	// Device Code
	for (unsigned long  mask = 1UL << (8 - 1);  mask;  mask >>= 1) {
		mark(PIONEER_BIT_MARK);
		if (devicecode & mask)  space(PIONEER_ONE_SPACE) ;
		else                 space(PIONEER_ZERO_SPACE) ;
    }
    // Inverted Device Code
	for (unsigned long  mask = 1UL << (8 - 1);  mask;  mask >>= 1) {
		mark(PIONEER_BIT_MARK);
		if (devicecode & mask)  space(PIONEER_ZERO_SPACE) ;
		else                 space(PIONEER_ONE_SPACE) ;
    }
	// Data
	for (unsigned long  mask = 1UL << (8 - 1);  mask;  mask >>= 1) {
        mark(PIONEER_BIT_MARK);
        if (data & mask)  space(PIONEER_ONE_SPACE) ;
        else              space(PIONEER_ZERO_SPACE) ;
    }

    // Inverted Data
    for (unsigned long  mask = 1UL << (8 - 1);  mask;  mask >>= 1) {
	    mark(PIONEER_BIT_MARK);
	    if (data & mask)  space(PIONEER_ZERO_SPACE) ;
	    else              space(PIONEER_ONE_SPACE) ;
    }

	// Footer
	mark(PIONEER_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

