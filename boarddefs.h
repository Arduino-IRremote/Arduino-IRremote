//******************************************************************************
// IRremote
// Version 2.0.1 June, 2015
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html

// This file contains all board specific information. It was previously contained within
// IRremoteInt.h

// Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// Whynter A/C ARC-110WD added by Francesco Meschia
//******************************************************************************

#ifndef boarddefs_h
#define boarddefs_h

//------------------------------------------------------------------------------
// Defines for blinking the LED
//

#if defined(CORE_LED0_PIN)
#	define BLINKLED        CORE_LED0_PIN
#	define BLINKLED_ON()   (digitalWrite(CORE_LED0_PIN, HIGH))
#	define BLINKLED_OFF()  (digitalWrite(CORE_LED0_PIN, LOW))

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#	define BLINKLED        13
#	define BLINKLED_ON()   (PORTB |= B10000000)
#	define BLINKLED_OFF()  (PORTB &= B01111111)

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#	define BLINKLED        0
#	define BLINKLED_ON()   (PORTD |= B00000001)
#	define BLINKLED_OFF()  (PORTD &= B11111110)

#else
#	define BLINKLED        13
	#define BLINKLED_ON()  (PORTB |= B00100000)
#	define BLINKLED_OFF()  (PORTB &= B11011111)
#endif

//------------------------------------------------------------------------------
// CPU Frequency
//
#ifdef F_CPU
#	define SYSCLOCK  F_CPU     // main Arduino clock
#else
#	define SYSCLOCK  16000000  // main Arduino clock
#endif

// microseconds per clock interrupt tick
#define USECPERTICK    50

//------------------------------------------------------------------------------
// Define which timer to use
//
// Uncomment the timer you wish to use on your board.
// If you are using another library which uses timer2, you have options to
//   switch IRremote to use a different timer.
//

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

// Teensy 3.0 / Teensy 3.1
#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
	#define IR_USE_TIMER_CMT  // tx = pin 5

// Teensy-LC
#elif defined(__MKL26Z64__)
  #define IR_USE_TIMER_TPM1 // tx = pin 16

// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
	//#define IR_USE_TIMER1   // tx = pin 25
	#define IR_USE_TIMER2     // tx = pin 1
	//#define IR_USE_TIMER3   // tx = pin 16

// MightyCore - ATmega1284
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
	//#define IR_USE_TIMER1   // tx = pin 13
	#define IR_USE_TIMER2     // tx = pin 14
	//#define IR_USE_TIMER3   // tx = pin 6

// MightyCore - ATmega164, ATmega324, ATmega644
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__)
	//#define IR_USE_TIMER1   // tx = pin 13
	#define IR_USE_TIMER2     // tx = pin 14
	
//MegaCore - ATmega64, ATmega128
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
 	#define IR_USE_TIMER1     // tx = pin 13

// MightyCore - ATmega8535, ATmega16, ATmega32
#elif defined(__AVR_ATmega8535__) || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__)
 	#define IR_USE_TIMER1     // tx = pin 13

// Atmega8
#elif defined(__AVR_ATmega8__)
	#define IR_USE_TIMER1     // tx = pin 9

// ATtiny84
#elif defined(__AVR_ATtiny84__)
  #define IR_USE_TIMER1     // tx = pin 6

//ATtiny85
#elif defined(__AVR_ATtiny85__)
  #define IR_USE_TIMER_TINY0   // tx = pin 1

// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, Nano, etc
// ATmega48, ATmega88, ATmega168, ATmega328
#else
	//#define IR_USE_TIMER1   // tx = pin 9
	#define IR_USE_TIMER2     // tx = pin 3

#endif

//------------------------------------------------------------------------------
// Defines for Timer

//---------------------------------------------------------
// Timer2 (8 bits)
//
#if defined(IR_USE_TIMER2)

