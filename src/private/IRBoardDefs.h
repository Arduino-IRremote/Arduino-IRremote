/**
 * @file IRremoteBoardDefs.h
 *
 * @brief All board specific information should be contained in this file.
 * All timer related definitions are in irTimer.cpp.h.
 * All feedback LED related declarations are in IRFeedbackLEDDefs.h.
 */

#ifndef IRremoteBoardDefs_h
#define IRremoteBoardDefs_h

/**
 * Check for CPU frequency macro and "copy" it to SYSCLOCK
 */
#if !defined(SYSCLOCK) // allow for processor specific code to define SYSCLOCK
#  ifndef F_CPU
#error SYSCLOCK or F_CPU cannot be determined. Define it for your board in IRremoteBoardDefs.h.
#  endif // ! F_CPU
/**
 * Clock frequency to be used for timing.
 */
#define SYSCLOCK F_CPU // main Arduino clock
#endif // ! SYSCLOCK

/*
 * digitalWrite() is supposed to be slow. If this is an issue, define faster,
 * board-dependent versions of these macros SENDPIN_ON(pin) and SENDPIN_OFF(pin).
 * Currently not implemented for any board.
 */
#ifndef SENDPIN_ON
/** Default macro to turn on the pin given as argument. */
#define SENDPIN_ON(pin)  digitalWrite(pin, HIGH)
#endif

#ifndef SENDPIN_OFF
/** Default macro to turn off the pin given as argument. */
#define SENDPIN_OFF(pin) digitalWrite(pin, LOW)
#endif/*
 * digitalWrite() is supposed to be slow. If this is an issue, define faster,
 * board-dependent versions of these macros SENDPIN_ON(pin) and SENDPIN_OFF(pin).
 * Currently not implemented for any board.
 */
#ifndef SENDPIN_ON
/** Default macro to turn on the pin given as argument. */
#define SENDPIN_ON(pin)  digitalWrite(pin, HIGH)
#endif

#ifndef SENDPIN_OFF
/** Default macro to turn off the pin given as argument. */
#define SENDPIN_OFF(pin) digitalWrite(pin, LOW)
#endif

/**
 * Duty cycle in percent for sent signals.
 */
#if ! defined(IR_SEND_DUTY_CYCLE)
#define IR_SEND_DUTY_CYCLE 30 // 30 saves power and is compatible to the old existing code
#endif

//------------------------------------------------------------------------------
// microseconds per clock interrupt tick
#if ! defined(MICROS_PER_TICK)
#define MICROS_PER_TICK    50
#endif

/***************************************
 * Settings and plausi checks for different boards
 ***************************************/

// Teensy 3.0 / Teensy 3.1
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#  if F_BUS < 8000000
#error IRremote requires at least 8 MHz on Teensy 3.x
#  endif

#elif defined(__STM32F1__) || defined(ARDUINO_ARCH_STM32F1) // Recommended original Arduino_STM32 by Roger Clark.
#  if ! defined(strncpy_P)
// this define is not included in the pgmspace.h file :-(
#define strncpy_P(dest, src, size) strncpy((dest), (src), (size))
#  endif

#elif defined(PARTICLE)
  #define SYSCLOCK 16000000
#endif

#endif // ! IRremoteBoardDefs_h

#pragma once

