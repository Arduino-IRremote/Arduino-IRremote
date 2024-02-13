/*
 * IRCommandDispatcher.hpp
 *
 * Library to process IR commands by calling functions specified in a mapping array.
 * Commands can be tagged as blocking or non blocking.
 *
 * To run this example you need to install the "IRremote" or "IRMP" library.
 * Install it under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"
 *
 * The IR library calls a callback function, which executes a non blocking command directly in ISR (Interrupt Service Routine) context!
 * A blocking command is stored and sets a stop flag for an already running blocking function to terminate.
 * The blocking command can in turn be executed by main loop by calling IRDispatcher.checkAndRunSuspendedBlockingCommands().
 *
 *  Copyright (C) 2019-2024  Armin Joachimsmeyer
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

/*
 * Program behavior is modified by the following macros
 * USE_TINY_IR_RECEIVER
 * USE_IRMP_LIBRARY
 * IR_COMMAND_HAS_MORE_THAN_8_BIT
 */

#ifndef _IR_COMMAND_DISPATCHER_HPP
#define _IR_COMMAND_DISPATCHER_HPP

#include <Arduino.h>

#include "IRCommandDispatcher.h"

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
/*
 * Enable this to see information on each call.
 * Since there should be no library which uses Serial, it should only be enabled for development purposes.
 */
#if defined(INFO) && !defined(LOCAL_INFO)
#define LOCAL_INFO
#else
//#define LOCAL_INFO // This enables info output only for this file
#endif
#if defined(DEBUG) && !defined(LOCAL_DEBUG)
#define LOCAL_DEBUG
// Propagate debug level
#define LOCAL_INFO
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

IRCommandDispatcher IRDispatcher;

#if defined(LOCAL_INFO)
#define CD_INFO_PRINT(...)      Serial.print(__VA_ARGS__);
#define CD_INFO_PRINTLN(...)    Serial.println(__VA_ARGS__);
#else
#define CD_INFO_PRINT(...)      void();
#define CD_INFO_PRINTLN(...)    void();
#endif

#if defined(USE_TINY_IR_RECEIVER)
#define USE_CALLBACK_FOR_TINY_RECEIVER  // Call the fixed function "void handleReceivedTinyIRData()" each time a frame or repeat is received.
#include "TinyIRReceiver.hpp" // included in "IRremote" library

void IRCommandDispatcher::init() {
    initPCIInterruptForTinyReceiver();
}

/*
 * @return true, if IR Receiver is attached
 */
void IRCommandDispatcher::printIRInfo(Print *aSerial) {
    aSerial->println();
    // For available IR commands see IRCommandMapping.h https://github.com/ArminJo/PWMMotorControl/blob/master/examples/SmartCarFollower/IRCommandMapping.h
    aSerial->print(F("Listening to IR remote of type "));
    aSerial->print(IR_REMOTE_NAME);
    aSerial->println(F(" at pin " STR(IR_RECEIVE_PIN)));
}

/*
 * This is the TinyReceiver callback function, which is called if a complete command was received
 * It checks for right address and then call the dispatcher
 */
#  if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#  endif
void handleReceivedTinyIRData() {
    IRDispatcher.IRReceivedData.address = TinyIRReceiverData.Address;
    IRDispatcher.IRReceivedData.command = TinyIRReceiverData.Command;
    IRDispatcher.IRReceivedData.isRepeat = TinyIRReceiverData.Flags & IRDATA_FLAGS_IS_REPEAT;
    IRDispatcher.IRReceivedData.MillisOfLastCode = millis();

#  if defined(LOCAL_INFO)
    printTinyReceiverResultMinimal(&Serial);
#  endif

    if (TinyIRReceiverData.Address == IR_ADDRESS) { // IR_ADDRESS is defined in *IRCommandMapping.h
        IRDispatcher.IRReceivedData.isAvailable = true;
        if(!IRDispatcher.doNotUseDispatcher) {
            /*
             * Only short (non blocking) commands are executed directly in ISR (Interrupt Service Routine) context,
             * others are stored for main loop which calls checkAndRunSuspendedBlockingCommands()
             */
            IRDispatcher.checkAndCallCommand(false);
        }

    } else {
        CD_INFO_PRINT(F("Wrong address. Expected 0x"));
        CD_INFO_PRINTLN(IR_ADDRESS, HEX);
    }
}

#elif defined(USE_IRMP_LIBRARY)
#  if !defined(IRMP_USE_COMPLETE_CALLBACK)
# error IRMP_USE_COMPLETE_CALLBACK must be activated for IRMP library
#  endif

