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

//#define USE_TINY_IR_RECEIVER // Recommended and default, but only for NEC protocol!!! If disabled and IRMP_INPUT_PIN is defined, the IRMP library is used for decoding
//#define USE_IRREMOTE_LIBRARY // The IRremote library is used for decoding
//#define USE_IRMP_LIBRARY     // The IRMP library is used for decoding
#if !defined(USE_TINY_IR_RECEIVER) && !defined(USE_IRREMOTE_LIBRARY) && !defined(USE_IRMP_LIBRARY)
#define USE_TINY_IR_RECEIVER // Set TiniIR as default library
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
#if defined(DEBUG)
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
/******************************
 * Code for the TinyIR library
 ******************************/
#define USE_CALLBACK_FOR_TINY_RECEIVER  // Call the function "handleReceivedTinyIRData()" below each time a frame or repeat is received.
#include "TinyIRReceiver.hpp" // included in "IRremote" and "IRMP" library

void IRCommandDispatcher::init() {
    initPCIInterruptForTinyReceiver();
}
/*
 * This is the TinyReceiver callback function, which is called if a complete command was received.
 * Interrupts are enabled here to allow e.g. delay() in commands.
 * Copy the (volatile) IR data in order not to be overwritten on receiving of next frame.
 * Next, check for right address if IR_ADDRESS is defined.
 * At last call the dispatcher.
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

#  if defined(IR_ADDRESS)
    // if available, compare address
    if (TinyIRReceiverData.Address != IR_ADDRESS) { // IR_ADDRESS is defined in *IRCommandMapping.h
        CD_INFO_PRINT(F("Wrong address. Expected 0x"));
        CD_INFO_PRINTLN(IR_ADDRESS, HEX);
    } else
#  endif
    {
        IRDispatcher.IRReceivedData.isAvailable = true;
        // check if dispatcher enabled
        if(!IRDispatcher.doNotUseDispatcher) {
            /*
             * Only short (non blocking) commands are executed directly in ISR (Interrupt Service Routine) context,
             * others are stored for main loop which calls checkAndRunSuspendedBlockingCommands()
             */
            IRDispatcher.checkAndCallCommand(false);
        }
    }
}

#elif defined(USE_IRMP_LIBRARY)
/******************************
 * Code for the IRMP library
 ******************************/
#define IRMP_USE_COMPLETE_CALLBACK  1 // Enable callback functionality. It is required if IRMP library is used.
#define IRMP_PROTOCOL_NAMES         1 // Enable protocol number mapping to protocol strings for printIRInfo()
#include "irmp.hpp"
void handleReceivedIRData();

void IRCommandDispatcher::init()
{
    irmp_init();
    irmp_register_complete_callback_function(&handleReceivedIRData); // fixed function in IRCommandDispatcher.hpp
}

/*
 * This is the callback function, which is called if a complete command was received.
 * Interrupts are NOT enabled here so we must do it manually to allow e.g. delay() in commands.
 * Copy the (volatile) IR data in order not to be overwritten on receiving of next frame.
 * Next, check for right address if IR_ADDRESS is defined.
 * At last call the dispatcher.
 */
#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif
void handleReceivedIRData()
{
    IRMP_DATA tTeporaryData;
    irmp_get_data(&tTeporaryData);
    IRDispatcher.IRReceivedData.address = tTeporaryData.address;
    IRDispatcher.IRReceivedData.command = tTeporaryData.command;
    IRDispatcher.IRReceivedData.isRepeat = tTeporaryData.flags & IRMP_FLAG_REPETITION;
    IRDispatcher.IRReceivedData.MillisOfLastCode = millis();

#if !defined(ARDUINO_ARCH_MBED) && !defined(ESP32) // no Serial etc. possible in callback for RTOS based cores like ESP, even when interrupts are enabled
    interrupts(); // To enable delay() for commands. Be careful with non-blocking and repeatable commands which lasts longer than the IR repeat duration.
#  endif

    CD_INFO_PRINT(F("A=0x"));
    CD_INFO_PRINT(IRDispatcher.IRReceivedData.address, HEX);
    CD_INFO_PRINT(F(" C=0x"));
    CD_INFO_PRINT(IRDispatcher.IRReceivedData.command, HEX);
    if (IRDispatcher.IRReceivedData.isRepeat)
    {
        CD_INFO_PRINT(F("R"));
    }
    CD_INFO_PRINTLN();

#  if defined(IR_ADDRESS)
    // if available, compare address
    if (IRDispatcher.IRReceivedData.address != IR_ADDRESS) {
        CD_INFO_PRINT(F("Wrong address. Expected 0x"));
        CD_INFO_PRINTLN(IR_ADDRESS, HEX);
    } else
#  endif
    {
        IRDispatcher.IRReceivedData.isAvailable = true;
        // check if dispatcher enabled
        if (!IRDispatcher.doNotUseDispatcher)
        {
            /*
             * Only short (non blocking) commands are executed directly in ISR (Interrupt Service Routine) context,
             * others are stored for main loop which calls checkAndRunSuspendedBlockingCommands()
             */
            IRDispatcher.checkAndCallCommand(false);
        }
    }
}
#endif // elif defined(USE_IRMP_LIBRARY)

/*******************************************
 * Start of the IR library independent code
 *******************************************/
