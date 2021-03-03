/**
 * @file IRremoteBoardDefs.h
 *
 * @brief All board specific information should be contained in this file.
 * It defines a number of macros, depending on the board, as determined by
 * pre-proccesor symbols.
 * It was previously contained within IRremoteInt.h.
 */
// IRremote
// Version 2.0.1 June, 2015
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
// Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// Whynter A/C ARC-110WD added by Francesco Meschia
// Sparkfun Pro Micro support by Alastair McCormack
//******************************************************************************
#ifndef IRremoteBoardDefs_h
#define IRremoteBoardDefs_h

/*
 * Define some defaults, portable but possibly slower than necessary,
 * that some boards may like to override
 * (This is to avoid negative logic, ! DONT_... is just awkward.)
 */

/**
 * Defined if the standard enableIRIn function should be used.
 * Undefined for boards supplying their own.
 */
#define USE_DEFAULT_ENABLE_IR_IN

/*
 * digitalWrite() is supposed to be slow. If this is an issue, define faster,
 * board-dependent versions of these macros SENDPIN_ON(pin) and SENDPIN_OFF(pin).
 */
#ifndef SENDPIN_ON
/** Board dependent macro to turn on the pin given as argument. */
#define SENDPIN_ON(pin)  digitalWrite(pin, HIGH)
#endif

#ifndef SENDPIN_OFF
/**
 * Board dependent macro to turn off the pin given as argument.
 */
#define SENDPIN_OFF(pin) digitalWrite(pin, LOW)
#endif

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

//------------------------------------------------------------------------------
// Define which timer to use
//
// Uncomment the timer you wish to use on your board.
// Here you have the option to switch IRremote to use a different timer and send pin.

/*********************
 * ARDUINO Boards
 *********************/
// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, Nano, etc
// ATmega48, ATmega88, ATmega168, ATmega328
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega168__)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER2)
//#define IR_USE_TIMER1   // send pin = pin 9
#define IR_USE_TIMER2     // send pin = pin 3
#  endif

// Arduino Mega
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER2) && !defined(IR_USE_TIMER3) && !defined(IR_USE_TIMER4) && !defined(IR_USE_TIMER5)
//#define IR_USE_TIMER1   // send pin = pin 11
#define IR_USE_TIMER2     // send pin = pin 9
//#define IR_USE_TIMER3   // send pin = pin 5
//#define IR_USE_TIMER4   // send pin = pin 6
//#define IR_USE_TIMER5   // send pin = pin 46
#  endif

// Leonardo
#elif defined(__AVR_ATmega32U4__) && ! defined(TEENSYDUINO) && ! defined(ARDUINO_AVR_PROMICRO)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER3) && !defined(IR_USE_TIMER4_HS)
//#define IR_USE_TIMER1     // send pin = pin 9
#define IR_USE_TIMER3       // send pin = pin 5
//#define IR_USE_TIMER4_HS  // send pin = pin 13
#  endif

// Nano Every, Uno WiFi Rev2
#elif defined(__AVR_ATmega4809__)
#  if !defined(IR_USE_TIMER_4809_1) && !defined(IR_USE_TIMER_4809_2)
#define IR_USE_TIMER_4809_1     //  send pin = pin 24
//#define IR_USE_TIMER_4809_2     // Not yet implemented send pin = pin 21
#  endif

/*****************************
 * Plain AVR CPU's, no boards
 *****************************/
// ATmega8u2, ATmega16U2, ATmega32U2
#elif defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__)  || defined(__AVR_ATmega32U2__)
#  if !defined(IR_USE_TIMER1)
    #define IR_USE_TIMER1     // send pin = pin C6
#endif

// Atmega8
#elif defined(__AVR_ATmega8__)
#  if !defined(IR_USE_TIMER1)
#define IR_USE_TIMER1     // send pin = pin 9
#  endif


// ATtiny84
#elif defined(__AVR_ATtiny84__)
#  if !defined(IR_USE_TIMER1)
#define IR_USE_TIMER1     // send pin = pin 6
#  endif

//ATtiny85
#elif defined(__AVR_ATtiny85__)
#  if !defined(IR_USE_TIMER_TINY0) && !defined(IR_USE_TIMER_TINY1)
// standard Digispark and ATTinyCore settings use timer 0 for millis() and micros()
#define IR_USE_TIMER_TINY1   // send pin = pin 4
//#define IR_USE_TIMER_TINY0   // send pin = pin 1
#  endif

/*********************
 * SPARKFUN Boards
 *********************/
// Sparkfun Pro Micro
#elif defined(ARDUINO_AVR_PROMICRO)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER3) && !defined(IR_USE_TIMER4_HS)
//#define IR_USE_TIMER1     // send pin = pin 9
#define IR_USE_TIMER3       // send pin = pin 5
//#define IR_USE_TIMER4_HS  // send pin = pin 13
#  endif

/*********************
 * TEENSY Boards
 *********************/
// Teensy 1.0
#elif defined(__AVR_AT90USB162__)
#  if !defined(IR_USE_TIMER1)
#define IR_USE_TIMER1     // send pin = pin 17
#  endif

// Teensy 2.0
#elif defined(__AVR_ATmega32U4__) && defined(TEENSYDUINO)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER3) && !defined(IR_USE_TIMER4_HS)
//#define IR_USE_TIMER1     // send pin = pin 14 (Teensy 2.0 - physical pin: B5)
#define IR_USE_TIMER3       // send pin = pin 9  (Teensy 2.0 - physical pin: C6)
//#define IR_USE_TIMER4_HS  // send pin = pin 10 (Teensy 2.0 - physical pin: C7)
#  endif

