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
 * Copyright (c) 2021-2024 Armin Joachimsmeyer
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

/*
 * !!! The value of RAW_BUFFER_LENGTH (and some other macros) must be the same in all compile units !!!
 * Otherwise you may get warnings like "type 'struct IRData' itself violates the C++ One Definition Rule"
 */
#if !defined(RAW_BUFFER_LENGTH)
// For air condition remotes it requires 750. Default is 200.
#  if (defined(RAMEND) && RAMEND <= 0x4FF) || (defined(RAMSIZE) && RAMSIZE < 0x4FF)
#define RAW_BUFFER_LENGTH  360
#  elif (defined(RAMEND) && RAMEND <= 0x8FF) || (defined(RAMSIZE) && RAMSIZE < 0x8FF)
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

bool IRExtensionClass::printIRResultShort(Print *aSerial, bool aPrintRepeatGap, bool aCheckForRecordGapsMicros) {
    return MyIrReceiver->printIRResultShort(aSerial, aPrintRepeatGap, aCheckForRecordGapsMicros);
}

void IRExtensionClass::resume() {
    Serial.println(F("Call resume()"));
    MyIrReceiver->resume();
}
