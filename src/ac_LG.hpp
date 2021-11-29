/*
 * ac_LG.hpp
 *
 *  Contains functions for receiving and sending LG air conditioner IR Protocol
 *  There is no state plausibility check, e.g. you can send fan speed in Mode D and change temperature in mode F
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
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
#ifndef AC_LG_HPP
#define AC_LG_HPP
#include <Arduino.h>

//#define INFO // save program space and suppress info output from the LG-AC driver.
//#define DEBUG // for more output from the LG-AC driver.
#include "IRremoteInt.h"
#include "ac_LG.h" // useful constants
#include "LongUnion.h"

/** \addtogroup Airconditoners Air conditioner special code
 * @{
 */
/*
 * LG remote measurements: Type AKB73315611, Ver1.1 from 2011.03.01
 * Internal crystal: 4 MHz
 * Header:  8.9 ms mark 4.15 ms space
 * Data:    500 / 540 and 500 / 1580;
 * Clock is nor synchronized with gate so you have 19 and sometimes 19 and a spike pulses for mark
 * Duty:    9 us on 17 us off => around 33 % duty
 * NO REPEAT: If value like temperature has changed during long press, the last value is send at button release
 * If you do a double press -tested with the fan button-, the next value can be sent after 118 ms
 */
#define SIZE_OF_FAN_SPEED_MAPPING_TABLE     4
const int AC_FAN_TOWER[SIZE_OF_FAN_SPEED_MAPPING_TABLE] = { 0, 4, 6, 6 }; // last dummy entry to avoid out of bounds access
const int AC_FAN_WALL[SIZE_OF_FAN_SPEED_MAPPING_TABLE] = { 0, 2, 4, 5 }; // 0 -> low, 4 high, 5 -> cycle

void Aircondition_LG::setType(bool aIsWallType) {
    ACIsWallType = aIsWallType;
    IR_INFO_PRINT(F("Set wall type to "));
    IR_INFO_PRINTLN(aIsWallType);
}

void Aircondition_LG::printMenu(Print *aSerial) {
    aSerial->println();
    aSerial->println();
    aSerial->println(F("Type command and optional parameter without a separator"));
    aSerial->println(F("0 Off"));
    aSerial->println(F("1 On"));
    aSerial->println(F("s Swing <0 or 1>"));
    aSerial->println(F("a Auto clean <0 or 1>"));
    aSerial->println(F("j Jet on"));
    aSerial->println(F("e Energy saving <0 or 1>"));
    aSerial->println(F("l Lights toggle"));
    aSerial->println(F("f Fan speed <0 to 2 or 3 for cycle>"));
    aSerial->println(F("t Temperature <18 to 30> degree"));
    aSerial->println(F("+ Temperature + 1"));
    aSerial->println(F("- Temperature - 1"));
    aSerial->println(F("m <c(ool) or a(uto) or d(ehumidifying) or h(eating) or f(an) mode>"));
    aSerial->println(F("S Sleep after <0 to 420> minutes"));
    aSerial->println(F("T Timer on after <0 to 1439> minutes"));
    aSerial->println(F("O Timer off after <0 to 1439> minutes"));
    aSerial->println(F("C Clear all timer and sleep"));
    aSerial->println(F("e.g. \"s1\" or \"t23\" or \"mc\" or \"O60\" or \"+\""));
    aSerial->println(F("No plausibility check is made!"));
    aSerial->println();
}

/*
 * Send repeat
 * Repeat commands should be sent in a 110 ms raster.
 * @param aCommand one of LG_COMMAND_OFF, LG_COMMAND_ON etc.
 */