// Teensy 3.0 / Teensy 3.1
#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#  if !defined(IR_USE_TIMER_CMT)
#define IR_USE_TIMER_CMT    // send pin = pin 5
#  endif

// Teensy-LC
#elif defined(__MKL26Z64__)
#  if !defined(IR_USE_TIMER_TPM1)
#define IR_USE_TIMER_TPM1 // send pin = pin 16
#  endif

// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER2) && !defined(IR_USE_TIMER3)
//#define IR_USE_TIMER1   // send pin = pin 25
#define IR_USE_TIMER2     // send pin = pin 1
//#define IR_USE_TIMER3   // send pin = pin 16
#  endif

/*********************
 * CPU's with MegaCore
 *********************/
// MegaCore - ATmega64, ATmega128
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__)
#  if !defined(IR_USE_TIMER1)
 #define IR_USE_TIMER1     // send pin = pin 13
#  endif

/*********************
 * CPU's with MajorCore
 *********************/
#elif defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER3)
    #define IR_USE_TIMER1     // send pin = pin 13
    //#define IR_USE_TIMER3   // send pin = pin 12 - ATmega162 only
#endif

/*********************
 * CPU's with MightyCore
 *********************/
// MightyCore - ATmega1284
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER2) && !defined(IR_USE_TIMER3)
//#define IR_USE_TIMER1   // send pin = pin 13
#define IR_USE_TIMER2     // send pin = pin 14
//#define IR_USE_TIMER3   // send pin = pin 6
#  endif

// MightyCore - ATmega164, ATmega324, ATmega644
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__)
#  if !defined(IR_USE_TIMER1) && !defined(IR_USE_TIMER2)
//#define IR_USE_TIMER1   // send pin = pin 13
#define IR_USE_TIMER2     // send pin = pin 14
#  endif

// MightyCore - ATmega8535, ATmega16, ATmega32
#elif defined(__AVR_ATmega8535__) || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__)
#  if !defined(IR_USE_TIMER1)
#define IR_USE_TIMER1     // send pin = pin 13
#  endif

/*********************
 * CPU's with TinyCore
 *********************/
#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__) // TinyCore boards
#  if !defined(IR_USE_TIMER_D)
#define IR_USE_TIMER_D     // send pin = pin 13 or 14
#  endif

/*********************
 * OTHER CPU's
 *********************/
#elif defined(ESP32)
#  if !defined(IR_USE_TIMER_ESP32)
#define IR_USE_TIMER_ESP32
#  endif

#elif defined(ARDUINO_ARCH_SAMD)
#define TIMER_PRESCALER_DIV 64

#elif defined(NRF5) // nRF5 BBC MicroBit
// It uses Timer2 so you cannot use the Adafruit_Microbit display driver
// PWM generation by hardware not implemented
#define SEND_PWM_BY_TIMER_NOT_SUPPORTED

// Supply own enbleIRIn
#undef USE_DEFAULT_ENABLE_IR_IN

/*********************
 * STM32 Boards
 *********************/
#elif defined(ARDUINO_ARCH_STM32)

// PWM generation by hardware tbd
#define SEND_PWM_BY_TIMER_NOT_SUPPORTED

// Supply own enbleIRIn
#undef USE_DEFAULT_ENABLE_IR_IN



/*********************
 * Particle Boards
 *********************/
#elif defined(PARTICLE)
  #define IR_USE_TIMER_PARTICLE
  #define SYSCLOCK 16000000

#else
// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, Nano, etc
// ATmega48, ATmega88, ATmega168, ATmega328
#define IR_USE_TIMER1   // send pin = pin 9
#warning Board could not be identified from pre-processor symbols. By Default, TIMER1 has been selected for use with IRremote. Please extend IRremoteBoardDefs.h.
#endif

//------------------------------------------------------------------------------
// Defines for Timer

// We define static board specific functions here, but they are only used in a few files.
#pragma GCC diagnostic ignored "-Wunused-function"
//---------------------------------------------------------
#ifdef DOXYGEN
/**
 * If applicable, pin number for sending IR. Note that in most cases, this is not
 * used and ignored if set. Instead, the sending pin is determined by the timer
 * deployed.
 */
#define IR_SEND_PIN

/**
 * Interrupt service routine. Called as interrupt routine to collect read IR data.
 */
#define  ISR

//---------------------------------------------------------
// Timer2 (8 bits)
//
#elif defined(IR_USE_TIMER2)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM       TCNT2 = 0; (TCCR2A |= _BV(COM2B1))    // Clear OC2B on Compare Match
#define TIMER_DISABLE_SEND_PWM      (TCCR2A &= ~(_BV(COM2B1))) // Normal port operation, OC2B disconnected.
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK2 = _BV(OCIE2B))  // Output Compare Match A Interrupt Enable
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK2 = 0)
#define TIMER_INTR_NAME             TIMER2_COMPB_vect // We use TIMER2_COMPB_vect to be compatible with tone() library

// The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
#pragma GCC diagnostic ignored "-Wunused-function"
/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 */
static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint16_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR2A = _BV(WGM20); // PWM, Phase Correct, Top is OCR2A
    TCCR2B = _BV(WGM22) | _BV(CS20); // CS20 -> no prescaling
    OCR2A = pwmval - 1;
    OCR2B = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT2 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
}

