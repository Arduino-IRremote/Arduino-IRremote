/*
 *  TinyIR.h
 *
 *
 *  Copyright (C) 2021-2026  Armin Joachimsmeyer
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *
 *  TinyIRReceiver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#ifndef _TINY_IR_H
#define _TINY_IR_H

#include <Arduino.h>

#include "LongUnion.h"

/** \addtogroup TinyIRReceiver Minimal receiver for NEC and FAST protocol
 * @{
 */

#define VERSION_TINYIR "3.0.0"
#define VERSION_TINYIR_MAJOR 3
#define VERSION_TINYIR_MINOR 0
#define VERSION_TINYIR_PATCH 0
// The change log is at the bottom of the file

/**
 * Timing for NEC protocol
 *
 * see: https://www.sbprojects.net/knowledge/ir/nec.php
 * LSB first, 1 start bit + 16 bit address + 8 bit data + 8 bit inverted data + 1 stop bit.
 */
#if !defined(NEC_ADDRESS_BITS)
#define NEC_ADDRESS_BITS        16 // 16 bit address or 8 bit address and 8 bit inverted address
#define NEC_COMMAND_BITS        16 // Command and inverted command
#define NEC_BITS                (NEC_ADDRESS_BITS + NEC_COMMAND_BITS)

#define NEC_UNIT                560

#define NEC_HEADER_MARK         (16 * NEC_UNIT) // 8960
#define NEC_HEADER_SPACE        (8 * NEC_UNIT)  // 4480

#define NEC_BIT_MARK            NEC_UNIT
#define NEC_ONE_SPACE           (3 * NEC_UNIT)  // 1680
#define NEC_ZERO_SPACE          NEC_UNIT

#define NEC_REPEAT_HEADER_SPACE (4 * NEC_UNIT)  // 2240

#define NEC_REPEAT_PERIOD       110000 // Commands are repeated every 110 ms (measured from start to start) for as long as the key on the remote control is held down.
#define NEC_MINIMAL_DURATION     49900 // NEC_HEADER_MARK + NEC_HEADER_SPACE + 32 * 2 * NEC_UNIT + NEC_UNIT // 2.5 because we assume more zeros than ones
#define NEC_MAXIMUM_REPEAT_DISTANCE (NEC_REPEAT_PERIOD - NEC_MINIMAL_DURATION + 10000) // 70 ms
#endif

/**
 * The FAST protocol is a proprietary modified JVC protocol without address, with parity and with a shorter header.
 * FAST protocol characteristics:
 * - Bit timing is like NEC or JVC
 * - The header is shorter, 3156 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command, but sending of 16 bit is also supported
 *     leading to a fixed protocol length of (6 + (16 * 3) + 1) * 526 = 55 * 526 = 28930 microseconds or 29 ms.
 * - Repeats are sent as complete frames but in a 50 ms period / with a 21 ms distance.
 */
/*
 Protocol=FAST Address=0x0 Command=0x76 Raw-Data=0x8976 16 bits LSB first
 +2100,-1050
 + 550,- 500 + 550,-1550 + 550,-1550 + 550,- 500
 + 550,-1550 + 550,-1550 + 550,-1550 + 550,- 500
 + 550,-1550 + 550,- 500 + 550,- 500 + 550,-1550
 + 550,- 500 + 550,- 500 + 550,- 500 + 550,-1550
 + 550
 Sum: 28900
 */
#define FAST_KHZ                  38
#define FAST_ADDRESS_BITS          0 // No address
#define FAST_COMMAND_BITS         16 // Command and inverted command (parity)
#define FAST_BITS                 (FAST_ADDRESS_BITS + FAST_COMMAND_BITS)

#define FAST_UNIT                 526 // 20 periods of 38 kHz (526.315789)

#define FAST_BIT_MARK             FAST_UNIT
#define FAST_ONE_SPACE            (3 * FAST_UNIT)     // 1578 -> bit period = 2104
#define FAST_ZERO_SPACE           FAST_UNIT           //  526 -> bit period = 1052

