#include "IRremoteInt.h"

//==============================================================================
//                       DDDD   IIIII   SSSS  H   H
//                        D  D    I    S      H   H
//                        D  D    I     SSS   HHHHH
//                        D  D    I        S  H   H
//                       DDDD   IIIII  SSSS   H   H
//==============================================================================

// Sharp and DISH support by Todd Treece ( http://unionbridge.org/design/ircommand )
//
// The send function needs to be repeated 4 times
//
// Only send the last for characters of the hex.
// I.E.  Use 0x1C10 instead of 0x0000000000001C10 as listed in the LIRC file.
//
// Here is the LIRC file I found that seems to match the remote codes from the
// oscilloscope:
//   DISH NETWORK (echostar 301):
//   http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx
#ifndef _IR_DISH_HPP
#define _IR_DISH_HPP

#define DISH_BITS             16
#define DISH_HEADER_MARK     400
#define DISH_HEADER_SPACE   6100
#define DISH_BIT_MARK        400
#define DISH_ONE_SPACE      1700
#define DISH_ZERO_SPACE     2800
#define DISH_REPEAT_SPACE   6200

//+=============================================================================
void IRsend::sendDISH(unsigned long data, int nbits) {
    // Set IR carrier frequency
    enableIROut(56);

    mark(DISH_HEADER_MARK);
    space(DISH_HEADER_SPACE);

    sendPulseDistanceWidthData(DISH_BIT_MARK, DISH_ONE_SPACE, DISH_BIT_MARK, DISH_ZERO_SPACE, data, nbits, PROTOCOL_IS_MSB_FIRST, SEND_NO_STOP_BIT);
    mark(DISH_HEADER_MARK); //added 26th March 2016, by AnalysIR ( https://www.AnalysIR.com )
    IrReceiver.restartAfterSend();
}

#endif // _IR_DISH_HPP
