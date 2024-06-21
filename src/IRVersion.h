/**
 * @file IRversion.hpp
 *
 * This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2015-2024 Ken Shirriff http://www.righto.com, Rafi Khan, Armin Joachimsmeyer
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
 *
 * For Ken Shiriffs original blog entry, see http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html
 * Initially influenced by:
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * and http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#ifndef _IR_VERSION_HPP
#define _IR_VERSION_HPP

#define VERSION_IRREMOTE "4.4.0"
#define VERSION_IRREMOTE_MAJOR 4
#define VERSION_IRREMOTE_MINOR 4
#define VERSION_IRREMOTE_PATCH 0

/*
 * Macro to convert 3 version parts into an integer
 * To be used in preprocessor comparisons, such as #if VERSION_IRREMOTE_HEX >= VERSION_HEX_VALUE(3, 7, 0)
 */
#define VERSION_HEX_VALUE(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define VERSION_IRREMOTE_HEX  VERSION_HEX_VALUE(VERSION_IRREMOTE_MAJOR, VERSION_IRREMOTE_MINOR, VERSION_IRREMOTE_PATCH)

#endif // _IR_VERSION_HPP
