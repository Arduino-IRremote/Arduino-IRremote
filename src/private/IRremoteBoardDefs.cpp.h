/**
 * @file IRremoteBoardDefs.cpp.h
 *
 * @brief All board specific definitions, which can not be static, should be contained in this file.
 * It was previously contained in esp32.cpp etc.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021 Armin Joachimsmeyer
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
#include "IRremoteInt.h"
/***************************************
 * ESP32 boards
 ***************************************/
#if defined(ESP32)
// Variables specific to the ESP32.
hw_timer_t *timer;
IRAM_ATTR void IRTimerInterruptHandler(); // defined in IRremote.cpp, masqueraded as ISR(TIMER_INTR_NAME)

void timerConfigForSend(uint8_t aFrequencyKHz) {
    ledcSetup(LED_CHANNEL, aFrequencyKHz * 1000, 8);  // 8 bit PWM resolution
    ledcAttachPin(IrSender.sendPin, LED_CHANNEL); // bind pin to channel
}

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    // ESP32 has a proper API to setup timers, no weird chip macros needed
    // simply call the readable API versions :)
    // 3 timers, choose #1, 80 divider for microsecond precision @80MHz clock, count_up = true
    timer = timerBegin(1, 80, true);
    timerAttachInterrupt(timer, &IRTimerInterruptHandler, 1);
    // every 50 us, autoreload = true
    timerAlarmWrite(timer, MICROS_PER_TICK, true);
}

/***************************************
 * SAMD boards like DUE and Zero
 ***************************************/
#elif defined(ARDUINO_ARCH_SAMD)
#  if defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM)
#error PWM generation by hardware not implemented for SAMD
#  endif
// use timer 3 hard coded here

// functions based on setup from GitHub jdneo/timerInterrupt.ino
void setTimerFrequency(unsigned int aFrequencyHz) {
    int compareValue = (SYSCLOCK / (TIMER_PRESCALER_DIV * aFrequencyHz)) - 1;
    //Serial.println(compareValue);
    TcCount16 *TC = (TcCount16*) TC3;
    // Make sure the count is in a proportional position to where it was
    // to prevent any jitter or disconnect when changing the compare value.
    TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
    TC->CC[0].reg = compareValue;
    //Serial.print("COUNT.reg ");
    //Serial.println(TC->COUNT.reg);
    //Serial.print("CC[0].reg ");
    //Serial.println(TC->CC[0].reg);
    while (TC->STATUS.bit.SYNCBUSY == 1) {
    }
}

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
    while (GCLK->STATUS.bit.SYNCBUSY == 1) {
    }

    TcCount16 *TC = (TcCount16*) TC3;

    // The TC should be disabled before the TC is reset in order to avoid undefined behavior.
    TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    // When write-synchronization is ongoing for a register, any subsequent write attempts to this register will be discarded, and an error will be reported.
    while (TC->STATUS.bit.SYNCBUSY == 1) {
    } // wait for sync
      // Reset TCx
    TC->CTRLA.reg = TC_CTRLA_SWRST;
    // When writing a ‘1’ to the CTRLA.SWRST bit it will immediately read as ‘1’.
    // CTRL.SWRST will be cleared by hardware when the peripheral has been reset.
    while (TC->CTRLA.bit.SWRST) {
    }

    // Use the 16-bit timer
    // Use match mode so that the timer counter resets when the count matches the compare register
    // Set prescaler to 64
    TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV64 | TC_CTRLA_ENABLE;

    setTimerFrequency(1000000 / MICROS_PER_TICK);

    // Enable the compare interrupt
    TC->INTENSET.reg = 0;
    TC->INTENSET.bit.MC0 = 1;
}
// ATSAMD Timer IRQ functions
void IRTimerInterruptHandler(); // Defined in IRremoteBoardDefs.h as ISR(TIMER_INTR_NAME)

void TC3_Handler(void) {
    TcCount16 *TC = (TcCount16*) TC3;
    // If this interrupt is due to the compare register matching the timer count
    // we toggle the LED.
    if (TC->INTFLAG.bit.MC0 == 1) {
        TC->INTFLAG.bit.MC0 = 1;
        IRTimerInterruptHandler();
    }
}