void IRCommandDispatcher::init() {
    irmp_init();
}

/*
 * This is the callback function, which is called if a complete command was received
 */
#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif
void handleReceivedIRData() {
    IRMP_DATA tTeporaryData;
    irmp_get_data(&tTeporaryData);
    IRDispatcher.IRReceivedData.address = tTeporaryData.address;
    IRDispatcher.IRReceivedData.command = tTeporaryData.command;
    IRDispatcher.IRReceivedData.isRepeat = tTeporaryData.flags & IRMP_FLAG_REPETITION;
    IRDispatcher.IRReceivedData.MillisOfLastCode = millis();

    CD_INFO_PRINT(F("A=0x"));
    CD_INFO_PRINT(IRDispatcher.IRReceivedData.address, HEX);
    CD_INFO_PRINT(F(" C=0x"));
    CD_INFO_PRINT(IRDispatcher.IRReceivedData.command, HEX);
    if (IRDispatcher.IRReceivedData.isRepeat) {
        CD_INFO_PRINT(F("R"));
    }
    CD_INFO_PRINTLN();

    // To enable delay() for commands
#  if !defined(ARDUINO_ARCH_MBED)
    interrupts(); // be careful with always executable commands which lasts longer than the IR repeat duration.
#  endif

    if (IRDispatcher.IRReceivedData.address == IR_ADDRESS) {
        IRDispatcher.checkAndCallCommand(true);
    } else {
        CD_INFO_PRINT(F("Wrong address. Expected 0x"));
        CD_INFO_PRINTLN(IR_ADDRESS, HEX);
    }
}
#endif // elif defined(USE_IRMP_LIBRARY)

/*
 * The main dispatcher function called by IR-ISR, main loop and checkAndRunSuspendedBlockingCommands()
 * Non blocking commands are executed directly, blocking commands are executed if enabled by parameter and no other command is just running.
 * Otherwise request to stop (requestToStopReceived) is set and command is stored for main loop to be later execute by checkAndRunSuspendedBlockingCommands().
 * Sets flags justCalledRegularIRCommand, executingBlockingCommand, requestToStopReceived
 * @param aCallBlockingCommandImmediately Run blocking command directly, if no other command is just running. Should be false if called by ISR in order not to block ISR.
 */
void IRCommandDispatcher::checkAndCallCommand(bool aCallBlockingCommandImmediately) {
    if (IRReceivedData.command == COMMAND_EMPTY) {
        return;
    }

    /*
     * Search for command in Array of IRToCommandMappingStruct
     */
    for (uint_fast8_t i = 0; i < sizeof(IRMapping) / sizeof(struct IRToCommandMappingStruct); ++i) {
        if (IRReceivedData.command == IRMapping[i].IRCode) {
            /*
             * Command found
             */
#if defined(LOCAL_INFO)
            const __FlashStringHelper *tCommandName = reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString);
#endif
            /*
             * Check for repeat and if repeat is allowed for the current command
             */
            if (IRReceivedData.isRepeat && !(IRMapping[i].Flags & IR_COMMAND_FLAG_REPEATABLE)) {
#if defined(LOCAL_DEBUG)
                Serial.print(F("Repeats of command \""));
                Serial.print(tCommandName);
                Serial.println("\" not accepted");
#endif
                return;
            }

            /*
             * Do not accept recursive call of the same command
             */
            if (currentBlockingCommandCalled == IRReceivedData.command) {
#if defined(LOCAL_DEBUG)
                Serial.print(F("Recursive command \""));
                Serial.print(tCommandName);
                Serial.println("\" not accepted");
#endif
                return;
            }

            /*
             * Execute commands
             */
            bool tIsNonBlockingCommand = (IRMapping[i].Flags & IR_COMMAND_FLAG_NON_BLOCKING);
            if (tIsNonBlockingCommand) {
                // short command here, just call
                CD_INFO_PRINT(F("Run non blocking command: "));
                CD_INFO_PRINTLN(tCommandName);
#if defined(BUZZER_PIN) && defined(USE_TINY_IR_RECEIVER)
                    /*
                     * Do (non blocking) buzzer feedback before command is executed
                     */
                    if(IRMapping[i].Flags & IR_COMMAND_FLAG_BEEP) {
                        tone(BUZZER_PIN, 2200, 50);
                    }
#endif
                IRMapping[i].CommandToCall();
            } else {
                /*
                 * Blocking command here
                 */
                if (aCallBlockingCommandImmediately && currentBlockingCommandCalled == COMMAND_EMPTY) {
                    /*
                     * Here no blocking command was running and we are called from main loop
                     */
                    requestToStopReceived = false;  // Do not stop the command executed now
                    justCalledBlockingCommand = true;
                    currentBlockingCommandCalled = IRReceivedData.command;  // set lock for recursive calls
                    lastBlockingCommandCalled = IRReceivedData.command;     // set history, can be evaluated by main loop
                    /*
                     * This call is blocking!!!
                     */
                    CD_INFO_PRINT(F("Run blocking command: "));
                    CD_INFO_PRINTLN(tCommandName);

#if defined(BUZZER_PIN) && defined(USE_TINY_IR_RECEIVER)
                    /*
                     * Do (non blocking) buzzer feedback before command is executed
                     */
                    if(IRMapping[i].Flags & IR_COMMAND_FLAG_BEEP) {
                        tone(BUZZER_PIN, 2200, 50);
                    }
#endif

                    IRMapping[i].CommandToCall();
#if defined(TRACE)
                    Serial.println(F("End of blocking command"));
#endif
                    currentBlockingCommandCalled = COMMAND_EMPTY;
                } else {
                    /*
                     * Called by ISR or another command still running.
                     * Do not run command directly, but set request to stop to true and store command
                     * for main loop to execute by checkAndRunSuspendedBlockingCommands()
                     */
                    BlockingCommandToRunNext = IRReceivedData.command;
                    requestToStopReceived = true; // to stop running command
                    CD_INFO_PRINT(F("Requested stop and stored blocking command "));
                    CD_INFO_PRINT(tCommandName);
                    CD_INFO_PRINTLN(F(" as next command to run."));
                }
            }
            break; // command found
        } // if (IRReceivedData.command == IRMapping[i].IRCode)
    } // for loop
    return;
}

