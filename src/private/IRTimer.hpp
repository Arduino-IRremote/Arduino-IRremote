/**
 * @file IRTimer.hpp
 *
 * @brief All timer specific definitions are contained in this file.
 * Sets IR_SEND_PIN if required, e.g. if SEND_PWM_BY_TIMER for AVR is defined, which restricts the output to a dedicated pin number
 *
 * timerConfigForSend(aFrequencyKHz) must set output pin mode and disable receive interrupt if it uses the same resource
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021-2022 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
#ifndef _IR_TIMER_HPP
#define _IR_TIMER_HPP

/** \addtogroup HardwareDependencies CPU / board dependent definitions
 * @{
 */
/** \addtogroup Timer Usage of timers for the different CPU / boards
 * @{
 */

#if defined(SEND_PWM_BY_TIMER) && ( (defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(PARTICLE)) || defined(ARDUINO_ARCH_MBED) )
#define SEND_PWM_DOES_NOT_USE_RECEIVE_TIMER // Receive timer and send generation are independent, so it is recommended to always define SEND_PWM_BY_TIMER
#endif

#if defined(IR_SEND_PIN) && defined(SEND_PWM_BY_TIMER) && !defined(SEND_PWM_DOES_NOT_USE_RECEIVE_TIMER) // For e.g ESP32 IR_SEND_PIN definition is useful
#undef IR_SEND_PIN // avoid warning below, user warning is done at IRremote.hpp
#endif

#if defined (DOXYGEN)
/**
 * Hardware / timer dependent pin number for sending IR if SEND_PWM_BY_TIMER is defined. Otherwise used as default for IrSender.sendPin.
 */
#define IR_SEND_PIN

#elif defined(__AVR__)
/**********************************************************************************************************************
 * Mapping of AVR boards to AVR timers
 * For some CPU's you have the option to switch the timer and the hardware send pin
 **********************************************************************************************************************/

/***************************************
 * Plain AVR CPU's, no boards
 ***************************************/
// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, Nano, etc
// ATmega328 and ATmega88
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega88P__) || defined(__AVR_ATmega88PB__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER2)
//#define IR_USE_AVR_TIMER1   // send pin = pin 9
#define IR_USE_AVR_TIMER2     // send pin = pin 3
#  endif

// Arduino Mega
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER2) && !defined(IR_USE_AVR_TIMER3) && !defined(IR_USE_AVR_TIMER4) && !defined(IR_USE_AVR_TIMER5)
//#define IR_USE_AVR_TIMER1   // send pin = pin 11
#define IR_USE_AVR_TIMER2     // send pin = pin 9
//#define IR_USE_AVR_TIMER3   // send pin = pin 5
//#define IR_USE_AVR_TIMER4   // send pin = pin 6
//#define IR_USE_AVR_TIMER5   // send pin = pin 46
#  endif

// Leonardo
#elif defined(__AVR_ATmega32U4__) && ! defined(TEENSYDUINO) && ! defined(ARDUINO_AVR_PROMICRO)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER3) && !defined(IR_USE_AVR_TIMER4_HS)
//#define IR_USE_AVR_TIMER1     // send pin = pin 9
#define IR_USE_AVR_TIMER3       // send pin = pin 5
//#define IR_USE_AVR_TIMER4_HS  // send pin = pin 13
#  endif

// Nano Every, Uno WiFi Rev2 and similar
#elif defined(__AVR_ATmega808__) || defined(__AVR_ATmega809__) || defined(__AVR_ATmega3208__) || defined(__AVR_ATmega3209__) \
     || defined(__AVR_ATmega1608__) || defined(__AVR_ATmega1609__) || defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__) || defined(__AVR_ATtiny1604__)
#  if !defined(IR_USE_AVR_TIMER_B)
#define IR_USE_AVR_TIMER_B     //  send pin = pin 6 on ATmega4809 1 on ATmega4809
#  endif

#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__) // e.g. TinyCore boards
#  if !defined(IR_USE_AVR_TIMER_A) && !defined(IR_USE_AVR_TIMER_D)
#define IR_USE_AVR_TIMER_A // use this if you use MegaTinyCore, Tone is on TCB and millis() on TCD
//#define IR_USE_AVR_TIMER_D // use this if you use TinyCore
#  endif

// ATmega8u2, ATmega16U2, ATmega32U2
#elif defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__)  || defined(__AVR_ATmega32U2__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1     // send pin = pin C6
#  endif

// Atmega8
#elif defined(__AVR_ATmega8__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1     // send pin = pin 9
#  endif

// ATtiny84
#elif defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny88__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1     // send pin = pin 6
#  endif

#elif  defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1   // send pin = pin PB1 / 8
#  endif
#define USE_TIMER_CHANNEL_B

//ATtiny85, 45, 25
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#  if !defined(IR_USE_AVR_TIMER_TINY0) && !defined(IR_USE_AVR_TIMER_TINY1)
#    if defined(ARDUINO_AVR_DIGISPARK) // tested with 16 and 8 MHz
#define IR_USE_AVR_TIMER_TINY0   // send pin = pin 1
// standard Digispark settings use timer 1 for millis() and micros()
#    else
// standard ATTinyCore settings use timer 0 for millis() and micros()
#define IR_USE_AVR_TIMER_TINY1   // send pin = pin 4
#    endif
#  endif

/***************************************
 * SPARKFUN Pro Micro board
 ***************************************/
#elif defined(ARDUINO_AVR_PROMICRO)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER3) && !defined(IR_USE_AVR_TIMER4_HS)
//#define IR_USE_AVR_TIMER1     // send pin = pin 9
#define IR_USE_AVR_TIMER3       // send pin = pin 5
//#define IR_USE_AVR_TIMER4_HS  // send pin = pin 13
#  endif

/***************************************
 * TEENSY Boards
 ***************************************/
// Teensy 1.0
#elif defined(__AVR_AT90USB162__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1     // send pin = pin 17
#  endif

// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER2) && !defined(IR_USE_AVR_TIMER3)
//#define IR_USE_AVR_TIMER1   // send pin = pin 25
#define IR_USE_AVR_TIMER2     // send pin = pin 1
//#define IR_USE_AVR_TIMER3   // send pin = pin 16
#  endif

// Teensy 2.0
#elif defined(__AVR_ATmega32U4__) && defined(TEENSYDUINO)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER3) && !defined(IR_USE_AVR_TIMER4_HS)
//#define IR_USE_AVR_TIMER1     // send pin = pin 14 (Teensy 2.0 - physical pin: B5)
//#define IR_USE_AVR_TIMER3     // send pin = pin 9  (Teensy 2.0 - physical pin: C6)
#define IR_USE_AVR_TIMER4_HS    // send pin = pin 10 (Teensy 2.0 - physical pin: C7)
#  endif

/***************************************
 * CPU's with MegaCore
 ***************************************/
// MegaCore - ATmega64, ATmega128
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1     // send pin = pin 13
#  endif

/***************************************
 * CPU's with MajorCore
 ***************************************/
#elif defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER3)
#define IR_USE_AVR_TIMER1     // send pin = pin 13
//#define IR_USE_AVR_TIMER3   // send pin = pin 12 - ATmega162 only
#  endif

/***************************************
 * CPU's with MightyCore
 ***************************************/
// MightyCore - ATmega1284
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER2) && !defined(IR_USE_AVR_TIMER3)
//#define IR_USE_AVR_TIMER1   // send pin = pin 13
#define IR_USE_AVR_TIMER2     // send pin = pin 14
//#define IR_USE_AVR_TIMER3   // send pin = pin 6
#  endif

// MightyCore - ATmega164, ATmega324, ATmega644
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__)
#  if !defined(IR_USE_AVR_TIMER1) && !defined(IR_USE_AVR_TIMER2)
//#define IR_USE_AVR_TIMER1   // send pin = pin 13
#define IR_USE_AVR_TIMER2     // send pin = pin 14
#  endif

// MightyCore - ATmega8535, ATmega16, ATmega32
#elif defined(__AVR_ATmega8535__) || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__)
#  if !defined(IR_USE_AVR_TIMER1)
#define IR_USE_AVR_TIMER1     // send pin = pin 13
#  endif