#define TIMER_COUNT_TOP  (SYSCLOCK * MICROS_PER_TICK / 1000000)
/*
 * timerConfigForReceive() is used exclusively by IRrecv::enableIRIn()
 * It generates an interrupt each 50 (MICROS_PER_TICK) us.
 */
static void timerConfigForReceive() {
#  if (TIMER_COUNT_TOP < 256)
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS20);
    OCR2A  = TIMER_COUNT_TOP;
    OCR2B  = TIMER_COUNT_TOP;
    TCNT2  = 0;
#  else
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS21);
    OCR2A = TIMER_COUNT_TOP / 8;
    OCR2B = TIMER_COUNT_TOP / 8;
    TCNT2 = 0;
#  endif
}

//-----------------
#  if defined(CORE_OC2B_PIN)
#define IR_SEND_PIN  CORE_OC2B_PIN  // Teensy

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  9              // Arduino Mega

#  elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__)
#define IR_SEND_PIN  14             // MightyCore, MegaCore

#  else
#define IR_SEND_PIN  3              // Arduino Duemilanove, Diecimila, LilyPad, etc
#  endif // defined(CORE_OC2B_PIN)

//---------------------------------------------------------
// Timer1 (16 bits)
//
#elif defined(IR_USE_TIMER1)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM   TCNT1 = 0; (TCCR1A |= _BV(COM1A1))
#define TIMER_DISABLE_SEND_PWM  (TCCR1A &= ~(_BV(COM1A1)))

//-----------------
#  if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__) \
|| defined(__AVR_ATmega8535__) || defined(__AVR_ATmega16__) \
|| defined(__AVR_ATmega32__) || defined(__AVR_ATmega64__) \
|| defined(__AVR_ATmega128__) || defined(__AVR_ATmega162__)
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK |= _BV(OCIE1A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK &= ~_BV(OCIE1A))
#  else
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK1 = _BV(OCIE1A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK1 = 0)
#  endif

//-----------------
#if defined(TIMER1_COMPA_vect)
#define TIMER_INTR_NAME       TIMER1_COMPA_vect
#elif defined(TIM1_COMPA_vect)
#define TIMER_INTR_NAME       TIM1_COMPA_vect
#endif

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR1A = _BV(WGM11); // PWM, Phase Correct, Top is ICR1
    TCCR1B = _BV(WGM13) | _BV(CS10); // CS10 -> no prescaling
    ICR1 = pwmval - 1;
    OCR1A = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT1 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
}

static void timerConfigForReceive() {
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10);
    OCR1A = SYSCLOCK * MICROS_PER_TICK / 1000000;
    TCNT1 = 0;
}

//-----------------
#  if defined(CORE_OC1A_PIN)
#define IR_SEND_PIN  CORE_OC1A_PIN  // Teensy

#  elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  11             // Arduino Mega

#  elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__) || defined(__AVR_ATmega32__) \
|| defined(__AVR_ATmega16__) || defined(__AVR_ATmega8535__) \
|| defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) \
|| defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) \
|| defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__)
#define IR_SEND_PIN  13             // MightyCore, MegaCore, MajorCore

#  elif defined(__AVR_ATtiny84__)
# define IR_SEND_PIN  6

#  else
#define IR_SEND_PIN  9              // Arduino Duemilanove, Diecimila, LilyPad, Sparkfun Pro Micro, Leonardo etc.
#  endif // defined(CORE_OC1A_PIN)

//---------------------------------------------------------
// Timer3 (16 bits)
//
#elif defined(IR_USE_TIMER3)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM       TCNT3 = 0; (TCCR3A |= _BV(COM3A1))
#define TIMER_DISABLE_SEND_PWM      (TCCR3A &= ~(_BV(COM3A1)))
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK3 = _BV(OCIE3B))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK3 = 0)
#define TIMER_INTR_NAME             TIMER3_COMPB_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR3A = _BV(WGM31);
    TCCR3B = _BV(WGM33) | _BV(CS30); // PWM, Phase Correct, ICRn as TOP, complete period is double of pwmval
    ICR3 = pwmval - 1;
    OCR3A = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT3 = 0; // required, since we have an 16 bit counter
}

static void timerConfigForReceive() {
    TCCR3A = 0;
    TCCR3B = _BV(WGM32) | _BV(CS30);
    OCR3A = SYSCLOCK * MICROS_PER_TICK / 1000000;
    OCR3B = SYSCLOCK * MICROS_PER_TICK / 1000000;
    TCNT3 = 0;
}

//-----------------
#  if defined(CORE_OC3A_PIN)
#define IR_SEND_PIN  CORE_OC3A_PIN  // Teensy

#  elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) \
|| defined(__AVR_ATmega32U4__) || defined(ARDUINO_AVR_PROMICRO)
#define IR_SEND_PIN  5              // Arduino Mega, Arduino Leonardo, Sparkfun Pro Micro

#  elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
#define IR_SEND_PIN  6              // MightyCore, MegaCore

#  else
#error Please add OC3A pin number here
#  endif

//---------------------------------------------------------
// Timer4 (10 bits, high speed option)
//
#elif defined(IR_USE_TIMER4_HS)

#define TIMER_RESET_INTR_PENDING
#  if defined(ARDUINO_AVR_PROMICRO) // Sparkfun Pro Micro
#define TIMER_ENABLE_SEND_PWM       TCNT4 = 0; (TCCR4A |= _BV(COM4A0))     // Use complimentary O̅C̅4̅A̅ output on pin 5
#define TIMER_DISABLE_SEND_PWM      (TCCR4A &= ~(_BV(COM4A0)))  // (Pro Micro does not map PC7 (32/ICP3/CLK0/OC4A)
                                                            // of ATmega32U4 )
