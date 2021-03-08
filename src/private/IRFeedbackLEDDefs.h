/**
 * @file IRFeedbackLEDDefs.h
 *
 * @brief All feedback LED definitions are contained in this file.
 */

#ifndef IRFeedbackLEDDefs_h
#define IRFeedbackLEDDefs_h

struct FeedbackLEDControlStruct {
    uint8_t FeedbackLEDPin;         ///< if 0, then take board specific FEEDBACK_LED_ON() and FEEDBACK_LED_OFF() functions
    bool LedFeedbackEnabled;        ///< true -> enable blinking of pin on IR processing
};

/*
 * The feedback LED control instance
 */
struct FeedbackLEDControlStruct FeedbackLEDControl;

#ifdef DOXYGEN
/**
 * If defined, denotes pin number of LED that should be blinked during IR reception.
 * Leave undefined to disable blinking.
 */
#define FEEDBACK_LED        LED_BUILTIN

/**
 * Board dependent macro to turn FEEDBACK_LED on.
 */
#define FEEDBACK_LED_ON()   digitalWrite(FEEDBACK_LED, HIGH)

/**
 * Board dependent macro to turn FEEDBACK_LED off.
 */
#define FEEDBACK_LED_OFF()  digitalWrite(FEEDBACK_LED, LOW)

// Sparkfun Pro Micro is __AVR_ATmega32U4__ but has different external circuit
#elif defined(ARDUINO_AVR_PROMICRO)
// We have no built in LED -> reuse RX LED
#define FEEDBACK_LED        LED_BUILTIN_RX
#define FEEDBACK_LED_ON()   RXLED1
#define FEEDBACK_LED_OFF()  RXLED0

// Arduino Leonardo + others
#elif defined(__AVR_ATmega32U4__)
#define FEEDBACK_LED        LED_BUILTIN
#define FEEDBACK_LED_ON()   (PORTC |= B10000000)
#define FEEDBACK_LED_OFF()  (PORTC &= B01111111)

#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__)  || defined(__AVR_ATmega32U2__)
#define FEEDBACK_LED        LED_BUILTIN
#define FEEDBACK_LED_ON()   (digitalWrite(LED_BUILTIN, HIGH))
#define FEEDBACK_LED_OFF()  (digitalWrite(LED_BUILTIN, LOW))

// Arduino Uno, Nano etc
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega168__)
#define FEEDBACK_LED        LED_BUILTIN
#define FEEDBACK_LED_ON()   (PORTB |= B00100000)
#define FEEDBACK_LED_OFF()  (PORTB &= B11011111)

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define FEEDBACK_LED        13
#define FEEDBACK_LED_ON()   (PORTB |= B10000000)
#define FEEDBACK_LED_OFF()  (PORTB &= B01111111)

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#define FEEDBACK_LED        0
#define FEEDBACK_LED_ON()   (PORTD |= B00000001)
#define FEEDBACK_LED_OFF()  (PORTD &= B11111110)

// TinyCore boards
#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
// No LED available on the board, take LED_BUILTIN which is also the DAC output
#define FEEDBACK_LED        LED_BUILTIN // PA6
#define FEEDBACK_LED_ON()   (PORTC.OUTSET = _BV(6))
#define FEEDBACK_LED_OFF()  (PORTC.OUTCLR = _BV(6))

#elif defined(ESP32)
// No system LED on ESP32, disable blinking by NOT defining FEEDBACK_LED

#elif defined(PARTICLE)
#define FEEDBACK_LED       D7
#define FEEDBACK_LED_ON()  digitalWrite(FEEDBACK_LED,1)
#define FEEDBACK_LED_OFF() digitalWrite(FEEDBACK_LED,0)

// Arduino Zero and BluePill have an LED which is active low
#elif defined(__STM32F1__) || defined(STM32F1xx)
#  if defined(LED_BUILTIN)
#     if !defined(FEEDBACK_LED)
#define FEEDBACK_LED        LED_BUILTIN
#    endif
#define FEEDBACK_LED_ON()   digitalWrite(FEEDBACK_LED, LOW)
#define FEEDBACK_LED_OFF()  digitalWrite(FEEDBACK_LED, HIGH)
#  endif
/*
 * These are the boards for which the default case was verified and the warning below is suppressed
 */
// Nano Every, Uno WiFi Rev2, nRF5 BBC MicroBit, Nano33_BLE
// Arduino Zero
#elif !(defined(__AVR_ATmega4809__) || defined(NRF5) || defined(ARDUINO_ARCH_NRF52840) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) \
    || defined(ARDUINO_ARCH_SAMD) \
    || defined(CORE_LED0_PIN) \
    || defined(ARDUINO_ARCH_STM32F1) \
    || defined(ARDUINO_ARCH_STM32) \
    )
/*
 * print a warning
 */
#warning No definition for default feedback LED found. Check private/IRFeedbackLEDDefs.h.

#else
/*
 * Default case
 */
#  ifdef LED_BUILTIN
#     if !defined(FEEDBACK_LED)
#define FEEDBACK_LED        LED_BUILTIN
#    endif
#define FEEDBACK_LED_ON()   digitalWrite(FEEDBACK_LED, HIGH)
#define FEEDBACK_LED_OFF()  digitalWrite(FEEDBACK_LED, LOW)
#  endif
#endif

#endif // ! IRFeedbackLEDDefs_h

#pragma once

