/*
 *  TinyIR.h
 *
 *
 *  Copyright (C) 2021-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  TinyIRReceiver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#ifndef _TINY_IR_H
#define _TINY_IR_H

#include <Arduino.h>

#include "LongUnion.h"

/** \addtogroup TinyReceiver Minimal receiver for NEC and FAST protocol
 * @{
 */

#define VERSION_IRTINY "1.0.0"
#define VERSION_IRTINY_MAJOR 1
#define VERSION_IRTINY_MINOR 0
#define VERSION_IRTINY_PATCH 0

/**
 * Timing for NEC protocol
 *
 * see: https://www.sbprojects.net/knowledge/ir/nec.php
 * LSB first, 1 start bit + 16 bit address + 8 bit data + 8 bit inverted data + 1 stop bit.
 */
#define NEC_ADDRESS_BITS        16 // 16 bit address or 8 bit address and 8 bit inverted address
#define NEC_COMMAND_BITS        16 // Command and inverted command
#define NEC_BITS                (NEC_ADDRESS_BITS + NEC_COMMAND_BITS)

#define NEC_UNIT                560

#define NEC_HEADER_MARK         (16 * NEC_UNIT) // 9000
#define NEC_HEADER_SPACE        (8 * NEC_UNIT)  // 4500

#define NEC_BIT_MARK            NEC_UNIT
#define NEC_ONE_SPACE           (3 * NEC_UNIT)  // 1690
#define NEC_ZERO_SPACE          NEC_UNIT

#define NEC_REPEAT_HEADER_SPACE (4 * NEC_UNIT)  // 2250

#define NEC_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
#define NEC_MINIMAL_DURATION     49900 // NEC_HEADER_MARK + NEC_HEADER_SPACE + 32 * 2 * NEC_UNIT + NEC_UNIT // 2.5 because we assume more zeros than ones
#define NEC_MAXIMUM_REPEAT_DISTANCE (NEC_REPEAT_PERIOD - NEC_MINIMAL_DURATION + 10000) // 70 ms

/**
 * FAST_8_BIT_CS Protocol characteristics:
 * - Bit timing is like JVC
 * - The header is shorter, 4000 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *     leading to a fixed protocol length of (7 + (16 * 2) + 1) * 526 = 40 * 560 = 21040 microseconds or 21 ms.
 * - Repeats are sent as complete frames but in a 50 ms period.
 */
#define FAST_8_BIT_PARITY_ADDRESS_BITS          0 // No address
#define FAST_8_BIT_PARITY_COMMAND_BITS         16 // Command and inverted command
#define FAST_8_BIT_PARITY_BITS                 (FAST_8_BIT_PARITY_ADDRESS_BITS + FAST_8_BIT_PARITY_COMMAND_BITS)

#define FAST_8_BIT_PARITY_UNIT                 526 // 20 periods of 38 kHz (526.315789)

#define FAST_8_BIT_PARITY_BIT_MARK             FAST_8_BIT_PARITY_UNIT
#define FAST_8_BIT_PARITY_ONE_SPACE            (3 * FAST_8_BIT_PARITY_UNIT)     // 1578 -> bit period = 2104
#define FAST_8_BIT_PARITY_ZERO_SPACE           FAST_8_BIT_PARITY_UNIT           //  526 -> bit period = 1052

#define FAST_8_BIT_PARITY_HEADER_MARK          (4 * FAST_8_BIT_PARITY_UNIT)     // 2104
#define FAST_8_BIT_PARITY_HEADER_SPACE         (FAST_8_BIT_PARITY_ONE_SPACE)    // 1578

#define FAST_8_BIT_PARITY_REPEAT_PERIOD        50000 // Commands are repeated every 50 ms (measured from start to start) for as long as the key on the remote control is held down.
#define FAST_8_BIT_PARITY_REPEAT_DISTANCE      (FAST_8_BIT_PARITY_REPEAT_PERIOD - (40 * FAST_8_BIT_PARITY_UNIT)) // 29 ms
#define FAST_8_BIT_PARITY_MAXIMUM_REPEAT_DISTANCE (FAST_8_BIT_PARITY_REPEAT_DISTANCE + 10000) // 47.5 ms