#  else
#define TIMER_ENABLE_SEND_PWM       TCNT4 = 0; (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_SEND_PWM      (TCCR4A &= ~(_BV(COM4A1)))
#  endif
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK4 = 0)
#define TIMER_INTR_NAME             TIMER4_OVF_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = ((SYSCLOCK / 2000) / (aFrequencyKHz)) - 1; // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR4A = (1 << PWM4A);
    TCCR4B = _BV(CS40);
    TCCR4C = 0;
    TCCR4D = (1 << WGM40);
    TCCR4E = 0;
    TC4H = pwmval >> 8;
    OCR4C = pwmval;
    TC4H = (pwmval * IR_SEND_DUTY_CYCLE / 100) >> 8;
    OCR4A = (pwmval * IR_SEND_DUTY_CYCLE / 100) & 255;
    TCNT4 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
}

static void timerConfigForReceive() {
    TCCR4A = 0;
    TCCR4B = _BV(CS40);
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;
    TC4H = (SYSCLOCK * MICROS_PER_TICK / 1000000) >> 8;
    OCR4C = (SYSCLOCK * MICROS_PER_TICK / 1000000) & 255;
    TC4H = 0;
    TCNT4 = 0;
}

//-----------------
#  if defined(CORE_OC4A_PIN)
#define IR_SEND_PIN  CORE_OC4A_PIN  // Teensy
#  elif defined(ARDUINO_AVR_PROMICRO)
#define IR_SEND_PIN  5              // Sparkfun Pro Micro
#  elif defined(__AVR_ATmega32U4__)
#define IR_SEND_PIN  13             // Leonardo
#  else
#error Please add OC4A pin number here
#  endif

//---------------------------------------------------------
// Timer4 (16 bits)
//
#elif defined(IR_USE_TIMER4)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM       TCNT4 = 0; (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_SEND_PWM      (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK4 = _BV(OCIE4A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK4 = 0)
#define TIMER_INTR_NAME             TIMER4_COMPA_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR4A = _BV(WGM41);
    TCCR4B = _BV(WGM43) | _BV(CS40);
    ICR4 = pwmval - 1;
    OCR4A = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT4 = 0; // required, since we have an 16 bit counter
}

static void timerConfigForReceive() {
    TCCR4A = 0;
    TCCR4B = _BV(WGM42) | _BV(CS40);
    OCR4A = SYSCLOCK * MICROS_PER_TICK / 1000000;
    TCNT4 = 0;
}

//-----------------
#  if defined(CORE_OC4A_PIN)
#define IR_SEND_PIN  CORE_OC4A_PIN
#  elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  6  // Arduino Mega
#  else
#error Please add OC4A pin number here
#  endif

//---------------------------------------------------------
// Timer5 (16 bits)
//
#elif defined(IR_USE_TIMER5)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM       TCNT5 = 0; (TCCR5A |= _BV(COM5A1))
#define TIMER_DISABLE_SEND_PWM      (TCCR5A &= ~(_BV(COM5A1)))
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK5 = _BV(OCIE5A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK5 = 0)
#define TIMER_INTR_NAME             TIMER5_COMPA_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR5A = _BV(WGM51);
    TCCR5B = _BV(WGM53) | _BV(CS50);
    ICR5 = pwmval - 1;
    OCR5A = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT5 = 0; // required, since we have an 16 bit counter
}

static void timerConfigForReceive() {
    TCCR5A = 0;
    TCCR5B = _BV(WGM52) | _BV(CS50);
    OCR5A = SYSCLOCK * MICROS_PER_TICK / 1000000;
    TCNT5 = 0;
}

//-----------------
#  if defined(CORE_OC5A_PIN)
#define IR_SEND_PIN  CORE_OC5A_PIN
#  elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  46  // Arduino Mega
#  else
#error Please add OC5A pin number here
#  endif

#elif defined(IR_USE_TIMER_D)
#define TIMER_RESET_INTR_PENDING    (TCD0.INTFLAGS = TCD_OVF_bm)
#define TIMER_ENABLE_SEND_PWM       (timerEnablSendPWM())
#define TIMER_DISABLE_SEND_PWM      (TCD0.CTRLA = 0) // do not disable output, disable complete timer
#define TIMER_ENABLE_RECEIVE_INTR   (TCD0.INTCTRL = TCD_OVF_bm)
#define TIMER_DISABLE_RECEIVE_INTR  (TCD0.INTCTRL = 0)
#define TIMER_INTR_NAME             TCD0_OVF_vect

static void timerEnablSendPWM() {
    TCD0.CTRLA = 0;                                                 // reset enable bit in order to unprotect the other bits
    _PROTECTED_WRITE(TCD0.FAULTCTRL, FUSE_CMPAEN_bm);               // enable WOA output at pin 13/PA4
//    _PROTECTED_WRITE(TCD0.FAULTCTRL, FUSE_CMPAEN_bm | FUSE_CMPBEN_bm); // enable WOA + WOB output pins at 13/PA4 + 14/PA5
    TCD0.CTRLA = TCD_ENABLE_bm | TCD_CLKSEL_SYSCLK_gc| TCD_CNTPRES_DIV1_gc; // System clock, no prescale, no synchronization prescaler
}

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = (SYSCLOCK / 1000) / (aFrequencyKHz);    // 526,31 for 38 kHz @20 MHz clock
    // use one ramp mode and overflow interrupt
    TCD0.CTRLA = 0;                                                 // reset enable bit in order to unprotect the other bits