#define TIMER_RESET
#define TIMER_ENABLE_PWM    (TCCR2A |= _BV(COM2B1))
#define TIMER_DISABLE_PWM   (TCCR2A &= ~(_BV(COM2B1)))
#define TIMER_ENABLE_INTR   (TIMSK2 = _BV(OCIE2A))
#define TIMER_DISABLE_INTR  (TIMSK2 = 0)
#define TIMER_INTR_NAME     TIMER2_COMPA_vect

#define TIMER_CONFIG_KHZ(val) ({ \
	const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
	TCCR2A               = _BV(WGM20); \
	TCCR2B               = _BV(WGM22) | _BV(CS20); \
	OCR2A                = pwmval; \
	OCR2B                = pwmval / 3; \
})

#define TIMER_COUNT_TOP  (SYSCLOCK * USECPERTICK / 1000000)

//-----------------
#if (TIMER_COUNT_TOP < 256)
#	define TIMER_CONFIG_NORMAL() ({ \
		TCCR2A = _BV(WGM21); \
		TCCR2B = _BV(CS20); \
		OCR2A  = TIMER_COUNT_TOP; \
		TCNT2  = 0; \
	})
#else
#	define TIMER_CONFIG_NORMAL() ({ \
		TCCR2A = _BV(WGM21); \
		TCCR2B = _BV(CS21); \
		OCR2A  = TIMER_COUNT_TOP / 8; \
		TCNT2  = 0; \
	})
#endif

//-----------------
#if defined(CORE_OC2B_PIN)
#	define TIMER_PWM_PIN  CORE_OC2B_PIN  // Teensy
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#	define TIMER_PWM_PIN  9              // Arduino Mega
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__)
#	define TIMER_PWM_PIN  14             // MightyCore
#else
#	define TIMER_PWM_PIN  3              // Arduino Duemilanove, Diecimila, LilyPad, etc
#endif					     // ATmega48, ATmega88, ATmega168, ATmega328

//---------------------------------------------------------
// Timer1 (16 bits)
//
#elif defined(IR_USE_TIMER1)

#define TIMER_RESET
#define TIMER_ENABLE_PWM   (TCCR1A |= _BV(COM1A1))
#define TIMER_DISABLE_PWM  (TCCR1A &= ~(_BV(COM1A1)))

//-----------------
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8535__) \
|| defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) \
|| defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
#	define TIMER_ENABLE_INTR   (TIMSK |= _BV(OCIE1A))
#	define TIMER_DISABLE_INTR  (TIMSK &= ~_BV(OCIE1A))
#else
#	define TIMER_ENABLE_INTR   (TIMSK1 = _BV(OCIE1A))
#	define TIMER_DISABLE_INTR  (TIMSK1 = 0)
#endif

//-----------------
#define TIMER_INTR_NAME       TIMER1_COMPA_vect

#define TIMER_CONFIG_KHZ(val) ({ \
	const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
	TCCR1A                = _BV(WGM11); \
	TCCR1B                = _BV(WGM13) | _BV(CS10); \
	ICR1                  = pwmval; \
	OCR1A                 = pwmval / 3; \
})

#define TIMER_CONFIG_NORMAL() ({ \
	TCCR1A = 0; \
	TCCR1B = _BV(WGM12) | _BV(CS10); \
	OCR1A  = SYSCLOCK * USECPERTICK / 1000000; \
	TCNT1  = 0; \
})

//-----------------
#if defined(CORE_OC1A_PIN)
#	define TIMER_PWM_PIN  CORE_OC1A_PIN  // Teensy
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#	define TIMER_PWM_PIN  11             // Arduino Mega
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
#	define TIMER_PWM_PIN  13	     // MegaCore
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) \
|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) \
|| defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324A__) \
|| defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega164A__) \
|| defined(__AVR_ATmega164P__) || defined(__AVR_ATmega32__) \
|| defined(__AVR_ATmega16__) || defined(__AVR_ATmega8535__)
#	define TIMER_PWM_PIN  13             // MightyCore
#elif defined(__AVR_ATtiny84__)
# 	define TIMER_PWM_PIN  6
#else
#	define TIMER_PWM_PIN  9              // Arduino Duemilanove, Diecimila, LilyPad, etc
#endif					     // ATmega48, ATmega88, ATmega168, ATmega328

