/*
 * ir_Others.hpp
 *
 *  Contains functions for miscellaneous protocols
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
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

/** \addtogroup Decoder Decoders and encoders for different protocols
 * @{
 */

#ifndef _IR_OTHERS_HPP
#define _IR_OTHERS_HPP
//==============================================================================
//                       DDDD   IIIII   SSSS  H   H
//                        D  D    I    S      H   H
//                        D  D    I     SSS   HHHHH
//                        D  D    I        S  H   H
//                       DDDD   IIIII  SSSS   H   H
//==============================================================================

// DISH support by Todd Treece
//
// The send function needs to be repeated 4 times
// Only send the last for characters of the hex.
// I.E.  Use 0x1C10 instead of 0x0000000000001C10 as listed in the LIRC file.
// Here is the LIRC file I found that seems to match the remote codes from the
// oscilloscope: DISH NETWORK (echostar 301):
//   http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx
#define DISH_BITS             16
#define DISH_HEADER_MARK     400
#define DISH_HEADER_SPACE   6100
#define DISH_BIT_MARK        400
#define DISH_ONE_SPACE      1700
#define DISH_ZERO_SPACE     2800
#define DISH_REPEAT_SPACE   6200 // really?

struct PulseDistanceWidthProtocolConstants DishProtocolConstants = { UNKNOWN, 56, DISH_HEADER_MARK, DISH_HEADER_SPACE,
DISH_BIT_MARK, DISH_ONE_SPACE, DISH_BIT_MARK, DISH_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST, 40, NULL };

void IRsend::sendDish(uint16_t aData) {
    sendPulseDistanceWidth(&DishProtocolConstants, aData, DISH_BITS, 4);
}

//==============================================================================
//               W   W  H   H  Y   Y N   N TTTTT EEEEE  RRRRR
//               W   W  H   H   Y Y  NN  N   T   E      R   R
//               W W W  HHHHH    Y   N N N   T   EEE    RRRR
//               W W W  H   H    Y   N  NN   T   E      R  R
//                WWW   H   H    Y   N   N   T   EEEEE  R   R
//==============================================================================
// Whynter A/C ARC-110WD added by Francesco Meschia
// see https://docs.google.com/spreadsheets/d/1dsr4Jh-nzC6xvSKGpLlPBF0NRwvlpyw-ozg8eZU813w/edit#gid=0
// Looking at the code table the protocol is LSB first with start and stop bit.
// 4 bit checksum, constant address 0xAA00, 8 bit Command and 4 bit Command group
// but we use MSB first to be backwards compatible
#define WHYNTER_BITS            32
#define WHYNTER_HEADER_MARK   2850
#define WHYNTER_HEADER_SPACE  2850
#define WHYNTER_BIT_MARK       750
#define WHYNTER_ONE_SPACE     2150
#define WHYNTER_ZERO_SPACE     750

struct PulseDistanceWidthProtocolConstants WhynterProtocolConstants = { WHYNTER, 38, WHYNTER_HEADER_MARK, WHYNTER_HEADER_SPACE,
WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_BIT_MARK, WHYNTER_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST, 110, NULL };

void IRsend::sendWhynter(uint32_t aData, uint8_t aNumberOfBitsToSend) {
    sendPulseDistanceWidth(&WhynterProtocolConstants, aData, NEC_BITS, aNumberOfBitsToSend);
}

bool IRrecv::decodeWhynter() {
    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawDataPtr->rawlen != (2 * WHYNTER_BITS) + 4) {
        return false;
    }
    if (!checkHeader(&WhynterProtocolConstants)) {
        return false;
    }
    if (!decodePulseDistanceWidthData(&WhynterProtocolConstants, WHYNTER_BITS)) {
        return false;
    }
    // Success
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.numberOfBits = WHYNTER_BITS;
    decodedIRData.protocol = WHYNTER;
    return true;
}

/** @}*/
#endif // _IR_OTHERS_HPP
