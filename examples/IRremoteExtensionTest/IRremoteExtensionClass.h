/*
 * IRremoteExtensionClass.h
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

#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

class IRExtensionClass
{
public:
    IRrecv * MyIrReceiver;
    IRExtensionClass(IRrecv * aIrReceiver);
    bool decode();
    bool printIRResultShort(Print *aSerial, bool aPrintRepeatGap = true, bool aCheckForRecordGapsMicros = true);
    void resume();
};