//    while ((TCD0.STATUS & TCD_ENRDY_bm) == 0);                      // Wait for Enable Ready to be high - I guess it is not required
    TCD0.CTRLB = TCD_WGMODE_ONERAMP_gc;                             // must be set since it is used by PWM
    TCD0.CTRLC = 0;                                                 // reset WOx output settings
//    TCD0.CMPBSET = 80;
    TCD0.CMPBCLR = pwmval - 1;

    // Generate duty cycle signal for debugging etc.
    TCD0.CMPASET = 0;
    TCD0.CMPACLR = (pwmval * IR_SEND_DUTY_CYCLE / 100) - 1;         // duty cycle for WOA

    TCD0.INTFLAGS = TCD_OVF_bm;                                     // reset interrupt flags
    TCD0.INTCTRL = TCD_OVF_bm;                                      // overflow interrupt
    // Do not enable timer, this is done at timerEnablSendPWM()
}

static void timerConfigForReceive() {
    TCD0.CTRLA = 0;                                                 // reset enable bit in order to unprotect the other bits
    TCD0.CTRLB = TCD_WGMODE_ONERAMP_gc;                             // must be set since it is used by PWM
//    TCD0.CMPBSET = 80;
    TCD0.CMPBCLR = ((SYSCLOCK * MICROS_PER_TICK) / 1000000) - 1;

    _PROTECTED_WRITE(TCD0.FAULTCTRL, 0);                            // must disable WOA output at pin 13/PA4

    TCD0.INTFLAGS = TCD_OVF_bm;                                     // reset interrupt flags
    TCD0.INTCTRL = TCD_OVF_bm;                                      // overflow interrupt
    // check enable ready
//    while ((TCD0.STATUS & TCD_ENRDY_bm) == 0); // Wait for Enable Ready to be high - I guess it is not required
    // enable timer - this locks the other bits and static registers and activates values in double buffered registers
    TCD0.CTRLA = TCD_ENABLE_bm | TCD_CLKSEL_SYSCLK_gc| TCD_CNTPRES_DIV1_gc; // System clock, no prescale, no synchronization prescaler
}

#define IR_SEND_PIN 13  // TinyCore 32

//---------------------------------------------------------
// Special carrier modulator timer for Teensy 3.0 / Teensy 3.1
//
#elif defined(IR_USE_TIMER_CMT)

#define TIMER_RESET_INTR_PENDING ({     \
uint8_t tmp __attribute__((unused)) = CMT_MSC; \
CMT_CMD2 = 30;         \
})

#define TIMER_ENABLE_SEND_PWM  do {                                         \
CORE_PIN5_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE | PORT_PCR_SRE;  \
} while(0)

#define TIMER_DISABLE_SEND_PWM  do {                                        \
CORE_PIN5_CONFIG = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;  \
} while(0)

#define TIMER_ENABLE_RECEIVE_INTR   NVIC_ENABLE_IRQ(IRQ_CMT)
#define TIMER_DISABLE_RECEIVE_INTR  NVIC_DISABLE_IRQ(IRQ_CMT)
#define TIMER_INTR_NAME     cmt_isr

//-----------------
#  ifdef ISR
#undef ISR
#  endif
#define ISR(f) void do_not_use__(void)

//-----------------
#define CMT_PPS_DIV  ((F_BUS + 7999999) / 8000000)
#  if F_BUS < 8000000
#error IRremote requires at least 8 MHz on Teensy 3.x
#  endif

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    SIM_SCGC4 |= SIM_SCGC4_CMT;
    SIM_SOPT2 |= SIM_SOPT2_PTD7PAD;
    CMT_PPS = CMT_PPS_DIV - 1;
    CMT_CGH1 = ((F_BUS / CMT_PPS_DIV / 3000) + ((aFrequencyKHz) / 2)) / (aFrequencyKHz);
    CMT_CGL1 = ((F_BUS / CMT_PPS_DIV / 1500) + ((aFrequencyKHz) / 2)) / (aFrequencyKHz);
    CMT_CMD1 = 0;
    CMT_CMD2 = 30;
    CMT_CMD3 = 0;
    CMT_CMD4 = 0;
    CMT_OC = 0x60;
    CMT_MSC = 0x01;
}

static void timerConfigForReceive() {
    SIM_SCGC4 |= SIM_SCGC4_CMT;
    CMT_PPS = CMT_PPS_DIV - 1;
    CMT_CGH1 = 1;
    CMT_CGL1 = 1;
    CMT_CMD1 = 0;
    CMT_CMD2 = 30;
    CMT_CMD3 = 0;
    CMT_CMD4 = (F_BUS / 160000 + CMT_PPS_DIV / 2) / CMT_PPS_DIV - 31;
    CMT_OC = 0;
    CMT_MSC = 0x03;
}

#define IR_SEND_PIN  5

// defines for TPM1 timer on Teensy-LC
#elif defined(IR_USE_TIMER_TPM1)
#define TIMER_RESET_INTR_PENDING        FTM1_SC |= FTM_SC_TOF;
#define TIMER_ENABLE_SEND_PWM           FTM1_CNT = 0; CORE_PIN16_CONFIG = PORT_PCR_MUX(3)|PORT_PCR_DSE|PORT_PCR_SRE
#define TIMER_DISABLE_SEND_PWM          CORE_PIN16_CONFIG = PORT_PCR_MUX(1)|PORT_PCR_SRE
#define TIMER_ENABLE_RECEIVE_INTR       NVIC_ENABLE_IRQ(IRQ_FTM1)
#define TIMER_DISABLE_RECEIVE_INTR      NVIC_DISABLE_IRQ(IRQ_FTM1)
#define TIMER_INTR_NAME                 ftm1_isr
#  ifdef ISR
#undef ISR
#  endif
#define ISR(f) void do_not_use__(void)

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    SIM_SCGC6 |= SIM_SCGC6_TPM1;
    FTM1_SC = 0;
    FTM1_CNT = 0;
    FTM1_MOD = ((F_PLL / 2000) / aFrequencyKHz) - 1;
    FTM1_C0V = ((F_PLL / 6000) / aFrequencyKHz) - 1;
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);
}