#if defined(USE_FAST_8_BIT_AND_PARITY_TIMING)
#define TINY_ADDRESS_BITS          FAST_8_BIT_PARITY_ADDRESS_BITS
#define TINY_COMMAND_BITS          FAST_8_BIT_PARITY_COMMAND_BITS
#define TINY_COMMAND_HAS_8_BIT_PARITY  true

#define TINY_BITS                  FAST_8_BIT_PARITY_BITS
#define TINY_UNIT                  FAST_8_BIT_PARITY_UNIT

#define TINY_HEADER_MARK           FAST_8_BIT_PARITY_HEADER_MARK
#define TINY_HEADER_SPACE          FAST_8_BIT_PARITY_HEADER_SPACE

#define TINY_BIT_MARK              FAST_8_BIT_PARITY_BIT_MARK
#define TINY_ONE_SPACE             FAST_8_BIT_PARITY_ONE_SPACE
#define TINY_ZERO_SPACE            FAST_8_BIT_PARITY_ZERO_SPACE

#define TINY_MAXIMUM_REPEAT_DISTANCE  FAST_8_BIT_PARITY_MAXIMUM_REPEAT_DISTANCE // for repeat detection

#else
#define ENABLE_NEC_REPEAT_SUPPORT    // Activating this, enables detection of special short frame NEC repeats. Requires 40 bytes program memory.

#define TINY_ADDRESS_BITS          NEC_ADDRESS_BITS // the address bits + parity
#define TINY_COMMAND_BITS          NEC_COMMAND_BITS // the command bits + parity
#define TINY_ADDRESS_HAS_8_BIT_PARITY  true
#define TINY_COMMAND_HAS_8_BIT_PARITY  true

#define TINY_BITS                  NEC_BITS
#define TINY_UNIT                  NEC_UNIT

#define TINY_HEADER_MARK           NEC_HEADER_MARK
#define TINY_HEADER_SPACE          NEC_HEADER_SPACE

#define TINY_BIT_MARK              NEC_BIT_MARK
#define TINY_ONE_SPACE             NEC_ONE_SPACE
#define TINY_ZERO_SPACE            NEC_ZERO_SPACE

#define TINY_MAXIMUM_REPEAT_DISTANCE  NEC_MAXIMUM_REPEAT_DISTANCE
#endif

/*
 * This function is called if a complete command was received and must be implemented by the including file (user code)
 * We have 6 cases: 0, 8 bit or 16 bit address, each with 8 or 16 bit command
 */
#if (TINY_ADDRESS_BITS > 0)
#  if TINY_ADDRESS_HAS_8_BIT_PARITY
// 8 bit address here
#    if TINY_COMMAND_HAS_8_BIT_PARITY
extern void handleReceivedTinyIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags);
#    else
extern void handleReceivedTinyIRData(uint8_t aAddress, uint16_t aCommand, uint8_t aFlags);
#    endif

#  else // TINY_ADDRESS_HAS_8_BIT_PARITY
// 16 bit address here
#    if TINY_COMMAND_HAS_8_BIT_PARITY
extern void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, uint8_t aFlags);
#    else
extern void handleReceivedTinyIRData(uint16_t aAddress, uint16_t aCommand, uint8_t aFlags);
#    endif
#  endif

#else
// No address here
#  if TINY_COMMAND_HAS_8_BIT_PARITY
extern void handleReceivedTinyIRData(uint8_t aCommand, uint8_t aFlags);
#  else
extern void handleReceivedTinyIRData(uint16_t aCommand, uint8_t aFlags);
#  endif
#endif

#if !defined(MICROS_IN_ONE_SECOND)
#define MICROS_IN_ONE_SECOND 1000000L
#endif

#if !defined(MICROS_IN_ONE_MILLI)
#define MICROS_IN_ONE_MILLI 1000L
#endif

/*
 * Macros for comparing timing values
 */