#define FAST_HEADER_MARK          (4 * FAST_UNIT)     // 2104
#define FAST_HEADER_SPACE         (2 * FAST_UNIT)     // 1052

#define FAST_REPEAT_PERIOD        50000 // Commands are repeated every 50 ms (measured from start to start) for as long as the key on the remote control is held down.
#define FAST_MAXIMUM_DURATION     ((6 + (FAST_BITS * 4U)) * FAST_UNIT) // 70 * 526 = 37 ms
#define FAST_REPEAT_DISTANCE      (FAST_REPEAT_PERIOD - FAST_MAXIMUM_DURATION) // 13 ms
#define FAST_MAXIMUM_REPEAT_DISTANCE (FAST_REPEAT_DISTANCE + 10000) // 31 ms for repeat detection

/*
 * Definitions to switch between FAST and NEC/ONKYO timing with the same code.
 */
#if defined(USE_FAST_PROTOCOL)
#define ENABLE_NEC2_REPEATS    // Disables detection of special short frame NEC repeats. Saves 40 bytes program memory.

#define TINY_RECEIVER_ADDRESS_BITS          FAST_ADDRESS_BITS
#define TINY_RECEIVER_COMMAND_BITS          FAST_COMMAND_BITS
#  if !defined(TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY)
#define TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY  true     // 8 bit and 8 bit parity
//#define TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY  false    //  16 bit command without parity - not tested
#  endif

#define TINY_RECEIVER_BITS                  FAST_BITS
#define TINY_RECEIVER_UNIT                  FAST_UNIT

#define TINY_RECEIVER_HEADER_MARK           FAST_HEADER_MARK
#define TINY_RECEIVER_HEADER_SPACE          FAST_HEADER_SPACE
#define TINY_RECEIVER_MARK_TIMEOUT          (2 * FAST_HEADER_MARK)

#define TINY_RECEIVER_BIT_MARK              FAST_BIT_MARK
#define TINY_RECEIVER_ONE_SPACE             FAST_ONE_SPACE
#define TINY_RECEIVER_ZERO_SPACE            FAST_ZERO_SPACE
#define TINY_RECEIVER_ONE_THRESHOLD         (2 * FAST_UNIT)  // 1052

#define TINY_RECEIVER_MAXIMUM_REPEAT_DISTANCE  FAST_MAXIMUM_REPEAT_DISTANCE // for repeat detection

#else // defined(USE_FAST_PROTOCOL)

#define TINY_RECEIVER_ADDRESS_BITS          NEC_ADDRESS_BITS // the address bits + parity
#  if defined(USE_ONKYO_PROTOCOL)
#define TINY_RECEIVER_ADDRESS_HAS_8_BIT_PARITY  false     // 16 bit address without parity
#  elif defined(USE_EXTENDED_NEC_PROTOCOL)
#define TINY_RECEIVER_ADDRESS_HAS_8_BIT_PARITY  false     // 16 bit address without parity
#  else
#define TINY_RECEIVER_ADDRESS_HAS_8_BIT_PARITY  true     // 8 bit and 8 bit parity
#  endif

#define TINY_RECEIVER_COMMAND_BITS          NEC_COMMAND_BITS // the command bits + parity
#  if defined(USE_ONKYO_PROTOCOL)
#define TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY  false    // 16 bit command without parity
#  else
#define TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY  true     // 8 bit and 8 bit parity
#  endif

#define TINY_RECEIVER_BITS                  NEC_BITS
#define TINY_RECEIVER_UNIT                  NEC_UNIT

#define TINY_RECEIVER_HEADER_MARK           NEC_HEADER_MARK
#define TINY_RECEIVER_MARK_TIMEOUT          (2 * NEC_HEADER_MARK)
#define TINY_RECEIVER_HEADER_SPACE          NEC_HEADER_SPACE