#endif // AVR CPU's
/**********************************************************************************************************************
 * End of AVR mapping, start of AVR timers
 **********************************************************************************************************************/
/*
 * AVR Timer1 (16 bits)
 */
#if defined(IR_USE_AVR_TIMER1)

#define TIMER_RESET_INTR_PENDING

#  if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__) \
|| defined(__AVR_ATmega8535__) || defined(__AVR_ATmega16__) \
|| defined(__AVR_ATmega32__) || defined(__AVR_ATmega64__) \
|| defined(__AVR_ATmega128__) || defined(__AVR_ATmega162__)
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK |= _BV(OCIE1A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK &= ~_BV(OCIE1A))
#  else
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK1 = _BV(OCIE1A))          // Timer/Counter1, Output Compare A Match Interrupt Enable
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK1 = 0)
#  endif

#  if defined(USE_TIMER_CHANNEL_B)
#    if defined(TIMER1_COMPB_vect)
#define TIMER_INTR_NAME       TIMER1_COMPB_vect
#    elif defined(TIM1_COMPB_vect)
#define TIMER_INTR_NAME       TIM1_COMPB_vect
#    endif
#else
#    if defined(TIMER1_COMPA_vect)
#define TIMER_INTR_NAME       TIMER1_COMPA_vect
#    elif defined(TIM1_COMPA_vect)
#define TIMER_INTR_NAME       TIM1_COMPA_vect
#    endif
#  endif

void timerConfigForReceive() {
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10); // CTC mode, no prescaling
    OCR1A = (F_CPU * MICROS_PER_TICK) / MICROS_IN_ONE_SECOND; // 16 * 50 = 800
    TCNT1 = 0;
}


#  if defined(SEND_PWM_BY_TIMER)
#    if defined(CORE_OC1A_PIN)
#define IR_SEND_PIN  CORE_OC1A_PIN  // Teensy

#    elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  11             // Arduino Mega

// MightyCore, MegaCore, MajorCore
#    elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__) || defined(__AVR_ATmega32__) \
|| defined(__AVR_ATmega16__) || defined(__AVR_ATmega8535__) \
|| defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) \
|| defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) \
|| defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__)
#define IR_SEND_PIN  13

#    elif defined(__AVR_ATtiny84__)
#define IR_SEND_PIN  6

#    elif defined(__AVR_ATtiny88__)
#define IR_SEND_PIN  8

#    elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#define IR_SEND_PIN  PIN_PB1 // OC1BU / PB1 / Pin8 at ATTinyCore (BU/PB1, BV/PB3, BW/PB5 and BX/PB7 are available) see ENABLE_SEND_PWM_BY_TIMER

#    else
#define IR_SEND_PIN  9              // OC1A Arduino Duemilanove, Diecimila, LilyPad, Sparkfun Pro Micro, Leonardo, MH-ET Tiny88 etc.
#    endif // defined(CORE_OC1A_PIN)

#    if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
// Clear OC1A/OC1B on Compare Match when up-counting. Set OC1A/OC1B on Compare Match when downcounting.
#define ENABLE_SEND_PWM_BY_TIMER   TCNT1 = 0;  (TCCR1A |= _BV(COM1A1); (TCCR1D |= _BV(OC1BU)) // + enable OC1BU as output
#define DISABLE_SEND_PWM_BY_TIMER  (TCCR1D = 0)
#    else
#define ENABLE_SEND_PWM_BY_TIMER   TCNT1 = 0; (TCCR1A |= _BV(COM1A1))  // Clear OC1A/OC1B on Compare Match when up-counting. Set OC1A/OC1B on Compare Match when downcounting.
#define DISABLE_SEND_PWM_BY_TIMER  (TCCR1A &= ~(_BV(COM1A1)))
#    endif

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;

#  if (((F_CPU / 2000) / 38) < 256)
    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR1A = _BV(WGM11);// PWM, Phase Correct, Top is ICR1
    TCCR1B = _BV(WGM13) | _BV(CS10);// CS10 -> no prescaling
    ICR1 = tPWMWrapValue - 1;
#    if defined(USE_TIMER_CHANNEL_B)
    OCR1A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
#    else
    OCR1A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
#    endif
    TCNT1 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
#  else
    const uint16_t tPWMWrapValue = ((F_CPU / 8) / 2000) / (aFrequencyKHz); // 2000 instead of 1000 because of Phase Correct PWM
    TCCR1A = _BV(WGM11);// PWM, Phase Correct, Top is ICR1
    TCCR1B = _BV(WGM13) | _BV(CS11);// CS11 -> Prescaling by 8
    ICR1 = tPWMWrapValue - 1;
#    if defined(USE_TIMER_CHANNEL_B)
    OCR1A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
#    else
    OCR1A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
#    endif
    TCNT1 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
#  endif
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer2 (8 bits)
 */
#elif defined(IR_USE_AVR_TIMER2)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK2 = _BV(OCIE2B))              // Output Compare Match A Interrupt Enable
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK2 = 0)
#define TIMER_INTR_NAME             TIMER2_COMPB_vect                   // We use TIMER2_COMPB_vect to be compatible with tone() library


#define TIMER_COUNT_TOP  (F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND)
/*
 * timerConfigForReceive() is used exclusively by IRrecv::start()
 * It generates an interrupt each 50 (MICROS_PER_TICK) us.
 */
void timerConfigForReceive() {
#  if (TIMER_COUNT_TOP < 256)
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS20);
    OCR2A = TIMER_COUNT_TOP;
    OCR2B = TIMER_COUNT_TOP;
    TCNT2 = 0;
#  else
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS21);
    OCR2A = TIMER_COUNT_TOP / 8;
    OCR2B = TIMER_COUNT_TOP / 8;
    TCNT2 = 0;
#  endif
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(CORE_OC2B_PIN)
#define IR_SEND_PIN  CORE_OC2B_PIN  // Teensy

#    elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  9              // Arduino Mega

#    elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__)
#define IR_SEND_PIN  14             // MightyCore, MegaCore

#    else
#define IR_SEND_PIN  3              // Arduino Duemilanove, Diecimila, LilyPad, etc
#    endif // defined(CORE_OC2B_PIN)

#define ENABLE_SEND_PWM_BY_TIMER    TCNT2 = 0; (TCCR2A |= _BV(COM2B1))  // Clear OC2B on Compare Match
#define DISABLE_SEND_PWM_BY_TIMER   (TCCR2A &= ~(_BV(COM2B1)))          // Normal port operation, OC2B disconnected.

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;

#  if (((F_CPU / 2000) / 38) < 256)
    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR2A = _BV(WGM20);// PWM, Phase Correct, Top is OCR2A
    TCCR2B = _BV(WGM22) | _BV(CS20);// CS20 -> no prescaling
    OCR2A = tPWMWrapValue - 1; // The top value for the timer.  The modulation frequency will be F_CPU / 2 / OCR2A.
    OCR2B = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT2 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
#  else
    const uint16_t tPWMWrapValue = ((F_CPU / 8) / 2000) / (aFrequencyKHz); // 2000 instead of 1000 because of Phase Correct PWM
    TCCR2A = _BV(WGM20);// PWM, Phase Correct, Top is OCR2A
    TCCR2B = _BV(WGM22) | _BV(CS21);// CS21 -> Prescaling by 8
    OCR2A = tPWMWrapValue - 1;
    OCR2B = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT2 = 0; // not really required, since we have an 8 bit counter, but makes the signal more reproducible
#  endif
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer3 (16 bits)
 */
#elif defined(IR_USE_AVR_TIMER3)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK3 = _BV(OCIE3B))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK3 = 0)
#define TIMER_INTR_NAME             TIMER3_COMPB_vect