static void timerConfigForReceive() {
    SIM_SCGC6 |= SIM_SCGC6_TPM1;
    FTM1_SC = 0;
    FTM1_CNT = 0;
    FTM1_MOD = (F_PLL / 40000) - 1;
    FTM1_C0V = 0;
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(0) | FTM_SC_TOF | FTM_SC_TOIE;
}
#define IR_SEND_PIN        16

// defines for timer_tiny0 (8 bits)
#elif defined(IR_USE_TIMER_TINY0)
#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM           TCNT0 = 0; (TCCR0A |= _BV(COM0B1))
#define TIMER_DISABLE_SEND_PWM          (TCCR0A &= ~(_BV(COM0B1)))
#define TIMER_ENABLE_RECEIVE_INTR       (TIMSK |= _BV(OCIE0A))
#define TIMER_DISABLE_RECEIVE_INTR      (TIMSK &= ~(_BV(OCIE0A)))
#define TIMER_INTR_NAME                 TIMER0_COMPA_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint16_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR0A = _BV(WGM00); // PWM, Phase Correct, Top is OCR0A
    TCCR0B = _BV(WGM02) | _BV(CS00); // CS00 -> no prescaling
    OCR0A = pwmval - 1;
    OCR0B = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT0 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
}

#define TIMER_COUNT_TOP  (SYSCLOCK * MICROS_PER_TICK / 1000000)
static void timerConfigForReceive() {
#  if (TIMER_COUNT_TOP < 256)
    TCCR0A = _BV(WGM01); // CTC, Top is OCR0A
    TCCR0B = _BV(CS00); // No prescaling
    OCR0A = TIMER_COUNT_TOP;
    TCNT0 = 0;
#  else
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS01); // prescaling by 8
    OCR0A = TIMER_COUNT_TOP / 8;
    TCNT0 = 0;
#  endif
}

#define IR_SEND_PIN        1

// defines for timer_tiny1 (8 bits)
#elif defined(IR_USE_TIMER_TINY1)
#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM       TCNT1 = 0; GTCCR |= _BV(PWM1B) | _BV(COM1B0) // Enable pin 4 PWM output (PB4 - Arduino D4)
#define TIMER_DISABLE_SEND_PWM      (GTCCR &= ~(_BV(PWM1B) | _BV(COM1B0)))
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK |= _BV(OCIE1B))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK &= ~(_BV(OCIE1B)))
#define TIMER_INTR_NAME             TIMER1_COMPB_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
#  if (((SYSCLOCK / 1000) / 38) < 256)
    const uint16_t pwmval = (SYSCLOCK / 1000) / (aFrequencyKHz); // 421 @16 MHz, 26 @1 MHz and 38 kHz
    TCCR1 = _BV(CTC1) | _BV(CS10);  // CTC1 = 1: TOP value set to OCR1C, CS10 No Prescaling
    OCR1C = pwmval - 1;
    OCR1B = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT1 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
    GTCCR = _BV(PWM1B) | _BV(COM1B0); // PWM1B = 1: Enable PWM for OCR1B, COM1B0 Clear on compare match
#  else
    const uint16_t pwmval = ((SYSCLOCK / 2) / 1000) / (aFrequencyKHz); // 210 for 16 MHz and 38 kHz
    TCCR1 = _BV(CTC1) | _BV(CS11);  // CTC1 = 1: TOP value set to OCR1C, CS11 Prescaling by 2
    OCR1C = pwmval - 1;
    OCR1B = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCNT1 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
    GTCCR = _BV(PWM1B) | _BV(COM1B0); // PWM1B = 1: Enable PWM for OCR1B, COM1B0 Clear on compare match
#  endif
}

#define TIMER_COUNT_TOP  (SYSCLOCK * MICROS_PER_TICK / 1000000)
static void timerConfigForReceive() {
#  if (TIMER_COUNT_TOP < 256)
    TCCR1 = _BV(CTC1) | _BV(CS10); // Clear Timer/Counter on Compare Match, Top is OCR1C, No prescaling
    GTCCR = 0;  // normal, non-PWM mode
    OCR1C = TIMER_COUNT_TOP;
    TCNT1 = 0;
#  else
    TCCR1 = _BV(CTC1) | _BV(CS12); // Clear Timer/Counter on Compare Match, Top is OCR1C, prescaling by 8
    GTCCR = 0;  // normal, non-PWM mode
    OCR1C = TIMER_COUNT_TOP / 8;
    TCNT1 = 0;
#  endif
}

#define IR_SEND_PIN        4

