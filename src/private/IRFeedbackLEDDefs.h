/**
 * @file IRFeedbackLEDDefs.h
 *
 * @brief All feedback LED definitions are contained in this file.
 */

#ifndef IRFeedbackLEDDefs_h
#define IRFeedbackLEDDefs_h

/** \addtogroup HardwareDependencies CPU / board dependent definitions
 * @{
 */
/** @addtogroup FeedbackLEDHardware Definitions of FEEDBACK_LED_ON and FEEDBACK_LED_OFF for the different CPU / boards
 * @{
 */

#ifdef DOXYGEN
/**
 * Board dependent macro to turn LED_BUILTIN on.
 */
#define FEEDBACK_LED_ON()   digitalWrite(LED_BUILTIN, HIGH)

/**
 * Board dependent macro to turn LED_BUILTIN off.
 */
#define FEEDBACK_LED_OFF()  digitalWrite(LED_BUILTIN, LOW)

// Sparkfun Pro Micro is __AVR_ATmega32U4__ but has different external circuit
#elif defined(ARDUINO_AVR_PROMICRO)
// We have no built in LED at pin 13 -> reuse RX LED
#undef LED_BUILTIN
#define LED_BUILTIN        LED_BUILTIN_RX
#define FEEDBACK_LED_ON()   RXLED1
#define FEEDBACK_LED_OFF()  RXLED0

// Arduino Leonardo + others
#elif defined(__AVR_ATmega32U4__)
#define FEEDBACK_LED_ON()   (PORTC |= B10000000)
#define FEEDBACK_LED_OFF()  (PORTC &= B01111111)

// Arduino Uno, Nano etc
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega168__)
#define FEEDBACK_LED_ON()   (PORTB |= B00100000)
#define FEEDBACK_LED_OFF()  (PORTB &= B11011111)

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define LED_BUILTIN        13
#define FEEDBACK_LED_ON()   (PORTB |= B10000000)
#define FEEDBACK_LED_OFF()  (PORTB &= B01111111)

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATtiny88__)
#define LED_BUILTIN        0
#define FEEDBACK_LED_ON()   (PORTD |= B00000001)
#define FEEDBACK_LED_OFF()  (PORTD &= B11111110)

// TinyCore boards
#elif defined(__AVR_ATtiny1616__)  || defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny3217__)
// No LED available on the board, take LED_BUILTIN which is also the DAC output
#define FEEDBACK_LED_ON()   (PORTC.OUTSET = _BV(6))
#define FEEDBACK_LED_OFF()  (PORTC.OUTCLR = _BV(6))

#elif defined(PARTICLE)
#define LED_BUILTIN       D7
#define FEEDBACK_LED_ON()  digitalWrite(LED_BUILTIN, 1)
#define FEEDBACK_LED_OFF() digitalWrite(LED_BUILTIN, 0)

// Arduino Zero and BluePill and ESP8266 have an LED which is active low
#elif defined(__STM32F1__) || defined(STM32F1xx) || defined(ESP8266)
#define FEEDBACK_LED_ON()   digitalWrite(LED_BUILTIN, LOW)
#define FEEDBACK_LED_OFF()  digitalWrite(LED_BUILTIN, HIGH)

#else
/*
 * Default case suitable for most boards
 */
#  if defined(LED_BUILTIN)
#define FEEDBACK_LED_ON()   digitalWrite(LED_BUILTIN, HIGH)
#define FEEDBACK_LED_OFF()  digitalWrite(LED_BUILTIN, LOW)
#  else
/*
 * print a warning
 */
#warning No definition for LED_BUILTIN for default feedback LED found. Check private/IRFeedbackLEDDefs.h.
#  endif
#endif

/** @}*/
/** @}*/

#endif // ! IRFeedbackLEDDefs_h

#pragma once

