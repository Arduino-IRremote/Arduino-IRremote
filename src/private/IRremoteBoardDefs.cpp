/**
 * @file IRremoteBoardDefs.hp
 *
 * @brief All board specific definitions, which can not be static, should be contained in this file.
 * It was previously contained in esp32.cpp etc.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021 pmalasp, Armin Joachimsmeyer
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
#if defined(ESP32)
// Variables specific to the ESP32.
hw_timer_t *timer;
IRAM_ATTR void IRTimer(); // defined in IRremote.cpp, masqueraded as ISR(TIMER_INTR_NAME)

void timerConfigForSend(uint8_t aFrequencyKHz) {
    ledcSetup(LED_CHANNEL, aFrequencyKHz * 1000, 8);  // 8 bit PWM resolution
    ledcAttachPin(IR_SEND_PIN, LED_CHANNEL); // bind pin to channel
}

void timerConfigForReceive() {
    // Interrupt Service Routine - Fires every 50uS
    // ESP32 has a proper API to setup timers, no weird chip macros needed
    // simply call the readable API versions :)
    // 3 timers, choose #1, 80 divider nanosecond precision, 1 to count up
    timer = timerBegin(1, 80, 1);
    timerAttachInterrupt(timer, &IRTimer, 1);
    // every 50ns, autoreload = true
    timerAlarmWrite(timer, 50, true);
}

#elif defined(ARDUINO_ARCH_SAMD)
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
void IRTimer(); // Defined in IRremoteBoardDefs.h as ISR(TIMER_INTR_NAME)

void TC3_Handler(void) {
    TcCount16 *TC = (TcCount16*) TC3;
    // If this interrupt is due to the compare register matching the timer count
    // we toggle the LED.
    if (TC->INTFLAG.bit.MC0 == 1) {
        TC->INTFLAG.bit.MC0 = 1;
        IRTimer();
    }
}

#elif defined(NRF5)  || defined(ARDUINO_ARCH_NRF52840)

void timerConfigForReceive() {
// Interrupt Service Routine - Fires every 50uS
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;              // Set the timer in Timer Mode
    NRF_TIMER2->TASKS_CLEAR = 1;                           // clear the task first to be usable for later
    NRF_TIMER2->PRESCALER = 4;                             // f TIMER = 16 MHz / (2 ^ PRESCALER ) : 4 -> 1 MHz, 1 uS
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;     //Set counter to 16 bit resolution
    NRF_TIMER2->CC[0] = 50;                                //Set value for TIMER2 compare register 0, to trigger every 50 uS
    NRF_TIMER2->CC[1] = 0;                                 //Set value for TIMER2 compare register 1

    // Enable interrupt on Timer 2, for CC[0] compare match events
    NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
    NRF_TIMER2->TASKS_START = 1;               // Start TIMER2

    // timerAttachInterrupt(timer, &IRTimer, 1);
}

void IRTimer(); // Defined in IRremoteBoardDefs.h as ISR(TIMER_INTR_NAME)

void timer_pal(void) {
    if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) && ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0)) {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;          //Clear compare register 0 event
        IRTimer();                                  // call the IR-receive function
        NRF_TIMER2->CC[0] += 50;
    }
}

/** TIMTER2 peripheral interrupt handler. This interrupt handler is called whenever there it a TIMER2 interrupt
 * Don't mess with this line. really.
 */
extern "C" {
void TIMER2_IRQHandler(void) {
    timer_pal();
}
}

#elif defined(ARDUINO_ARCH_STM32)

// Functions specific to the STM32.

#include <HardwareTimer.h>

void IRTimer(); // defined in IRremote.cpp, masqueraded as ISR(TIMER_INTR_NAME)

HardwareTimer hTim(TIM4); // global variable for timer handle

//+=============================================================================
// initialization
//
void timerConfigForReceive() {


    hTim.setOverflow(50, MICROSEC_FORMAT); // 50 uS
    hTim.attachInterrupt(IRTimer);

    hTim.resume();

    // Set pin modes
    pinMode(irparams.recvpin, INPUT);
}



#endif // defined(ESP32)