//---------------------------------------------------------
// Timer3 (16 bits)
//
#elif defined(IR_USE_TIMER3)

#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR3A |= _BV(COM3A1))
#define TIMER_DISABLE_PWM    (TCCR3A &= ~(_BV(COM3A1)))
#define TIMER_ENABLE_INTR    (TIMSK3 = _BV(OCIE3A))
#define TIMER_DISABLE_INTR   (TIMSK3 = 0)
#define TIMER_INTR_NAME      TIMER3_COMPA_vect

#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR3A = _BV(WGM31); \
  TCCR3B = _BV(WGM33) | _BV(CS30); \
  ICR3 = pwmval; \
  OCR3A = pwmval / 3; \
})

#define TIMER_CONFIG_NORMAL() ({ \
  TCCR3A = 0; \
  TCCR3B = _BV(WGM32) | _BV(CS30); \
  OCR3A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT3 = 0; \
})

//-----------------
#if defined(CORE_OC3A_PIN)
#	define TIMER_PWM_PIN  CORE_OC3A_PIN  // Teensy
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#	define TIMER_PWM_PIN  5              // Arduino Mega
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
#	define TIMER_PWM_PIN  6              // MightyCore
#else
#	error "Please add OC3A pin number here\n"
#endif

//---------------------------------------------------------
// Timer4 (10 bits, high speed option)
//
#elif defined(IR_USE_TIMER4_HS)

#define TIMER_RESET
#define TIMER_ENABLE_PWM    (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM   (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR   (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_INTR  (TIMSK4 = 0)
#define TIMER_INTR_NAME     TIMER4_OVF_vect

#define TIMER_CONFIG_KHZ(val) ({ \
	const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
	TCCR4A                = (1<<PWM4A); \
	TCCR4B                = _BV(CS40); \
	TCCR4C                = 0; \
	TCCR4D                = (1<<WGM40); \
	TCCR4E                = 0; \
	TC4H                  = pwmval >> 8; \
	OCR4C                 = pwmval; \
	TC4H                  = (pwmval / 3) >> 8; \
	OCR4A                 = (pwmval / 3) & 255; \
})

#define TIMER_CONFIG_NORMAL() ({ \
	TCCR4A = 0; \
	TCCR4B = _BV(CS40); \
	TCCR4C = 0; \
	TCCR4D = 0; \
	TCCR4E = 0; \
	TC4H   = (SYSCLOCK * USECPERTICK / 1000000) >> 8; \
	OCR4C  = (SYSCLOCK * USECPERTICK / 1000000) & 255; \
	TC4H   = 0; \
	TCNT4  = 0; \
})

//-----------------
#if defined(CORE_OC4A_PIN)
#	define TIMER_PWM_PIN  CORE_OC4A_PIN  // Teensy
#elif defined(__AVR_ATmega32U4__)
#	define TIMER_PWM_PIN  13             // Leonardo
#else
#	error "Please add OC4A pin number here\n"
#endif

//---------------------------------------------------------
// Timer4 (16 bits)
//
#elif defined(IR_USE_TIMER4)

#define TIMER_RESET
#define TIMER_ENABLE_PWM    (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM   (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR   (TIMSK4 = _BV(OCIE4A))
#define TIMER_DISABLE_INTR  (TIMSK4 = 0)
#define TIMER_INTR_NAME     TIMER4_COMPA_vect

#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR4A = _BV(WGM41); \
  TCCR4B = _BV(WGM43) | _BV(CS40); \
  ICR4 = pwmval; \
  OCR4A = pwmval / 3; \
})

