/*
 * IRCommandDispatcher.hpp
 *
 * Library to process IR commands by calling functions specified in a mapping array.
 * Commands can be tagged as blocking or non blocking.
 *
 * To run this example you need to install the "IRremote" or "IRMP" library.
 * Install it under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"
 *
 * The IR library calls a callback function, which executes a non blocking command directly in ISR context!
 * A blocking command is stored and sets a stop flag for an already running blocking function to terminate.
 * The blocking command can in turn be executed by main loop by calling IRDispatcher.checkAndRunSuspendedBlockingCommands().
 *
 *  Copyright (C) 2019-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of ServoEasing https://github.com/ArminJo/ServoEasing.
 *  This file is part of IRMP https://github.com/ukw100/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  ServoEasing is free software: you can redistribute it and/or modify
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 */
#ifndef IR_COMMAND_DISPATCHER_HPP
#define IR_COMMAND_DISPATCHER_HPP

#include <Arduino.h>

#include "IRCommandDispatcher.h"

//#define INFO // activate this out to see serial info output
//#define DEBUG // activate this out to see serial info output
#if defined(DEBUG) && !defined(INFO)
// Propagate level
#define INFO
#endif

IRCommandDispatcher IRDispatcher;

#if defined(USE_TINY_IR_RECEIVER)
#include "TinyIRReceiver.hpp" // included in "IRremote" library

#if defined(INFO)
#define CD_INFO_PRINT(...)      Serial.print(__VA_ARGS__);
#define CD_INFO_PRINTLN(...)    Serial.println(__VA_ARGS__);
#else
#define CD_INFO_PRINT(...)      void();
#define CD_INFO_PRINTLN(...)    void();
#endif

void IRCommandDispatcher::init() {
    initPCIInterruptForTinyReceiver();
}

/*
 * This is the TinyReceiver callback function which is called if a complete command was received
 * It checks for right address and then call the dispatcher
 */
#if defined(ESP32) || defined(ESP8266)
void IRAM_ATTR handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat)
#  else
void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat)
#  endif
{
    IRDispatcher.IRReceivedData.address = aAddress;
    IRDispatcher.IRReceivedData.command = aCommand;
    IRDispatcher.IRReceivedData.isRepeat = isRepeat;
    IRDispatcher.IRReceivedData.MillisOfLastCode = millis();

    CD_INFO_PRINT(F("A=0x"));
    CD_INFO_PRINT(aAddress, HEX);
    CD_INFO_PRINT(F(" C=0x"));
    CD_INFO_PRINT(aCommand, HEX);
    if (isRepeat) {
        CD_INFO_PRINT(F("R"));
    }
    CD_INFO_PRINTLN();

    if (aAddress == IR_ADDRESS) { // IR_ADDRESS is defined in IRCommandMapping.h
        IRDispatcher.IRReceivedData.isAvailable = true;
        if(!IRDispatcher.doNotUseDispatcher) {
            IRDispatcher.checkAndCallCommand(false); // only short commands are executed directly
        }

    } else {
        CD_INFO_PRINT(F("Wrong address. Expected 0x"));
        CD_INFO_PRINTLN(IR_ADDRESS, HEX);
    }
}

#elif defined(USE_IRMP_LIBRARY)
#if !defined(IRMP_USE_COMPLETE_CALLBACK)
# error IRMP_USE_COMPLETE_CALLBACK must be activated for IRMP library
#endif

void IRCommandDispatcher::init() {
    irmp_init();
}

/*
 * This is the callback function is called if a complete command was received
 */
#if defined(ESP32) || defined(ESP8266)
void IRAM_ATTR handleReceivedIRData()
#else
void handleReceivedIRData()
#endif
{
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
#if !defined(ARDUINO_ARCH_MBED)
    interrupts(); // be careful with always executable commands which lasts longer than the IR repeat duration.
#endif

    if (IRDispatcher.IRReceivedData.address == IR_ADDRESS) {
        IRDispatcher.checkAndCallCommand(true);
#ifdef INFO
        } else {
        CD_INFO_PRINT(F("Wrong address. Expected 0x"));
        CD_INFO_PRINTLN(IR_ADDRESS, HEX);
#endif
    }
}
#endif

/*
 * The main dispatcher function
 * Sets flags justCalledRegularIRCommand, executingBlockingCommand
 */
void IRCommandDispatcher::checkAndCallCommand(bool aCallAlsoBlockingCommands) {
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
#ifdef INFO
            const __FlashStringHelper *tCommandName = reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString);