#define TINY_RECEIVER_BIT_MARK              NEC_BIT_MARK
#define TINY_RECEIVER_ONE_SPACE             NEC_ONE_SPACE
#define TINY_RECEIVER_ZERO_SPACE            NEC_ZERO_SPACE
#define TINY_RECEIVER_ONE_THRESHOLD         (2 * NEC_UNIT)  // 1120

#define TINY_RECEIVER_MAXIMUM_REPEAT_DISTANCE  NEC_MAXIMUM_REPEAT_DISTANCE
#endif

#if defined(USE_CALLBACK_FOR_TINY_RECEIVER)
/*
 * This function is called, if a complete command was received and must be implemented in the file (user code)
 * which includes this library if USE_CALLBACK_FOR_TINY_RECEIVER is activated.
 */
extern void handleReceivedTinyIRData();
#endif

#if !defined(MICROS_IN_ONE_SECOND)
#define MICROS_IN_ONE_SECOND 1000000L
#endif

#if !defined(MICROS_IN_ONE_MILLISECOND)
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
 * Control and data variables of the state machine for TinyIRReceiver
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
#if (TINY_RECEIVER_BITS > 16)
    uint32_t IRRawDataMask;         ///< The corresponding bit mask for IRRawDataBitCounter.
    LongUnion IRRawData;            ///< The current raw data. LongUnion helps with decoding of address and command.
#else
    uint16_t IRRawDataMask;         ///< The corresponding bit mask for IRRawDataBitCounter.
    WordUnion IRRawData;            ///< The current raw data. WordUnion helps with decoding of command.
#endif
    uint8_t flags;  ///< One of IRDATA_FLAGS_EMPTY, IRDATA_FLAGS_IS_REPEAT, and IRDATA_FLAGS_PARITY_FAILED
};

/*
 * Definitions for member TinyIRReceiverDataStruct.flags
 * This is a copy of flags from IRremoteInt.h
 */
#define IRDATA_FLAGS_EMPTY              0x00
#define IRDATA_FLAGS_IS_REPEAT          0x01
#define IRDATA_FLAGS_IS_AUTO_REPEAT     0x02 // not used for TinyIR
#define IRDATA_FLAGS_PARITY_FAILED      0x04 ///< the current (autorepeat) frame violated parity check

/**
 * Is also filled before calling the user callback to transfer received data to main loop for further processing.
 */
struct TinyIRReceiverDataStruct {
#if (TINY_RECEIVER_ADDRESS_BITS > 0)
#  if (TINY_RECEIVER_ADDRESS_BITS == 16) && !TINY_RECEIVER_ADDRESS_HAS_8_BIT_PARITY
    union{
        uint16_t address;
        uint16_t Address;
    };
#  else
    union{
        uint8_t address;
        uint8_t Address;
    };
#  endif
#endif

#  if (TINY_RECEIVER_COMMAND_BITS == 16) && !TINY_RECEIVER_COMMAND_HAS_8_BIT_PARITY
    union{
         uint16_t command;
         uint16_t Command;
    };
#else
    union{
        uint8_t command;
        uint8_t Command;
    };
#endif
    // Bit coded flags. Can contain one of the bits: IRDATA_FLAGS_IS_REPEAT and IRDATA_FLAGS_PARITY_FAILED
    union{
        uint8_t flags;
        uint8_t Flags;
    };
    bool justWritten; ///< Is set true if new data is available. Used by the main loop / decodeTinyIRReceiver(), to avoid multiple evaluations of the same IR frame.
};
extern volatile TinyIRReceiverDataStruct TinyIRReceiverData;

bool isTinyIRReceiverAttached();
bool initPCIInterruptForTinyIRReceiver();
bool enablePCIInterruptForTinyIRReceiver();
void disablePCIInterruptForTinyIRReceiver();
bool isTinyIRReceiverIdle();
bool decodeTinyIRReceiver();
bool decodeTinyIRReceiver(TinyIRReceiverDataStruct *aData);
void printTinyIRReceiverResultMinimal(Print *aSerial);
void printTinyIRReceiverResultMinimal(Print *aSerial, TinyIRReceiverDataStruct *aTinyIRReceiverDataToPrint);