#elif defined(IR_USE_TIMER_4809_1)
// ATmega4809 TCB0
#define TIMER_RESET_INTR_PENDING    TCB0.INTFLAGS = TCB_CAPT_bm
#define TIMER_ENABLE_SEND_PWM       TCB0.CNT = 0; (TCB0.CTRLB |= TCB_CCMPEN_bm)
#define TIMER_DISABLE_SEND_PWM      (TCB0.CTRLB &= ~(TCB_CCMPEN_bm))
#define TIMER_ENABLE_RECEIVE_INTR   (TCB0.INTCTRL = TCB_CAPT_bm)
#define TIMER_DISABLE_RECEIVE_INTR  (TCB0.INTCTRL &= ~(TCB_CAPT_bm))
#define TIMER_INTR_NAME             TCB0_INT_vect

static void timerConfigForSend(uint8_t aFrequencyKHz) {
    const uint32_t pwmval = (SYSCLOCK / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCB0.CTRLB = TCB_CNTMODE_PWM8_gc;
    TCB0.CCMPL = pwmval - 1;
    TCB0.CCMPH = ((pwmval * IR_SEND_DUTY_CYCLE) / 100) - 1;
    TCB0.CTRLA = (TCB_CLKSEL_CLKDIV2_gc) | (TCB_ENABLE_bm);
    TCB0.CNT = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
}

static void timerConfigForReceive() {
    TCB0.CTRLB = (TCB_CNTMODE_INT_gc);
    TCB0.CCMP = ((SYSCLOCK * MICROS_PER_TICK) / 1000000);
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = (TCB_CLKSEL_CLKDIV1_gc) | (TCB_ENABLE_bm);
}

#define IR_SEND_PIN        6  /* Nano Every, Uno WiFi Rev2 */
//---------------------------------------------------------
// ESP32 (ESP8266 should likely be added here too)
//

// ESP32 has it own timer API and does not use these macros, but to avoid ifdef'ing
// them out in the common code, they are defined to no-op. This allows the code to compile
// (which it wouldn't otherwise) but irsend will not work until ESP32 specific code is written
//
// The timer code is in the esp32.cpp file
//
// An IRremote version for ESP8266 and ESP32 is available at https://github.com/crankyoldgit/IRremoteESP8266
#elif defined(IR_USE_TIMER_ESP32)
void setTimerFrequency(unsigned int aFrequencyHz);
void timerConfigForReceive();
void timerConfigForSend(uint8_t aFrequencyKHz);
#if !defined(IR_SEND_PIN)
#define IR_SEND_PIN 4 // can use any pin, no timer restrictions
#endif

#if ! defined(LED_CHANNEL)
#define LED_CHANNEL 0 // The channel used for PWM 0 to 7 are high speed PWM channels
#endif

extern hw_timer_t *timer;
extern IRAM_ATTR void IRTimer(); // defined in IRremote.cpp, masqueraded as ISR(TIMER_INTR_NAME)

#define TIMER_ENABLE_RECEIVE_INTR   timerAlarmEnable(timer)
#define TIMER_DISABLE_RECEIVE_INTR  timerEnd(timer); timerDetachInterrupt(timer)
#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM    ledcWrite(LED_CHANNEL, IR_SEND_DUTY_CYCLE) // we must use channel here not pin number
#define TIMER_DISABLE_SEND_PWM   ledcWrite(LED_CHANNEL, 0)

#ifdef ISR
#undef ISR
#endif
#define ISR(f) IRAM_ATTR void IRTimer()

#elif defined(ARDUINO_ARCH_SAMD)
void setTimerFrequency(unsigned int aFrequencyHz);
void timerConfigForReceive();

#if !defined(IR_SEND_PIN)
#define IR_SEND_PIN 9
#endif

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM     // Not presently used
#define TIMER_DISABLE_SEND_PWM
#define TIMER_ENABLE_RECEIVE_INTR   NVIC_EnableIRQ(TC3_IRQn)
#define TIMER_DISABLE_RECEIVE_INTR  NVIC_DisableIRQ(TC3_IRQn) // or TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
#define TIMER_INTR_NAME             TC3_Handler // Not presently used

#ifdef ISR
#undef ISR
#endif

#define ISR(f) void IRTimer(void)

#define SEND_PWM_BY_TIMER_NOT_SUPPORTED

#elif defined(NRF5) || defined(ARDUINO_ARCH_NRF52840)
void setTimerFrequency(unsigned int aFrequencyHz);
void timerConfigForReceive();
// The default pin used used for sending. 3, A0 - left pad
#define IR_SEND_PIN   3 // dummy since sending not yet supported

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   NVIC_EnableIRQ(TIMER2_IRQn);
#define TIMER_DISABLE_RECEIVE_INTR  NVIC_DisableIRQ(TIMER2_IRQn);
#  ifdef ISR
#undef ISR
#  endif
#define ISR(f) void IRTimer(void)

#define SEND_PWM_BY_TIMER_NOT_SUPPORTED


#elif  defined(ARDUINO_ARCH_STM32)
//void setTimerFrequency(unsigned int aFrequencyHz);
void timerConfigForReceive();

#include <HardwareTimer.h>
extern HardwareTimer hTim;

// The default pin used used for sending. 3, A0 - left pad
#define IR_SEND_PIN   3 // dummy since sending not yet supported

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   hTim.resume();
#define TIMER_DISABLE_RECEIVE_INTR  hTim.pause();

#  ifdef ISR
#undef ISR
#  endif
#define ISR(f) void IRTimer(void)

#define SEND_PWM_BY_TIMER_NOT_SUPPORTED



// defines for Particle special IntervalTimer
#elif defined(IR_USE_TIMER_PARTICLE)

#  ifndef __INTERVALTIMER_H__
#include "SparkIntervalTimer.h" // SparkIntervalTimer.h is required if PARTICLE is defined.
#  endif

#  ifndef IR_SEND_PIN
#define IR_SEND_PIN         A5 // Particle supports multiple pins
#  endif

#  ifndef IR_OUT_KHZ
#define IR_OUT_KHZ          38 // default set to 38 KHz
#  endif

extern IntervalTimer timer;
extern int ir_send_pin;
extern int ir_out_kHz;


void IRTimer();

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_SEND_PWM       analogWrite(ir_send_pin, 128, ir_out_kHz*1000)
#define TIMER_DISABLE_SEND_PWM      analogWrite(ir_send_pin, 0, ir_out_kHz*1000)
#define TIMER_ENABLE_RECEIVE_INTR   timer.begin(IRTimer, 50, uSec);
#define TIMER_DISABLE_RECEIVE_INTR  timer.end()

#  ifdef ISR
#undef ISR
#  endif
#define ISR(f)  IntervalTimer timer; \
                ir_out_kHz = IR_OUT_KHZ; \
                ir_send_pin = IR_SEND_PIN; \
                void IRTimer(void)

static void timerConfigForSend(uint8_t aFrequencyKHz) {
  ir_out_kHz = aFrequencyKHz;
}

static void timerConfigForReceive() {
}

//---------------------------------------------------------
// Unknown Timer
//
#else
#error Internal code configuration error, no known IR_USE_TIMER* defined
#endif

//------------------------------------------------------------------------------
// Here are defines for blinking the LED,
// as well as all other board specific information, with the exception of
// timers and the sending pin (IR_SEND_PIN).

#ifdef DOXYGEN
/**
 * If defined, denotes pin number of LED that should be blinked during IR reception.
 * Leave undefined to disable blinking.
 */
#define BLINKLED        LED_BUILTIN

/**
 * Board dependent macro to turn BLINKLED on.
 */
#define BLINKLED_ON()   digitalWrite(BLINKLED, HIGH)

/**
 * Board dependent macro to turn BLINKLED off.
 */
#define BLINKLED_OFF()  digitalWrite(BLINKLED, LOW)

#elif defined(CORE_LED0_PIN)
#define BLINKLED        CORE_LED0_PIN
#define BLINKLED_ON()   (digitalWrite(CORE_LED0_PIN, HIGH))
#define BLINKLED_OFF()  (digitalWrite(CORE_LED0_PIN, LOW))

// Sparkfun Pro Micro is __AVR_ATmega32U4__ but has different external circuit
#elif defined(ARDUINO_AVR_PROMICRO)
// We have no built in LED -> reuse RX LED
#define BLINKLED        LED_BUILTIN_RX
#define BLINKLED_ON()   RXLED1
#define BLINKLED_OFF()  RXLED0

// Arduino Leonardo + others
#elif defined(__AVR_ATmega32U4__)
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   (PORTC |= B10000000)
#define BLINKLED_OFF()  (PORTC &= B01111111)

#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__)  || defined(__AVR_ATmega32U2__)
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   (digitalWrite(LED_BUILTIN, HIGH))
#define BLINKLED_OFF()  (digitalWrite(LED_BUILTIN, LOW))

// Arduino Uno, Nano etc
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega168__)
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   (PORTB |= B00100000)
#define BLINKLED_OFF()  (PORTB &= B11011111)

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define BLINKLED        13
#define BLINKLED_ON()   (PORTB |= B10000000)
#define BLINKLED_OFF()  (PORTB &= B01111111)

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#define BLINKLED        0
#define BLINKLED_ON()   (PORTD |= B00000001)
#define BLINKLED_OFF()  (PORTD &= B11111110)

