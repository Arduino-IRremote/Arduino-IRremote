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
 * DISPATCHER_IR_COMMAND_HAS_MORE_THAN_8_BIT
 */

#ifndef _IR_COMMAND_DISPATCHER_HPP
#define _IR_COMMAND_DISPATCHER_HPP

#include <Arduino.h>

#include "IRCommandDispatcher.h"

//#define NO_LED_FEEDBACK_CODE   // Activate this if you want to suppress LED feedback or if you do not have a LED. This saves 14 bytes code and 2 clock cycles per interrupt.

//#define USE_TINY_IR_RECEIVER // Recommended and default, but only for NEC protocol!!! If disabled and IRMP_INPUT_PIN is defined, the IRMP library is used for decoding
//#define USE_IRREMOTE_LIBRARY // The IRremote library is used for decoding
//#define USE_IRMP_LIBRARY     // The IRMP library is used for decoding
#if !defined(USE_TINY_IR_RECEIVER) && !defined(USE_IRREMOTE_LIBRARY) && !defined(USE_IRMP_LIBRARY)
#define USE_TINY_IR_RECEIVER // Set TiniIR as default library
#endif

IRCommandDispatcher IRDispatcher;

#if defined(USE_TINY_IR_RECEIVER)
/******************************
 * Code for the TinyIR library
 ******************************/
#if defined(USE_ONKYO_PROTOCOL) && ! defined(DISPATCHER_IR_COMMAND_HAS_MORE_THAN_8_BIT)
#warning ONKYO protocol has 16 bit commands so activating of DISPATCHER_IR_COMMAND_HAS_MORE_THAN_8_BIT is recommended
#endif
#define USE_CALLBACK_FOR_TINY_RECEIVER  // Call the function "handleReceivedTinyIRData()" below each time a frame or repeat is received.
#include "TinyIRReceiver.hpp" // included in "IRremote" and "IRMP" library

// This block must be located after the includes of other *.hpp files
//#define LOCAL_INFO  // This enables info output only for this file
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

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
    // if available, compare address. TinyIRReceiverData.Address saves 6 bytes
    if (TinyIRReceiverData.Address != IR_ADDRESS) { // IR_ADDRESS is defined in *IRCommandMapping.h
        INFO_PRINT(F("Wrong address. Expected 0x"));
        INFO_PRINTLN(IR_ADDRESS, HEX);
    } else
#  endif
    {
        IRDispatcher.IRReceivedData.isAvailable = true;
        // check if dispatcher enabled
        if (!IRDispatcher.doNotUseDispatcher) {
            /*
             * Only short (non blocking) commands are executed directly in ISR (Interrupt Service Routine) context,
             * others are stored for main loop which calls checkAndRunSuspendedBlockingCommands()
             */
            IRDispatcher.checkAndCallCommand(false);
        }
    }
}

#elif defined(USE_IRREMOTE_LIBRARY)
/*********************************
 * Code for the IRremote library
 *********************************/
#define DECODE_NEC          // Includes Apple and Onkyo
#include "IRremote.hpp"

// This block must be located after the includes of other *.hpp files
//#define LOCAL_INFO  // This enables info output only for this file
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

void ReceiveCompleteCallbackHandler();

void IRCommandDispatcher::init() {
    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    /*
     * Tell the ISR to call this function, when a complete frame has been received
     */
    IrReceiver.registerReceiveCompleteCallback(ReceiveCompleteCallbackHandler);
}

/*
 * Callback function
 * Here we know, that data is available.
 * This function is executed in an ISR (Interrupt Service Routine) context.
 * This means that interrupts are blocked here, so delay(), millis() and Serial prints of data longer than the print buffer size etc. will block forever.
 * This is because they require their internal interrupt routines to run in order to return.
 * Therefore it is best to make this callback function short and fast!
 * A dirty hack is to enable interrupts again by calling sei() (enable interrupt again), but you should know what you are doing,
 */
