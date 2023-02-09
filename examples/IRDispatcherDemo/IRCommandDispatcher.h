/*
 * IRCommandDispatcher.h
 *
 * Library to process IR commands by calling functions specified in a mapping array.
 *
 * To run this example you need to install the "IRremote" or "IRMP" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"
 *
 *  Copyright (C) 2019-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of ServoEasing https://github.com/ArminJo/ServoEasing.
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  ServoEasing is free software: you can redistribute it and/or modify
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

/*
 * For command mapping file
 */
#define IR_COMMAND_FLAG_BLOCKING        0x00 // default - blocking command, repeat not accepted, only one command at a time. Stops an already running command.
#define IR_COMMAND_FLAG_REPEATABLE      0x01 // repeat accepted
#define IR_COMMAND_FLAG_NON_BLOCKING    0x02 // Non blocking (short) command that can be processed any time and may interrupt other IR commands - used for stop, set direction etc.
#define IR_COMMAND_FLAG_REPEATABLE_NON_BLOCKING (IR_COMMAND_FLAG_REPEATABLE | IR_COMMAND_FLAG_NON_BLOCKING)

// Basic mapping structure
struct IRToCommandMappingStruct {
#if defined(IR_COMMAND_HAS_MORE_THAN_8_BIT)
    uint16_t IRCode;
#else
    uint8_t IRCode;
#endif
    uint8_t Flags;
    void (*CommandToCall)();
    const char *CommandString;
};

struct IRDataForCommandDispatcherStruct {
    uint16_t address;           // to distinguish between multiple senders
#if defined(IR_COMMAND_HAS_MORE_THAN_8_BIT)
    uint16_t command;
#else
    uint8_t command;
#endif
    bool isRepeat;
    uint32_t MillisOfLastCode;  // millis() of last IR command -including repeats!- received - for timeouts etc.
    volatile bool isAvailable;  // flag for a polling interpreting function, that a new command has arrived. Is set true by library and set false by main loop.
};

/*
 * Special codes (hopefully) not sent by the remote - otherwise please redefine it here
 */
#if defined(IR_COMMAND_HAS_MORE_THAN_8_BIT)
#define COMMAND_EMPTY       0xFFFF // code no command
#else
#define COMMAND_EMPTY       0xFF // code no command
#endif

#define RETURN_IF_STOP  if (IRDispatcher.requestToStopReceived) return
#define BREAK_IF_STOP   if (IRDispatcher.requestToStopReceived) break
#define DELAY_AND_RETURN_IF_STOP(aDurationMillis)   if (IRDispatcher.delayAndCheckForStop(aDurationMillis)) return

class IRCommandDispatcher {
public:
    void init();

    bool checkAndRunNonBlockingCommands();
    bool checkAndRunSuspendedBlockingCommands();
#if defined(IR_COMMAND_HAS_MORE_THAN_8_BIT)
    void setNextBlockingCommand(uint16_t aBlockingCommandToRunNext);
#else
    void setNextBlockingCommand(uint8_t aBlockingCommandToRunNext);
#endif
    bool delayAndCheckForStop(uint16_t aDelayMillis);

    // The main dispatcher function
    void checkAndCallCommand(bool aCallBlockingCommandImmediately);

    void printIRCommandString(Print *aSerial);
    void setRequestToStopReceived(bool aRequestToStopReceived = true);

#if defined(IR_COMMAND_HAS_MORE_THAN_8_BIT)
    uint16_t currentBlockingCommandCalled = COMMAND_EMPTY; // The code for the current called command
    uint16_t lastBlockingCommandCalled = COMMAND_EMPTY; // The code for the last called command. Can be evaluated by main loop
    uint16_t BlockingCommandToRunNext = COMMAND_EMPTY;  // Storage for command currently suspended to allow the current command to end, before it is called by main loop
#else
    uint8_t currentBlockingCommandCalled = COMMAND_EMPTY; // The code for the current called command
    uint8_t lastBlockingCommandCalled = COMMAND_EMPTY;  // The code for the last called command. Can be evaluated by main loop
    uint8_t BlockingCommandToRunNext = COMMAND_EMPTY;   // Storage for command currently suspended to allow the current command to end, before it is called by main loop
#endif
    bool justCalledBlockingCommand = false;             // Flag that a blocking command was received and called - is set before call of command
    /*
     * Flag for running blocking commands to terminate. To check, you can use "if (IRDispatcher.requestToStopReceived) return;" (available as macro RETURN_IF_STOP).
     * Is reset by next IR command received. Can be reset by main loop, if command has stopped.
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