#define TIMER_CONFIG_NORMAL() ({ \
  TCCR4A = 0; \
  TCCR4B = _BV(WGM42) | _BV(CS40); \
  OCR4A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT4 = 0; \
})

//-----------------
#if defined(CORE_OC4A_PIN)
#	define TIMER_PWM_PIN  CORE_OC4A_PIN
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#	define TIMER_PWM_PIN  6  // Arduino Mega
#else
#	error "Please add OC4A pin number here\n"
#endif

//---------------------------------------------------------
// Timer5 (16 bits)
//
#elif defined(IR_USE_TIMER5)

#define TIMER_RESET
#define TIMER_ENABLE_PWM    (TCCR5A |= _BV(COM5A1))
#define TIMER_DISABLE_PWM   (TCCR5A &= ~(_BV(COM5A1)))
#define TIMER_ENABLE_INTR   (TIMSK5 = _BV(OCIE5A))
#define TIMER_DISABLE_INTR  (TIMSK5 = 0)
#define TIMER_INTR_NAME     TIMER5_COMPA_vect

#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR5A = _BV(WGM51); \
  TCCR5B = _BV(WGM53) | _BV(CS50); \
  ICR5 = pwmval; \
  OCR5A = pwmval / 3; \
})

#define TIMER_CONFIG_NORMAL() ({ \
  TCCR5A = 0; \
  TCCR5B = _BV(WGM52) | _BV(CS50); \
  OCR5A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT5 = 0; \
})

//-----------------
#if defined(CORE_OC5A_PIN)
#	define TIMER_PWM_PIN  CORE_OC5A_PIN
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#	define TIMER_PWM_PIN  46  // Arduino Mega
#else
#	error "Please add OC5A pin number here\n"
#endif

//---------------------------------------------------------
// Special carrier modulator timer
//
#elif defined(IR_USE_TIMER_CMT)

#define TIMER_RESET ({     \
	uint8_t tmp __attribute__((unused)) = CMT_MSC; \
	CMT_CMD2 = 30;         \
})

#define TIMER_ENABLE_PWM  do {                                         \
	CORE_PIN5_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE | PORT_PCR_SRE;  \
} while(0)

#define TIMER_DISABLE_PWM  do {                                        \
	CORE_PIN5_CONFIG = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;  \
} while(0)

#define TIMER_ENABLE_INTR   NVIC_ENABLE_IRQ(IRQ_CMT)
#define TIMER_DISABLE_INTR  NVIC_DISABLE_IRQ(IRQ_CMT)
#define TIMER_INTR_NAME     cmt_isr

//-----------------
#ifdef ISR
#	undef ISR
#endif
#define  ISR(f)  void f(void)

//-----------------
#define CMT_PPS_DIV  ((F_BUS + 7999999) / 8000000)
#if F_BUS < 8000000
#error IRremote requires at least 8 MHz on Teensy 3.x
#endif

//-----------------
#define TIMER_CONFIG_KHZ(val) ({ 	 \
	SIM_SCGC4 |= SIM_SCGC4_CMT;      \
	SIM_SOPT2 |= SIM_SOPT2_PTD7PAD;  \
	CMT_PPS    = CMT_PPS_DIV - 1;    \
	CMT_CGH1   = ((F_BUS / CMT_PPS_DIV / 3000) + ((val)/2)) / (val); \
	CMT_CGL1   = ((F_BUS / CMT_PPS_DIV / 1500) + ((val)/2)) / (val); \
	CMT_CMD1   = 0;                  \
	CMT_CMD2   = 30;                 \
	CMT_CMD3   = 0;                  \
	CMT_CMD4   = 0;                  \
	CMT_OC     = 0x60;               \
	CMT_MSC    = 0x01;               \
})