void IRCommandDispatcher::printIRInfo(Print *aSerial)
{
    aSerial->println();
    // For available IR commands see IRCommandMapping.h https://github.com/ArminJo/PWMMotorControl/blob/master/examples/SmartCarFollower/IRCommandMapping.h
#if defined(USE_IRMP_LIBRARY)
#  if defined(IR_REMOTE_NAME)
    aSerial->println(F("Listening to IR remote of type " STR(IR_REMOTE_NAME) " at pin " STR(IRMP_INPUT_PIN)));
#  else
    aSerial->println(F("Listening to IR remote at pin " STR(IRMP_INPUT_PIN)));
#  endif
    Serial.print(F("Accepted protocols are: "));
    irmp_print_active_protocols(&Serial);
    Serial.println();
#else
#  if defined(IR_REMOTE_NAME)
    aSerial->println(F("Listening to IR remote of type " STR(IR_REMOTE_NAME) " at pin " STR(IR_RECEIVE_PIN)));
#  else
    aSerial->println(F("Listening to IR remote at pin " STR(IR_RECEIVE_PIN)));
#  endif
#endif
}

/*
 * The main dispatcher function called by IR-ISR, main loop and checkAndRunSuspendedBlockingCommands()
 * Non blocking commands are executed immediately, blocking commands are executed if no other command is just running.
 * If another blocking command is currently running, the request to stop is set
 * and the command is stored for main loop to be later execute by checkAndRunSuspendedBlockingCommands().
 * This function sets flags justCalledRegularIRCommand, executingBlockingCommand, requestToStopReceived
 * @param aCallBlockingCommandImmediately Run blocking command directly, if no other command is just running.
 *        Should be false if called by ISR in order not to block ISR. Is true when called from checkAndRunSuspendedBlockingCommands().
 */
void IRCommandDispatcher::checkAndCallCommand(bool aCallBlockingCommandImmediately)
{
    if (IRReceivedData.command == COMMAND_EMPTY)
    {
        return;
    }

    /*
     * Search for command in Array of IRToCommandMappingStruct
     */
    for (uint_fast8_t i = 0; i < sizeof(IRMapping) / sizeof(struct IRToCommandMappingStruct); ++i)
    {
        if (IRReceivedData.command == IRMapping[i].IRCode)
        {
            /*
             * Command found
             */
#if defined(LOCAL_INFO)
            const __FlashStringHelper *tCommandName = reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString);
#endif
            /*
             * Check for repeat and if repeat is allowed for the current command
             */
            if (IRReceivedData.isRepeat && !(IRMapping[i].Flags & IR_COMMAND_FLAG_REPEATABLE))
            {
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
            if (currentBlockingCommandCalled == IRReceivedData.command)
            {
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
            if (tIsNonBlockingCommand)
            {
                // short command here, just call
                CD_INFO_PRINT(F("Run non blocking command: "));
                CD_INFO_PRINTLN(tCommandName);
#if defined(DISPATCHER_BUZZER_FEEDBACK_PIN) && defined(USE_TINY_IR_RECEIVER)
                    /*
                     * Do (non blocking) buzzer feedback before command is executed
                     */
                    if(IRMapping[i].Flags & IR_COMMAND_FLAG_BEEP) {
                        tone(DISPATCHER_BUZZER_FEEDBACK_PIN, 2200, 50);
                    }
#endif
                IRMapping[i].CommandToCall();
            }
            else
            {
                /*
                 * Blocking command here
                 */
                if (aCallBlockingCommandImmediately && currentBlockingCommandCalled == COMMAND_EMPTY)
                {
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

#if defined(DISPATCHER_BUZZER_FEEDBACK_PIN) && defined(USE_TINY_IR_RECEIVER)
                    /*
                     * Do (non blocking) buzzer feedback before command is executed
                     */
                    if(IRMapping[i].Flags & IR_COMMAND_FLAG_BEEP) {
                        tone(DISPATCHER_BUZZER_FEEDBACK_PIN, 2200, 50);
                    }
#endif

                    IRMapping[i].CommandToCall();
#if defined(TRACE)
                    Serial.println(F("End of blocking command"));
#endif
                    currentBlockingCommandCalled = COMMAND_EMPTY;
                }
                else
                {
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
bool IRCommandDispatcher::checkAndRunSuspendedBlockingCommands()
{
    /*
     * Take last rejected command and call associated function
     */
    if (BlockingCommandToRunNext != COMMAND_EMPTY)
    {

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
bool IRCommandDispatcher::delayAndCheckForStop(uint16_t aDelayMillis)
{
    uint32_t tStartMillis = millis();
    do
    {
        if (requestToStopReceived)
        {
            return true;
        }
    }
    while (millis() - tStartMillis < aDelayMillis);
    return false;
}

void IRCommandDispatcher::printIRCommandString(Print *aSerial)
{
    for (uint_fast8_t i = 0; i < sizeof(IRMapping) / sizeof(struct IRToCommandMappingStruct); ++i)
    {
        if (IRReceivedData.command == IRMapping[i].IRCode)
        {
            aSerial->println(reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString));
            return;
        }
    }
    aSerial->println(F("unknown"));
}

void IRCommandDispatcher::setRequestToStopReceived(bool aRequestToStopReceived)
{
    requestToStopReceived = aRequestToStopReceived;
}

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#if defined(LOCAL_INFO)
#undef LOCAL_INFO
#endif
#endif // _IR_COMMAND_DISPATCHER_HPP