void timerConfigForReceive() {
    TCCR3A = 0;
    TCCR3B = _BV(WGM32) | _BV(CS30);
    OCR3A = F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND;
    OCR3B = F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND;
    TCNT3 = 0;
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(CORE_OC3A_PIN)
#define IR_SEND_PIN  CORE_OC3A_PIN  // Teensy

#    elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) \
|| defined(__AVR_ATmega32U4__) || defined(ARDUINO_AVR_PROMICRO)
#define IR_SEND_PIN  5              // Arduino Mega, Arduino Leonardo, Sparkfun Pro Micro

#    elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
#define IR_SEND_PIN  6              // MightyCore, MegaCore

#    else
#error Please add OC3A pin number here
#    endif

#define ENABLE_SEND_PWM_BY_TIMER    TCNT3 = 0; (TCCR3A |= _BV(COM3A1))
#define DISABLE_SEND_PWM_BY_TIMER   (TCCR3A &= ~(_BV(COM3A1)))

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
#if F_CPU > 16000000
#error "Creating timer PWM with timer 3 is not supported for F_CPU > 16 MHz"
#endif
    TIMER_DISABLE_RECEIVE_INTR;

    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR3A = _BV(WGM31);
    TCCR3B = _BV(WGM33) | _BV(CS30);// PWM, Phase Correct, ICRn as TOP, complete period is double of tPWMWrapValue
    ICR3 = tPWMWrapValue - 1;
    OCR3A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT3 = 0;// required, since we have an 16 bit counter
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer4 (16 bits)
 */
#elif defined(IR_USE_AVR_TIMER4)
#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK4 = _BV(OCIE4A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK4 = 0)
#define TIMER_INTR_NAME             TIMER4_COMPA_vect

void timerConfigForReceive() {
    TCCR4A = 0;
    TCCR4B = _BV(WGM42) | _BV(CS40);
    OCR4A = F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND;
    TCNT4 = 0;
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(CORE_OC4A_PIN)
#define IR_SEND_PIN  CORE_OC4A_PIN
#    elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  6  // Arduino Mega
#    else
#error Please add OC4A pin number here
#    endif

#define ENABLE_SEND_PWM_BY_TIMER    TCNT4 = 0; (TCCR4A |= _BV(COM4A1))
#define DISABLE_SEND_PWM_BY_TIMER   (TCCR4A &= ~(_BV(COM4A1)))

void timerConfigForSend(uint8_t aFrequencyKHz) {
#if F_CPU > 16000000
#error "Creating timer PWM with timer 4 is not supported for F_CPU > 16 MHz"
#endif
    TIMER_DISABLE_RECEIVE_INTR;
    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR4A = _BV(WGM41);
    TCCR4B = _BV(WGM43) | _BV(CS40);
    ICR4 = tPWMWrapValue - 1;
    OCR4A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT4 = 0;// required, since we have an 16 bit counter
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer4 (10 bits, high speed option)
 */
#elif defined(IR_USE_AVR_TIMER4_HS)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK4 = 0)
#define TIMER_INTR_NAME             TIMER4_OVF_vect

void timerConfigForReceive() {
    TCCR4A = 0;
    TCCR4B = _BV(CS40);
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;
    TC4H = (F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND) >> 8;
    OCR4C = (F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND) & 255;
    TC4H = 0;
    TCNT4 = 0;
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(CORE_OC4A_PIN)
#define IR_SEND_PIN  CORE_OC4A_PIN  // Teensy 2.0
#    elif defined(ARDUINO_AVR_PROMICRO)
#define IR_SEND_PIN  5              // Sparkfun Pro Micro
#    elif defined(__AVR_ATmega32U4__)
#define IR_SEND_PIN  13             // Leonardo
#    else
#error Please add OC4A pin number here
#    endif

#    if defined(ARDUINO_AVR_PROMICRO) // Sparkfun Pro Micro
#define ENABLE_SEND_PWM_BY_TIMER    TCNT4 = 0; (TCCR4A |= _BV(COM4A0))     // Use complementary OC4A output on pin 5
#define DISABLE_SEND_PWM_BY_TIMER   (TCCR4A &= ~(_BV(COM4A0)))  // (Pro Micro does not map PC7 (32/ICP3/CLK0/OC4A)
// of ATmega32U4 )
#    else
#define ENABLE_SEND_PWM_BY_TIMER    TCNT4 = 0; (TCCR4A |= _BV(COM4A1)); DDRC |= (1<<7)
#define DISABLE_SEND_PWM_BY_TIMER   (TCCR4A &= ~(_BV(COM4A1)))
#    endif

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
#if F_CPU > 16000000
#error "Creating timer PWM with timer 4 HS is not supported for F_CPU > 16 MHz"
#endif
    TIMER_DISABLE_RECEIVE_INTR;

    const uint16_t tPWMWrapValue = ((F_CPU / 2000) / (aFrequencyKHz)) - 1; // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR4A = (1 << PWM4A);
    TCCR4B = _BV(CS40);
    TCCR4C = 0;
    TCCR4D = (1 << WGM40);
    TCCR4E = 0;
    TC4H = tPWMWrapValue >> 8;
    OCR4C = tPWMWrapValue;
    TC4H = (tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT / 100) >> 8;
    OCR4A = (tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT / 100) & 255;
    TCNT4 = 0;// not really required, since we have an 8 bit counter, but makes the signal more reproducible
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer5 (16 bits)
 */
#elif defined(IR_USE_AVR_TIMER5)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK5 = _BV(OCIE5A))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK5 = 0)
#define TIMER_INTR_NAME             TIMER5_COMPA_vect

void timerConfigForReceive() {
    TCCR5A = 0;
    TCCR5B = _BV(WGM52) | _BV(CS50);
    OCR5A = F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND;
    TCNT5 = 0;
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(CORE_OC5A_PIN)
#define IR_SEND_PIN  CORE_OC5A_PIN
#    elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IR_SEND_PIN  46  // Arduino Mega
#    else
#error Please add OC5A pin number here
#    endif

#define ENABLE_SEND_PWM_BY_TIMER    TCNT5 = 0; (TCCR5A |= _BV(COM5A1))
#define DISABLE_SEND_PWM_BY_TIMER   (TCCR5A &= ~(_BV(COM5A1)))

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
#if F_CPU > 16000000
#error "Creating timer PWM with timer 5 is not supported for F_CPU > 16 MHz"
#endif
    TIMER_DISABLE_RECEIVE_INTR;

    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR5A = _BV(WGM51);
    TCCR5B = _BV(WGM53) | _BV(CS50);
    ICR5 = tPWMWrapValue - 1;
    OCR5A = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT5 = 0;// required, since we have an 16 bit counter
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer0 for ATtinies (8 bits)
 */
#elif defined(IR_USE_AVR_TIMER_TINY0)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR       (TIMSK |= _BV(OCIE0A))
#define TIMER_DISABLE_RECEIVE_INTR      (TIMSK &= ~(_BV(OCIE0A)))
#define TIMER_INTR_NAME                 TIMER0_COMPA_vect

#define TIMER_COUNT_TOP  (F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND)
void timerConfigForReceive() {
#  if (TIMER_COUNT_TOP < 256)
    TCCR0A = _BV(WGM01); // CTC, Top is OCR0A
    TCCR0B = _BV(CS00);// No prescaling
    OCR0A = TIMER_COUNT_TOP;
    TCNT0 = 0;
#  else
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS01); // prescaling by 8
    OCR0A = TIMER_COUNT_TOP / 8;
    TCNT0 = 0;
#  endif
}

#  if defined(SEND_PWM_BY_TIMER)
#define IR_SEND_PIN        1

#define ENABLE_SEND_PWM_BY_TIMER        TCNT0 = 0; (TCCR0A |= _BV(COM0B1))
#define DISABLE_SEND_PWM_BY_TIMER       (TCCR0A &= ~(_BV(COM0B1)))

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
#if F_CPU > 16000000
#error "Creating timer PWM with timer TINY0 is not supported for F_CPU > 16 MHz"
#endif
    TIMER_DISABLE_RECEIVE_INTR;

    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of Phase Correct PWM
    TCCR0A = _BV(WGM00);// PWM, Phase Correct, Top is OCR0A
    TCCR0B = _BV(WGM02) | _BV(CS00);// CS00 -> no prescaling
    OCR0A = tPWMWrapValue - 1;
    OCR0B = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT0 = 0;// not really required, since we have an 8 bit counter, but makes the signal more reproducible
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR Timer1 for ATtinies (8 bits)
 */
#elif defined(IR_USE_AVR_TIMER_TINY1)

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   (TIMSK |= _BV(OCIE1B))
#define TIMER_DISABLE_RECEIVE_INTR  (TIMSK &= ~(_BV(OCIE1B)))
#define TIMER_INTR_NAME             TIMER1_COMPB_vect

#define TIMER_COUNT_TOP  (F_CPU * MICROS_PER_TICK / MICROS_IN_ONE_SECOND)
void timerConfigForReceive() {
#  if (TIMER_COUNT_TOP < 256)
    TCCR1 = _BV(CTC1) | _BV(CS10); // Clear Timer/Counter on Compare Match, Top is OCR1C, No prescaling
    GTCCR = 0;// normal, non-PWM mode
    OCR1C = TIMER_COUNT_TOP;
    TCNT1 = 0;
#  else
    TCCR1 = _BV(CTC1) | _BV(CS12); // Clear Timer/Counter on Compare Match, Top is OCR1C, prescaling by 8
    GTCCR = 0;// normal, non-PWM mode
    OCR1C = TIMER_COUNT_TOP / 8;
    TCNT1 = 0;
#  endif
}

#  if defined(SEND_PWM_BY_TIMER)
#define IR_SEND_PIN        4

#define ENABLE_SEND_PWM_BY_TIMER    TCNT1 = 0; GTCCR |= _BV(PWM1B) | _BV(COM1B0) // Enable pin 4 PWM output (PB4 - Arduino D4)
#define DISABLE_SEND_PWM_BY_TIMER   (GTCCR &= ~(_BV(PWM1B) | _BV(COM1B0)))

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;

    #  if (((F_CPU / 1000) / 38) < 256)
    const uint16_t tPWMWrapValue = (F_CPU / 1000) / (aFrequencyKHz); // 421 @16 MHz, 26 @1 MHz and 38 kHz
    TCCR1 = _BV(CTC1) | _BV(CS10);// CTC1 = 1: TOP value set to OCR1C, CS10 No Prescaling
    OCR1C = tPWMWrapValue - 1;
    OCR1B = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT1 = 0;// not really required, since we have an 8 bit counter, but makes the signal more reproducible
    GTCCR = _BV(PWM1B) | _BV(COM1B0);// PWM1B = 1: Enable PWM for OCR1B, COM1B0 Clear on compare match
#  else
    const uint16_t tPWMWrapValue = ((F_CPU / 2) / 1000) / (aFrequencyKHz); // 210 for 16 MHz and 38 kHz
    TCCR1 = _BV(CTC1) | _BV(CS11);// CTC1 = 1: TOP value set to OCR1C, CS11 Prescaling by 2
    OCR1C = tPWMWrapValue - 1;
    OCR1B = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;
    TCNT1 = 0;// not really required, since we have an 8 bit counter, but makes the signal more reproducible
    GTCCR = _BV(PWM1B) | _BV(COM1B0);// PWM1B = 1: Enable PWM for OCR1B, COM1B0 Clear on compare match
#  endif
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR TimerA for TinyCore 32 (16 bits)
 */
#elif defined(IR_USE_AVR_TIMER_A)
#define TIMER_RESET_INTR_PENDING    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm
#define TIMER_ENABLE_RECEIVE_INTR   TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm
#define TIMER_DISABLE_RECEIVE_INTR  (TCA0.SINGLE.INTCTRL &= ~(TCA_SINGLE_OVF_bm))
#define TIMER_INTR_NAME             TCA0_OVF_vect
// For MegaTinyCore:
// TCB1 is used by Tone()
// TCB2 is used by Servo, but we cannot hijack the ISR, so we must use a dedicated timer for the 20 ms interrupt
// TCB3 is used by millis()
// Must use TCA0, since TCBx have only prescaler %2. Use single (16bit) mode, because it seems to be easier :-)
void timerConfigForReceive() {
    TCA0.SINGLE.CTRLD = 0; // Single mode - required at least for MegaTinyCore
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;                        // Normal mode, top = PER
    TCA0.SINGLE.PER = (F_CPU / MICROS_IN_ONE_SECOND) * MICROS_PER_TICK;     // 800 at 16 MHz
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;   // set prescaler to 1 and enable timer
}

#  if defined(SEND_PWM_BY_TIMER)
#error "No support for hardware PWM generation for ATtiny3216/17 etc."
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR TimerB  (8 bits) for ATmega4809 (Nano Every, Uno WiFi Rev2)
 */
#elif defined(IR_USE_AVR_TIMER_B)

// ATmega4809 TCB0
#define TIMER_RESET_INTR_PENDING    TCB0.INTFLAGS = TCB_CAPT_bm
#define TIMER_ENABLE_RECEIVE_INTR   (TCB0.INTCTRL = TCB_CAPT_bm)
#define TIMER_DISABLE_RECEIVE_INTR  (TCB0.INTCTRL &= ~(TCB_CAPT_bm))
#define TIMER_INTR_NAME             TCB0_INT_vect

void timerConfigForReceive() {
    TCB0.CTRLB = (TCB_CNTMODE_INT_gc);  // Periodic interrupt mode
    TCB0.CCMP = ((F_CPU * MICROS_PER_TICK) / MICROS_IN_ONE_SECOND);
    TCB0.INTFLAGS = TCB_CAPT_bm;// reset interrupt flags
    TCB0.CTRLA = (TCB_CLKSEL_CLKDIV1_gc) | (TCB_ENABLE_bm);
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__)
#define IR_SEND_PIN        6 // PF4 on ATmega4809 / Nano Every (see pins_arduino.h digital_pin_to_timer)
#    else
#error SEND_PWM_BY_TIMER not yet supported for this CPU
#    endif

#define ENABLE_SEND_PWM_BY_TIMER    TCB0.CNT = 0; (TCB0.CTRLB |= TCB_CCMPEN_bm) // set Compare/Capture Output Enable
#define DISABLE_SEND_PWM_BY_TIMER   (TCB0.CTRLB &= ~(TCB_CCMPEN_bm))

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
#if F_CPU > 16000000
    // we have only prescaler 2 or must take clock of timer A (which is non deterministic)
#error "Creating timer PWM with timer TCB0 is not possible for F_CPU > 16 MHz"
#endif
    TIMER_DISABLE_RECEIVE_INTR;

    const uint16_t tPWMWrapValue = (F_CPU / 2000) / (aFrequencyKHz); // 210,52 for 38 kHz @16 MHz clock, 2000 instead of 1000 because of using CLK / 2
    TCB0.CTRLB = TCB_CNTMODE_PWM8_gc;// 8 bit PWM mode
    TCB0.CCMPL = tPWMWrapValue - 1;// Period of 8 bit PWM
    TCB0.CCMPH = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1;// Duty cycle of waveform of 8 bit PWM
    TCB0.CTRLA = (TCB_CLKSEL_CLKDIV2_gc) | (TCB_ENABLE_bm);// use CLK / 2
    TCB0.CNT = 0;// not really required, since we have an 8 bit counter, but makes the signal more reproducible
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * AVR TimerD for TinyCore 32 (16 bits)
 */
#elif defined(IR_USE_AVR_TIMER_D)

#define TIMER_RESET_INTR_PENDING    (TCD0.INTFLAGS = TCD_OVF_bm)
#define TIMER_ENABLE_RECEIVE_INTR   (TCD0.INTCTRL = TCD_OVF_bm)
#define TIMER_DISABLE_RECEIVE_INTR  (TCD0.INTCTRL = 0)
#define TIMER_INTR_NAME             TCD0_OVF_vect

void timerConfigForReceive() {
    TCD0.CTRLA = 0;                     // reset enable bit in order to unprotect the other bits
    TCD0.CTRLB = TCD_WGMODE_ONERAMP_gc; // must be set since it is used by PWM
//    TCD0.CMPBSET = 80;
    TCD0.CMPBCLR = ((F_CPU * MICROS_PER_TICK) / MICROS_IN_ONE_SECOND) - 1;

    _PROTECTED_WRITE(TCD0.FAULTCTRL, 0); // must disable WOA output at pin 13/PA4

    TCD0.INTFLAGS = TCD_OVF_bm;         // reset interrupt flags
    // check enable ready
//    while ((TCD0.STATUS & TCD_ENRDY_bm) == 0); // Wait for Enable Ready to be high - I guess it is not required
    // enable timer - this locks the other bits and static registers and activates values in double buffered registers
    TCD0.CTRLA = TCD_ENABLE_bm | TCD_CLKSEL_SYSCLK_gc| TCD_CNTPRES_DIV1_gc;// System clock, no prescale, no synchronization prescaler
}

#  if defined(SEND_PWM_BY_TIMER)
#define IR_SEND_PIN 13

#define ENABLE_SEND_PWM_BY_TIMER    (timerEnableSendPWM())
#define DISABLE_SEND_PWM_BY_TIMER   (TCD0.CTRLA = 0) // do not disable output, disable complete timer

void timerEnableSendPWM() {
    TCD0.CTRLA = 0;                                                 // reset enable bit in order to unprotect the other bits
    _PROTECTED_WRITE(TCD0.FAULTCTRL, FUSE_CMPAEN_bm);// enable WOA output at pin 13/PA4
//    _PROTECTED_WRITE(TCD0.FAULTCTRL, FUSE_CMPAEN_bm | FUSE_CMPBEN_bm); // enable WOA + WOB output pins at 13/PA4 + 14/PA5
    TCD0.CTRLA = TCD_ENABLE_bm | TCD_CLKSEL_SYSCLK_gc| TCD_CNTPRES_DIV1_gc;// System clock, no prescale, no synchronization prescaler
}

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;

    const uint16_t tPWMWrapValue = (F_CPU / 1000) / (aFrequencyKHz);    // 526,31 for 38 kHz @20 MHz clock
    // use one ramp mode and overflow interrupt
    TCD0.CTRLA = 0;// reset enable bit in order to unprotect the other bits
//    while ((TCD0.STATUS & TCD_ENRDY_bm) == 0);                      // Wait for Enable Ready to be high - I guess it is not required
    TCD0.CTRLB = TCD_WGMODE_ONERAMP_gc;// must be set since it is used by PWM
    TCD0.CTRLC = 0;// reset WOx output settings
//    TCD0.CMPBSET = 80;
    TCD0.CMPBCLR = tPWMWrapValue - 1;

    // Generate duty cycle signal for debugging etc.
    TCD0.CMPASET = 0;
    TCD0.CMPACLR = (tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT / 100) - 1;// duty cycle for WOA

    TCD0.INTFLAGS = TCD_OVF_bm;// reset interrupt flags
    TCD0.INTCTRL = TCD_OVF_bm;// overflow interrupt
    // Do not enable timer, this is done at timerEnablSendPWM()
}
#  endif // defined(SEND_PWM_BY_TIMER)

#else
#error Internal code configuration error, no timer functions implemented for this AVR CPU / board
#endif //defined(IR_USE_AVR_TIMER*)
/**********************************************************************************************************************
 * End of AVR timers
 **********************************************************************************************************************/

/***************************************
 * Teensy 3.0 / Teensy 3.1 boards
 ***************************************/
#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)

// Special carrier modulator timer for Teensy 3.0 / Teensy 3.1
#define TIMER_RESET_INTR_PENDING    uint8_t tmp __attribute__((unused)) = CMT_MSC; CMT_CMD2 = 30
#define TIMER_ENABLE_RECEIVE_INTR   NVIC_ENABLE_IRQ(IRQ_CMT), NVIC_SET_PRIORITY(IRQ_CMT, 48)
#define TIMER_DISABLE_RECEIVE_INTR  NVIC_DISABLE_IRQ(IRQ_CMT)
#define TIMER_INTR_NAME     cmt_isr

#  if defined(ISR)
#undef ISR
#  endif
#define ISR(f) void f(void)

#define CMT_PPS_DIV  ((F_BUS + 7999999) / 8000000)
#  if F_BUS < 8000000
#error IRremote requires at least 8 MHz on Teensy 3.x
#  endif

void timerConfigForReceive() {
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

#  if defined(SEND_PWM_BY_TIMER)
#define IR_SEND_PIN  5

#define ENABLE_SEND_PWM_BY_TIMER    do { CORE_PIN5_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE | PORT_PCR_SRE; } while(0)
#define DISABLE_SEND_PWM_BY_TIMER   do { CORE_PIN5_CONFIG = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE; } while(0)

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR; // TODO really required here? Do we have a common resource for Teensy3.0, 3.1
#    if defined(IR_SEND_PIN)
    pinMode(IR_SEND_PIN, OUTPUT);
#    else
    pinMode(IrSender.sendPin, OUTPUT);
#    endif

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
#  endif // defined(SEND_PWM_BY_TIMER)

/***************************************
 * Teensy-LC board
 ***************************************/
#elif defined(__MKL26Z64__)

// defines for TPM1 timer on Teensy-LC
#define TIMER_RESET_INTR_PENDING        FTM1_SC |= FTM_SC_TOF;
#define TIMER_ENABLE_RECEIVE_INTR       NVIC_ENABLE_IRQ(IRQ_FTM1), NVIC_SET_PRIORITY(IRQ_FTM1, 0)
#define TIMER_DISABLE_RECEIVE_INTR      NVIC_DISABLE_IRQ(IRQ_FTM1)
#define TIMER_INTR_NAME                 ftm1_isr
#  if defined(ISR)
#undef ISR
#  endif
#define ISR(f) void f(void)

void timerConfigForReceive() {
    SIM_SCGC6 |= SIM_SCGC6_TPM1;
    FTM1_SC = 0;
    FTM1_CNT = 0;
    FTM1_MOD = (F_PLL / 40000) - 1;
    FTM1_C0V = 0;
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(0) | FTM_SC_TOF | FTM_SC_TOIE;
}

#  if defined(SEND_PWM_BY_TIMER)
#define IR_SEND_PIN        16

#define ENABLE_SEND_PWM_BY_TIMER        FTM1_CNT = 0; CORE_PIN16_CONFIG = PORT_PCR_MUX(3)|PORT_PCR_DSE|PORT_PCR_SRE
#define DISABLE_SEND_PWM_BY_TIMER       CORE_PIN16_CONFIG = PORT_PCR_MUX(1)|PORT_PCR_SRE

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;
#    if defined(IR_SEND_PIN)
    pinMode(IR_SEND_PIN, OUTPUT);
#    else
    pinMode(IrSender.sendPin, OUTPUT);
#    endif

    SIM_SCGC6 |= SIM_SCGC6_TPM1;
    FTM1_SC = 0;
    FTM1_CNT = 0;
    FTM1_MOD = ((F_PLL / 2000) / aFrequencyKHz) - 1;
    FTM1_C0V = ((F_PLL / 6000) / aFrequencyKHz) - 1;
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);
}
#  endif // defined(SEND_PWM_BY_TIMER)

/***************************************
 * Teensy 4.0, 4.1, MicroMod boards
 ***************************************/
#elif defined(__IMXRT1062__)

// defines for FlexPWM1 timer on Teensy 4
#define TIMER_RESET_INTR_PENDING        FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF;
#define TIMER_ENABLE_RECEIVE_INTR       attachInterruptVector(IRQ_FLEXPWM1_3, pwm1_3_isr),\
                                        FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF, \
                                        FLEXPWM1_SM3INTEN = FLEXPWM_SMINTEN_RIE, \
                                        NVIC_ENABLE_IRQ(IRQ_FLEXPWM1_3), \
                                        NVIC_SET_PRIORITY(IRQ_FLEXPWM1_3, 48)
#define TIMER_DISABLE_RECEIVE_INTR      NVIC_DISABLE_IRQ(IRQ_FLEXPWM1_3)
#define TIMER_INTR_NAME                 pwm1_3_isr
#  if defined(ISR)
#undef ISR
#  endif
#define ISR(f) void (f)(void)
void pwm1_3_isr();

void timerConfigForReceive() {
    uint32_t period = (float)F_BUS_ACTUAL * (float)(MICROS_PER_TICK) * 0.0000005f;
    uint32_t prescale = 0;
    while (period > 32767) {
        period = period >> 1;
        if (prescale < 7) prescale++;
    }
    FLEXPWM1_FCTRL0 |= FLEXPWM_FCTRL0_FLVL(8);
    FLEXPWM1_FSTS0 = 0x0008;
    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(8);
    FLEXPWM1_SM3CTRL2 = FLEXPWM_SMCTRL2_INDEP;
    FLEXPWM1_SM3CTRL = FLEXPWM_SMCTRL_HALF | FLEXPWM_SMCTRL_PRSC(prescale);
    FLEXPWM1_SM3INIT = -period;
    FLEXPWM1_SM3VAL0 = 0;
    FLEXPWM1_SM3VAL1 = period;
    FLEXPWM1_SM3VAL2 = 0;
    FLEXPWM1_SM3VAL3 = 0;
    FLEXPWM1_SM3VAL4 = 0;
    FLEXPWM1_SM3VAL5 = 0;
    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(8) | FLEXPWM_MCTRL_RUN(8);
}

#  if defined(SEND_PWM_BY_TIMER)
#define IR_SEND_PIN        7
#define ENABLE_SEND_PWM_BY_TIMER        FLEXPWM1_OUTEN |= FLEXPWM_OUTEN_PWMA_EN(8), \
                                        IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 = 6
#define DISABLE_SEND_PWM_BY_TIMER       IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 = 5, \
                                        FLEXPWM1_OUTEN &= ~FLEXPWM_OUTEN_PWMA_EN(8)

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;
#    if defined(IR_SEND_PIN)
    pinMode(IR_SEND_PIN, OUTPUT);
#    else
    pinMode(IrSender.sendPin, OUTPUT);
#    endif

    uint32_t period = (float)F_BUS_ACTUAL / (float)((aFrequencyKHz) * 2000);
    uint32_t prescale = 0;
    while (period > 32767) {
        period = period >> 1;
        if (prescale < 7) prescale++;
    }
    FLEXPWM1_FCTRL0 |= FLEXPWM_FCTRL0_FLVL(8);
    FLEXPWM1_FSTS0 = 0x0008;
    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(8);
    FLEXPWM1_SM3CTRL2 = FLEXPWM_SMCTRL2_INDEP;
    FLEXPWM1_SM3CTRL = FLEXPWM_SMCTRL_HALF | FLEXPWM_SMCTRL_PRSC(prescale);
    FLEXPWM1_SM3INIT = -period;
    FLEXPWM1_SM3VAL0 = 0;
    FLEXPWM1_SM3VAL1 = period;
    FLEXPWM1_SM3VAL2 = -(period / 3);
    FLEXPWM1_SM3VAL3 = period / 3;
    FLEXPWM1_SM3VAL4 = 0;
    FLEXPWM1_SM3VAL5 = 0;
    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(8) | FLEXPWM_MCTRL_RUN(8);
}
#  endif // defined(SEND_PWM_BY_TIMER)

#elif defined(ESP8266)
#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   timer1_attachInterrupt(&IRTimerInterruptHandler) // enables interrupt too
#define TIMER_DISABLE_RECEIVE_INTR  timer1_detachInterrupt() // disables interrupt too

// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() IRAM_ATTR void IRTimerInterruptHandler()
IRAM_ATTR void IRTimerInterruptHandler();

#  if defined(SEND_PWM_BY_TIMER)
#error "No support for hardware PWM generation for ESP8266"
#  endif // defined(SEND_PWM_BY_TIMER)

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    timer1_isr_init();
    /*
     * TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
     * TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)
     * TIM_DIV256 = 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
     */
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write((80 / 16) * MICROS_PER_TICK); // 80 for 80 and 160! MHz clock, 16 for TIM_DIV16 above
}

