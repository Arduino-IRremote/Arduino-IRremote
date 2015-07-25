#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                               L       GGGG    AAA    CCCC
//                               L      G       A   A  C
//                               L      G  GG   AAAAA  C
//                               L      G   G   A   A  C
//                               LLLLL   GGG    A   A   CCCC
//==============================================================================
// chaeplin@gmail.com

/*

#define LGAC_HDR_MARK 3200
#define LGAC_HDR_SPACE 10000
#define LGAC_BIT_MARK 1500
#define LGAC_ONE_SPACE 500
#define LGAC_ZERO_SPACE 500

*/

#define LGAC_HDR_MARK 4100
#define LGAC_HDR_SPACE 8800
#define LGAC_BIT_MARK 1550
#define LGAC_ONE_SPACE 500
#define LGAC_ZERO_SPACE 550


#define LGAC_RPT_LENGTH 60000

//+=============================================================================
#if SEND_LGAC
void  IRsend::sendLGAC (unsigned long data,  int nbits)
{
    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(LGAC_HDR_MARK);
    space(LGAC_HDR_SPACE);
    mark(LGAC_ONE_SPACE);

    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
        if (data & mask) {
            space(LGAC_BIT_MARK);
            mark(LGAC_ONE_SPACE);
        } else {
            space(LGAC_ZERO_SPACE);
            mark(LGAC_ONE_SPACE);
        }
    }
    space(0);  // Always end with the LED off
}
#endif