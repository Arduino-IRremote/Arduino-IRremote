/*
 * DemoIRCommandMapping.h
 *
 * Contains IR remote button codes, strings, and the mapping of codes to functions to call by the dispatcher
 *
 *  Copyright (C) 2019-2026  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  IRMP is free software: you can redistribute it and/or modify
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

#ifndef _IR_COMMAND_MAPPING_H
#define _IR_COMMAND_MAPPING_H

#include <Arduino.h>

#include "IRCommandDispatcher.h" // IRToCommandMappingStruct, IR_COMMAND_FLAG_BLOCKING etc. are defined here

/*
 * !!! Choose your remote !!!
 */
//#define USE_KEYES_REMOTE_CLONE With number pad and direction control swapped, will be taken as default
//#define USE_KEYES_REMOTE
#if !defined(USE_KEYES_REMOTE) && !defined(USE_KEYES_REMOTE_CLONE)
#define USE_KEYES_REMOTE_CLONE // the one you can buy at Aliexpress
#endif

#if (defined(USE_KEYES_REMOTE) && defined(USE_KEYES_REMOTE_CLONE))
#error "Please choose only one remote for compile"
#endif

#if defined(USE_KEYES_REMOTE_CLONE)
#define IR_REMOTE_NAME "KEYES_CLONE"
// Codes for the KEYES CLONE remote control with 17 keys with number pad above direction control
#define IR_ADDRESS 0x00

#define IR_UP    0x18
#define IR_DOWN  0x52
#define IR_RIGHT 0x5A
#define IR_LEFT  0x08
#define IR_OK    0x1C

#define IR_1    0x45
#define IR_2    0x46
#define IR_3    0x47
#define IR_4    0x44
#define IR_5    0x40
#define IR_6    0x43
#define IR_7    0x07
#define IR_8    0x15
#define IR_9    0x09
#define IR_0    0x19

#define IR_STAR 0x16
#define IR_HASH 0x0D
/*
 * SECOND:
 * IR button to command mapping for better reading. IR buttons should only referenced here.
 */
#define COMMAND_ON              IR_UP
#define COMMAND_OFF             IR_DOWN
#define COMMAND_INCREASE_BLINK  IR_RIGHT
#define COMMAND_DECREASE_BLINK  IR_LEFT

#define COMMAND_START           IR_OK
#define COMMAND_STOP            IR_HASH
#define COMMAND_RESET           IR_STAR
#define COMMAND_BLINK           IR_0
#define COMMAND_TONE1           IR_1

#define COMMAND_TONE2           IR_2
#define COMMAND_TONE3           IR_3
//#define            IR_4
//#define            IR_5
//#define            IR_6
//#define            IR_7
//#define            IR_8
//#define            IR_9

#endif

#if defined(USE_KEYES_REMOTE)
#define IR_REMOTE_NAME "KEYES"
/*
 * FIRST:
 * IR code to button mapping for better reading. IR codes should only referenced here.
 */
// Codes for the KEYES remote control with 17 keys and direction control above number pad
#define IR_ADDRESS 0x00

#define IR_UP    0x46
#define IR_DOWN  0x15
#define IR_RIGHT 0x43
#define IR_LEFT  0x44
#define IR_OK    0x40

#define IR_1    0x16
#define IR_2    0x19
#define IR_3    0x0D
#define IR_4    0x0C
#define IR_5    0x18
#define IR_6    0x5E
#define IR_7    0x08
#define IR_8    0x1C
#define IR_9    0x5A
#define IR_0    0x52

#define IR_STAR 0x42
#define IR_HASH 0x4A

/*
 * SECOND:
 * IR button to command mapping for better reading. IR buttons should only referenced here.
 */
#define COMMAND_ON              IR_UP
#define COMMAND_OFF             IR_DOWN
#define COMMAND_INCREASE_BLINK  IR_RIGHT
#define COMMAND_DECREASE_BLINK  IR_LEFT