/*
 * Intended to be called from main loop
 * @return true, if command was called
 */
bool IRCommandDispatcher::checkAndRunSuspendedBlockingCommands() {
    /*
     * Take last rejected command and call associated function
     */
    if (BlockingCommandToRunNext != COMMAND_EMPTY) {

        CD_INFO_PRINT(F("Run stored command=0x"));
        CD_INFO_PRINTLN(BlockingCommandToRunNext, HEX);

        IRReceivedData.command = BlockingCommandToRunNext;
        BlockingCommandToRunNext = COMMAND_EMPTY;
        IRReceivedData.isRepeat = false;
        requestToStopReceived = false; // Do not stop the command executed now
        checkAndCallCommand(true);
        return true;
    }
    return false;
}

/*
 * Not used internally
 */
#if defined(IR_COMMAND_HAS_MORE_THAN_8_BIT)
void IRCommandDispatcher::setNextBlockingCommand(uint16_t aBlockingCommandToRunNext)
#else
void IRCommandDispatcher::setNextBlockingCommand(uint8_t aBlockingCommandToRunNext)
#endif
        {
    CD_INFO_PRINT(F("Set next command to run to 0x"));
    CD_INFO_PRINTLN(aBlockingCommandToRunNext, HEX);
    BlockingCommandToRunNext = aBlockingCommandToRunNext;
    requestToStopReceived = true;
}

/*
 * Special delay function for the IRCommandDispatcher. Returns prematurely if requestToStopReceived is set.
 * To be used in blocking functions as delay
 * @return  true - as soon as stop received
 */
bool IRCommandDispatcher::delayAndCheckForStop(uint16_t aDelayMillis) {
    uint32_t tStartMillis = millis();
    do {
        if (requestToStopReceived) {
            return true;
        }
    } while (millis() - tStartMillis < aDelayMillis);
    return false;
}

void IRCommandDispatcher::printIRCommandString(Print *aSerial) {
    for (uint_fast8_t i = 0; i < sizeof(IRMapping) / sizeof(struct IRToCommandMappingStruct); ++i) {
        if (IRReceivedData.command == IRMapping[i].IRCode) {
            aSerial->println(reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString));
            return;
        }
    }
    aSerial->println(reinterpret_cast<const __FlashStringHelper*>(unknown));
}

void IRCommandDispatcher::setRequestToStopReceived(bool aRequestToStopReceived) {
    requestToStopReceived = aRequestToStopReceived;
}

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#if defined(LOCAL_INFO)
#undef LOCAL_INFO
#endif
#endif // _IR_COMMAND_DISPATCHER_HPP
