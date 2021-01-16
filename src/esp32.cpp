/*
 * esp32.cpp
 *
 *  Contains functions for ESP32 CPU's
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 *************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2021 Rafi Khan, Armin Joachimsmeyer
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
#ifdef ESP32

// This file contains functions specific to the ESP32.

#include "IRremote.h"

// "Idiot check"
#ifdef USE_DEFAULT_ENABLE_IR_IN
#error Must undef USE_DEFAULT_ENABLE_IR_IN
#endif

hw_timer_t *timer;
IRAM_ATTR void IRTimer(); // defined in IRremote.cpp, masqueraded as ISR(TIMER_INTR_NAME)

//+=============================================================================
// initialization
//
void IRrecv::enableIRIn() {
// Interrupt Service Routine - Fires every 50uS
    // ESP32 has a proper API to setup timers, no weird chip macros needed
    // simply call the readable API versions :)
    // 3 timers, choose #1, 80 divider nanosecond precision, 1 to count up
    timer = timerBegin(1, 80, 1);
    timerAttachInterrupt(timer, &IRTimer, 1);
    // every 50ns, autoreload = true
    timerAlarmWrite(timer, 50, true);
    timerAlarmEnable(timer);

    // Initialize state machine variables
    resume();

    // Set pin modes
    pinMode(irparams.recvpin, INPUT);
}

void IRrecv::disableIRIn() {
    timerEnd(timer);
    timerDetachInterrupt(timer);
}

void IRsend::enableIROut(int khz) {
    ledcSetup(LED_CHANNEL, khz * 1000, 8);  // 8 bit PWM resolution
    ledcAttachPin(IR_SEND_PIN, LED_CHANNEL); // bind pin to channel
}

#endif // ESP32
