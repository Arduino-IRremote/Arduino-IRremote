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

struct PulseDistanceWidthProtocolConstants const DishProtocolConstants PROGMEM = {UNKNOWN, 56, DISH_HEADER_MARK, DISH_HEADER_SPACE,
    DISH_BIT_MARK, DISH_ONE_SPACE, DISH_BIT_MARK, DISH_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE, 40, nullptr};

void IRsend::sendDish(uint16_t aData) {
    sendPulseDistanceWidth_P(&DishProtocolConstants, aData, DISH_BITS, 4);
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

struct PulseDistanceWidthProtocolConstants const WhynterProtocolConstants PROGMEM = {WHYNTER, 38, WHYNTER_HEADER_MARK, WHYNTER_HEADER_SPACE,
    WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_BIT_MARK, WHYNTER_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE, 110, nullptr};

void IRsend::sendWhynter(uint32_t aData, int_fast8_t aNumberOfRepeats) {
    sendPulseDistanceWidth_P(&WhynterProtocolConstants, aData, NEC_BITS, aNumberOfRepeats);
}

bool IRrecv::decodeWhynter() {
    // Check we have the right amount of data (68). The +4 is for initial gap, start bit mark and space + stop bit mark.
    if (decodedIRData.rawlen != (2 * WHYNTER_BITS) + 4) {
        return false;
    }
    if (!checkHeader_P(&WhynterProtocolConstants)) {
        return false;
    }
    decodePulseDistanceWidthData_P(&WhynterProtocolConstants, WHYNTER_BITS);

    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;
    decodedIRData.numberOfBits = WHYNTER_BITS;
    decodedIRData.protocol = WHYNTER;
    return true;
}

/**
 * VELUX
 * see https://github.com/XPModder/Velux-IR-protocol
 * see https://github.com/Arduino-IRremote/Arduino-IRremote/issues/612
 * We have a pulse width protocol with constant bit length of 1700 us with no header and one autorepeat after 27 ms.
 * Length of one frame is constant 23 * 1700 + 425 (for stop bit) = 39525 = 40 ms
 *
 * BIT meaning for MSB first:
 * Bit 23 is stop
 * Bit 22 is up = 0 or down = 1, 0 for stop
 * Bit 21 is automatic and also set for stop
 * Bit 20 is Motor 3
 * Bit 19 is Motor 2
 * Bit 18 is Motor 1
 * [14:17] is 4 bit motor set (from 1 to 10), 0 = all sets
 * [4:13] 10 bit security code
 * [1:3] is checksum
 *
 * Checksum is independent of the 10 bit security code
 *
 * !!! MOTOR 3 is coded with 0x04, since motor numbers are bit position coded to enable 0x07 as all motors!!!
 * Automatic bit is set for all following codes!
 * Checksum for UP (0x1):
 * All (Motor = 0x7, set = 0) -> 2 == Motor 1 set A or Motor 3 set 5
 * Table of checksum (x) for motor / set combinations
 *        Set 1 2 3 4 5 6 7 8 9 A
 * Motor 1    9 A B C D E F 0 1 2 3 4 5 6 | x XOR set = 8 or set XOR 8 = checksum
 * Motor 2    C F E 9 8 B A 5 4 7 6 1 0 2 | x XOR set = D = 8 XOR 5 or set XOR 8 XOR 5 = checksum
 * Motor 3/4  6 5 4 3 2 1 0 F E D C B A 9 | x XOR set = 7 = 8 XOR F or set XOR 8 XOR F = checksum
 * Checksum for DOWN (0x3):
 * All -> 7 == Motor 1 set A
 * Table of checksum (x) for motor / set combinations
 *        Set 1 2 3 4 5 6 7 8 9 A
 * Motor 1    C F E 9 8 B A 5 4 7 6 1 0 2 | x XOR set = D
 * Motor 2    9 A B C D E F 0 1 2 3 4 5 6 | x XOR set = 8 = D XOR 5
 * Motor 3/4  3 0 1 6 7 4 5 A B 8 9 E F D | x XOR set = 2 = D XOR F
 * Checksum for STOP (0x5):
 * All -> 8 == Motor 1 set A
 * Table of checksum (x) for motor / set combinations
 *        Set 1 2 3 4 5 6 7 8 9 A
 * Motor 1    3 0 1 6 7 4 5 A B 8 9 E F D | x XOR set = 2
 * Motor 2    6 5 4 3 2 1 0 F E D C B A 9 | x XOR set = 7 = 2 XOR 5
 * Motor 3/4  C F E 9 8 B A 5 4 7 6 1 0 2 | x XOR set = D = 2 XOR F
 */

// All timings are in microseconds
#define VELUX_BITS          24 // We have pulse width, so we have no stop bit
#define VELUX_HEADER_MARK   0
#define VELUX_HEADER_SPACE  0
#define VELUX_UNIT          425
#define VELUX_ONE_MARK      (3*VELUX_UNIT) // 1275
#define VELUX_ONE_SPACE     VELUX_UNIT
#define VELUX_ZERO_MARK     VELUX_UNIT
#define VELUX_ZERO_SPACE    (3*VELUX_UNIT)
#define VELUX_PERIOD        ((23 * 4) + 1 ) * VELUX_UNIT) // 39525
#define VELUX_AUTOREPEAT_SPACE  27000 // 27ms
#define VELUX_REPEAT_SPACE  100000 // 100ms, which is just a guess

#define VELUX_COMMAND_AUTO_UP       0x1
#define VELUX_COMMAND_AUTO_DOWN     0x3
#define VELUX_COMMAND_STOP          0x5

struct PulseDistanceWidthProtocolConstants const VeluxProtocolConstants PROGMEM = {OTHER, 30, VELUX_HEADER_MARK, VELUX_HEADER_SPACE,
    VELUX_ONE_MARK, VELUX_ONE_SPACE, VELUX_ZERO_MARK, VELUX_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST | PROTOCOL_IS_PULSE_WIDTH, 27, nullptr};

/*
 * @param aCommand      VELUX_COMMAND_AUTO_UP or VELUX_COMMAND_AUTO_DOWN or VELUX_COMMAND_STOP
 * @param aMotorNumber  1, 2, 4 = Motor3, 7 = All
 * @param aMotorSet     0 = All, 1 to 10
 * !!!NO parameter range check here!!!
 */
void IRsend::sendVelux(uint8_t aCommand, uint8_t aMotorNumber, uint8_t aMotorSet, uint16_t aSecurityCode,
        int_fast8_t aNumberOfRepeats) {
    // Just in case...
    if (aMotorNumber == 3) {
        aMotorNumber = 4; // motor numbers are bit position coded
    }

    /*
     * Compute checksum (only for automatic bit set to 1)
     */
    uint8_t tChecksum = 8; // Start checksum for command
    if (aCommand == VELUX_COMMAND_AUTO_DOWN) {
        tChecksum = 0xD;
    }
    if (aCommand == VELUX_COMMAND_STOP) {
        tChecksum = 2;
    }

    uint8_t tXORForMotor = 0;
    if (aMotorNumber == 2) {
        tXORForMotor = 5;
    }
    if (aMotorNumber == 4) {
        tXORForMotor = 0xF;
    }

    tChecksum ^= aMotorSet;
    tChecksum ^= tXORForMotor;

    sendVelux(
            ((uint32_t) aCommand << 21) | ((uint32_t) aMotorNumber << 18) | (aMotorSet << 14) | ((uint32_t) aSecurityCode << 4)
                    | tChecksum, aNumberOfRepeats);
}

void IRsend::sendVelux(uint32_t aData, int_fast8_t aNumberOfRepeats) {
    do {
        sendPulseDistanceWidth_P(&VeluxProtocolConstants, aData, VELUX_BITS, 0);
        delay(VELUX_AUTOREPEAT_SPACE / MICROS_IN_ONE_MILLI);
        sendPulseDistanceWidth_P(&VeluxProtocolConstants, aData, VELUX_BITS, 0);
        delay(VELUX_REPEAT_SPACE / MICROS_IN_ONE_MILLI);
        aNumberOfRepeats--;
    } while (aNumberOfRepeats >= 0);
}
/** @}*/
#endif // _IR_OTHERS_HPP
