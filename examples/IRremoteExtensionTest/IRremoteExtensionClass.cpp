/*
 * IRremoteExtensionClass.cpp
 *
 * Example for a class which itself uses the IRrecv class from IRremote
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021-2025 Armin Joachimsmeyer
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
#include <Arduino.h>

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc. Sets FLASHEND and RAMSIZE and evaluates value of SEND_PWM_BY_TIMER.

/*
 * !!! The value of RAW_BUFFER_LENGTH (and some other macros) must be the same in all compile units !!!
 * Otherwise you may get warnings like "type 'struct IRData' itself violates the C++ One Definition Rule"
 */
#if !defined(RAW_BUFFER_LENGTH)
// Use more than the default values of 100 for 512 bytes RAM, 200 for 2k RAM and 750 for more than 2k RAM
#  if RAMSIZE <= 0x400
// Here we have 1 k RAM or less
#define RAW_BUFFER_LENGTH  360
#  else
// Here we most likely have 2 k RAM or more
#define RAW_BUFFER_LENGTH  750
#  endif
#endif

#include "IRremoteExtensionClass.h"

IRExtensionClass::IRExtensionClass(IRrecv *aIrReceiver) {
    MyIrReceiver = aIrReceiver;
}
bool IRExtensionClass::decode() {
    return MyIrReceiver->decode();
}

bool IRExtensionClass::printIRResultShort(Print *aSerial, bool aCheckForRecordGapsMicros) {
    return MyIrReceiver->printIRResultShort(aSerial, aCheckForRecordGapsMicros);
}

void IRExtensionClass::resume() {
    Serial.println(F("Call resume()"));
    MyIrReceiver->resume();
}
