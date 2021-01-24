//******************************************************************************
// IRremoteint.h
// Version 2.0.1 June, 2015
// Initially coded 2009 Ken Shirriff http://www.righto.com
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

/*
 * Activate this line if your receiver has an external output driver transistor / "inverted" output
 */
//#define IR_INPUT_IS_ACTIVE_HIGH
//------------------------------------------------------------------------------
// Include the Arduino header
//
#include <Arduino.h>

// All board specific stuff have been moved to its own file, included here.
#include "IRremoteBoardDefs.h"

//------------------------------------------------------------------------------
// Information for the Interrupt Service Routine
//

// ISR State-Machine : Receiver States
#define IR_REC_STATE_IDLE      0
#define IR_REC_STATE_MARK      1
#define IR_REC_STATE_SPACE     2
#define IR_REC_STATE_STOP      3

/**
 * This struct contains the data and control used for static functions and the ISR (interrupt service routine)
 * Only rcvstate needs to be volatile. All the other fields are not written by ISR during decoding in loop.
 */
struct irparams_struct {
    // The fields are ordered to reduce memory over caused by struct-padding
    volatile uint8_t rcvstate;      ///< State Machine state
    uint8_t recvpin;                ///< Pin connected to IR data from detector
    uint8_t blinkpin;               ///< 0 means not valid pin
    bool blinkflag;                 ///< true -> enable blinking of pin on IR processing
#if RAW_BUFFER_LENGTH <= 255        // saves around 75 bytes program space and speeds up ISR
    uint8_t rawlen;                 ///< counter of entries in rawbuf
#else
    unsigned int rawlen;            ///< counter of entries in rawbuf
#endif
    uint16_t timer;                 ///< State timer, counts 50uS ticks. The value is copied into the rawbuf array on every transition.
    uint16_t rawbuf[RAW_BUFFER_LENGTH]; ///< raw data / tick counts per mark/space, first entry is the length of the gap between previous and current command
    uint8_t overflow;               ///< Raw buffer overflow occurred
};

extern struct irparams_struct irparams;

//------------------------------------------------------------------------------
// Defines for setting and clearing register bits
//
#ifndef cbi
#define cbi(sfr, bit)  (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit)  (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//------------------------------------------------------------------------------
// Pulse parms are ((X*50)-100) for the Mark and ((X*50)+100) for the Space.
// First MARK is the one after the long gap
// Pulse parameters in uSec
//

/** Relative tolerance (in percent) for some comparisons on measured data. */
#define TOLERANCE       25

/** Lower tolerance for comparison of measured data */
//#define LTOL            (1.0 - (TOLERANCE/100.))
#define LTOL            (100 - TOLERANCE)
/** Upper tolerance for comparison of measured data */
//#define UTOL            (1.0 + (TOLERANCE/100.))
#define UTOL            (100 + TOLERANCE)

/** Minimum gap between IR transmissions, in microseconds */
#define RECORD_GAP_MICROS   5000 // NEC header space is 4500

/** Minimum gap between IR transmissions, in MICROS_PER_TICK */
#define RECORD_GAP_TICKS    (RECORD_GAP_MICROS / MICROS_PER_TICK)

//#define TICKS_LOW(us)   ((int)(((us)*LTOL/MICROS_PER_TICK)))
//#define TICKS_HIGH(us)  ((int)(((us)*UTOL/MICROS_PER_TICK + 1)))
#if MICROS_PER_TICK == 50 && TOLERANCE == 25           // Defaults
#define TICKS_LOW(us)   ((us)/67 )     // (us) / ((MICROS_PER_TICK:50 / LTOL:75 ) * 100)
#define TICKS_HIGH(us)  ((us)/40 + 1)  // (us) / ((MICROS_PER_TICK:50 / UTOL:125) * 100) + 1
#else
    #define TICKS_LOW(us)   ((uint16_t) ((long) (us) * LTOL / (MICROS_PER_TICK * 100) ))
    #define TICKS_HIGH(us)  ((uint16_t) ((long) (us) * UTOL / (MICROS_PER_TICK * 100) + 1))
#endif

//------------------------------------------------------------------------------
// IR receivers on a board with an external output transistor may have "inverted" output
#ifdef IR_INPUT_IS_ACTIVE_HIGH
// IR detector output is active high
#define MARK   1 ///< Sensor output for a mark ("flash")
#define SPACE  0 ///< Sensor output for a space ("gap")
#else
// IR detector output is active low
#define MARK   0 ///< Sensor output for a mark ("flash")
#define SPACE  1 ///< Sensor output for a space ("gap")
#endif

#endif