/**********************************************************
 * ESP32 boards - can use any pin for send PWM
 * Receive timer and send generation are independent,
 * so it is recommended to always define SEND_PWM_BY_TIMER
 **********************************************************/
#elif defined(ESP32)

#  if !defined(SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL)
#define SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL 0 // The channel used for PWM 0 to 7 are high speed PWM channels
#  endif

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   timerAlarmEnable(s50usTimer)
#define TIMER_DISABLE_RECEIVE_INTR  if (s50usTimer != NULL) {timerEnd(s50usTimer); timerDetachInterrupt(s50usTimer);}
// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() IRAM_ATTR void IRTimerInterruptHandler()
IRAM_ATTR void IRTimerInterruptHandler();

// Variables specific to the ESP32.
// the ledc functions behave like hardware timers for us :-), so we do not require our own soft PWM generation code.
hw_timer_t *s50usTimer;

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    // ESP32 has a proper API to setup timers, no weird chip macros needed
    // simply call the readable API versions :)
    // 3 timers, choose #1, 80 divider for microsecond precision @80MHz clock, count_up = true
    s50usTimer = timerBegin(1, 80, true);
    timerAttachInterrupt(s50usTimer, &IRTimerInterruptHandler, 1);
    // every 50 us, autoreload = true
    timerAlarmWrite(s50usTimer, MICROS_PER_TICK, true);
}

