/*
 * IRremoteHw - hardware-dependent code
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#include "IRremote.h"
#include "IRremoteInt.h"
// For ISR(...)
#include "avr/interrupt.h"

#define CLKFUDGE 5      // fudge factor for clock interrupt overhead
#define CLK 256      // max value for clock (timer 2)
#define PRESCALE 8      // timer2 clock prescale
#define SYSCLOCK 16000000  // main Arduino clock
#define CLKSPERUSEC (SYSCLOCK/PRESCALE/1000000)   // timer clocks per microsecond
#define INIT_TIMER_COUNT2 (CLK - USECPERTICK*CLKSPERUSEC + CLKFUDGE)

void IRremoteEnablePwm() {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  TCCR2A |= _BV(COM2B1); // Enable pin 3 PWM output
}

/* Leave pin off for time (given in microseconds) */
void IRremoteDisablePwm() {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  TCCR2A &= ~(_BV(COM2B1)); // Disable pin 3 PWM output
}

void IRremoteEnableIRoutput(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (OC2B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.

  
  // Disable the Timer2 Interrupt (which is used for receiving IR)
  TIMSK2 &= ~_BV(TOIE2); //Timer2 Overflow Interrupt
  
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW); // When not sending PWM, we want it low
  
  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);

  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  OCR2A = SYSCLOCK / 2 / khz / 1000;
  OCR2B = OCR2A / 3; // 33% duty cycle
}

// initialization
void IRremoteEnableIRinput() {
  // setup pulse clock timer interrupt
  TCCR2A = 0;  // normal mode

  //Prescale /8 (16M/8 = 0.5 microseconds per tick)
  // Therefore, the timer interval can range from 0.5 to 128 microseconds
  // depending on the reset value (255 to 0)
  cbi(TCCR2B,CS22);
  sbi(TCCR2B,CS21);
  cbi(TCCR2B,CS20);

  //Timer2 Overflow Interrupt Enable
  sbi(TIMSK2,TOIE2);

  TCNT2 = INIT_TIMER_COUNT2; // Reset TIMER2

  sei();  // enable interrupts
}

static void (*handler)() = 0;

void IRremoteRegisterHandler(void (*newhandler)()) {
	handler = newhandler;
}

// Callback to the interrupt handler code.
ISR(TIMER2_OVF_vect)
{
  TCNT2 = INIT_TIMER_COUNT2; // Reset TIMER2

  if (handler != 0) {
	  (*handler)();
  }
}