/***************************************
 * NRF5 boards like the BBC:Micro
 ***************************************/
#elif defined(NRF5) || defined(ARDUINO_ARCH_NRF52840)
#  if defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM)
#error PWM generation by hardware not implemented for NRF5
#  endif

/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;              // Set the timer in Timer Mode
    NRF_TIMER2->TASKS_CLEAR = 1;                           // clear the task first to be usable for later
    NRF_TIMER2->PRESCALER = 4;                             // f TIMER = 16 MHz / (2 ^ PRESCALER ) : 4 -> 1 MHz, 1 uS
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;     //Set counter to 16 bit resolution
    NRF_TIMER2->CC[0] = MICROS_PER_TICK;                                //Set value for TIMER2 compare register 0, to trigger every 50 uS
    NRF_TIMER2->CC[1] = 0;                                 //Set value for TIMER2 compare register 1

    // Enable interrupt on Timer 2, for CC[0] compare match events
    NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
    NRF_TIMER2->TASKS_START = 1;               // Start TIMER2

    // timerAttachInterrupt(timer, &IRTimerInterruptHandler, 1);
}

void IRTimerInterruptHandler(); // Defined in IRremoteBoardDefs.h as ISR(TIMER_INTR_NAME)

/** TIMTER2 peripheral interrupt handler. This interrupt handler is called whenever there it a TIMER2 interrupt
 * Don't mess with this line. really.
 */
extern "C" {
void TIMER2_IRQHandler(void) {
    // Interrupt Service Routine - Fires every 50uS
    if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) && ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0)) {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;          //Clear compare register 0 event
        IRTimerInterruptHandler();                                  // call the IR-receive function
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
#  if defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM)
#error PWM generation by hardware not implemented for STM32
#  endif
/*
 * Use timer 3 as IRMP timer.
 * Timer 3 blocks PA6, PA7, PB0, PB1, so if you need one them as tone() or Servo output, you must choose another timer.
 */
HardwareTimer sSTM32Timer(3);
void IRTimerInterruptHandler(); // Defined in IRremoteBoardDefs.h as ISR(TIMER_INTR_NAME)
/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    sSTM32Timer.setMode(TIMER_CH1, TIMER_OUTPUT_COMPARE);
    sSTM32Timer.setPrescaleFactor(1);
    sSTM32Timer.setOverflow((F_CPU / 1000000) * MICROS_PER_TICK);
    sSTM32Timer.attachInterrupt(TIMER_CH1, IRTimerInterruptHandler);
    sSTM32Timer.refresh();
}

/**********************************************************************************************************************
 * STM32duino by ST Microsystems.
 * https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json
 * stm32 architecture for "Generic STM32F1 series" from "STM32 Boards (selected from submenu)" of Arduino Board manager
 **********************************************************************************************************************/
#elif defined(STM32F1xx) || defined(ARDUINO_ARCH_STM32)
#include <HardwareTimer.h> // 4 timers and 3. timer is used for tone(), 2. for Servo
#  if defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM)
#error PWM generation by hardware not implemented for STM32
#  endif
/*
 * Use timer 4 as IRMP timer.
 * Timer 4 blocks PB6, PB7, PB8, PB9, so if you need one them as Servo output, you must choose another timer.
 */
#  if defined(TIM4)
HardwareTimer sSTM32Timer(TIM4);
#  else
HardwareTimer sSTM32Timer(TIM2);
#  endif
void IRTimerInterruptHandler(); // Defined in IRremoteBoardDefs.h as ISR(TIMER_INTR_NAME)
/*
 * Set timer for interrupts every MICROS_PER_TICK (50 us)
 */
void timerConfigForReceive() {
    sSTM32Timer.setOverflow(MICROS_PER_TICK, MICROSEC_FORMAT); // 50 uS
    sSTM32Timer.attachInterrupt(IRTimerInterruptHandler);
    sSTM32Timer.resume();
}

#endif // defined(ESP32)