#  if defined(SEND_PWM_BY_TIMER)
#define ENABLE_SEND_PWM_BY_TIMER    ledcWrite(SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL, (IR_SEND_DUTY_CYCLE_PERCENT * 256 )/ 100) //  * 256 since we have 8 bit resolution
#define DISABLE_SEND_PWM_BY_TIMER   ledcWrite(SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL, 0)

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * ledcWrite since ESP 2.0.2 does not work if pin mode is set. Disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    ledcSetup(SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL, aFrequencyKHz * 1000, 8);  // 8 bit PWM resolution
#    if defined(IR_SEND_PIN)
    ledcAttachPin(IR_SEND_PIN, SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL);  // bind pin to channel
#    else
    ledcAttachPin(IrSender.sendPin, SEND_AND_RECEIVE_TIMER_LEDC_CHANNEL);  // bind pin to channel
#    endif
}
#  endif // defined(SEND_PWM_BY_TIMER)

/***************************************
 * SAMD boards like DUE and Zero
 ***************************************/
#elif defined(ARDUINO_ARCH_SAMD)
#  if defined(SEND_PWM_BY_TIMER)
#error PWM generation by hardware is not yet implemented for SAMD
#  endif

#  if !defined(IR_SAMD_TIMER)
#    if defined(__SAMD51__)
#define IR_SAMD_TIMER       TC5
#define IR_SAMD_TIMER_IRQ   TC5_IRQn
#    else
// SAMD21
#define IR_SAMD_TIMER       TC3
#define IR_SAMD_TIMER_ID    GCLK_CLKCTRL_ID_TCC2_TC3
#define IR_SAMD_TIMER_IRQ   TC3_IRQn
#    endif
#  endif

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   NVIC_EnableIRQ(IR_SAMD_TIMER_IRQ)
#define TIMER_DISABLE_RECEIVE_INTR  NVIC_DisableIRQ(IR_SAMD_TIMER_IRQ) // or TC5->INTENCLR.bit.MC0 = 1; or TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR(f) void IRTimerInterruptHandler(void)
// ATSAMD Timer IRQ functions
void IRTimerInterruptHandler();