bool Aircondition_LG::sendCommandAndParameter(char aCommand, int aParameter) {
    // Commands without parameter
    switch (aCommand) {
    case LG_COMMAND_OFF: // off
        sendIRCommand(LG_POWER_DOWN);
        PowerIsOn = false;
        return true;

    case LG_COMMAND_ON: // on
        PowerIsOn = false; // set to false in order to suppress on bit
        sendTemperatureFanSpeedAndMode();
        return true;

    case LG_COMMAND_JET:
        IR_DEBUG_PRINTLN(F("Send jet on"));
        sendIRCommand(LG_JET_ON);
        return true;

    case LG_COMMAND_LIGHT:
        sendIRCommand(LG_LIGHT);
        return true;

    case LG_COMMAND_CLEAR_ALL:
        sendIRCommand(LG_CLEAR_ALL);
        return true;

    case LG_COMMAND_TEMPERATURE_PLUS:
        if (18 <= Temperature && Temperature <= 29) {
            Temperature++;
            sendTemperatureFanSpeedAndMode();
        } else {
            return false;
        }
        return true;

    case LG_COMMAND_TEMPERATURE_MINUS:
        if (19 <= Temperature && Temperature <= 30) {
            Temperature--;
            sendTemperatureFanSpeedAndMode();
        } else {
            return false;
        }
        return true;

    }

    PowerIsOn = true;

    /*
     * Now the commands which require a parameter
     */
    if (aParameter < 0) {
        IR_DEBUG_PRINT(F("Error: Parameter is less than 0"));
        return false;
    }
    switch (aCommand) {

    case LG_COMMAND_MODE:
        Mode = aParameter + '0';
        sendTemperatureFanSpeedAndMode();
        break;

    case LG_COMMAND_SWING:
        IR_DEBUG_PRINT(F("Send air swing="));
        IR_DEBUG_PRINTLN(aParameter);
        if (ACIsWallType) {
            if (aParameter) {
                sendIRCommand(LG_WALL_SWING_ON);
            } else {
                sendIRCommand(LG_WALL_SWING_OFF);
            }
        } else {
            if (aParameter) {
                sendIRCommand(LG_SWING_ON);
            } else {
                sendIRCommand(LG_SWING_OFF);
            }
        }
        break;

    case LG_COMMAND_AUTO_CLEAN:
        IR_DEBUG_PRINT(F("Send auto clean="));
        IR_DEBUG_PRINTLN(aParameter);
        if (aParameter) {
            sendIRCommand(LG_AUTO_CLEAN_ON);
        } else {
            sendIRCommand(LG_AUTO_CLEAN_OFF);
        }
        break;

    case LG_COMMAND_ENERGY:
        IR_DEBUG_PRINT(F("Send energy saving on="));
        IR_DEBUG_PRINTLN(aParameter);
        if (aParameter) {
            sendIRCommand(LG_ENERGY_SAVING_ON);
        } else {
            sendIRCommand(LG_ENERGY_SAVING_OFF);
        }
        break;

    case LG_COMMAND_FAN_SPEED:
        if (aParameter < SIZE_OF_FAN_SPEED_MAPPING_TABLE) {
            FanIntensity = aParameter;
            sendTemperatureFanSpeedAndMode();
        } else {
            return false;
        }
        break;

    case LG_COMMAND_TEMPERATURE:
        if (18 <= aParameter && aParameter <= 30) {
            Temperature = aParameter;
            sendTemperatureFanSpeedAndMode();
        } else {
            return false;
        }
        break;

    case LG_COMMAND_SLEEP:
        // 420 = maximum I have recorded
        if (aParameter <= 420) {
            sendIRCommand(LG_SLEEP + aParameter);
        } else {
            return false;
        }
        break;

    case LG_COMMAND_TIMER_ON:
        // 1440 = minutes of a day
        if (aParameter <= 1439) {
            sendIRCommand(LG_TIMER_ON + aParameter);
        } else {
            return false;
        }
        break;

    case LG_COMMAND_TIMER_OFF:
        if (aParameter <= 1439) {
            sendIRCommand(LG_TIMER_OFF + aParameter);
        } else {
            return false;
        }
        break;

    default:
        return false;
    }
    return true;
}

void Aircondition_LG::sendIRCommand(uint16_t aCommand) {

    IR_INFO_PRINT(F("Send code=0x"));
    IR_INFO_PRINT(aCommand, HEX);
    IR_INFO_PRINT(F(" | 0b"));
    IR_INFO_PRINTLN(aCommand, BIN);

    IrSender.sendLG((uint8_t) LG_ADDRESS, aCommand, 0, false, useLG2Protocol);
}

/*
 * Takes values from static variables
 */
void Aircondition_LG::sendTemperatureFanSpeedAndMode() {

    uint8_t tTemperature = Temperature;
    IR_INFO_PRINT(F("Send temperature="));
    IR_INFO_PRINT(tTemperature);
    IR_INFO_PRINT(F(" fan intensity="));
    IR_INFO_PRINT(FanIntensity);
    IR_INFO_PRINT(F(" mode="));
    IR_INFO_PRINTLN((char )Mode);

    WordUnion tIRCommand;
    tIRCommand.UWord = 0;

    // Temperature is coded in the upper nibble of the LowByte
    tIRCommand.UByte.LowByte = ((tTemperature - 15) << 4); // 16 -> 0x00, 18 -> 0x30, 30 -> 0xF0

    // Fan intensity is coded in the lower nibble of the LowByte
    if (ACIsWallType) {
        tIRCommand.UByte.LowByte |= AC_FAN_WALL[FanIntensity];
    } else {
        tIRCommand.UByte.LowByte |= AC_FAN_TOWER[FanIntensity];
    }

    switch (Mode) {
    case AC_MODE_COOLING:
        tIRCommand.UByte.HighByte = LG_MODE_COOLING >> 8;
        break;
    case AC_MODE_HEATING:
        tIRCommand.UByte.HighByte = LG_MODE_HEATING >> 8;
        break;
    case AC_MODE_AUTO:
        tIRCommand.UByte.HighByte = LG_MODE_AUTO >> 8;
        break;
    case AC_MODE_FAN:
        tTemperature = 18;
        tIRCommand.UByte.HighByte = LG_MODE_FAN >> 8;
        break;
    case AC_MODE_DEHUMIDIFIYING:
        tIRCommand.UWord = LG_MODE_DEHUMIDIFIYING;
        break;
    default:
        break;
    }
    if (!PowerIsOn) {
        // switch on requires masked bit
        tIRCommand.UByte.HighByte &= ~(LG_SWITCH_ON_MASK >> 8);
    }
    PowerIsOn = true;

    sendIRCommand(tIRCommand.UWord);
}

/** @}*/
#endif // #ifndef AC_LG_HPP
#pragma once