#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
# endif
void ReceiveCompleteCallbackHandler() {
    /*
     * Fill IrReceiver.decodedIRData
     */
    IrReceiver.decode();

    IRDispatcher.IRReceivedData.address = IrReceiver.decodedIRData.address;
    IRDispatcher.IRReceivedData.command = IrReceiver.decodedIRData.command;
    IRDispatcher.IRReceivedData.isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
    IRDispatcher.IRReceivedData.MillisOfLastCode = millis();

    // Interrupts are already enabled for SUPPORT_MULTIPLE_RECEIVER_INSTANCES
#if !defined(SUPPORT_MULTIPLE_RECEIVER_INSTANCES)  && !defined(ARDUINO_ARCH_MBED) && !defined(ESP32) // no Serial etc. possible in callback for RTOS based cores like ESP, even when interrupts are enabled
    interrupts(); // To enable tone(), delay() etc. for commands. Be careful with non-blocking and repeatable commands which lasts longer than the IR repeat duration.
#  endif

#  if defined(LOCAL_INFO)
    IrReceiver.printIRResultMinimal(&Serial);
    Serial.println();
#  endif
    /*
     * Enable receiving of the next frame.
     */
    IrReceiver.resume();

#  if defined(IR_ADDRESS)
    // if available, compare address
    if (IRDispatcher.IRReceivedData.address != IR_ADDRESS) { // IR_ADDRESS is defined in *IRCommandMapping.h
        INFO_PRINT(F("Wrong address. Expected 0x"));
        INFO_PRINTLN(IR_ADDRESS, HEX);
    } else
#  endif
    {
        IRDispatcher.IRReceivedData.isAvailable = true;
        // check if dispatcher enabled
        if (!IRDispatcher.doNotUseDispatcher) {
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
#include "LocalDebugLevelStart.h"

IRMP_DATA irmp_data;

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
    irmp_get_data(&irmp_data);

    IRDispatcher.IRReceivedData.address = irmp_data.address;
    IRDispatcher.IRReceivedData.command = irmp_data.command;
    IRDispatcher.IRReceivedData.isRepeat = irmp_data.flags & IRMP_FLAG_REPETITION;
    IRDispatcher.IRReceivedData.MillisOfLastCode = millis();

#if !defined(ARDUINO_ARCH_MBED) && !defined(ESP32) // no Serial etc. possible in callback for RTOS based cores like ESP, even when interrupts are enabled
    interrupts(); // To enable tone(), delay() etc. for commands. Be careful with non-blocking and repeatable commands which lasts longer than the IR repeat duration.
#  endif

#  if defined(LOCAL_INFO)
    irmp_result_print(&Serial, &irmp_data);
#  endif

#  if defined(IR_ADDRESS)
    // if available, compare address
    if (IRDispatcher.IRReceivedData.address != IR_ADDRESS) {
        INFO_PRINT(F("Wrong address. Expected 0x"));
        INFO_PRINTLN(IR_ADDRESS, HEX);
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
void IRCommandDispatcher::printIRInfo(Print *aSerial) {
    aSerial->println();
    // For available IR commands see IRCommandMapping.h https://github.com/ArminJo/PWMMotorControl/blob/master/examples/SmartCarFollower/IRCommandMapping.h
#if defined(USE_IRMP_LIBRARY)
#  if defined(IR_REMOTE_NAME)
    aSerial->println(F("Listening to IR remote of type " STR(IR_REMOTE_NAME) " at pin " STR(IRMP_INPUT_PIN)));
#  else
    aSerial->println(F("Listening to IR remote at pin " STR(IRMP_INPUT_PIN)));
#  endif
    aSerial->print(F("Accepted protocols are: "));
    irmp_print_active_protocols(&Serial);
    aSerial->println();
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
#  if defined(__AVR__)
#    if defined(USE_DISPATCHER_COMMAND_STRINGS)
            const __FlashStringHelper *tCommandName = reinterpret_cast<const __FlashStringHelper*>(IRMapping[i].CommandString);
#    else
            char tCommandName[7];
            snprintf_P(tCommandName, sizeof(tCommandName), PSTR("0x%x"), IRMapping[i].IRCode);
#    endif
#  else
#    if defined(USE_DISPATCHER_COMMAND_STRINGS)
            const char *tCommandName = IRMapping[i].CommandString;
#    else
            char tCommandName[7];
            snprintf(tCommandName, sizeof(tCommandName), "0x%x", IRMapping[i].IRCode);
#    endif
#  endif
#endif
            /*
             * Check for repeat and if repeat is allowed for the current command
             */
            if (IRReceivedData.isRepeat && !(IRMapping[i].Flags & IR_COMMAND_FLAG_REPEATABLE)) {

                DEBUG_PRINT(F("Repeats of command \""));
                DEBUG_PRINT(tCommandName);
                DEBUG_PRINTLN("\" not accepted");

                return;
            }

            /*
             * Do not accept recursive call of the same command
             */
            if (currentBlockingCommandCalled == IRReceivedData.command) {

                DEBUG_PRINT(F("Recursive command \""));
                DEBUG_PRINT(tCommandName);
                DEBUG_PRINTLN("\" not accepted");

                return;
            }

            /*
             * Execute commands
             */
            bool tIsNonBlockingCommand = (IRMapping[i].Flags & IR_COMMAND_FLAG_NON_BLOCKING);
            if (tIsNonBlockingCommand) {
                // short command here, just call
                INFO_PRINT(F("Run non blocking command: "));
                INFO_PRINTLN(tCommandName);
#if defined(DISPATCHER_BUZZER_FEEDBACK_PIN) && defined(USE_TINY_IR_RECEIVER)
                    /*
                     * Do (non blocking) buzzer feedback before command is executed
                     */
                    if(IRMapping[i].Flags & IR_COMMAND_FLAG_BEEP) {
                        tone(DISPATCHER_BUZZER_FEEDBACK_PIN, 2200, 50);
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
                    INFO_PRINT(F("Run blocking command: "));
                    INFO_PRINTLN(tCommandName);

#if defined(DISPATCHER_BUZZER_FEEDBACK_PIN) && defined(USE_TINY_IR_RECEIVER)
                    /*
                     * Do (non blocking) buzzer feedback before command is executed
                     */
                    if(IRMapping[i].Flags & IR_COMMAND_FLAG_BEEP) {
                        tone(DISPATCHER_BUZZER_FEEDBACK_PIN, 2200, 50);
                    }
#endif

                    IRMapping[i].CommandToCall();
                    TRACE_PRINTLN(F("End of blocking command"));

                    currentBlockingCommandCalled = COMMAND_EMPTY;
                } else {
                    /*
                     * Called by ISR or another command still running.
                     * Do not run command directly, but set request to stop to true and store command
                     * for main loop to execute by checkAndRunSuspendedBlockingCommands()
                     */
                    BlockingCommandToRunNext = IRReceivedData.command;
                    requestToStopReceived = true; // to stop running command
                    INFO_PRINT(F("Requested stop and stored blocking command "));
                    INFO_PRINT(tCommandName);
                    INFO_PRINTLN(F(" as next command to run."));
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

        INFO_PRINT(F("Run stored command=0x"));
        INFO_PRINTLN(BlockingCommandToRunNext, HEX);

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
void IRCommandDispatcher::setNextBlockingCommand(IRCommandType aBlockingCommandToRunNext) {
    INFO_PRINT(F("Set next command to run to 0x"));
    INFO_PRINTLN(aBlockingCommandToRunNext, HEX);
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

void IRCommandDispatcher::printIRCommandString(Print *aSerial, uint_fast8_t aMappingArrayIndex) {
#if defined(__AVR__)
#  if defined(USE_DISPATCHER_COMMAND_STRINGS)
    aSerial->println(reinterpret_cast<const __FlashStringHelper*>(IRMapping[aMappingArrayIndex].CommandString));
#  else
    aSerial->print(F("0x"));
    aSerial->println(IRMapping[aMappingArrayIndex].IRCode, HEX);
#  endif
#else
#  if defined(USE_DISPATCHER_COMMAND_STRINGS)
    aSerial->println(IRMapping[aMappingArrayIndex].CommandString);
#  else
    aSerial->print("0x");
    aSerial->println(IRMapping[aMappingArrayIndex].IRCode, HEX);
#  endif
#endif
}

void IRCommandDispatcher::printIRCommandString(Print *aSerial) {
    for (uint_fast8_t i = 0; i < sizeof(IRMapping) / sizeof(struct IRToCommandMappingStruct); ++i) {
        if (IRReceivedData.command == IRMapping[i].IRCode) {
            printIRCommandString(aSerial, i);
            return;
        }
    }
    aSerial->println(F("unknown"));
}

void IRCommandDispatcher::setRequestToStopReceived(bool aRequestToStopReceived) {
    requestToStopReceived = aRequestToStopReceived;
}

#include "LocalDebugLevelEnd.h"

#endif // _IR_COMMAND_DISPATCHER_HPP