/**
 * Adafruit M4 code (cores/arduino/startup.c) configures these clock generators:
 * GCLK0 = F_CPU
 * GCLK2 = 100 MHz
 * GCLK1 = 48 MHz // This Clock is present in SAMD21 and SAMD51
 * GCLK4 = 12 MHz
 * GCLK3 = XOSC32K
 */
/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    TcCount16 *TC = (TcCount16*) IR_SAMD_TIMER;

#  if defined(__SAMD51__)
    // Enable the TC5 clock, use generic clock generator 0 (F_CPU) for TC5
    GCLK->PCHCTRL[TC5_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

    // The TC should be disabled before the TC is reset in order to avoid undefined behavior.
    TC->CTRLA.reg &= ~TC_CTRLA_ENABLE; // Disable the Timer
    while (TC->SYNCBUSY.bit.ENABLE)
        ; // Wait for disabled
    // Reset TCx
    TC->CTRLA.reg = TC_CTRLA_SWRST;
    // When writing a '1' to the CTRLA.SWRST bit it will immediately read as '1'.
    while (TC->SYNCBUSY.bit.SWRST)
        ; // CTRL.SWRST will be cleared by hardware when the peripheral has been reset.

    // SAMD51 has F_CPU = 120 MHz
    TC->CC[0].reg = ((MICROS_PER_TICK * (F_CPU / MICROS_IN_ONE_SECOND)) / 16) - 1;   // (375 - 1);

    /*
     * Set timer counter mode to 16 bits, set mode as match frequency, prescaler is DIV16 => 7.5 MHz clock, start counter
     */
    TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_WAVE_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_ENABLE;
//    while (TC5->COUNT16.STATUS.bit.SYNCBUSY == 1);                                // The next commands do an implicit wait :-)
#  else
    // Enable GCLK and select GCLK0 (F_CPU) as clock for TC4 and TC5
    REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | IR_SAMD_TIMER_ID);
    while (GCLK->STATUS.bit.SYNCBUSY == 1);

    // The TC should be disabled before the TC is reset in order to avoid undefined behavior.
    TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    // When write-synchronization is ongoing for a register, any subsequent write attempts to this register will be discarded, and an error will be reported.
    while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync to ensure that we can write again to COUNT16.CTRLA.reg
    // Reset TCx
    TC->CTRLA.reg = TC_CTRLA_SWRST;
    // When writing a ‘1’ to the CTRLA.SWRST bit it will immediately read as ‘1’.
    while (TC->CTRLA.bit.SWRST); // CTRL.SWRST will be cleared by hardware when the peripheral has been reset.

    // SAMD51 has F_CPU = 48 MHz
    TC->CC[0].reg = ((MICROS_PER_TICK * (F_CPU / MICROS_IN_ONE_SECOND)) / 16) - 1;   // (150 - 1);

    /*
     * Set timer counter mode to 16 bits, set mode as match frequency, prescaler is DIV16 => 3 MHz clock, start counter
     */
    TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_ENABLE;

