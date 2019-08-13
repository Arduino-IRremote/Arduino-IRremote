//******************************************************************************
// IRremote
// Version 2.0.1 June, 2015
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
//
// Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// Whynter A/C ARC-110WD added by Francesco Meschia
//******************************************************************************

#ifndef IRremoteint_h
#define IRremoteint_h

//------------------------------------------------------------------------------
// Include the right Arduino header
//
#if defined(ARDUINO) && (ARDUINO >= 100)
#	include <Arduino.h>
// define which timer to use
//
// Uncomment the timer you wish to use on your board.  If you
// are using another library which uses timer2, you have options
// to switch IRremote to use a different timer.

// Arduino Mega
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
 //#define IR_USE_TIMER1   // tx = pin 11
 #define IR_USE_TIMER2     // tx = pin 9
 //#define IR_USE_TIMER3   // tx = pin 5
 //#define IR_USE_TIMER4   // tx = pin 6
 //#define IR_USE_TIMER5   // tx = pin 46

// Teensy 1.0
#elif defined(__AVR_AT90USB162__)
 #define IR_USE_TIMER1     // tx = pin 17

// Teensy 2.0
#elif defined(__AVR_ATmega32U4__)
 //#define IR_USE_TIMER1   // tx = pin 14
 //#define IR_USE_TIMER3   // tx = pin 9
 #define IR_USE_TIMER4_HS  // tx = pin 10

// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
 //#define IR_USE_TIMER1   // tx = pin 25
 #define IR_USE_TIMER2     // tx = pin 1
 //#define IR_USE_TIMER3   // tx = pin 16

// Sanguino
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
 //#define IR_USE_TIMER1   // tx = pin 13
 #define IR_USE_TIMER2     // tx = pin 14

// Atmega8
#elif defined(__AVR_ATmega8P__) || defined(__AVR_ATmega8__)
 #define IR_USE_TIMER1   // tx = pin 9

// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, etc
#else
 #define IR_USE_TIMER1   // tx = pin 9   uncomment out this line
 //#define IR_USE_TIMER2     // tx = pin 3   comment out this line
#endif

EXTERN  volatile irparams_t  irparams;

//------------------------------------------------------------------------------
// Defines for setting and clearing register bits
//
#ifndef cbi
#	define cbi(sfr, bit)  (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#	define sbi(sfr, bit)  (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//------------------------------------------------------------------------------
// Pulse parms are ((X*50)-100) for the Mark and ((X*50)+100) for the Space.
// First MARK is the one after the long gap
// Pulse parameters in uSec
//

// Due to sensor lag, when received, Marks  tend to be 100us too long and
//                                   Spaces tend to be 100us too short
#define MARK_EXCESS    100

// Upper and Lower percentage tolerances in measurements
#define TOLERANCE       25
#define LTOL            (1.0 - (TOLERANCE/100.))
#define UTOL            (1.0 + (TOLERANCE/100.))

// Minimum gap between IR transmissions
#define _GAP            5000
#define GAP_TICKS       (_GAP/USECPERTICK)

#define TICKS_LOW(us)   ((int)(((us)*LTOL/USECPERTICK)))
#define TICKS_HIGH(us)  ((int)(((us)*UTOL/USECPERTICK + 1)))

//------------------------------------------------------------------------------
// IR detector output is active low
//
#define MARK   0
#define SPACE  1

// All board specific stuff has been moved to its own file, included here.
#include "boarddefs.h"

#endif