#endif
            /*
             * Check for repeat and if it is allowed for the current command
             */
            if (IRReceivedData.isRepeat && !(IRMapping[i].Flags & IR_COMMAND_FLAG_REPEATABLE)) {
#ifdef DEBUG
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
#ifdef DEBUG
                Serial.print(F("Recursive command \""));
                Serial.print(tCommandName);
                Serial.println("\" not accepted");
#endif
                return;
            }

            /*
             * Handle stop commands
             */
            if (IRMapping[i].Flags & IR_COMMAND_FLAG_IS_STOP_COMMAND) {
                requestToStopReceived = true;
                CD_INFO_PRINTLN(F("Stop command received"));
            } else {
                // lets start a new turn
                requestToStopReceived = false;
            }

            bool tIsNonBlockingCommand = (IRMapping[i].Flags & IR_COMMAND_FLAG_NON_BLOCKING);
            if (tIsNonBlockingCommand) {
                // short command here, just call
                CD_INFO_PRINT(F("Run non blocking command: "));
                CD_INFO_PRINTLN (tCommandName);
                IRMapping[i].CommandToCall();
            } else {
                if (!aCallAlsoBlockingCommands) {
                    /*
                     * Store for main loop to execute
                     */
                    BlockingCommandToRunNext = IRReceivedData.command;
                    requestToStopReceived = true; // to stop running command
                    CD_INFO_PRINT(F("Blocking command "));
                    CD_INFO_PRINT (tCommandName);
                    CD_INFO_PRINTLN(F(" stored as next command and requested stop"));
                } else {
                    if (executingBlockingCommand) {
                        // Logical error has happened
                        CD_INFO_PRINTLN(
                                F("Request to execute blocking command while another command is running. This should not happen!"));
                        /*
                         * A blocking command may not be called as long as another blocking command is running.
                         * Try to stop again
                         */
                        BlockingCommandToRunNext = IRReceivedData.command;
                        requestToStopReceived = true; // to stop running command
                        return;
                    }

                    /*
                     * here we are called from main loop and execute a command
                     */
                    justCalledBlockingCommand = true;
                    executingBlockingCommand = true; // set lock for recursive calls
                    currentBlockingCommandCalled = IRReceivedData.command;
                    /*
                     * This call is blocking!!!
                     */
                    CD_INFO_PRINT(F("Run blocking command: "));
                    CD_INFO_PRINTLN (tCommandName);

                    IRMapping[i].CommandToCall();
#ifdef TRACE
                    Serial.println(F("End of blocking command"));
#endif
                    executingBlockingCommand = false;
                    currentBlockingCommandCalled = COMMAND_INVALID;
                }

            }
            break; // command found
        }
    } // for loop
    return;
}

/*
 * Special delay function for the IRCommandDispatcher. Returns prematurely if requestToStopReceived is set.
 * To be used in blocking functions as delay
 * @return  true - as soon as stop received
 */
bool IRCommandDispatcher::delayAndCheckForStop(uint16_t aDelayMillis) {
    uint32_t tStartMillis = millis();
    do {
        if (IRDispatcher.requestToStopReceived) {
            return true;
        }
    } while (millis() - tStartMillis < aDelayMillis);
    return false;
}

/*
 * Intended to be called from main loop
 */
void IRCommandDispatcher::checkAndRunSuspendedBlockingCommands() {
    /*
     * search IR code or take last rejected command and call associated function
     */
    if (BlockingCommandToRunNext != COMMAND_INVALID) {

        CD_INFO_PRINT(F("Take stored command = 0x"));
        CD_INFO_PRINTLN(BlockingCommandToRunNext, HEX);

        IRReceivedData.command = BlockingCommandToRunNext;
        BlockingCommandToRunNext = COMMAND_INVALID;
        IRReceivedData.isRepeat = false;
        checkAndCallCommand(true);
    }
}

void IRCommandDispatcher::printIRCommandString(Print *aSerial) {
    aSerial->print(F("IRCommand="));
    for (uint_fast8_t i = 0; i < sizeof(IRMapping) / sizeof(struct IRToCommandMappingStruct); ++i) {
        if (IRReceivedData.command == IRMapping[i].IRCode) {
            aSerial->println(reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString));
            return;
        }
    }
    aSerial->println(reinterpret_cast<const __FlashStringHelper*>(unknown));
}

void IRCommandDispatcher::setRequestToStopReceived() {
    requestToStopReceived = true;
}

#endif // #ifndef IR_COMMAND_DISPATCHER_HPP
#pragma once