#  endif
    // Configure interrupt request
    NVIC_DisableIRQ (IR_SAMD_TIMER_IRQ);
    NVIC_ClearPendingIRQ(IR_SAMD_TIMER_IRQ);
    NVIC_SetPriority(IR_SAMD_TIMER_IRQ, 0);
    NVIC_EnableIRQ(IR_SAMD_TIMER_IRQ);

    // Enable the compare interrupt
    TC->INTENSET.bit.MC0 = 1;

}

#  if defined(__SAMD51__)
void TC5_Handler(void) {
    TcCount16 *TC = (TcCount16*) IR_SAMD_TIMER;
    // Check for right interrupt bit
    if (TC->INTFLAG.bit.MC0 == 1) {
        // reset bit for next turn
        TC->INTFLAG.bit.MC0 = 1;
        IRTimerInterruptHandler();
    }
}
#  else
void TC3_Handler(void) {
    TcCount16 *TC = (TcCount16*) IR_SAMD_TIMER;
    // Check for right interrupt bit
    if (TC->INTFLAG.bit.MC0 == 1) {
        // reset bit for next turn
        TC->INTFLAG.bit.MC0 = 1;
        IRTimerInterruptHandler();
    }
}
#  endif // defined(__SAMD51__)

/***************************************
 * Mbed based boards
 ***************************************/
#elif defined(ARDUINO_ARCH_MBED) // Arduino Nano 33 BLE + Sparkfun Apollo3 + Nano RP2040 Connect
#include "mbed.h"

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   s50usTimer.attach(IRTimerInterruptHandler, std::chrono::microseconds(MICROS_PER_TICK));
#define TIMER_DISABLE_RECEIVE_INTR  s50usTimer.detach();

// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() void IRTimerInterruptHandler(void)
void IRTimerInterruptHandler();

mbed::Ticker s50usTimer;

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    s50usTimer.attach(IRTimerInterruptHandler, std::chrono::microseconds(MICROS_PER_TICK));
}

#  if defined(SEND_PWM_BY_TIMER)
#include "pins_arduino.h" // for digitalPinToPinName()

#    if defined(IR_SEND_PIN)
mbed::PwmOut sPwmOutForSendPWM(digitalPinToPinName(IR_SEND_PIN));
#    else
mbed::PwmOut sPwmOutForSendPWM(digitalPinToPinName(IrSender.sendPin));
#    endif
uint8_t sIROutPuseWidth;

#define ENABLE_SEND_PWM_BY_TIMER    sPwmOutForSendPWM.pulsewidth_us(sIROutPuseWidth)
//#define ENABLE_SEND_PWM_BY_TIMER    sPwmOutForSendPWM.resume();sPwmOutForSendPWM.pulsewidth_us(sIROutPuseWidth)
//#define DISABLE_SEND_PWM_BY_TIMER   sPwmOutForSendPWM.suspend() // this kills pulsewidth_us value and does not set output level to LOW
#define DISABLE_SEND_PWM_BY_TIMER   sPwmOutForSendPWM.pulsewidth_us(0) // this also sets output level to LOW :-)

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    sPwmOutForSendPWM.period_us(1000 / aFrequencyKHz);  // 26.315 for 38 kHz
    sIROutPuseWidth = (1000 * IR_SEND_DUTY_CYCLE_PERCENT) / (aFrequencyKHz * 100);
}
#  endif // defined(SEND_PWM_BY_TIMER)

/*************************************************************************************************************************************
 * RP2040 based boards for pico core
 * https://github.com/earlephilhower/arduino-pico
 * https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
 * Can use any pin for PWM, no timer restrictions
 *************************************************************************************************************************************/
#elif defined(ARDUINO_ARCH_RP2040) // Raspberry Pi Pico, Adafruit Feather RP2040, etc.
#include "pico/time.h"

repeating_timer_t s50usTimer;

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   add_repeating_timer_us(-(MICROS_PER_TICK), IRTimerInterruptHandlerHelper, NULL, &s50usTimer);
#define TIMER_DISABLE_RECEIVE_INTR  cancel_repeating_timer(&s50usTimer);

// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() void IRTimerInterruptHandler(void)
void IRTimerInterruptHandler();
// The timer callback has a parameter and a return value
bool IRTimerInterruptHandlerHelper(repeating_timer_t*) {
    IRTimerInterruptHandler();
    return true;
}

void timerConfigForReceive() {
    // no need for initializing timer at setup()
}

#  if defined(SEND_PWM_BY_TIMER)
#include "hardware/pwm.h"

uint sSliceNumberForSendPWM;
uint sChannelNumberForSendPWM;
uint sIROutPuseWidth;

/*
 * If we just disable the PWM, the counter stops and the output stays at the state is currently has
 */
#define ENABLE_SEND_PWM_BY_TIMER        pwm_set_counter(sSliceNumberForSendPWM, 0); pwm_set_chan_level(sSliceNumberForSendPWM, sChannelNumberForSendPWM, sIROutPuseWidth);
#define DISABLE_SEND_PWM_BY_TIMER       pwm_set_chan_level(sSliceNumberForSendPWM, sChannelNumberForSendPWM, 0); // this sets output also to LOW

/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
#    if defined(IR_SEND_PIN)
    gpio_set_function(IR_SEND_PIN, GPIO_FUNC_PWM);
    // Find out which PWM slice is connected to IR_SEND_PIN
    sSliceNumberForSendPWM = pwm_gpio_to_slice_num(IR_SEND_PIN);
    sChannelNumberForSendPWM = pwm_gpio_to_channel(IR_SEND_PIN);
#    else
    gpio_set_function(IrSender.sendPin, GPIO_FUNC_PWM);
    // Find out which PWM slice is connected to IR_SEND_PIN
    sSliceNumberForSendPWM = pwm_gpio_to_slice_num(IrSender.sendPin);
    sChannelNumberForSendPWM = pwm_gpio_to_channel(IrSender.sendPin);
#    endif
    uint16_t tPWMWrapValue = (clock_get_hz(clk_sys)) / (aFrequencyKHz * 1000); // 3289.473 for 38 kHz @125 MHz clock. We have a 16 bit counter and use system clock (125 MHz)

    pwm_config tPWMConfig = pwm_get_default_config();
    pwm_config_set_wrap(&tPWMConfig, tPWMWrapValue - 1);
    pwm_init(sSliceNumberForSendPWM, &tPWMConfig, false); // we do not want to send now
    sIROutPuseWidth = ((tPWMWrapValue * IR_SEND_DUTY_CYCLE_PERCENT) / 100) - 1; // 985.84 for 38 kHz
    pwm_set_chan_level(sSliceNumberForSendPWM, sChannelNumberForSendPWM, 0);
    pwm_set_enabled(sSliceNumberForSendPWM, true);
}
#  endif // defined(SEND_PWM_BY_TIMER)

/***************************************
 * NRF5 boards like the BBC:Micro
 ***************************************/
