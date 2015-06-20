//******************************************************************************
// IRremote
// Version 0.11 August, 2009
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
//
// Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
// Modified  by Mitra Ardron <mitra@mitra.biz>
// Added Sanyo and Mitsubishi controllers
// Modified Sony to spot the repeat codes that some Sony's send
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// LG added by Darryl Smith (based on the JVC protocol)
// Whynter A/C ARC-110WD added by Francesco Meschia
//******************************************************************************

// Defining IR_GLOBAL here allows us to declare the instantiation of global variables
#define IR_GLOBAL
#	include "IRremote.h"
#	include "IRremoteInt.h"
#undef IR_GLOBAL

//------------------------------------------------------------------------------
// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG to 1 in IRremoteInt.h
// (Normally macros are used for efficiency)
//  ...but every time i reduce these functions down to macros, the decoders stop working!!??

//+=============================================================================
int  MATCH (int measured,  int desired)
{
  DBG_PRINT("Testing: ");
  DBG_PRINT(TICKS_LOW(desired), DEC);
  DBG_PRINT(" <= ");
  DBG_PRINT(measured, DEC);
  DBG_PRINT(" <= ");
  DBG_PRINTLN(TICKS_HIGH(desired), DEC);

  return ((measured >= TICKS_LOW(desired)) && (measured <= TICKS_HIGH(desired)));
}

//+=============================================================================
int  MATCH_MARK (int measured_ticks,  int desired_us)
{
  DBG_PRINT("Testing mark ");
  DBG_PRINT(measured_ticks * USECPERTICK, DEC);
  DBG_PRINT(" vs ");
  DBG_PRINT(desired_us, DEC);
  DBG_PRINT(": ");
  DBG_PRINT(TICKS_LOW(desired_us + MARK_EXCESS), DEC);
  DBG_PRINT(" <= ");
  DBG_PRINT(measured_ticks, DEC);
  DBG_PRINT(" <= ");
  DBG_PRINTLN(TICKS_HIGH(desired_us + MARK_EXCESS), DEC);

  return ((measured_ticks >= TICKS_LOW (desired_us + MARK_EXCESS))
       && (measured_ticks <= TICKS_HIGH(desired_us + MARK_EXCESS)));
}

//+=============================================================================
int  MATCH_SPACE (int measured_ticks,  int desired_us)
{
  DBG_PRINT("Testing space ");
  DBG_PRINT(measured_ticks * USECPERTICK, DEC);
  DBG_PRINT(" vs ");
  DBG_PRINT(desired_us, DEC);
  DBG_PRINT(": ");
  DBG_PRINT(TICKS_LOW(desired_us - MARK_EXCESS), DEC);
  DBG_PRINT(" <= ");
  DBG_PRINT(measured_ticks, DEC);
  DBG_PRINT(" <= ");
  DBG_PRINTLN(TICKS_HIGH(desired_us - MARK_EXCESS), DEC);

  return ((measured_ticks >= TICKS_LOW (desired_us - MARK_EXCESS))
       && (measured_ticks <= TICKS_HIGH(desired_us - MARK_EXCESS)));
}
