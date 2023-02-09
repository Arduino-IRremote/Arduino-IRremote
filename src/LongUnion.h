/*
 * LongUnion.h
 *
 *  Copyright (C) 2020-2022  Armin Joachimsmeyer
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
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#if !defined(_WORD_UNION_H) || !defined(_LONG_UNION_H) || !defined(_LONG_LONG_UNION_H)

#include <stdint.h>

#ifndef _WORD_UNION_H
#define _WORD_UNION_H
/**
 * Union to specify parts / manifestations of a 16 bit Word without casts and shifts.
 * It also supports the compiler generating small code.
 */
union WordUnion {
    struct {
        uint8_t LowByte;
        uint8_t HighByte;
    } UByte;
    struct {
        int8_t LowByte;
        int8_t HighByte;
    } Byte;
    uint8_t UBytes[2]; // UBytes[0] is LowByte
    int8_t Bytes[2];
    uint16_t UWord;
    int16_t Word;
    uint8_t *BytePointer;
};
#endif // _WORD_UNION_H

#ifndef _LONG_UNION_H
#define _LONG_UNION_H
/**
 * Union to specify parts / manifestations of a 32 bit Long without casts and shifts.
 * It also supports the compiler generating small code.
 */
union LongUnion {
    struct {
        uint8_t LowByte;
        uint8_t MidLowByte;
        uint8_t MidHighByte;
        uint8_t HighByte;
    } UByte;
    struct {
        int8_t LowByte;
        int8_t MidLowByte;
        int8_t MidHighByte;
        int8_t HighByte;
    } Byte;
    /* Does not work for STM32
    struct {
        uint8_t LowByte;
        uint16_t MidWord;
        uint8_t HighByte;
    } UByteWord;
    */
    struct {
        uint16_t LowWord;
        uint16_t HighWord;
    } UWord;
    struct {
        int16_t LowWord;
        int16_t HighWord;
    } Word;
    struct {
        WordUnion LowWord;
        WordUnion HighWord;
    } WordUnion;
    uint8_t UBytes[4]; // seems to have the same code size as using struct UByte
    int8_t Bytes[4]; // Bytes[0] is LowByte
    uint16_t UWords[2];
    int16_t Words[2];
    uint32_t ULong;
    int32_t Long;
};
#endif // _LONG_UNION_H

#ifndef _LONG_LONG_UNION_H
#define _LONG_LONG_UNION_H
/**
 * Union to specify parts / manifestations of a 64 bit LongLong without casts and shifts.
 * It also supports the compiler generating small code.
 */
union LongLongUnion {
    struct {
        uint16_t LowWord;
        uint16_t MidLowWord;
        uint16_t MidHighWord;
        uint16_t HighWord;
    } UWord;
    struct {
        int16_t LowWord;
        int16_t MidLowWord;
        int16_t MidHighWord;
        int16_t HighWord;
    } Word;
    struct {
        WordUnion LowWord;
        WordUnion MidLowWord;
        WordUnion MidHighWord;
        WordUnion HighWord;
    } WordUnion;
    struct {
        uint32_t LowLong;
        uint32_t HighLong;
    } ULong;
    struct {
        int32_t LowLong;
        int32_t HighLong;
    } Long;
    struct {
        LongUnion LowLong;
        LongUnion HighLong;
    } LongUnion;
    uint8_t UBytes[8]; // seems to have the same code size as using struct UByte
    int8_t Bytes[8];
    uint16_t UWords[4];
    int16_t Words[4];
    uint64_t ULongLong;
    int64_t LongLong;
};
#endif // _LONG_LONG_UNION_H

#endif //  !defined(_WORD_UNION_H) || !defined(_LONG_UNION_H) || !defined(_LONG_LONG_UNION_H)