#define lowerValue25Percent(aDuration)   (aDuration - (aDuration / 4))
#define upperValue25Percent(aDuration)   (aDuration + (aDuration / 4))
#define lowerValue50Percent(aDuration)   (aDuration / 2) // (aDuration - (aDuration / 2))
#define upperValue50Percent(aDuration)   (aDuration + (aDuration / 2))

/*
 * The states for the state machine
 */
#define IR_RECEIVER_STATE_WAITING_FOR_START_MARK        0
#define IR_RECEIVER_STATE_WAITING_FOR_START_SPACE       1
#define IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK   2
#define IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE        3
#define IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK         4
#define IR_RECEIVER_STATE_WAITING_FOR_STOP_MARK         5
/**
 * Control and data variables of the state machine for TinyReceiver
 */
struct TinyIRReceiverStruct {
    /*
     * State machine
     */
    uint32_t LastChangeMicros;      ///< Microseconds of last Pin Change Interrupt.
    uint8_t IRReceiverState;        ///< The state of the state machine.
    uint8_t IRRawDataBitCounter;    ///< How many bits are currently contained in raw data.
    /*
     * Data
     */
#if (TINY_BITS > 16)
    uint32_t IRRawDataMask;         ///< The corresponding bit mask for IRRawDataBitCounter.
    LongUnion IRRawData;            ///< The current raw data. LongUnion helps with decoding of address and command.
#else
    uint16_t IRRawDataMask;         ///< The corresponding bit mask for IRRawDataBitCounter.
    WordUnion IRRawData;            ///< The current raw data. WordUnion helps with decoding of command.
#endif
    uint8_t Flags;  ///< One of IRDATA_FLAGS_EMPTY, IRDATA_FLAGS_IS_REPEAT, and IRDATA_FLAGS_PARITY_FAILED
};

/*
 * Definitions for member TinyIRReceiverCallbackDataStruct.Flags
 * From IRremoteInt.h
 */
#define IRDATA_FLAGS_EMPTY              0x00
#define IRDATA_FLAGS_IS_REPEAT          0x01
#define IRDATA_FLAGS_IS_AUTO_REPEAT     0x02 // not used here, overwritten with _IRDATA_FLAGS_IS_SHORT_REPEAT
#define IRDATA_FLAGS_PARITY_FAILED      0x04 ///< the current (autorepeat) frame violated parity check

/**
 * Can be used by the callback to transfer received data to main loop for further processing.
 * E.g. with volatile struct TinyIRReceiverCallbackDataStruct sCallbackData;
 */
struct TinyIRReceiverCallbackDataStruct {
#if (TINY_ADDRESS_BITS > 0)
#  if (TINY_ADDRESS_BITS == 16) && !TINY_ADDRESS_HAS_8_BIT_PARITY
    uint16_t Address;
#  else
    uint8_t Address;
#  endif
#endif

#  if (TINY_COMMAND_BITS == 16) && !TINY_COMMAND_HAS_8_BIT_PARITY
    uint16_t Command;
#else
    uint8_t Command;
#endif
    uint8_t Flags; // Bit coded flags. Can contain one of the bits: IRDATA_FLAGS_IS_REPEAT and IRDATA_FLAGS_PARITY_FAILED
    bool justWritten; ///< Is set true if new data is available. Used by the main loop, to avoid multiple evaluations of the same IR frame.
};

bool initPCIInterruptForTinyReceiver();
bool enablePCIInterruptForTinyReceiver();
void disablePCIInterruptForTinyReceiver();
bool isTinyReceiverIdle();
#if defined(USE_FAST_8_BIT_AND_PARITY_TIMING)
void printTinyReceiverResultMinimal(uint16_t aCommand, uint8_t aFlags, Print *aSerial);
#else
void printTinyReceiverResultMinimal(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags, Print *aSerial);
#endif

void sendFast8BitAndParity(uint8_t aSendPin, uint8_t aCommand, uint_fast8_t aNumberOfRepeats = 0);
void sendNECMinimal(uint8_t aSendPin, uint8_t aCommand, uint_fast8_t aNumberOfRepeats = 0);
/** @}*/

#endif // _TINY_IR_H