#define TIMER_CONFIG_NORMAL() ({  \
	SIM_SCGC4 |= SIM_SCGC4_CMT;   \
	CMT_PPS    = CMT_PPS_DIV - 1; \
	CMT_CGH1   = 1;               \
	CMT_CGL1   = 1;               \
	CMT_CMD1   = 0;               \
	CMT_CMD2   = 30;              \
	CMT_CMD3   = 0;               \
	CMT_CMD4   = (F_BUS / 160000 + CMT_PPS_DIV / 2) / CMT_PPS_DIV - 31; \
	CMT_OC     = 0;               \
	CMT_MSC    = 0x03;            \
})

#define TIMER_PWM_PIN  5

// defines for TPM1 timer on Teensy-LC
#elif defined(IR_USE_TIMER_TPM1)
#define TIMER_RESET          FTM1_SC |= FTM_SC_TOF;
#define TIMER_ENABLE_PWM     CORE_PIN16_CONFIG = PORT_PCR_MUX(3)|PORT_PCR_DSE|PORT_PCR_SRE
#define TIMER_DISABLE_PWM    CORE_PIN16_CONFIG = PORT_PCR_MUX(1)|PORT_PCR_SRE
#define TIMER_ENABLE_INTR    NVIC_ENABLE_IRQ(IRQ_FTM1)
#define TIMER_DISABLE_INTR   NVIC_DISABLE_IRQ(IRQ_FTM1)
#define TIMER_INTR_NAME      ftm1_isr
#ifdef ISR
#undef ISR
#endif
#define ISR(f) void f(void)
#define TIMER_CONFIG_KHZ(val) ({                     \
	SIM_SCGC6 |= SIM_SCGC6_TPM1;                 \
	FTM1_SC = 0;                                 \
	FTM1_CNT = 0;                                \
	FTM1_MOD = (F_PLL/2000) / val - 1;           \
	FTM1_C0V = (F_PLL/6000) / val - 1;           \
	FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);     \
})
#define TIMER_CONFIG_NORMAL() ({                     \
	SIM_SCGC6 |= SIM_SCGC6_TPM1;                 \
	FTM1_SC = 0;                                 \
	FTM1_CNT = 0;                                \
	FTM1_MOD = (F_PLL/40000) - 1;                \
	FTM1_C0V = 0;                                \
	FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(0) | FTM_SC_TOF | FTM_SC_TOIE; \
})
#define TIMER_PWM_PIN        16

// defines for timer_tiny0 (8 bits)
#elif defined(IR_USE_TIMER_TINY0)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR0A |= _BV(COM0B1))
#define TIMER_DISABLE_PWM    (TCCR0A &= ~(_BV(COM0B1)))
#define TIMER_ENABLE_INTR    (TIMSK |= _BV(OCIE0A))
#define TIMER_DISABLE_INTR   (TIMSK &= ~(_BV(OCIE0A)))
#define TIMER_INTR_NAME      TIMER0_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR0A = _BV(WGM00); \
  TCCR0B = _BV(WGM02) | _BV(CS00); \
  OCR0A = pwmval; \
  OCR0B = pwmval / 3; \
})
#define TIMER_COUNT_TOP      (SYSCLOCK * USECPERTICK / 1000000)
#if (TIMER_COUNT_TOP < 256)
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR0A = _BV(WGM01); \
  TCCR0B = _BV(CS00); \
  OCR0A = TIMER_COUNT_TOP; \
  TCNT0 = 0; \
})
#else
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR0A = _BV(WGM01); \
  TCCR0B = _BV(CS01); \
  OCR0A = TIMER_COUNT_TOP / 8; \
  TCNT0 = 0; \
})
#endif

#define TIMER_PWM_PIN        1  /* ATtiny85 */

//---------------------------------------------------------
// Unknown Timer
//
#else
#	error "Internal code configuration error, no known IR_USE_TIMER# defined\n"
#endif

#endif // ! boarddefs_h
