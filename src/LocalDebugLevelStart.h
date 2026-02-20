/*
 * LocalDebugLevelStart.h
 * Include to propagate global debug levels to file local ones and to define appropriate print macros.
 * !!! If used in included (.hpp) files, #include "LocalDebugLevelEnd.h" must be used at end of file to undefine local macros.
 * If, for example, #define LOCAL_TRACE is placed before this include, it will not be propagated. This enables TRACE-level output to be selected only.
 *
 * LOCAL_TRACE   // Information you need to understand details of a function or if you hunt a bug.
 * LOCAL_DEBUG   // Information need to understand the operating of your program. E.g. function calls and values of control variables.
 * LOCAL_INFO    // Information you want to see in regular operation to see what the program is doing. E.g. "Now playing Muppets melody".
 * LOCAL_WARN    // Information that the program may encounter problems, like small Heap/Stack area.
 * LOCAL_ERROR   // Informations to explain why the program will not run. E.g. not enough Ram for all created objects.
 *
 *  Copyright (C) 2024-2026  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  Arduino-Utils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public INFOse for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/////////////// put this before the include /////////////////////////
// This block must be located after the includes of other *.hpp files
//#define LOCAL_INFO  // This enables info output only for this file
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development

/*
 * Propagate debug level to local ones but at first not to each other, i.e. enabling TRACE does not enable DEBUG and INFO
 */
#if defined(TRACE) // Information you need to understand details of a function or if you hunt a bug.
#define LOCAL_TRACE
#  if !defined(DO_NOT_PROPAGATE_DEBUG_LEVELS) // Propagate levels by default i.e. enabling TRACE does enable DEBUG and INFO
#define LOCAL_DEBUG
#define LOCAL_INFO
#  endif
#endif

#if defined(DEBUG) // Information need to understand the operating of your program. E.g. function calls and values of control variables.
#define LOCAL_DEBUG
#  if !defined(DO_NOT_PROPAGATE_DEBUG_LEVELS)
#define LOCAL_INFO
#  endif
#endif

#if defined(INFO) // Information you want to see in regular operation to see what the program is doing. E.g. "START ../src/LightToTone.cpp Version 1.2 from Dec 31 2019" or "Now playing Muppets melody".
#define LOCAL_INFO
#endif

/*
 * Define appropriate print macros
 */
#if defined(LOCAL_TRACE)
#define TRACE_PRINT(...)      Serial.print(__VA_ARGS__)
#define TRACE_PRINTLN(...)    Serial.println(__VA_ARGS__)
#define TRACE_FLUSH()         Serial.flush()
#else
#define TRACE_PRINT(...)      void()
#define TRACE_PRINTLN(...)    void()
#define TRACE_FLUSH()         void()
#endif

#if defined(LOCAL_DEBUG)
#define DEBUG_PRINT(...)      Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...)    Serial.println(__VA_ARGS__)
#define DEBUG_FLUSH()         Serial.flush()
#else
#define DEBUG_PRINT(...)      void()
#define DEBUG_PRINTLN(...)    void()
#define DEBUG_FLUSH()         void()

#endif

#if defined(LOCAL_INFO)
#define INFO_PRINT(...)      Serial.print(__VA_ARGS__)
#define INFO_PRINTLN(...)    Serial.println(__VA_ARGS__)
#define INFO_FLUSH()         Serial.flush()
#else
#define INFO_PRINT(...)      void()
#define INFO_PRINTLN(...)    void()
#define INFO_FLUSH()         void()
#endif