/*
 * aNumberOfRepeats is unsigned in contrast to IRremote.
 * Therefore there is no possibility to send just one repeat frame with aNumberOfRepeats as -1.
 */
void sendFAST(uint_fast8_t aSendPin, uint8_t aCommand, uint_fast8_t aNumberOfRepeats);
void sendFAST16Bit(uint_fast8_t aSendPin, uint16_t aCommand, uint_fast8_t aNumberOfRepeats);
void sendOnkyo(uint_fast8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendNEC2Repeats = false); // Send NEC with 16 bit command, even if aCommand < 0x100
void sendNEC(uint_fast8_t aSendPin, uint16_t aAddress, uint16_t aCommand, uint_fast8_t aNumberOfRepeats, bool aSendNEC2Repeats = false);

#if defined(NO_LED_FEEDBACK_CODE)
#  if !defined(NO_LED_RECEIVE_FEEDBACK_CODE)
#define NO_LED_RECEIVE_FEEDBACK_CODE
#  endif
#  if !defined(NO_LED_SEND_FEEDBACK_CODE)
#define NO_LED_SEND_FEEDBACK_CODE
#  endif
#endif

#if !defined(IR_FEEDBACK_LED_PIN) && defined(LED_BUILTIN)
#define IR_FEEDBACK_LED_PIN     LED_BUILTIN
#endif

/*
 *  Version 3.0.0 - 11/2026
 *  - Changed: Renamed TinyIRReceiverDecode() to decodeTinyIRReceiver() and isIRReceiverAttachedForTinyIRReceiver() to isTinyIRReceiverAttached().
 *  - Changed: Renamed TinyIRReceiverCallbackDataStruct to TinyIRReceiverDataStruct.
 *  - Changed: Members of TinyIRReceiverDataStruct are now address, command and flags. The old capitalized names still work.
 *  - Changed: Renamed sendONKYO() to sendOnkyo() and sendFast8BitAndParity() to sendFAST8BitAndParity().
 *  - Added: decodeTinyIRReceiver(TinyIRReceiverDataStruct *aData) for atomic copy of received data.
 *  - Removed: sendExtendedNEC(). Use sendNEC(), which sends a 16 bit address if the address is above 0xFF.
 *  - Removed: Default value 0 for parameter aNumberOfRepeats of all send functions.
 *  - Removed: All (replaced) TinyReceiver*() functions.
 *  - Fixed: Race condition and 16 bit timeout bug in receiver, micros() overflow in sender.
 *
 *  Version 2.3.0 - 3/2026
 *  - Changed: Renamed TinyReceiver*() functions to TinyIRReceiver*().
 *
 *  Version 2.2.0 - 7/2024
 *  - New TinyReceiverDecode() function to be used as drop in for IrReceiver.decode().
 *
 *  Version 2.1.0 - 2/2024
 *  - Added: sendExtendedNEC() function and new parameter aSendNEC2Repeats.
 *
 *  Version 2.0.0 - 10/2023
 *  - Changed: Callback function now only enabled if USE_CALLBACK_FOR_TINY_RECEIVER is activated.
 *  - Added: TinyIRReceiverData which is filled with address, command and flags.
 *  - Removed: parameters address, command and flags from callback handleReceivedTinyIRData() and printTinyReceiverResultMinimal().
 *
 *  Version 1.2.0 - 01/2023
 * - Changed: Renamed functions and macros.
 * - Added: ONKYO protocol, NEC with 16 bit address and command, instead of 8 bit + 8 bit parity address and command.
 *
 * Version 1.1.0 - 01/2023
 * - Added: FAST protocol.
 */
/** @}*/

#endif // _TINY_IR_H