#elif defined(NRF5) || defined(ARDUINO_ARCH_NRF52840)
#  if defined(SEND_PWM_BY_TIMER)
#error PWM generation by hardware not implemented for NRF5
#  endif

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   NVIC_EnableIRQ(TIMER2_IRQn);
#define TIMER_DISABLE_RECEIVE_INTR  NVIC_DisableIRQ(TIMER2_IRQn);
#  if defined(ISR)
#undef ISR
#  endif
#define ISR(f) void IRTimerInterruptHandler(void)
void IRTimerInterruptHandler();

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;              // Set the timer in Timer Mode
    NRF_TIMER2->TASKS_CLEAR = 1;// clear the task first to be usable for later
    NRF_TIMER2->PRESCALER = 4;// f TIMER = 16 MHz / (2 ^ PRESCALER ) : 4 -> 1 MHz, 1 uS
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;//Set counter to 16 bit resolution
    NRF_TIMER2->CC[0] = MICROS_PER_TICK;//Set value for TIMER2 compare register 0, to trigger every 50 uS
    NRF_TIMER2->CC[1] = 0;//Set value for TIMER2 compare register 1

    // Enable interrupt on Timer 2, for CC[0] compare match events
    NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
    NRF_TIMER2->TASKS_START = 1;// Start TIMER2

    // timerAttachInterrupt(timer, &IRTimerInterruptHandler, 1);
}

/** TIMTER2 peripheral interrupt handler. This interrupt handler is called whenever there it a TIMER2 interrupt
 * Don't mess with this line. really.
 */
extern "C" {
    void TIMER2_IRQHandler(void) {
        // Interrupt Service Routine - Fires every 50uS
        if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) && ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0)) {
            NRF_TIMER2->EVENTS_COMPARE[0] = 0;          //Clear compare register 0 event
            IRTimerInterruptHandler();// call the IR-receive function
            NRF_TIMER2->CC[0] += 50;
        }
    }
}

/**********************************************************************************************************************
 * BluePill in 2 flavors see https://samuelpinches.com.au/3d-printer/cutting-through-some-confusion-on-stm32-and-arduino/
 *
 * Recommended original Arduino_STM32 by Roger Clark.
 * http://dan.drown.org/stm32duino/package_STM32duino_index.json
 * STM32F1 architecture for "Generic STM32F103C series" from "STM32F1 Boards (Arduino_STM32)" of Arduino Board manager
 **********************************************************************************************************************/
#elif defined(__STM32F1__) || defined(ARDUINO_ARCH_STM32F1)
#include <HardwareTimer.h> // 4 timers and 4. timer (4.channel) is used for tone()
#  if defined(SEND_PWM_BY_TIMER)
#error PWM generation by hardware not implemented for STM32
#  endif

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   s50usTimer.resume()
#define TIMER_DISABLE_RECEIVE_INTR  s50usTimer.pause()

// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() void IRTimerInterruptHandler(void)
void IRTimerInterruptHandler();

/*
 * Use timer 3 as IR timer.
 * Timer 3 blocks PA6, PA7, PB0, PB1, so if you require one of them as tone() or Servo output, you must choose another timer.
 */
HardwareTimer s50usTimer(3);

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    s50usTimer.setMode(TIMER_CH1, TIMER_OUTPUT_COMPARE);
    s50usTimer.setPrescaleFactor(1);
    s50usTimer.setOverflow((F_CPU / MICROS_IN_ONE_SECOND) * MICROS_PER_TICK);
    s50usTimer.attachInterrupt(TIMER_CH1, IRTimerInterruptHandler);
    s50usTimer.refresh();
}

/**********************************************************************************************************************
 * STM32duino by ST Microsystems.
 * https://github.com/stm32duino/Arduino_Core_STM32
 * https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json
 * stm32 architecture for "Generic STM32F1 series" from "STM32 Boards (selected from submenu)" of Arduino Board manager
 **********************************************************************************************************************/
#elif defined(STM32F1xx) || defined(ARDUINO_ARCH_STM32)
#include <HardwareTimer.h> // 4 timers and 3. timer is used for tone(), 2. for Servo
#  if defined(SEND_PWM_BY_TIMER)
#error PWM generation by hardware not implemented for STM32
#  endif

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   s50usTimer.resume()
#define TIMER_DISABLE_RECEIVE_INTR  s50usTimer.pause()

// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() void IRTimerInterruptHandler(void)
void IRTimerInterruptHandler();

/*
 * Use timer 4 as IR timer.
 * Timer 4 blocks PB6, PB7, PB8, PB9, so if you need one them as tone() or Servo output, you must choose another timer.
 */
#  if defined(TIM4)
HardwareTimer s50usTimer(TIM4);
#  else
HardwareTimer s50usTimer(TIM2);
#  endif

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    s50usTimer.setOverflow(MICROS_PER_TICK, MICROSEC_FORMAT); // 50 uS
    s50usTimer.attachInterrupt(IRTimerInterruptHandler);
    s50usTimer.resume();
}

/***************************************
 * Particle special IntervalTimer
 * !!!UNTESTED!!!
 ***************************************/
#elif defined(PARTICLE)
#  ifndef __INTERVALTIMER_H__
#include "SparkIntervalTimer.h" // SparkIntervalTimer.h is required if PARTICLE is defined.
#  endif

extern IntervalTimer timer;

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR   timer.begin(IRTimerInterruptHandler, MICROS_PER_TICK, uSec)
#define TIMER_DISABLE_RECEIVE_INTR  timer.end()

// Redefinition of ISR macro which creates a plain function now
#  if defined(ISR)
#undef ISR
#  endif
#define ISR() void IRTimerInterruptHandler(void)

void timerConfigForReceive() {
}

#  if defined(SEND_PWM_BY_TIMER)
#    if defined(IR_SEND_PIN)
#define ENABLE_SEND_PWM_BY_TIMER    analogWrite(IR_SEND_PIN, ((256L * 100) / IR_SEND_DUTY_CYCLE_PERCENT)), ir_out_kHz*1000)
#define DISABLE_SEND_PWM_BY_TIMER   analogWrite(IR_SEND_PIN, 0, ir_out_kHz*1000)
#    else
#define ENABLE_SEND_PWM_BY_TIMER    analogWrite(IrSender.sendPin, ((256L * 100) / IR_SEND_DUTY_CYCLE_PERCENT), ir_out_kHz*1000)
#define DISABLE_SEND_PWM_BY_TIMER   analogWrite(IrSender.sendPin, 0, ir_out_kHz*1000)
#    endif

extern int ir_out_kHz;
/*
 * timerConfigForSend() is used exclusively by IRsend::enableIROut()
 * Set output pin mode and disable receive interrupt if it uses the same resource
 */
void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;
#    if defined(IR_SEND_PIN)
    pinMode(IR_SEND_PIN, OUTPUT);
#    else
    pinMode(IrSender.sendPin, OUTPUT);
#    endif
    ir_out_kHz = aFrequencyKHz;
}
#  endif // defined(SEND_PWM_BY_TIMER)

 /***************************************
  * Unknown CPU board
  ***************************************/
#else
#error Internal code configuration error, no timer functions implemented for this CPU / board
/*
 * Dummy definitions to avoid more irritating compile errors
 */

#define TIMER_RESET_INTR_PENDING
#define TIMER_ENABLE_RECEIVE_INTR
#define TIMER_DISABLE_RECEIVE_INTR

#  if defined(ISR)
#undef ISR
#  endif
#define ISR() void notImplemented(void)



void timerConfigForReceive() {
}

#  if defined(SEND_PWM_BY_TIMER)
#define ENABLE_SEND_PWM_BY_TIMER
#define DISABLE_SEND_PWM_BY_TIMER

void timerConfigForSend(uint8_t aFrequencyKHz) {
    TIMER_DISABLE_RECEIVE_INTR;
#    if defined(IR_SEND_PIN)
    pinMode(IR_SEND_PIN, OUTPUT);
#    else
    pinMode(IrSender.sendPin, OUTPUT);
#    endif
    (void) aFrequencyKHz;
}
#  endif // defined(SEND_PWM_BY_TIMER)

#endif // defined(DOXYGEN / CPU_TYPES)

/** @}*/
/** @}*/
#endif // _IR_TIMER_HPP
