/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#ifndef IRremoteint_h
#define IRremoteint_h

#include <WProgram.h>

#define RAWBUF 76 // Length of raw duration buffer

#define CLKFUDGE 5      // fudge factor for clock interrupt overhead
#define CLK 256      // max value for clock (timer 2)
#define PRESCALE 8      // timer2 clock prescale
#define SYSCLOCK 16000000  // main Arduino clock
#define CLKSPERUSEC (SYSCLOCK/PRESCALE/1000000)   // timer clocks per microsecond

#define ERR 0
#define DECODED 1

// clock timer reset value
#define INIT_TIMER_COUNT2 (CLK - USECPERTICK*CLKSPERUSEC + CLKFUDGE)
#define RESET_TIMER2 TCNT2 = INIT_TIMER_COUNT2

// pulse parameters in usec
#define NEC_HDR_MARK	9000
#define NEC_BIT_MARK	560
#define NEC_RPT_SPACE	2250

#define TOLERANCE 25  // percent tolerance in measurements
#define LTOL (100 - TOLERANCE) 
#define UTOL (100 + TOLERANCE) 

#define _GAP 5000 // Minimum map between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

#define TICKS_LOW(us) (((us)*(long)LTOL/USECPERTICK/100))
#define TICKS_HIGH(us) ((us)*(long)UTOL/USECPERTICK/100 + 1)

#ifndef DEBUG
#define MATCH(measured_ticks, desired_us) ((measured_ticks) >= TICKS_LOW(desired_us) && (measured_ticks) <= TICKS_HIGH(desired_us))
#define MATCH_MARK(measured_ticks, desired_us) MATCH(measured_ticks, (desired_us) + MARK_EXCESS)
#define MATCH_SPACE(measured_ticks, desired_us) MATCH((measured_ticks), (desired_us) - MARK_EXCESS)
// Debugging versions are in IRremoteRecv.cpp
#endif

// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5

// information for the interrupt handler
typedef struct irparams {
  uint8_t recvpin;           // pin for IR data from detector
  uint8_t rcvstate;          // state machine
  unsigned int timer;        // state timer, counts 50uS ticks.
  unsigned int rawbuf[RAWBUF]; // raw data
  uint8_t rawlen;            // counter of entries in rawbuf
  volatile struct irparams *next;   // Link together multiple inputs
} 
irparams_t;

// Defined in IRremote.cpp
extern volatile irparams_t *irparamsList;

// IR detector output is active low
#define MARK  0
#define SPACE 1

#define TOPBIT 0x8000000000000000ull

#define NEC_BITS 32
#define SONY_BITS 12
#define MIN_RC5_SAMPLES 11
#define MIN_RC6_SAMPLES 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Define the IRremoteHw.cpp "API"

void IRremoteEnablePwm();

void IRremoteDisablePwm();

void IRremoteEnableIRoutput(int khz);

void IRremoteEnableIRinput();

void IRremoteRegisterHandler(void (*newhandle)());

#endif