#define COMMAND_RESET           IR_OK
#define COMMAND_STOP            IR_HASH
#define COMMAND_STOP            IR_STAR
#define COMMAND_BLINK           IR_0
#define COMMAND_TONE2           IR_1

#define COMMAND_TONE1           IR_2
#define COMMAND_TONE2           IR_3
#define COMMAND_TONE2           IR_4
#define COMMAND_TONE2           IR_5
#define COMMAND_TONE2           IR_6
#define COMMAND_TONE2           IR_7
#define COMMAND_TONE2           IR_8
#define COMMAND_TONE2           IR_9
#endif

/*
 * THIRD:
 * Main mapping of commands to C functions
 */

// Strings of commands for Serial output
static const char LEDon[] PROGMEM ="LED on";
static const char LEDoff[] PROGMEM ="LED off";

static const char blink20times[] PROGMEM ="blink 20 times";
static const char blinkStart[] PROGMEM ="blink start";

static const char increaseBlink[] PROGMEM ="increase blink frequency";
static const char decreaseBlink[] PROGMEM ="decrease blink frequency";

static const char tone2200[] PROGMEM ="tone 2200";
static const char tone1800[] PROGMEM ="tone 1800";
static const char printMenu[] PROGMEM ="printMenu";

static const char reset[] PROGMEM ="reset";
static const char stop[] PROGMEM ="stop";

/*
 * Main mapping array of commands to C functions and command strings
 * The macro COMMAND_STRING() removes the strings from memory, if USE_DISPATCHER_COMMAND_STRINGS is not enabled
 */
const struct IRToCommandMappingStruct IRMapping[] = { /**/
{ COMMAND_BLINK, IR_COMMAND_FLAG_BLOCKING | IR_COMMAND_FLAG_BEEP, &doLedBlink20times, COMMAND_STRING(blink20times) }, /**/
{ COMMAND_STOP, IR_COMMAND_FLAG_BLOCKING, &doStop, COMMAND_STRING(stop) }, /* */

/*
 * Short commands that can always be executed, but must be able to terminate other blocking commands (only doLedBlink20times() in this example)
 */
{ COMMAND_START, IR_COMMAND_FLAG_BLOCKING, &doLedBlinkStart, COMMAND_STRING(blinkStart) }, /**/
{ COMMAND_ON, IR_COMMAND_FLAG_BLOCKING, &doLedOn, COMMAND_STRING(LEDon) }, /**/
{ COMMAND_OFF, IR_COMMAND_FLAG_BLOCKING, &doLedOff, COMMAND_STRING(LEDoff) }, /**/

/*
 * Short commands that can always be executed
 */
{ COMMAND_TONE1, IR_COMMAND_FLAG_NON_BLOCKING, &doTone1800, COMMAND_STRING(tone1800) }, /* Lasts 200 ms and blocks receiving of repeats. tone() requires interrupts enabled */
{ COMMAND_TONE3, IR_COMMAND_FLAG_NON_BLOCKING, &doPrintMenu, COMMAND_STRING(printMenu) }, /**/
{ COMMAND_RESET, IR_COMMAND_FLAG_NON_BLOCKING, &doResetBlinkFrequency, COMMAND_STRING(reset) },

/*
 * Repeatable short commands
 */
{ COMMAND_TONE2, IR_COMMAND_FLAG_REPEATABLE_NON_BLOCKING, &doTone2200, COMMAND_STRING(tone2200) }, /* Lasts 50 ms and allows receiving of repeats */
{ COMMAND_INCREASE_BLINK, IR_COMMAND_FLAG_REPEATABLE_NON_BLOCKING, &doIncreaseBlinkFrequency, COMMAND_STRING(increaseBlink) }, /**/
{ COMMAND_DECREASE_BLINK, IR_COMMAND_FLAG_REPEATABLE_NON_BLOCKING, &doDecreaseBlinkFrequency, COMMAND_STRING(decreaseBlink) } };

#endif // _IR_COMMAND_MAPPING_H
