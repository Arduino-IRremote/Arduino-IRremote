/*
 * IRCommandDispatcher.h
 *
 * Library to process IR commands by calling functions specified in a mapping array.
 *
 * To run this example you need to install the "IRremote" or "IRMP" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"
 *
 *  Copyright (C) 2019-2026  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of ServoEasing https://github.com/ArminJo/ServoEasing.
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  IRCommandDispatcher is free software: you can redistribute it and/or modify
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
 */

#ifndef _IR_COMMAND_DISPATCHER_H
#define _IR_COMMAND_DISPATCHER_H

#include <stdint.h>

//#define DISPATCHER_IR_COMMAND_HAS_MORE_THAN_8_BIT // Enables mapping and dispatching of IR commands consisting of more than 8 bits. Saves up to 160 bytes program memory and 5 bytes RAM + 1 byte RAM per mapping entry.
//#define USE_DISPATCHER_COMMAND_STRINGS // Enables the printing of command strings. Requires additional 2 bytes RAM for each command mapping. Requires program memory for strings, but saves snprintf() code (1.5k) if INFO or DEBUG is activated, which has no effect if snprintf() is also used in other parts of your program / libraries.
#if defined(USE_DISPATCHER_COMMAND_STRINGS)
#define COMMAND_STRING(anyString)   anyString
#else
#define COMMAND_STRING(anyString)
#endif
/*
 * For command mapping file
 */
#define IR_COMMAND_FLAG_BLOCKING        0x00 // default - blocking command, repeat not accepted, only one command at a time. Stops an already running command.
#define IR_COMMAND_FLAG_REPEATABLE      0x01 // repeat accepted
#define IR_COMMAND_FLAG_NON_BLOCKING    0x02 // Non blocking (short) command that can be processed any time and may interrupt other IR commands - used for stop, set direction etc.
#define IR_COMMAND_FLAG_REPEATABLE_NON_BLOCKING (IR_COMMAND_FLAG_REPEATABLE | IR_COMMAND_FLAG_NON_BLOCKING)
#define IR_COMMAND_FLAG_BEEP            0x04 // Do a single short beep before executing command. May not be useful for short or repeating commands.
#define IR_COMMAND_FLAG_BLOCKING_BEEP   (IR_COMMAND_FLAG_BLOCKING | IR_COMMAND_FLAG_BEEP)

#if !defined(IS_STOP_REQUESTED)
#define IS_STOP_REQUESTED               IRDispatcher.requestToStopReceived
#endif
#if !defined(RETURN_IF_STOP)
#define RETURN_IF_STOP                  if (IRDispatcher.requestToStopReceived) return
#endif
#if !defined(BREAK_IF_STOP)
#define BREAK_IF_STOP                   if (IRDispatcher.requestToStopReceived) break
#endif
#if !defined(DELAY_AND_RETURN_IF_STOP)
#define DELAY_AND_RETURN_IF_STOP(aDurationMillis)   if (IRDispatcher.delayAndCheckForStop(aDurationMillis)) return
#endif

/*
 * Define as COMMAND_EMPTY a code which is not sent by the remote - otherwise please redefine it here
 */
#if defined(DISPATCHER_IR_COMMAND_HAS_MORE_THAN_8_BIT)
#define COMMAND_EMPTY       __UINT_FAST16_MAX__ // 0xFFFF code no command
typedef uint_fast16_t IRCommandType;
#else
typedef uint_fast8_t IRCommandType;
#define COMMAND_EMPTY       __UINT_FAST8_MAX__ // 0xFF code no command
#endif

// Basic mapping structure
struct IRToCommandMappingStruct {
    IRCommandType IRCode;
    uint8_t Flags;
    void (*CommandToCall)();
#if defined(USE_DISPATCHER_COMMAND_STRINGS)
    const char *CommandString;
#endif
};

struct IRDataForCommandDispatcherStruct {
    uint16_t address;           // to distinguish between multiple senders
    IRCommandType command;
    bool isRepeat;
    volatile uint32_t MillisOfLastCode;  // millis() of last IR command -including repeats!- received - for timeouts etc.
    volatile bool isAvailable; // flag for a polling interpreting function, that a new command has arrived. Is set true by library and set false by main loop.
};

class IRCommandDispatcher {
public:
    void init();
    void printIRInfo(Print *aSerial);

    bool checkAndRunNonBlockingCommands();
    bool checkAndRunSuspendedBlockingCommands();
    void setNextBlockingCommand(IRCommandType aBlockingCommandToRunNext);
    bool delayAndCheckForStop(uint16_t aDelayMillis);

    // The main dispatcher function
    void checkAndCallCommand(bool aCallBlockingCommandImmediately);

    void printIRCommandString(Print *aSerial);
    void printIRCommandString(Print *aSerial, uint_fast8_t aMappingArrayIndex);
    void setRequestToStopReceived(bool aRequestToStopReceived = true);

    IRCommandType currentBlockingCommandCalled = COMMAND_EMPTY; // The code for the current called command
    IRCommandType lastBlockingCommandCalled = COMMAND_EMPTY;  // The code for the last called command. Can be evaluated by main loop
    IRCommandType BlockingCommandToRunNext = COMMAND_EMPTY; // Storage for command currently suspended to allow the current command to end, before it is called by main loop
    bool justCalledBlockingCommand = false;  // Flag that a blocking command was received and called - is set before call of command
    /*
     * Flag for running blocking commands to terminate. To check, you can use "if (IRDispatcher.requestToStopReceived) return;" (available as macro RETURN_IF_STOP).
     * It is set if a blocking IR command received, which cannot be executed directly. Can be reset by main loop, if command has stopped.
     * It is reset before executing a blocking command.
     */
    volatile bool requestToStopReceived;
    /*
     * This flag must be true, if we have a function, which want to interpret the IR codes by itself e.g. the calibrate function of QuadrupedControl
     */
    bool doNotUseDispatcher = false;

    struct IRDataForCommandDispatcherStruct IRReceivedData;

};

extern IRCommandDispatcher IRDispatcher;

#endif // _IR_COMMAND_DISPATCHER_H
