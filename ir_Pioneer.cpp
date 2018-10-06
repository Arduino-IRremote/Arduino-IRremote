#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                        PPPP  III  OOO  N   N EEEE EEEE RRRR  
//                        P   P  I  O   O NN  N E    E    R   R 
//                        PPPP   I  O   O N N N EEE  EEE  RRRR  
//                        P      I  O   O N  NN E    E    R R   
//                        P     III  OOO  N   N EEEE EEEE R  RR 
//==============================================================================

#define PIONEER_BITS          32
#define PIONEER_HDR_MARK    9000
#define PIONEER_HDR_SPACE   4500
#define PIONEER_BIT_MARK     560
#define PIONEER_ONE_SPACE   1690
#define PIONEER_ZERO_SPACE   560
#define PIONEER_RPT_SPACE   2250
#define PIONEER_SPR_SPACE  24725

//+=============================================================================
#if SEND_PIONEER
void  IRsend::sendPioneer (unsigned long data1, unsigned long data2, int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	int nbits_temp = nbits;
	
	// Header
	mark(PIONEER_HDR_MARK);
	space(PIONEER_HDR_SPACE);

	// 1st Data HEX
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data1 & mask) {
			mark(PIONEER_BIT_MARK);
			space(PIONEER_ONE_SPACE);
		} else {
			mark(PIONEER_BIT_MARK);
			space(PIONEER_ZERO_SPACE);
		}
	}

	nbits = nbits_temp;			// restore bits count for next loop
	
	mark(PIONEER_BIT_MARK);
	space(PIONEER_SPR_SPACE);	// separating signal
	
	mark(PIONEER_HDR_MARK);		
	space(PIONEER_HDR_SPACE);	// signaling next 32bit HEX
	
	// 2nd Data HEX
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data2 & mask) {
			mark(PIONEER_BIT_MARK);
			space(PIONEER_ONE_SPACE);
		} else {
			mark(PIONEER_BIT_MARK);
			space(PIONEER_ZERO_SPACE);
		}
	}
	
	mark(PIONEER_BIT_MARK);
	space(PIONEER_RPT_SPACE);  // Always end with the LED off
	// Footer
}
#endif