// Nano Every, Uno WiFi Rev2, nRF5 BBC MicroBit, Nano33_BLE
#elif defined(__AVR_ATmega4809__) || defined(NRF5) || defined(ARDUINO_ARCH_NRF52840) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   (digitalWrite(BLINKLED, HIGH))
#define BLINKLED_OFF()  (digitalWrite(BLINKLED, LOW))

// STM32
#elif defined(ARDUINO_ARCH_STM32) 
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   (digitalWrite(BLINKLED, HIGH))
#define BLINKLED_OFF()  (digitalWrite(BLINKLED, LOW))


// TinyCore boards
#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
// No LED available on the board, take LED_BUILTIN which is also the DAC output
#define BLINKLED        LED_BUILTIN // PA6
#define BLINKLED_ON()   (PORTC.OUTSET = _BV(6))
#define BLINKLED_OFF()  (PORTC.OUTCLR = _BV(6))

// Arduino Zero
#elif defined(ARDUINO_ARCH_SAMD)
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   (digitalWrite(LED_BUILTIN, HIGH))
#define BLINKLED_OFF()  (digitalWrite(LED_BUILTIN, LOW))

#elif defined(ESP32)
// No system LED on ESP32, disable blinking by NOT defining BLINKLED

// Supply own enableIRIn() and enableIROut()
#undef USE_DEFAULT_ENABLE_IR_IN

#elif defined(PARTICLE)

#define BLINKLED       D7
#define BLINKLED_ON()  digitalWrite(BLINKLED,1)
#define BLINKLED_OFF() digitalWrite(BLINKLED,0)

#else
#warning No blinking definition found. Check IRremoteBoardDefs.h.
#  ifdef LED_BUILTIN
#define BLINKLED        LED_BUILTIN
#define BLINKLED_ON()   digitalWrite(BLINKLED, HIGH)
#define BLINKLED_OFF()  digitalWrite(BLINKLED, LOW)
#  endif
#endif

#endif // ! IRremoteBoardDefs_h

#pragma once

